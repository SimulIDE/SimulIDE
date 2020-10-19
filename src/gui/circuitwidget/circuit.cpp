/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "circuit.h"
#include "itemlibrary.h"
#include "mainwindow.h"
#include "circuitwidget.h"
#include "propertieswidget.h"
#include "subpackage.h"
#include "connectorline.h"
#include "mcucomponent.h"
#include "simuapi_apppath.h"
#include "node.h"
#include "utils.h"

#include <QCoreApplication>

static const char* Circuit_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Speed"),
    QT_TRANSLATE_NOOP("App::Property","ReactStep"),
    QT_TRANSLATE_NOOP("App::Property","NoLinAcc"),
    QT_TRANSLATE_NOOP("App::Property","Draw Grid"),
    QT_TRANSLATE_NOOP("App::Property","Show ScrollBars"),
    QT_TRANSLATE_NOOP("App::Property","Animate"),
    QT_TRANSLATE_NOOP("App::Property","Font Scale"),
    QT_TRANSLATE_NOOP("App::Property","Auto Backup Secs")
};

Circuit*  Circuit::m_pSelf = 0l;

Circuit::Circuit( qreal x, qreal y, qreal width, qreal height, QGraphicsView*  parent)
       : QGraphicsScene(x, y, width, height, parent)
{
    Q_UNUSED( Circuit_properties );
    
    setObjectName( "Circuit" );
    setParent( parent );
    m_graphicView = parent;
    m_scenerect.setRect( x, y, width, height );
    setSceneRect( QRectF(x, y, width, height) );

    m_pSelf = this;

    m_busy        = false;
    m_changed     = false;
    m_pasting     = false;
    m_deleting    = false;
    m_con_started = false;

    new_connector = 0l;
    m_seqNumber   = 0;
    
    m_hideGrid   = MainWindow::self()->settings()->value( "Circuit/hideGrid" ).toBool();
    m_showScroll = MainWindow::self()->settings()->value( "Circuit/showScroll" ).toBool();
    m_filePath = qApp->applicationDirPath()+"/new.simu";

    connect( &m_bckpTimer, SIGNAL(timeout() ), this, SLOT( saveChanges()) );
    //m_bckpTimer.start( m_autoBck*1000 );
}

Circuit::~Circuit()
{
    m_bckpTimer.stop();

    // Avoid PropertyEditor problem: comps not unregistered
    QPropertyEditorWidget::self()->removeObject( this );

    for( Component* comp : m_compList )
    {
        QPropertyEditorWidget::self()->removeObject( comp );
    }

    // Clear Undo/Redo stacks
    for( QDomDocument* doc : m_redoStack ) delete doc;
    for( QDomDocument* doc : m_undoStack ) delete doc;
    m_undoStack.clear();
    m_redoStack.clear();

    if( !m_backupPath.isEmpty() )
    {
        QFile file( m_backupPath );
        if( !file.exists() ) return;
        QFile::remove( m_backupPath ); // Remove backup file
    }
}

void Circuit::setLang( Langs lang )
{
    if( lang == m_lang ) return;
    m_lang = lang;

    MainWindow::self()->settings()->setValue( "language", loc() );
}

QString Circuit::loc()
{
    QString locale = "en";
    if     ( m_lang == French )    locale = "fr";
    else if( m_lang == German )    locale = "de";
    else if( m_lang == Russian )   locale = "ru";
    else if( m_lang == Spanish )   locale = "es";
    else if( m_lang == Pt_Brasil ) locale = "pt_BR";

    return locale;
}

void Circuit::setLoc( QString loc )
{
    Langs lang = English;

    if     ( loc == "fr" )    lang = French;
    else if( loc == "de" )    lang = German;
    else if( loc == "ru" )    lang = Russian;
    else if( loc == "es" )    lang = Spanish;
    else if( loc == "pt_BR" ) lang = Pt_Brasil;

    m_lang = lang;
}

QString Circuit::getCompId( QString name )
{
    QStringList nameSplit = name.split("-");
    if( nameSplit.isEmpty() ) return "";

    QString compId  = nameSplit.takeFirst();
    if( nameSplit.isEmpty() ) return "";

    QString compNum = nameSplit.takeFirst();

    return compId+"-"+compNum;
}

Pin* Circuit::findPin( int x, int y, QString id )
{
    qDebug() << "Circuit::findPin" << id;
    QRectF itemRect = QRectF ( x-4, y-4, 8, 8 );

    QList<QGraphicsItem*> list = items( itemRect ); // List of items in (x, y)
    for( QGraphicsItem* it : list )
    {
        Pin* pin =  qgraphicsitem_cast<Pin*>( it );

        // Check if names start by same letter
        if( pin && (pin->pinId().left(1) == id.left(1))) return pin;
    }
    for( QGraphicsItem* it : list ) // Not found by first letter, take first Pin
    {
        Pin* pin =  qgraphicsitem_cast<Pin*>( it );
        if( pin ) return pin;
    }
    return 0l;
}

void Circuit::loadCircuit( QString &fileName )
{
    if( m_con_started ) return;

    m_filePath = fileName;
    QFile file( fileName );

    if( !file.open(QFile::ReadOnly | QFile::Text) )
    {
        QMessageBox::warning(0l, "Circuit::loadCircuit",
        tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }
    //qDebug() << "Circuit::loadCircuit"<<m_filePath;

    if( !m_domDoc.setContent(&file) )
    {
        QMessageBox::warning( 0l, "Circuit::loadCircuit",
        tr("Cannot set file %1\nto DomDocument").arg(fileName));
        file.close();
        return;
    }
    file.close();

    m_error = 0;
    loadDomDoc( &m_domDoc );
    m_domDoc.clear();
    
    if( m_error != 0 ) 
    {
        remove();
        for( Component* comp : m_compList ) removeComp( comp ); // Clean Nodes
    }
    else m_graphicView->centerOn( QPointF( 1200+itemsBoundingRect().center().x(), 950+itemsBoundingRect().center().y() ) );
    
    if( MainWindow::self()->autoBck() )
    {
        saveState();
        saveChanges();
    }
}

void Circuit::loadDomDoc( QDomDocument* doc )
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Component*> compList;   // Component List
    QList<Component*> conList;    // Connector List
    QList<Node*>      jointList;  // Joint List

    QHash<QString, QString> idMap;
    QHash<QString, eNode*> nodMap;

    m_busy    = true;
    m_animate = false;

    QDomElement circuit = doc->documentElement();
    QString docType = circuit.attribute("type");
    
    if( circuit.hasAttribute( "speed" ))     setCircSpeed( circuit.attribute("speed").toInt() );
    if( circuit.hasAttribute( "reactStep" )) setReactStep( circuit.attribute("reactStep").toInt() );
    if( circuit.hasAttribute( "noLinAcc" ))  setNoLinAcc( circuit.attribute("noLinAcc").toInt() );
    if( circuit.hasAttribute( "animate" ))   setAnimate( circuit.attribute("animate").toInt() );

    QDomNode node = circuit.firstChild();

    while( !node.isNull() )
    {
        QDomElement   element = node.toElement();
        const QString tagName = element.tagName();

        if( tagName == "item" )
        {
            QString objNam = element.attribute( "objectName"  ); // Data in simu file
            QString type   = element.attribute( "itemtype"  );
            QString id     = objNam.split("-").first()+"-"+newSceneId(); // Create new id

            element.setAttribute( "objectName", id  );

            if( type == "Connector" )
            {
                Pin* startpin  = 0l;
                Pin* endpin    = 0l;
                QString startpinid    = element.attribute( "startpinid" );
                QString endpinid      = element.attribute( "endpinid" );
                QString startCompName = getCompId( startpinid );
                QString endCompName   = getCompId( endpinid );

                startpinid.replace( startCompName, idMap[startCompName] );
                endpinid.replace( endCompName, idMap[endCompName] );

                startpin = m_pinMap[startpinid];
                endpin   = m_pinMap[endpinid];
                
                if( !startpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.first().toInt();
                    int itemY = pointList.at(1).toInt();

                    startpin = findPin( itemX, itemY, startpinid );
                }
                if( !endpin ) // Pin not found by name... find it by pos
                {
                    QStringList pointList   = element.attribute( "pointList" ).split(",");
                    int itemX = pointList.at(pointList.size()-2).toInt();
                    int itemY = pointList.last().toInt();

                    endpin = findPin( itemX, itemY, endpinid );
                }

                if( m_pasting )
                {
                    if( startpin && !startpin->component()->isSelected() ) startpin = 0l;
                    if( endpin   && !endpin->component()->isSelected() )   endpin = 0l;
                }
                if( startpin && startpin->isConnected() ) startpin = 0l;
                if( endpin   && endpin->isConnected() )   endpin   = 0l;

                if( startpin && endpin )    // Create Connector
                {
                    Connector* con  = new Connector( this, type, id, startpin, endpin );

                    element.setAttribute( "startpinid", startpin->objectName() );
                    element.setAttribute(   "endpinid", endpin->objectName() );

                    loadProperties( element, con );

                    QString enodeId = element.attribute( "enodeid" );
                    eNode*  enode   = nodMap[enodeId];
                    if( !enode )                    // Create eNode and add to enodList
                    {
                        enode = new eNode( "Circ_eNode-"+newSceneId() );
                        nodMap[enodeId] = enode;
                    }
                    con->setEnode( enode );

                    QStringList plist = con->pointList();   // add lines to connector
                    int p1x = plist.first().toInt();
                    int p1y = plist.at(1).toInt();
                    int p2x = plist.at(plist.size()-2).toInt();
                    int p2y = plist.last().toInt();

                    con->addConLine( con->x(),con->y(), p1x, p1y, 0 );

                    int count = plist.size();
                    for (int i=2; i<count; i+=2)
                    {
                        p2x = plist.at(i).toInt();
                        p2y = plist.at(i+1).toInt();
                        con->addConLine( p1x, p1y, p2x, p2y, i/2 );
                        p1x = p2x;
                        p1y = p2y;
                    }
                    con->updateConRoute( startpin, startpin->scenePos() );
                    con->updateConRoute( endpin, endpin->scenePos() );
                    con->remNullLines();
                    conList.append( con );
                }
                else // Start or End pin not found
                {
                    if( !startpin ) qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null startpin in " << objNam << startpinid;
                    if( !endpin )   qDebug() << "\n   ERROR!!  Circuit::loadDomDoc:  null endpin in "   << objNam << endpinid;
                }
            }
            else if( type == "Node")
            {
                idMap[objNam] = id;                              // Map simu id to new id
                
                Node* joint = new Node( this, type, id );
                loadProperties( element, joint );
                compList.append( joint );
                jointList.append( joint );

                if( m_pasting ) joint->setSelected( true );
            }
            else if( type == "LEDSMD" ); // TODO: this type shouldnt be saved to circuit file
            else if( type == "Plotter")    loadObjectProperties( element, PlotterWidget::self() );
            else
            {
                if( docType != "simulide_0.4" )
                {
                    if( type == "SerialPort")
                    {
                        if( element.hasAttribute( "visible" ) )
                        {
                            if( element.attribute( "visible" ) == "false" )
                            {
                                node = node.nextSibling();
                                continue;
                            }
                        }
                    }
                }
                idMap[objNam] = id;                              // Map simu id to new id
                
                Component* item = createItem( type, id );
                
                if( item )
                {
                    loadProperties( element, item );

                    if( type.contains( "Relay" ) )
                    {
                        if( element.hasAttribute( "Itrig") )
                        {
                            QVariant value( element.attribute( "Itrig" ) );
                            item->setProperty( "IOn", value.toDouble() );
                        }
                    }

                    compList.append( item );

                    if( docType != "simulide_0.4" )
                    {
                        if( type == "SerialPort") item->setPos( McuComponent::self()->pos() );
                    }
                    if( m_pasting ) item->setSelected( true );
                }
                else 
                {
                    qDebug() << " ERROR Creating Component: "<< type << id;
                    QApplication::restoreOverrideCursor();
                    m_error = 1;
                    return;
                }
            }
        }
        node = node.nextSibling();
    }
    if( m_pasting )
    {
        for( Component *item : compList )
        {
            item->move( m_deltaMove );
        }
        for( Component* item : conList )
        {
            Connector* con = static_cast<Connector*>( item );
            con->setSelected( true );
            con->move( m_deltaMove );
        }
    }
    // Take care about unconnected Joints
    for( Node* joint : jointList ) joint->remove(); // Only removed if some missing connector
    
    m_busy = false;
    QApplication::restoreOverrideCursor();
}

void Circuit::circuitToDom()
{
    m_domDoc.clear();
    QDomElement circuit = m_domDoc.createElement("circuit");

    circuit.setAttribute( "type",      "simulide_0.4" );
    circuit.setAttribute( "speed",     QString::number( circSpeed() ) );
    circuit.setAttribute( "reactStep", QString::number( reactStep() ) );
    circuit.setAttribute( "noLinAcc",  QString::number( noLinAcc() ) );
    circuit.setAttribute( "animate",   QString::number( animate() ) );
    //circuit.setAttribute( "drawGrid",    QString( drawGrid()?"true":"false"));
    //circuit.setAttribute( "showScroll",  QString( showScroll()?"true":"false"));

    m_domDoc.appendChild(circuit);

    listToDom( &m_domDoc, &m_compList );

    for( Component* comp : m_conList )
    {
        Connector* con = static_cast<Connector*>( comp );
        con->remNullLines();
    }
    listToDom( &m_domDoc, &m_conList );

    objectToDom( &m_domDoc, PlotterWidget::self() );

    circuit.appendChild( m_domDoc.createTextNode( "\n \n" ) );
}

void Circuit::listToDom( QDomDocument* doc, QList<Component*>* complist )
{
    int count = complist->count();
    for( int i=0; i<count; i++ )
    {
        Component* item = complist->at(i);

        // Don't save internal items
        bool isNumber = false;
        item->objectName().split("-").last().toInt( &isNumber );

        if( isNumber ) objectToDom( doc, item );
    }
}

void Circuit::objectToDom( QDomDocument* doc, QObject* object )
{
    QDomElement root = doc->firstChild().toElement();
    QDomElement elm = m_domDoc.createElement("item");
    const QMetaObject* metaobject = object->metaObject();

    int count = metaobject->propertyCount();
    for( int i=0; i<count; i++ )
    {
        QMetaProperty metaproperty = metaobject->property(i);
        const char* name = metaproperty.name();

        QVariant value = object->property( name );
        if( metaproperty.type() == QVariant::StringList )
        {
            QStringList list= value.toStringList();
            elm.setAttribute( name, list.join(",") );
        }
        else if( (QString(name)=="Mem") || (QString(name)=="eeprom") )
        {
            QVector<int> vmem = value.value<QVector<int>>();

            QStringList list;
            for( int val : vmem ) list << QString::number( val );

            elm.setAttribute( name, list.join(",") );

            //qDebug() << "typename" << value.typeName();
            //qDebug() << "Value:\n" << vmem;
            //qDebug() << "Data:\n" << list;
            //qDebug() << "type" << value.type()<< "typename" << value.typeName()<< "name " << name
            //     << "   value " << value << "saved" << value.toString();
        }
        else
        {
            elm.setAttribute( name, value.toString() );
            if( QString(name)=="Text" )
            qDebug() << "type:" << value.type()<< "typename:" << value.typeName()<< "name:" << name
                 << "   value:" << value << "saved:" << value.toString();
        }

    }
    QDomText blank = m_domDoc.createTextNode( "\n \n" );
    QDomText objNme = m_domDoc.createTextNode( object->objectName() );
    root.appendChild( blank );
    root.appendChild( objNme );
    blank = m_domDoc.createTextNode( ": \n" );
    root.appendChild( blank );
    root.appendChild( elm );
}

bool Circuit::saveDom( QString &fileName, QDomDocument* doc )
{
    QFile file( fileName );

    if( !file.open( QFile::WriteOnly | QFile::Text ))
    {
        QApplication::restoreOverrideCursor();
        QMessageBox::warning(0l, "Circuit::saveCircuit",
        tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << doc->toString();
    file.close();

    return true;
}

bool Circuit::saveCircuit( QString &fileName )
{
    if( m_con_started ) return false;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if( !fileName.endsWith(".simu") ) fileName.append(".simu");

    QString oldFilePath = m_filePath;
    m_filePath = fileName;

    circuitToDom();
    bool saved = saveDom( fileName, &m_domDoc );

    if( saved )
    {
        if( !m_backupPath.isEmpty() )
        {
            QFile::remove( m_backupPath ); // remove backup file
            m_backupPath = "";
        }
    }
    else m_filePath = oldFilePath;

    QApplication::restoreOverrideCursor();
    return saved;
}


void Circuit::importCirc(  QPointF eventpoint  )
{
    if( m_con_started ) return;

    m_pasting = true;

    m_deltaMove = QPointF( 160, 160 );//togrid(eventpoint);

    const QString dir = m_filePath;
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Circuit"), dir,
                                          tr("Circuits (*.simu);;All files (*.*)"));

    if( !fileName.isEmpty() && fileName.endsWith(".simu") )
        loadCircuit( fileName );

    m_pasting = false;
}

Component* Circuit::createItem( QString type, QString id )
{
    //qDebug() << "Circuit::createItem" << type << id;
    for( LibraryItem* libItem : ItemLibrary::self()->items() )
    {
        if( libItem->type()==type )
        {
            Component* comp = libItem->createItemFnPtr()( this, type, id );
            
            if( comp )
            {
                QString category = libItem->category();
                if( ( category != "Meters" )
                &&  ( category != "Sources" )
                &&  ( category != "Other" ) )
                    comp->setPrintable( true );
            }
            return comp;
        }
    }
    return 0l;
}

void Circuit::loadProperties( QDomElement element, Component* Item )
{
    loadObjectProperties( element, Item );
    
    Item->setLabelPos();
    Item->setValLabelPos();

    addItem(Item);

    int number = Item->objectName().split("-").last().toInt();

    if ( number > m_seqNumber ) m_seqNumber = number;               // Adjust item counter: m_seqNumber
}

void Circuit::loadObjectProperties( QDomElement element, QObject* Item )
{
    QHash<QString, QString> atrMap;          // Make properties case insentive
    QDomNamedNodeMap atrs = element.attributes();

    for( int i=0; i<atrs.length(); i++ )   // Get List of property names in Circuit file
    {
        QString attrib = atrs.item( i ).nodeName();
        atrMap[attrib.toLower()] = attrib;
        //qDebug() << "Circuit::loadObjectProperties" << attrib;
    }

    const QMetaObject* metaobject = Item->metaObject();
    int count = metaobject->propertyCount();
    
    for( int i=0; i<count; ++i )
    {
        QMetaProperty metaproperty = metaobject->property(i);
        const char* chName = metaproperty.name();
        QString n = chName;
        QString lowN = n.toLower();
        //qDebug() << "Circuit::loadObjectProperties.....";
        //qDebug() << "Circuit::loadObjectProperties" << n << chName;

        if( !atrMap.contains( lowN )) continue;

        n = atrMap.value( lowN );                 // Take actual property name in Circuit file
        //QByteArray ba = n.toLocal8Bit();
        //const char* proName = ba.data();
        //qDebug() << "Circuit::loadObjectProperties" << n;
        QVariant value( element.attribute( n ) );
        
        if     ( metaproperty.type() == QVariant::Int    ) Item->setProperty( chName, value.toInt() );
        else if( metaproperty.type() == QVariant::Double ) Item->setProperty( chName, value.toDouble() );
        else if( metaproperty.type() == QVariant::PointF ) Item->setProperty( chName, value.toPointF() );
        else if( metaproperty.type() == QVariant::Bool   ) Item->setProperty( chName, value.toBool() );
        else if( metaproperty.type() == QVariant::StringList )
        {
            QStringList list= value.toString().split(",");
            Item->setProperty( chName, list );
        }
        else if( (lowN=="mem") || (lowN=="eeprom") )
        {
            QStringList list = value.toString().split(",");
            
            QVector<int> vmem;
            int lsize = list.size();
            vmem.resize( lsize );
            //qDebug() << "Circuit::loadObjectProperties eeprom size:" << lsize;
            
            for( int x=0; x<lsize; x++ )
            {
                vmem[x] = list.at(x).toInt();
                //qDebug() << x << vmem[x];
            }
            QVariant value = QVariant::fromValue( vmem );
            Item->setProperty( chName, value );
        }
        else Item->setProperty( chName, value );
        //else qDebug() << "    ERROR!!! Circuit::loadObjectProperties\n  unknown type:  "<<"name "<<name<<"   value "<<value ;
    }
}

void Circuit::removeItems()                     // Remove Selected items
{
    if( m_con_started ) return;

    m_busy = true;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    saveState();

    foreach( Component* comp, m_compList )
    {
        if( comp->isSelected() && !(comp->itemType()=="Node") )
        {
            removeComp( comp );          // Don't remove Graphical Nodes
        }
    }
    QList<Connector*> connectors;

    for( QGraphicsItem* item : selectedItems() )
    {
        ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine* >( item );
        if( line->objectName() == "" )
        {
            Connector* con = line->connector();
            if( !connectors.contains( con ) ) connectors.append( con );
        }
    }
    for( Connector* con : connectors ) con->remove();
    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );

    if( pauseSim ) Simulator::self()->runContinuous();
    m_busy = false;
}

void Circuit::removeComp( Component* comp )
{
    m_compRemoved = false;
    comp->remove();
    if( !m_compRemoved ) return;

    QPropertyEditorWidget::self()->removeObject( comp );
    compList()->removeOne( comp );
    if( items().contains( comp ) ) removeItem( comp );
    //comp->deleteLater();
    delete comp;
}

void Circuit::compRemoved( bool removed ) // Arduino doesn't like to be removed while circuit is running
{
    m_compRemoved = removed;
}

void Circuit::remove() // Remove everything ( Clear Circuit )
{
    if( m_con_started ) return;

    //qDebug() << m_compList.size();
    //m_deleting = true;
    m_busy = true;

    //foreach( Component* comp , m_compList )
    while( !m_compList.isEmpty() )
    {
        Component* comp = m_compList.takeFirst();
        //qDebug() << "Circuit::remove" << comp->itemID()<<comp->objectName();

        bool isNumber = false;               // Don't remove internal items

        comp->objectName().split("-").last().toInt( &isNumber ); // TODO: Find a proper way !!!!!!!!!!!

        if( isNumber && !(comp->itemType()=="Node") )
        {
            //qDebug() << "Circuit::remove" << comp->itemID();
            removeComp( comp );
        }
    }
    //m_deleting = false;
    m_busy = false;
}

bool Circuit::deleting()
{
    return m_deleting;
}

void Circuit::deselectAll()
{
    for( QGraphicsItem* item : selectedItems() ) item->setSelected( false );
}

void Circuit::saveState()
{
    if( m_con_started ) return;

    //qDebug() << "saving state";
    m_changed = true;

    if( m_busy )
    {
        //qDebug() << "saving state While Busy";
        return;
    }
    //m_busy = true;

    if( m_deleting )
    {
        qDebug() << "saving state Already Running";
        return;
    }

    m_deleting = true;

    for( QDomDocument* doc : m_redoStack ) delete doc;

    m_redoStack.clear();

    circuitToDom();
    m_undoStack.append( new QDomDocument() );
    m_undoStack.last()->setContent( m_domDoc.toString() );

    QString title = MainWindow::self()->windowTitle();
    if( !title.endsWith('*') ) MainWindow::self()->setWindowTitle(title+'*');

    m_deleting = false;
}

void Circuit::saveChanges()
{
    //qDebug() << "Circuit::saveChanges";
    if( m_busy )
    {
        qDebug() << "Backup While Busy";
        return;
    }

    if( !m_changed ) return;
    if( m_con_started ) return;

    m_changed = false;

    circuitToDom();

    m_backupPath = m_filePath;

    QFileInfo bckDir( m_backupPath );

    if( !bckDir.isWritable() )
        m_backupPath = SIMUAPI_AppPath::self()->RWDataFolder().absolutePath()+"/_backup.simu";

    if( !m_backupPath.endsWith( "_backup.simu" ))
        m_backupPath.replace( ".simu", "_backup.simu" );

    //qDebug() << "saving Backup"<<m_backupPath;

    if( saveDom( m_backupPath, &m_domDoc ) )
        MainWindow::self()->settings()->setValue( "backupPath", m_backupPath );
}

void Circuit::setChanged()
{
    m_changed = true;
}


void Circuit::undo()
{
    if( m_con_started ) return;

    if( m_undoStack.isEmpty() ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->stopSim();

    circuitToDom();
    m_redoStack.prepend( new QDomDocument() );
    m_redoStack.first()->setContent( m_domDoc.toString() );

    remove();
    QDomDocument* doc = m_undoStack.takeLast();
    m_domDoc.setContent( doc->toString());

    m_seqNumber = 0;
    loadDomDoc( &m_domDoc );

    if( pauseSim ) Simulator::self()->runContinuous();
}

void Circuit::redo()
{
    if( m_con_started ) return;

    if( m_redoStack.isEmpty() ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->stopSim();

    circuitToDom();
    m_undoStack.append( new QDomDocument() );
    m_undoStack.last()->setContent( m_domDoc.toString() );

    remove();
    QDomDocument* doc = m_redoStack.takeFirst();
    m_domDoc.setContent( doc->toString());

    m_seqNumber = 0;
    loadDomDoc( &m_domDoc );

    if( pauseSim ) Simulator::self()->runContinuous();
}

void Circuit::copy( QPointF eventpoint )
{
    if( m_con_started ) return;

    m_eventpoint = togrid(eventpoint);

    QList<Component*> complist;

    QList<QGraphicsItem*> itemlist = selectedItems();

    for( QGraphicsItem* item : itemlist )
    {
        Component* comp =  qgraphicsitem_cast<Component*>( item );
        if( comp )
        {
            if( comp->itemType() == "Connector" )
            {
                Connector* con = static_cast<Connector*>( comp );
                con->remNullLines();

                complist.append( con );
            }
            else
            {
                complist.prepend( comp );
            }
        }
    }
    m_copyDoc.clear();
    QDomElement root = m_copyDoc.createElement("circuit");
    root.setAttribute( "type", "simulide_0.1" );
    m_copyDoc.appendChild(root);

    listToDom( &m_copyDoc, &complist );

    QString px = QString::number( m_eventpoint.x() );
    QString py = QString::number( m_eventpoint.y() );
    QString clipTextText = px+","+py+"eventpoint"+m_copyDoc.toString();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText( clipTextText );
}

void Circuit::paste( QPointF eventpoint )
{
    if( m_con_started ) return;

    QClipboard *clipboard = QApplication::clipboard();
    QString clipText = clipboard->text();
    if( !clipText.contains( "eventpoint") ) return;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->stopSim();
    
    bool animate = m_animate;
    saveState();
    m_pasting = true;
    for( QGraphicsItem*item : selectedItems() ) item->setSelected( false );

    QStringList clipData = clipText.split( "eventpoint" );
    clipText = clipData.last();
    m_copyDoc.setContent( clipText );

    clipData = clipData.first().split(",");
    int px = clipData.first().toInt();
    int py = clipData.last().toInt();
    m_eventpoint = QPointF( px, py );

    m_deltaMove = togrid(eventpoint) - m_eventpoint;

    loadDomDoc( &m_copyDoc );

    m_pasting = false;
    setAnimate( animate );

    if( pauseSim ) Simulator::self()->runContinuous();
}

bool  Circuit::pasting() { return m_pasting; }
QPointF Circuit::deltaMove(){ return m_deltaMove; }

void Circuit::newconnector( Pin*  startpin )
{
    saveState();

    //if ( m_subcirmode ) return;
    m_con_started = true;

    QString type = QString("Connector");
    QString id = type;
    id.append( "-" );
    id.append( newSceneId() );

    new_connector = new Connector( this, type, id, startpin );

    QPoint p1 = startpin->scenePos().toPoint();
    QPoint p2 = startpin->scenePos().toPoint();

    new_connector->addConLine( p1.x(), p1.y(), p2.x(), p2.y(), 0 );

    addItem(new_connector);
}

void Circuit::closeconnector( Pin* endpin )
{
    m_con_started = false;
    new_connector->closeCon( endpin, /*connect=*/true );
}

void Circuit::deleteNewConnector()
{
    if( m_con_started )
    {
        new_connector->remove();
        m_con_started = false;
    }
}

void Circuit::updateConnectors()
{
    for( Component* comp : m_conList )
    {
        Connector* con = static_cast<Connector*>( comp );
        con->updateLines();
    }
}

void Circuit::constarted( bool started ) { m_con_started = started; }
bool Circuit::is_constarted() { return m_con_started ; }

void Circuit::convertSubCircs() // Convert old subcircuits:
{
    QString subCirPath = QFileDialog::getExistingDirectory( 0L,
                           tr("Select Directory"),
                           m_filePath,
                           QFileDialog::ShowDirsOnly
                         | QFileDialog::DontResolveSymlinks);

    if( subCirPath != "" ) subCirPath += "/";
    else return;

    QDir compSetDir = QDir( subCirPath );

    compSetDir.mkpath(subCirPath+"converted/");

    compSetDir.setNameFilters( QStringList( "*.subcircuit" ) );

    QStringList compList = compSetDir.entryList( QDir::Files );

    for( QString compName : compList )
    {
        QString subcir = compSetDir.absoluteFilePath( compName );
        QString packag = subcir;
        packag.replace( ".subcircuit", ".package" );

        QStringList pinList;

        QFile pkgFile( packag );
        if( !pkgFile.open(QFile::ReadOnly | QFile::Text) )
        {
              QMessageBox::warning( 0, "Circuit::convertSubCircs", tr("Cannot read file %1:\n%2.").arg(packag).arg(pkgFile.errorString()));
              continue;
        }
        QDomDocument domDoc;
        if( !domDoc.setContent(&pkgFile) )
        {
             MessageBoxNB( "Circuit::convertSubCircs",
                       tr( "Cannot set file:\n%1\nto DomDocument" ) .arg( packag ));
             pkgFile.close();
             continue;
        }
        pkgFile.close();

        QDomElement root  = domDoc.documentElement();
        QDomNode node = root.firstChild();

        while( !node.isNull() )
        {
            QDomElement element = node.toElement();
            if( element.tagName() == "pin" )
            {
                QString id    = element.attribute( "id" );
                pinList.append( "Package_"+id );
                //qDebug() << id;
            }
            node = node.nextSibling();
        }

        QString text = fileToString( subcir,"KK" );
        for( int i=pinList.size(); i>0; i-- )
        {
            QString idd=pinList.at(i-1);
            text.replace( QString( "packagePin"+QString::number(i)), idd ); // replace text in string
            qDebug() << QString( "packagePin"+QString::number(i))<< idd;
        }

        QString subName = subCirPath+"converted/"+strippedName( subcir );
        QFile fileS( subName );

        if( !fileS.open(QFile::WriteOnly | QFile::Text) )
        {
              QMessageBox::warning(0l, "Circuit::convertSubCircs", "Can't open file"+subName);
              continue;
        }
        QTextStream out(&fileS);
        out.setCodec("UTF-8");
        out << text;
        fileS.close();

        QString pkgPath = subCirPath+"converted/"+strippedName( packag ); // Copy pkg file as well
        pkgFile.copy( pkgPath );
    }
}

void Circuit::createSubcircuit()
{
    if( m_con_started ) return;

    QString fileName = m_filePath;
    fileName.replace( m_filePath.lastIndexOf( ".simu" ), 5, "" );
    //fileName += ".subcircuit";
    fileName = QFileDialog::getSaveFileName( MainWindow::self()
                            , tr( "Create Subcircuit" )
                            , fileName
                            , "All files (*)"  );

    QFileInfo fi( fileName );
    QString ext = fi.suffix();
    QString filePath = fileName;
    if( !ext.isEmpty() ) filePath.remove( fileName.lastIndexOf( ext )-1, ext.size()+1 );
    //qDebug() <<"Circuit::createSubcircuit filePath"<< filePath;

    QHash<QString, QString> compList;        // Get Components properties

    //qDebug() << compIdTip<<"--------------------------";
    for( Component* component : m_compList )
    {
        if( component->itemType() == "Package" )
        {
            SubPackage* pkg = (static_cast<SubPackage*>(component));

            QString ext = ".package";
            if( pkg->logicSymbol() ) ext = "_LS.package";

            pkg->savePackage( filePath+ext );
        }

        QString compId     = component->objectName();
        QString propString = "";

        const QMetaObject* metaObject = component->metaObject();

        int count = metaObject->propertyCount();
        for( int i=0; i<count; ++i )                // Get Properties
        {
            QMetaProperty property = metaObject->property(i);
            if( property.isUser() )
            {
                QString name = property.name();
                if( name.startsWith("_1") ) continue;  //BcdToDec 16Bits property breaks DomDocument

                if( !name.contains( "Show" ) 
                 && !name.contains( "Unit" ) 
                 && !name.contains( "itemtype" ) )
                {
                    QString valString = "";
                    
                    if(( name == "Resistance" )
                     | ( name == "Capacitance" )
                     | ( name == "Inductance" )
                     | ( name == "Voltage" )
                     | ( name == "Current" ))
                    {
                        valString = QString::number( component->getmultValue() );
                    }
                    else
                    {
                        const char* charname = property.name();

                        QVariant value = component->property( charname );
                        valString = value.toString();
                    }
                    if( name == "Functions" ) valString = valString.replace("&", "&amp;");
                    if( name == "id") ;//compId = valString;
                    else
                    {
                        name[0] = name[0].toLower();
                        name = name.replace( "_", "" );
                        propString += "        "+name+" = \""+valString+"\"\n";
                    }
                }
            }
        }
        compList[compId] = propString;
    }
    QList<eNode*> eNodeList = simulator.geteNodes();
    QList<QStringList> connectionList;

    int nodes = 0;
    for( eNode* node : eNodeList  ) // Get all the connections in each eNode
    {
        //qDebug() << "\nCircuit::createSubcircuit New Node ";
        if( ! node ) continue;
        
        QStringList pinConList;
        QList<ePin*> pinList =  node->getEpins();

        for( ePin* epin : pinList  )
        {
            Pin* pin = (static_cast<Pin*>(epin));
            Component* component = pin->component();
            QString    compId    = component->itemID();
            QString    compType  = component->itemType();
            QString    pinId     = pin->pinId().split( "-" ).last().replace( " ", "" );
            
            if( compType == "Package" )
            {
                if( pin->inverted() && !pinId.startsWith( "!" ) ) pinId = "!"+pinId;
                
                pinConList.prepend( "Package_"+pinId );
            }
            else if( ( compType == "Probe" )
                   ||( compType == "Fixed Voltage" ) )
            {
                // Take care about "packagepin" bad spelling
                //compId = compId.replace( test.indexOf("packagepin"), 10, "packagePin");
                pinConList.prepend( "Package_"+component->idLabel().replace("-","") );
            }
            else if( compId.contains( "Node") ) ;
            else
            {
                pinConList.append( compId );
                pinConList.append( compType );
                pinConList.append( pinId );
            }
        }
        QString conType = "Node";
        if( pinConList.length() == 4 ) conType = "Connection";

        if( conType == "Connection" )               // PackagePin to pin
        {
            QString pin1   = pinConList.takeLast();     // Component pin
            QString pin2   = pinConList.takeFirst();      // Package Pin
            QString compty = pinConList.takeFirst();   // Component type
            QString compId = pinConList.takeFirst();     // Component Id

            pinConList << compty << compId << pin1+"-"+pin2;
            connectionList.append( pinConList );
            //qDebug() << "Circuit::createSubcircuit PackagePin to pin\n" << pinConList;
        }
        else                                      // Multiple connection
        {
            QString pin2   = "eNode"+QString::number(nodes);
            bool isNode = true;

            int packPins = 0;
            for( QString entry : pinConList )
            {
                //if( entry.contains("packagePin") ) // No Node, connection to packagePin Package
                if( entry.contains("Package") ) // No Node, connection to packagePin
                {
                    pin2 = entry;
                    pinConList.removeOne( entry );
                    isNode = false;
                    packPins++;
                    //break;
                }
            }
            if( packPins > 1 ) // 2 Package Pins connected together
            {
                MessageBoxNB( "Circuit::createSubcircuit", "                               \nERROR:\n"+
                          tr( "2 Package Pins connected together" ) );
                return;
            }
            while( !pinConList.isEmpty() ) // Create connection entries
            {
                QStringList pinConList2;
                QString compId = pinConList.takeFirst();
                QString compty = pinConList.takeFirst();
                QString pin1   = pinConList.takeFirst();

                pinConList2 << compId << compty << pin1+"-"+pin2;
                connectionList.append( pinConList2 );
                //qDebug() << "Circuit::createSubcircuit Multiple connection\n" << pinConList2;
            }
            if( isNode ) nodes++;
        }
    }
    QString subcircuit = "<!DOCTYPE SimulIDE>\n";
    subcircuit += "<!-- This file was generated by SimulIDE -->\n\n";
    subcircuit += "<subcircuit enodes=\""+QString::number(nodes)+"\">\n\n";

    while( !connectionList.isEmpty() )
    {
        QStringList list = connectionList.takeFirst();
        if( list.isEmpty() ) continue;

        QString compId = list.takeFirst();
        QString compty = "e"+list.takeFirst().replace( " ", "");
        QString conect = list.takeFirst();

        subcircuit += "    <item itemtype=\""+compty+"\"\n";
        //subcircuit += "    <item ";
        subcircuit += compList[compId];                        // Properties
        subcircuit += "        connections=\"";
        subcircuit += "\n        "+conect;

        int counter = -1;
        for( QStringList list2 : connectionList )
        {
            counter++;
            if( list2.isEmpty() ) continue;
            QString compId2 = list2.at(0);

            if( compId == compId2)
            {
                list2.removeFirst();
                list2.removeFirst();
                QString conect2 = list2.takeFirst();
                subcircuit += "\n        "+conect2;
                connectionList.replace( counter, list2 );
            }
        }
        subcircuit +="\" >\n    </item>\n\n";
    }
    subcircuit +="</subcircuit>";

    QFile file( filePath+".subcircuit" );

    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "Circuit::createSubcircuit",
          tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    }
    QTextStream out( &file );
    out.setCodec("UTF-8");
    out << subcircuit;
    file.close();
    //qDebug() <<"Circuit::createSubcircuit\n" << subcircuit;
}


void Circuit::bom()
{
    if( m_con_started ) return;

    QString fileName = m_filePath;
    fileName.replace( fileName.lastIndexOf( ".simu" ), 5, "-bom.txt" );

    fileName = QFileDialog::getSaveFileName( MainWindow::self()
                            , tr( "Bill Of Materials" )
                            , fileName
                            , "(*.*)"  );

    if( fileName.isEmpty() ) return;

    QStringList bom;

    for( Component* comp : m_compList )
    {
        bool isNumber = false;
        comp->objectName().split("-").last().toInt( &isNumber );

        if( isNumber ) bom.append( comp->print() );
    }

    QFile file( fileName );

    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "Circuit::bom",
          tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
    }
    bom.sort();

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out <<  "\nCircuit: ";
    out <<  QFileInfo( m_filePath ).fileName();
    out <<  "\n\n";
    out <<  "Bill of Materials:\n\n";
    for( QString line : bom ) out << line;

    file.close();
}

QString Circuit::newSceneId()
{
    return QString("%1").arg(++m_seqNumber) ;
}

void Circuit::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        QPropertyEditorWidget::self()->setObject( this );
        PropertiesWidget::self()->setHelpText( MainWindow::self()->circHelp() );

        if( m_con_started )  event->accept();//new_connector->incActLine() ;
        QGraphicsScene::mousePressEvent( event );
    }
    else if( event->button() == Qt::RightButton )
    {
        if( m_con_started ) event->accept();
        else                QGraphicsScene::mousePressEvent( event );
    }
    else if( event->button() == Qt::MidButton )
    {
        QGraphicsScene::mousePressEvent( event );
    }
}

void Circuit::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        if( m_con_started )  new_connector->incActLine() ;
        QGraphicsScene::mouseReleaseEvent( event );
    }
    else if( event->button() == Qt::RightButton )
    {
        if( !m_con_started ) QGraphicsScene::mouseReleaseEvent( event );
    }
}

void Circuit::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_con_started )
    {
        event->accept();
        
        if(event->modifiers() & Qt::ShiftModifier)
        {
            new_connector->m_freeLine = true;
        }
        new_connector->updateConRoute( 0l, event->scenePos() );
    }
    QGraphicsScene::mouseMoveEvent( event );
}

void Circuit::keyPressEvent( QKeyEvent* event )
{
    if( m_con_started ) return;

    int key = event->key();

    if( event->modifiers() & Qt::ControlModifier )
    {
        if( key == Qt::Key_C )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());

            copy( m_graphicView->mapToScene( p ) );
            clearSelection();
        }
        else if( key == Qt::Key_V )
        {
            QPoint p = CircuitWidget::self()->mapFromGlobal(QCursor::pos());

            paste( m_graphicView->mapToScene( p ) );
        }
        else if( key == Qt::Key_Z )
        {
            undo();
        }
        else if( key == Qt::Key_Y )
        {
            redo();
        }
        else if( key == Qt::Key_N )
        {
            CircuitWidget::self()->newCircuit();
        }
        else if( key == Qt::Key_S )
        {
            if( event->modifiers() & Qt::ShiftModifier)
                CircuitWidget::self()->saveCircAs();
            else
                CircuitWidget::self()->saveCirc();
        }
        else if( key == Qt::Key_O )
        {
            CircuitWidget::self()->openCirc();
        }
        else if( key == Qt::Key_K )
        {
            convertSubCircs();
        }
        else QGraphicsScene::keyPressEvent( event );
    }
    else if( key == Qt::Key_Delete )
    {
        removeItems();
        QGraphicsScene::keyPressEvent( event );
    }
    else
    {
        if( !event->isAutoRepeat() ) // Deliver Key events ( switches )
        {
            QString keys = event->text();
            //qDebug() << "Circuit::keyPressEven" << keys;
            while( keys.size() > 0 )
            {
                QString key = keys.left( 1 );
                keys.remove( 0, 1 );

                emit keyEvent( key, true );
            }
        }
        QGraphicsScene::keyPressEvent( event );
    }
}

void Circuit::keyReleaseEvent( QKeyEvent* event )
{
    if( !event->isAutoRepeat()
     && !( event->modifiers() & Qt::ControlModifier ) )  // Deliver Key events ( switches )
    {
        QString keys = event->text();
        //qDebug() << "Circuit::keyReleaseEvent" << keys;
        while( keys.size() > 0 )
        {
            QString key = keys.left( 1 );
            keys.remove( 0, 1 );

            emit keyEvent( key, false );
        }
    }
    QGraphicsScene::keyReleaseEvent( event );
}


void Circuit::dropEvent( QGraphicsSceneDragDropEvent* event )
{
    QString type = event->mimeData()->html();
    QString id   = event->mimeData()->text();

    QString file = "file://";
    if( id.startsWith( file ) )
    {
        id.replace( file, "" ).replace("\r\n", "" );
#ifdef _WIN32
        if( id.startsWith( "/" )) id.remove( 0, 1 );
#endif
        QString loId = id.toLower();

        if( loId.endsWith( ".jpg")
         || loId.endsWith( ".png")
         || loId.endsWith( ".gif"))
        {
            file = id;
            type = "Image";
            id   = "Image";
            Component* enterItem = createItem( type, id+"-"+newSceneId() );
            if( enterItem )
            {
                saveState();

                enterItem->setBackground( file );

                QPoint cPos = QCursor::pos()-CircuitView::self()->mapToGlobal( QPoint(0,0));
                enterItem->setPos( CircuitView::self()->mapToScene( cPos ) );
                addItem( enterItem );
            }
        }
        else CircuitWidget::self()->loadCirc( id );
    }
}

void Circuit::drawBackground ( QPainter*  painter, const QRectF & rect )
{
    Q_UNUSED( rect );
    /*painter->setBrush(QColor( 255, 255, 255 ) );
    painter->drawRect( m_scenerect );
    return;*/

    painter->setBrush( QColor( 240, 240, 210 ) );
    painter->drawRect( m_scenerect );
    painter->setPen( QColor( 210, 210, 210 ) );

    if( m_hideGrid ) return;

    int startx = int(m_scenerect.x());///2;
    int endx   = int(m_scenerect.width())/2;
    int starty = int(m_scenerect.y());///2;
    int endy   = int(m_scenerect.height())/2;

    for( int i = 4; i<endx; i+=8 )
    {
        painter->drawLine( i, starty, i, endy );
        painter->drawLine(-i, starty,-i, endy );
    }
    for( int i = 4; i<endy; i+=8 )
    {
        painter->drawLine( startx, i, endx, i);
        painter->drawLine( startx,-i, endx,-i);
    }
}

void Circuit::updatePin(ePin* epin, std::string newId )
{
    QString pinId = QString::fromStdString( newId );
    Pin* pin = static_cast<Pin*>( epin );

    addPin( pin, pinId );
}

void Circuit::addPin( Pin* pin, QString pinId )
{
    //qDebug() <<"Pin::Pin"<<pinId;
    m_pinMap[ pinId ] = pin;
}

void Circuit::removePin( QString pinId )
{
    m_pinMap.remove( pinId );
}

bool Circuit::drawGrid()
{
    return !m_hideGrid;
}
void Circuit::setDrawGrid( bool draw )
{
    m_hideGrid = !draw;
    if( m_hideGrid ) MainWindow::self()->settings()->setValue( "Circuit/hideGrid", "true" );
    else             MainWindow::self()->settings()->setValue( "Circuit/hideGrid", "false" );
    update();
}

bool Circuit::showScroll()
{
    return m_showScroll;
}

void Circuit::setShowScroll( bool show )
{
    m_showScroll = show;
    if( show )
    {
        m_graphicView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        m_graphicView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "true" );
    }
    else
    {
        m_graphicView->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        m_graphicView->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        MainWindow::self()->settings()->setValue( "Circuit/showScroll", "false" );
    }
}

bool Circuit::animate()
{
    return m_animate;
}

void Circuit::setAnimate( bool an )
{
    m_animate = an;
    update();
}

double Circuit::fontScale()
{
    return MainWindow::self()->fontScale();
}

void Circuit::setFontScale( double scale )
{
    MainWindow::self()->setFontScale( scale );
}

int Circuit::autoBck()
{
    return MainWindow::self()->autoBck();
}

void Circuit::setAutoBck( int secs )
{
    //qDebug() << "Circuit::setAutoBck"<< secs;
    m_bckpTimer.stop();
    if( secs < 1 ) secs = 0;
    else           m_bckpTimer.start( secs*1000 );

    MainWindow::self()->setAutoBck( secs );
}

int Circuit::noLinAcc()
{
    return Simulator::self()->noLinAcc();
}

void Circuit::setNoLinAcc( int ac )
{
    Simulator::self()->setNoLinAcc( ac );
}

int Circuit::reactStep()
{
    return Simulator::self()->reaClock();
}

void Circuit::setReactStep( int steps )
{
    Simulator::self()->setReaClock( steps );
}

int Circuit::circSpeed()
{
    return Simulator::self()->simuRate();
}
void Circuit::setCircSpeed( int rate )
{
    Simulator::self()->simuRateChanged( rate );
}

QList<Component*>* Circuit::compList() { return &m_compList; }
QList<Component*>* Circuit::conList()  { return &m_conList; }

#include "moc_circuit.cpp"



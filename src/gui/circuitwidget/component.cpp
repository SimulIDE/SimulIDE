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

#include "component.h"
#include "mainwindow.h"
#include "connector.h"
#include "connectorline.h"
#include "propertieswidget.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "utils.h"
#include "simuapi_apppath.h"

#include <math.h>

int Component::m_error = 0;

static const char* Component_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","id"),
    QT_TRANSLATE_NOOP("App::Property","Show id"),
    QT_TRANSLATE_NOOP("App::Property","Unit"),
    QT_TRANSLATE_NOOP("App::Property","Color")
};

Component::Component( QObject* parent, QString type, QString id )
         : QObject( parent )
         , QGraphicsItem()
         , multUnits( "TGMk munp" )
{
    Q_UNUSED( Component_properties );
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    m_help = 0l;
    m_value    = 0;
    m_unitMult = 1;
    m_Hflip  = 1;
    m_Vflip  = 1;
    m_mult   = " ";
    m_unit   = " ";
    m_type   = type;
    m_color  = QColor( Qt::white );
    m_showId = false;
    m_moving = false;
    m_printable = false;
    m_BackGround = "";

    if( ( type != "Connector" )&&( type != "Node" ) )
    {
        LibraryItem* li= ItemLibrary::self()->libraryItem( type );

        if( li )
        {
            if( (type == "Subcircuit")
              ||(type == "AVR")
              ||(type == "PIC")
              ||(type == "Arduino") )
            {
                QString name = id;
                name = name.split( "-" ).first();
                m_help = new QString( li->getHelpFile( name ) );
            }
            else m_help = li->help();
        }
    }
    
    QFont f;
    f.setPixelSize(10);
    
    m_idLabel = new Label( this );
    m_idLabel->setDefaultTextColor( Qt::darkBlue );
    m_idLabel->setFont(f);
    setLabelPos(-16,-24, 0);
    setShowId( false );
    
    m_valLabel = new Label( this );
    m_valLabel->setDefaultTextColor( Qt::black );
    setValLabelPos( 0, 0, 0);
    f.setPixelSize(9);
    m_valLabel->setFont(f);
    setShowVal( false );
    
    setObjectName( id );
    setIdLabel( id );
    setId(id);

    setCursor( Qt::OpenHandCursor );
    this->setFlag( QGraphicsItem::ItemIsSelectable, true );
    
    //setTransformOriginPoint( boundingRect().center() );

    if     ( type == "Connector" )  Circuit::self()->conList()->append( this );
    else if( type == "SerialPort" ) Circuit::self()->compList()->append( this );
    else if( type == "SerialTerm" ) Circuit::self()->compList()->append( this );
    else                            Circuit::self()->compList()->prepend( this );
}
Component::~Component(){}

void Component::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        event->accept();
        if( event->modifiers() == Qt::ControlModifier ) setSelected( !isSelected() );
        else
        {
            if( !isSelected() )     // Deselecciona los demas
            {
                QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();

                for( QGraphicsItem* item : itemlist ) item->setSelected( false );

                setSelected( true );
            }
            QPropertyEditorWidget::self()->setObject( this );
            PropertiesWidget::self()->setHelpText( m_help );
            
            setCursor( Qt::ClosedHandCursor );
        }
    }
}

void Component::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::LeftButton )
    {
        QPropertyEditorWidget::self()->setObject( this );
        PropertiesWidget::self()->setHelpText( m_help );
        //QPropertyEditorWidget::self()->setVisible( true );
    }
}

void Component::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    
    QPointF delta = togrid(event->scenePos()) - togrid(event->lastScenePos());
    
    bool deltaH  = fabs( delta.x() )> 0;
    bool deltaV  = fabs( delta.y() )> 0;
    
    if( !deltaH && !deltaV ) return;

    QList<QGraphicsItem*> itemlist = Circuit::self()->selectedItems();
    if( itemlist.size() > 1 )
    {
        if( !m_moving )
        {
            Circuit::self()->saveState();
            m_moving = true;
        }
        for( QGraphicsItem* item : itemlist )
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine* >( item );
            if( line->objectName() == "" ) 
            {
                //line->move( delta );
                line->moveSimple( delta );
            }

        }
        for( QGraphicsItem* item : itemlist )
        {
            Component* comp =  qgraphicsitem_cast<Component*>( item );
            if(comp && (comp->objectName() != "") && (!comp->objectName().contains("Connector")) )
            {
                comp->move( delta );
            }
        }
        for( Component* comp : *(Circuit::self()->conList()) )
        {
            Connector* con = static_cast<Connector*>( comp );
            con->startPin()->isMoved();
            con->endPin()->isMoved();
        }
    }
    else this->move( delta );
}

void Component::move( QPointF delta )
{
    setPos( pos() + delta );
    emit moved();
}

void Component::moveTo( QPointF pos )
{
    setPos( pos );
    emit moved();
}

void Component::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setCursor( Qt::OpenHandCursor );

    m_moving = false;
    Circuit::self()->update();
}

void Component::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu* menu = new QMenu();
        contextMenu( event, menu );
        menu->deleteLater();
    }
}

void Component::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    m_eventpoint = mapToScene( togrid(event->pos()) );

    QAction* copyAction = menu->addAction(QIcon(":/copy.png"),tr("Copy")+"\tCtrl+C");
    connect( copyAction, SIGNAL( triggered()), this, SLOT(slotCopy()) );

    QAction* removeAction = menu->addAction( QIcon( ":/remove.png"),tr("Remove")+"\tDel" );
    connect( removeAction, SIGNAL( triggered()), this, SLOT(slotRemove()) );
    
    QAction* propertiesAction = menu->addAction( QIcon( ":/properties.png"),tr("Properties") );
    connect( propertiesAction, SIGNAL( triggered()), this, SLOT(slotProperties()) );
    menu->addSeparator();

    QAction* rotateCWAction = menu->addAction( QIcon( ":/rotateCW.png"),tr("Rotate CW") );
    connect( rotateCWAction, SIGNAL( triggered()), this, SLOT(rotateCW()));

    QAction* rotateCCWAction = menu->addAction(QIcon( ":/rotateCCW.png"),tr("Rotate CCW") );
    connect( rotateCCWAction, SIGNAL( triggered()), this, SLOT(rotateCCW()) );

    QAction* rotateHalfAction = menu->addAction(QIcon(":/rotate180.png"),tr("Rotate 180") );
    connect( rotateHalfAction, SIGNAL( triggered()), this, SLOT(rotateHalf()) );
    
    QAction* H_flipAction = menu->addAction(QIcon(":/hflip.png"),tr("Horizontal Flip") );
    connect( H_flipAction, SIGNAL( triggered()), this, SLOT(H_flip()) );
    
    QAction* V_flipAction = menu->addAction(QIcon(":/vflip.png"),tr("Vertical Flip") );
    connect( V_flipAction, SIGNAL( triggered()), this, SLOT(V_flip()) );

    menu->exec(event->screenPos());
}

void Component::slotCopy()
{
    if( !isSelected() ) Circuit::self()->clearSelection();
    setSelected( true );
    Circuit::self()->copy( m_eventpoint );
}

void Component::slotRemove()
{
    if( !isSelected() ) Circuit::self()->clearSelection();
    setSelected( true );
    Circuit::self()->removeItems();
}

void Component::remove()
{
    for( uint i=0; i<m_pin.size(); i++ )   // Remove connectors attached
    {
        Pin* pin = m_pin[i];
        if( !pin ) continue;
        
        if( pin && pin->isConnected())
        {
            Connector* con = pin->connector();
            if( con ) con->remove();
        }
    }
    Circuit::self()->compRemoved( true );
}

void Component::slotProperties()
{
    QPropertyEditorWidget::self()->setObject( this );
    PropertiesWidget::self()->setHelpText( m_help );
    MainWindow::self()->m_sidepanel->setCurrentIndex( 2 ); // Open Properties tab
}

void Component::H_flip()
{
    Circuit::self()->saveState();
    m_Hflip = -m_Hflip;
    setflip();
}

void Component::V_flip()
{
    Circuit::self()->saveState();
    m_Vflip = -m_Vflip;
    setflip();
}

void Component::rotateCW()
{
    Circuit::self()->saveState();
    setRotation( rotation() + 90 );
    emit moved();
}

void Component::rotateCCW()
{
    Circuit::self()->saveState();
    setRotation( rotation() - 90 );
    emit moved();
}

void Component::rotateHalf()
{
    Circuit::self()->saveState();
    setRotation( rotation() - 180);
    emit moved();
}

void Component::updateLabel( Label* label, QString txt )
{
    if     ( label == m_idLabel ) m_id = txt;
    else if( label == m_valLabel )
    {
        QString value = "";
        int x;
        for( x=0; x<txt.length(); ++x ) 
        {
            QChar atx = txt.at(x);
            if( atx.isDigit() ) value.append( atx );
            else break;
        }
        QString unit = txt.mid( x, txt.length() );
        
        setUnit( unit );
        setValue( value.toDouble() );
    }
}

void Component::setLabelPos( int x, int y, int rot )
{
    m_idLabel->m_labelx = x;
    m_idLabel->m_labely = y;
    m_idLabel->m_labelrot = rot;
    m_idLabel->setLabelPos();
}

void Component::setLabelPos()
{
    m_idLabel->setLabelPos();
}

void Component::setValLabelPos( int x, int y, int rot )
{
    m_valLabel->m_labelx = x;
    m_valLabel->m_labely = y;
    m_valLabel->m_labelrot = rot;
    m_valLabel->setLabelPos();
}

void Component::setValLabelPos()
{
    m_valLabel->setLabelPos();
}

void Component::setValue( double val) 
{ 
    if( fabs(val) < 1e-12 ) 
    {
        m_value = 0;
        m_mult = " ";
    }
    else
    {
        val = val*m_unitMult;
        
        int index = 4;   // We are in bare units "TGMK munp"
        m_unitMult = 1;
        while( fabs(val) >= 1000 )
        {
            index--;
            m_unitMult = m_unitMult*1000;
            val = val/1000;
        }
        while( fabs(val) < 1 )
        {
            index++;
            m_unitMult = m_unitMult/1000;
            val = val*1000;
        }
        if( index > 8 )
        {
            index = 8;
            val = val/1000;
        }
        m_mult = multUnits.at( index );
        if( m_mult != " " ) m_mult.prepend( " " );
        m_value = val;
    }
    m_valLabel->setPlainText( QString::number(m_value)+m_mult+m_unit );
}

QString Component::unit()                { return m_mult+m_unit; }
void Component::setUnit( QString un ) 
{ 
    QString mul = " ";
    un.replace( " ", "" );
    if( un.size() > 0 ) 
    {
        mul = un.at(0);
        
        double unitMult = 1e12;        // We start in Tera units "TGMk munp"
        
        for( int x=0; x<9; x++ )
        {
            if( mul == multUnits.at(x) ) 
            {
                m_unitMult = unitMult;
                m_mult     = mul;
                if( m_mult != " " ) m_mult.prepend( " " );
                m_valLabel->setPlainText( QString::number(m_value)+m_mult+m_unit );
                return;
            }
            unitMult = unitMult/1000;
        }
    }
    m_unitMult = 1;
    m_mult     = " ";
    m_valLabel->setPlainText( QString::number(m_value)+m_mult+m_unit );
}

double Component::getmultValue() { return m_value*m_unitMult; }

bool Component::showId()               { return m_showId; }
void Component::setShowId( bool show ) 
{ 
    m_idLabel->setVisible( show );
    m_showId = show;
}

bool Component::showVal()               { return m_showVal;}
void Component::setShowVal( bool show ) 
{ 
    m_valLabel->setVisible( show );
    m_showVal = show; 
}

QString Component::idLabel() { return m_idLabel->toPlainText(); }
void Component::setIdLabel( QString id ) { m_idLabel->setPlainText( id ); }

QString Component::itemID()         { return  m_id; }
void Component::setId( QString id ) {  m_id = id;  }

int Component::labelx()            { return m_idLabel->m_labelx; }
void Component::setLabelX( int x ) { m_idLabel->m_labelx = x; }

int Component::labely()            { return m_idLabel->m_labely; }
void Component::setLabelY( int y ) { m_idLabel->m_labely = y; }

int Component::labelRot()              { return m_idLabel->m_labelrot; }
void Component::setLabelRot( int rot ) { m_idLabel->m_labelrot = rot; }

int Component::valLabelx()            { return m_valLabel->m_labelx; }
void Component::setValLabelX( int x ) { m_valLabel->m_labelx = x; }

int Component::valLabely()            { return m_valLabel->m_labely; }
void Component::setValLabelY( int y ) { m_valLabel->m_labely = y; }

int Component::valLabRot()              { return m_valLabel->m_labelrot; }
void Component::setValLabRot( int rot ) { m_valLabel->m_labelrot = rot; }

int Component::hflip()            { return m_Hflip; }
void Component::setHflip( int hf )
{ 
    if(( hf != 1 )&( hf != -1 )) hf = 1;
    m_Hflip = hf;
    setflip();
}
        
int Component::vflip(){ return m_Vflip; }
void Component::setVflip( int vf )
{ 
    if(( vf != 1 )&( vf != -1 )) vf = 1;
    m_Vflip = vf; 
    setflip();
}

void Component::setflip()
{
    setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    m_idLabel->setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    m_valLabel->setTransform(QTransform::fromScale( m_Hflip, m_Vflip ));
    emit moved();
}

QString Component::itemType()  { return m_type; }
QString Component::category()  { return m_category; }
QIcon   Component::icon()      { return m_icon; }

//bool Component::isChanged(){ return m_changed;}

void Component::setPrintable( bool p )
{
    m_printable = p;
}

QString Component::print()
{
    if( !m_printable ) return "";
    
    QString str = m_id+" : ";
    str += objectName().split("-").first()+" ";
    if( m_value > 0 ) str += QString::number( m_value );
    str += m_mult+m_unit+"\n";
    
    return str;
}

void Component::paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED(option); Q_UNUSED(widget);

    QPen pen(Qt::black, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if ( isSelected() )
    {
        pen.setColor( Qt::darkGray);
        painter->setBrush( Qt::darkGray);
        //label->setBrush( Qt::darkGray );
    }
    else
    {
        painter->setBrush( m_color );
        //label->setBrush( Qt::darkBlue );
    }
    //painter->setBrush( Qt::yellow );
    //painter->drawRect( boundingRect() );

    painter->setPen( pen );
}


// CLASS Label  *****************************************************************************

Label::Label( Component* parent )
     : QGraphicsTextItem( parent )
{
    m_parentComp = parent;
    m_labelrot = 0;
    setCursor( Qt::OpenHandCursor );
    
    this->document()->setDocumentMargin(0);
    
    connect( document(), SIGNAL( contentsChange(int, int, int)),
             this,       SLOT(   updateGeometry(int, int, int)));
            
    //document()->setDefaultStyleSheet( QString("p {max-width: 500px;}") );
}
Label::~Label() { }

void Label::updateGeometry( int, int, int )
{
    document()->setTextWidth( -1 );
    //setTextWidth( boundingRect().width() );
    //setItemSize(boundingRect().width(), boundingRect().height());
    //adjustSize();
}

void Label::focusOutEvent( QFocusEvent *event )
{
    setTextInteractionFlags( Qt::NoTextInteraction );
    m_parentComp->updateLabel( this, document()->toPlainText() );
    
    QGraphicsTextItem::focusOutEvent(event);
}

void Label::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    if( !isEnabled() ) return;
    //setTextInteractionFlags(Qt::TextEditorInteraction);

    QGraphicsTextItem::mouseDoubleClickEvent(event);
}

void Label::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if ( event->button() == Qt::LeftButton )
    {
        event->accept();
        setCursor( Qt::ClosedHandCursor );
        grabMouse();
    }
}

void Label::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setPos(  pos() + mapToItem( m_parentComp, event->pos() ) - mapToItem( m_parentComp, event->lastPos() ) );
    m_labelx = int(pos().x());
    m_labely = int(pos().y());
}

void Label::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    setCursor( Qt::OpenHandCursor );
    ungrabMouse();
}

void Label::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( !acceptedMouseButtons() ) event->ignore();
    else
    {
        event->accept();
        QMenu menu;

        QAction* rotateCWAction = menu.addAction(QIcon(":/rotateCW.png"),"Rotate CW");
        connect(rotateCWAction, SIGNAL(triggered()), this, SLOT(rotateCW()));

        QAction* rotateCCWAction = menu.addAction(QIcon(":/rotateCCW.png"),"Rotate CCW");
        connect(rotateCCWAction, SIGNAL(triggered()), this, SLOT(rotateCCW()));

        QAction* rotate180Action = menu.addAction(QIcon(":/rotate180.png"),"Rotate 180º");
        connect(rotate180Action, SIGNAL(triggered()), this, SLOT(rotate180()));

        /*QAction* selectedAction = */menu.exec(event->screenPos());
    }
}

void Label::setLabelPos()
{
    setX( m_labelx );
    setY( m_labely );
    setRotation( m_labelrot );
    adjustSize();
}

void Label::rotateCW()
{
    if( !isEnabled() ) return;
    setRotation( rotation() + 90 );
    m_labelrot = int(rotation()) ;
}

void Label::rotateCCW()
{
    if( !isEnabled() ) return;
    setRotation( rotation() - 90 );
    m_labelrot = int(rotation()) ;
}

void Label::rotate180()
{
    if( !isEnabled() ) return;
    setRotation( rotation() - 180 );
    m_labelrot = int(rotation()) ;
}

void Label::H_flip( int hf )
{
    if( !isEnabled() ) return;
    setTransform( QTransform::fromScale(hf, 1) );
    //m_idLabel->rotateCCW();
}

void Label::V_flip( int vf )
{
    if( !isEnabled() ) return;
    setTransform( QTransform::fromScale(1, vf) );
}

/*void Label::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    painter->setBrush( Qt::blue );
    painter->drawRect( boundingRect() );
    QGraphicsTextItem::paint( painter, option, widget );
}*/

#include "moc_component.cpp"

/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "subpackage.h"
#include "itemlibrary.h"
#include "circuit.h"
#include "simuapi_apppath.h"
#include "utils.h"

QString SubPackage::m_lastPkg = "";

static const char* SubPackage_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Package File"),
    QT_TRANSLATE_NOOP("App::Property","Width"),
    QT_TRANSLATE_NOOP("App::Property","Height")
};

Component* SubPackage::construct( QObject* parent, QString type, QString id )
{
    return new SubPackage( parent, type, id );
}

LibraryItem* SubPackage::libraryItem()
{
    return new LibraryItem(
        tr( "Package" ),
        tr( "Other" ),
        "subc2.png",
        "Package",
        SubPackage::construct );
}

SubPackage::SubPackage( QObject* parent, QString type, QString id )
          : Chip( parent, type, id )
{
    Q_UNUSED( SubPackage_properties );
    
    m_width = 4;
    m_height = 8;
    
    m_changed = false;
    m_fakePin = false;
    m_movePin = false;
    m_isLS    = true;
    
    m_lsColor = QColor( 210, 210, 255 );
    m_icColor = QColor( 40, 40, 120 );
    m_color = m_lsColor;
    
    m_area  = QRect(0, 0, m_width*8, m_height*8);
    
    setAcceptHoverEvents(true);
    
    m_pkgeFile = SIMUAPI_AppPath::self()->RODataFolder().absolutePath();
    if( m_lastPkg == "" ) m_lastPkg = m_pkgeFile;
}
SubPackage::~SubPackage(){}

void SubPackage::hoverMoveEvent( QGraphicsSceneHoverEvent* event ) 
{
    if( event->modifiers() & Qt::ShiftModifier) 
    {
        m_fakePin = true;
        
        int xPos = snapToCompGrid( (int)event->pos().x() );
        int yPos = snapToCompGrid( (int)event->pos().y() );
        
        if( xPos == 0 && yPos >= 8 && yPos <= m_height*8-8 ) // Left
        {
            m_angle = 180;
            m_p1X = -8;
            m_p1Y = yPos;
            m_p2X = 0;
            m_p2Y = yPos;
        }
        else if( xPos == m_width*8 && yPos >= 8 && yPos <= m_height*8-8 ) // Right
        {
            m_angle = 0;
            m_p1X = m_width*8+8;
            m_p1Y = yPos;
            m_p2X = m_width*8;
            m_p2Y = yPos;
        }
        else if( yPos == 0 && xPos >= 8&& xPos <= m_width*8-8 ) // Top 
        {
            m_angle = 90;
            m_p1X = xPos;
            m_p1Y = -8;
            m_p2X = xPos;
            m_p2Y = 0;
        }
        else if( yPos == m_height*8 && xPos >= 8 && xPos <= m_width*8-8 ) // Bottom
        {
            m_angle = 270;
            m_p1X = xPos;
            m_p1Y = m_height*8+8;
            m_p2X = xPos;
            m_p2Y = m_height*8;
        }
        else m_fakePin = false;

        Circuit::self()->update();
        //qDebug() <<"Mouse hovered"<<xPos<<yPos;
    }
    else QGraphicsItem::hoverMoveEvent(event);
}

void SubPackage::hoverLeaveEvent( QGraphicsSceneHoverEvent* event ) 
{
    m_fakePin = false;
    Circuit::self()->update();
    QGraphicsItem::hoverLeaveEvent(event);
}

void SubPackage::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_fakePin )
    {
        event->accept();

        Pin* pin = new Pin( m_angle, QPoint(m_p1X,m_p1Y ), "name", 0, this );
        m_fakePin = false;

        pin->setLabelText( "Name" );
        pin->setPinId( "Id" );
        pin->setLabelColor( QColor( Qt::black ) );
        pin->setLabelPos();

        if( m_angle == 0 )
        {
            m_rigPin.append( pin );
            qSort( m_rigPin.begin(), m_rigPin.end(), lessPinY );
            //for( Pin* pin : m_rigPin ) qDebug() << pin->pinId();
        }
        else if( m_angle == 90 )
        {
            m_topPin.append( pin );
            qSort( m_topPin.begin(), m_topPin.end(), lessPinX );
        }
        else if( m_angle == 180 )
        {
            m_lefPin.append( pin );
            qSort( m_lefPin.begin(), m_lefPin.end(), lessPinY );
        }
        else if( m_angle == 270 )
        {
            m_botPin.append( pin );
            qSort( m_botPin.begin(), m_botPin.end(), lessPinX );
        }
        m_eventPin = pin;
        editPin();
        Circuit::self()->update();
    }
    else if( m_movePin )
    {
        event->accept();
        ungrabMouse();
        setCursor( Qt::OpenHandCursor );
        
        if     ( m_angle == 0 )   qSort( m_rigPin.begin(), m_rigPin.end(), lessPinY );
        else if( m_angle == 90 )  qSort( m_topPin.begin(), m_topPin.end(), lessPinX );
        else if( m_angle == 180 ) qSort( m_lefPin.begin(), m_lefPin.end(), lessPinY );
        else if( m_angle == 270 ) qSort( m_botPin.begin(), m_botPin.end(), lessPinX );
        
        m_changed = true;
        m_movePin = false;
        m_eventPin = 0l;
    }
    else Component::mousePressEvent( event );
}

void SubPackage::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    if( m_movePin && m_eventPin )
    {
        event->accept();
        QPointF delta = toCompGrid(event->scenePos()) - toCompGrid(event->lastScenePos());
        
        int deltaX = delta.x();
        int deltaY = delta.y();
        bool deltaH  = fabs( deltaX ) > 0;
        bool deltaV  = fabs( deltaY ) > 0;
        
        if( ((m_angle == 0)||(m_angle == 180)) && deltaV )   
        {
            m_eventPin->moveBy( 0, deltaY );
            if     ( m_eventPin->y() < 8 )            m_eventPin->setY( 8 );
            else if( m_eventPin->y() > m_height*8-8 ) m_eventPin->setY( m_height*8-8 );
            m_eventPin->setLabelPos();
            m_eventPin->isMoved();
        }
        else if( ((m_angle == 90)||(m_angle == 270)) && deltaH )  
        {
            m_eventPin->moveBy( deltaX, 0 );
            if     ( m_eventPin->x() < 8 )          m_eventPin->setX( 8 );
            else if( m_eventPin->x() > m_width*8-8) m_eventPin->setX( m_width*8-8 );
            m_eventPin->setLabelPos();
            m_eventPin->isMoved();
        }
    }
    else Component::mouseMoveEvent( event );
}

void SubPackage::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    int xPos = snapToCompGrid( (int)event->pos().x() );
    int yPos = snapToCompGrid( (int)event->pos().y() );

    m_eventPin = 0l;

    if( xPos == 0 && yPos >= 8 && yPos <= m_height*8-8 ) // Left
    {
        for( Pin* pin : m_lefPin )
        {
            if( pin->y() == yPos )
            {
                m_eventPin = pin;
                break;
            }
        }
    }
    else if( xPos == m_width*8 && yPos >= 8 && yPos <= m_height*8-8 ) // Right
    {
        for( Pin* pin : m_rigPin )
        {
            if( pin->y() == yPos )
            {
                m_eventPin = pin;
                break;
            }
        }
    }
    else if( yPos == 0 && xPos >= 8 && xPos <= m_width*8-8 ) // Top
    {
        for( Pin* pin : m_topPin )
        {
            if( pin->x() == xPos )
            {
                m_eventPin = pin;
                break;
            }
        }
    }
    else if( yPos == m_height*8 && xPos >= 8 && xPos <= m_width*8-8 ) // Bottom
    {
        for( Pin* pin : m_botPin )
        {
            if( pin->x() == xPos )
            {
                m_eventPin = pin;
                break;
            }
        }
    }
    event->accept();
    QMenu* menu = new QMenu();
    contextMenu( event, menu );
    menu->deleteLater();
}

void SubPackage::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    if( m_eventPin )
    {
        QAction* moveAction = menu->addAction( QIcon(":/hflip.png"),tr("Move Pin ")+m_eventPin->getLabelText() );
        connect( moveAction, SIGNAL(triggered()), this, SLOT( movePin() ) );

        QAction* editAction = menu->addAction( QIcon(":/rename.png"),tr("Edit Pin ")+m_eventPin->getLabelText() );
        connect( editAction, SIGNAL(triggered()), this, SLOT( editPin() ) );

        QAction* deleteAction = menu->addAction( QIcon(":/remove.png"),tr("Delete Pin ")+m_eventPin->getLabelText() );
        connect( deleteAction, SIGNAL(triggered()), this, SLOT( deletePin() ) );

        menu->exec( event->screenPos() );
    }
    else
    {
        QAction* loadAction = menu->addAction( QIcon(":/open.png"),tr("Load Package") );
        connect( loadAction, SIGNAL(triggered()), this, SLOT( loadPackage() ) );

        QAction* saveAction = menu->addAction( QIcon(":/save.png"),tr("Save Package") );
        connect( saveAction, SIGNAL(triggered()), this, SLOT( slotSave() ) );

        menu->addSeparator();

        Component::contextMenu( event, menu );
    }
}

void SubPackage::remove()
{
    if( m_changed )
    {
        const QMessageBox::StandardButton ret
        = QMessageBox::warning( 0l, "SubPackage::remove",
                               tr("\nPackage has been modified.\n"
                                  "Do you want to save your changes?\n"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
                               
        if     ( ret == QMessageBox::Save ) slotSave();
        else if( ret == QMessageBox::Cancel ) return;
    }
    for( Pin* pin : m_rigPin )
    {
        if( !pin ) continue;
        
        if( pin && pin->isConnected())
        {
            Connector* con = pin->connector();
            if( con ) con->remove();
        }
    }
    for( Pin* pin : m_topPin )
    {
        if( !pin ) continue;
        
        if( pin && pin->isConnected())
        {
            Connector* con = pin->connector();
            if( con ) con->remove();
        }
    }
    for( Pin* pin : m_lefPin )
    {
        if( !pin ) continue;
        
        if( pin && pin->isConnected())
        {
            Connector* con = pin->connector();
            if( con ) con->remove();
        }
    }
    for( Pin* pin : m_botPin )
    {
        if( !pin ) continue;
        
        if( pin && pin->isConnected())
        {
            Connector* con = pin->connector();
            if( con ) con->remove();
        }
    }
    Circuit::self()->compRemoved( true );
}

int SubPackage::width()
{
    return m_width;
}

void SubPackage::setWidth( int width )
{
    if( m_width == width ) return;
    m_changed = true;
    
    int minTopWidth = 2;
    int minBotWidth = 2;
    if( !m_topPin.isEmpty() ) minTopWidth = m_topPin.last()->x()/8+1;
    if( !m_botPin.isEmpty() ) minBotWidth = m_botPin.last()->x()/8+1;
    //qDebug() <<m_topPin.last()->x()<< minTopWidth << minBotWidth;

    if( width < minTopWidth ) width = minTopWidth;
    if( width < minBotWidth ) width = minBotWidth;
    
    m_width = width;
    m_area = QRect(0, 0, m_width*8, m_height*8);
    
    for( Pin* pin : m_rigPin )
    {
        pin->setX( m_width*8+8 );
        pin->setLabelPos();
    }
    Circuit::self()->update();
}

int SubPackage::height()
{
    return m_height;
}

void SubPackage::setHeight( int height )
{
    if( m_height == height ) return;
    m_changed = true;
    
    int minRigHeight = 2;
    int minLefHeight = 2;
    if( !m_rigPin.isEmpty() ) minRigHeight = m_rigPin.last()->y()/8+1;
    if( !m_lefPin.isEmpty() ) minLefHeight = m_lefPin.last()->y()/8+1;

    if( height < minRigHeight ) height = minRigHeight;
    if( height < minLefHeight ) height = minLefHeight;
    
    m_height = height;
    m_area = QRect( 0, 0, m_width*8, m_height*8 );
    
    for( Pin* pin : m_botPin )
    {
        pin->setY( m_height*8+8 );
        pin->setLabelPos();
    }
    Circuit::self()->update();
}

void SubPackage::movePin()
{
    if( !m_eventPin ) return;
    
    m_changed = true;
    m_movePin = true;
    m_angle = m_eventPin->pinAngle();
    
    grabMouse();
}

void SubPackage::editPin()
{
    if( !m_eventPin ) return;
    EditDialog* editDialog = new EditDialog( this, m_eventPin, 0l );

    connect( editDialog, SIGNAL( finished(int) ),
                   this, SLOT( editFinished(int) ) );

    editDialog->exec();
    editDialog->deleteLater();
}

void SubPackage::editFinished( int r )
{
    if( m_changed ) Circuit::self()->saveState();
}

void SubPackage::deletePin()
{
    if( !m_eventPin ) return;
    m_changed = true;
    
    int angle = m_eventPin->pinAngle();
    
    if( angle == 0 )   
    {
        m_rigPin.removeOne( m_eventPin );
        qSort( m_rigPin.begin(), m_rigPin.end(), lessPinY );
    }
    else if( angle == 90 )  
    {
        m_topPin.removeOne( m_eventPin );
        qSort( m_topPin.begin(), m_topPin.end(), lessPinX );
    }
    else if( angle == 180 ) 
    {
        m_lefPin.removeOne( m_eventPin );
        qSort( m_lefPin.begin(), m_lefPin.end(), lessPinY );
    }
    else if( angle == 270 ) 
    {
        m_botPin.removeOne( m_eventPin );
        qSort( m_botPin.begin(), m_botPin.end(), lessPinX );
    }
       
    if( m_eventPin->isConnected() ) m_eventPin->connector()->remove();
    if( m_eventPin->scene() ) Circuit::self()->removeItem( m_eventPin );
    m_eventPin->reset();
    delete m_eventPin;
    m_eventPin = 0l;
    
    Circuit::self()->update();
}


void SubPackage::setPinId( QString id )
{
    m_eventPin->setPinId( id.replace("-","") );
    m_changed = true;
}

void SubPackage::setPinName( QString name )
{
    m_eventPin->setLabelText( name );
    m_changed = true;
}

void SubPackage::invertPin( bool invert )
{
    m_eventPin->setInverted( invert );
    Circuit::self()->update();
    m_changed = true;
}

void SubPackage::unusePin( bool unuse )
{
    m_eventPin->setUnused( unuse );
    Circuit::self()->update();
    m_changed = true;
}

QString SubPackage::package()
{
    return m_pkgeFile;
    Circuit::self()->update();
}

void SubPackage::setPackage( QString package )
{
    m_pkgeFile = package;
    Chip::initChip();
    
    setLogicSymbol( m_isLS );

    Circuit::self()->update();
}

void SubPackage::setLogicSymbol( bool ls )
{
    if( ls == m_isLS ) return;
    m_isLS = ls;

    QColor labelColor = QColor( 0, 0, 0 );

    if( ls ) m_color = m_lsColor;
    else
    {
        m_color = m_icColor;
        labelColor = QColor( 250, 250, 200 );
    }

    for( Pin* pin : m_lefPin )
    {
        if( !pin ) continue;
        pin->setLabelColor( labelColor );
    }
    for( Pin* pin : m_botPin )
    {
        if( !pin ) continue;
        pin->setLabelColor( labelColor );
    }
    for( Pin* pin : m_rigPin )
    {
        if( !pin ) continue;
        pin->setLabelColor( labelColor );
    }
    for( Pin* pin : m_topPin )
    {
        if( !pin ) continue;
        pin->setLabelColor( labelColor );
    }
    Circuit::self()->update();
}

void SubPackage::slotSave()
{
    QDir pdir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString pkgeFile = pdir.absoluteFilePath( m_pkgeFile );

    //qDebug() << "SubPackage::slotSave"<<pkgeFile;
    const QString dir = pkgeFile;
    QString fileName = QFileDialog::getSaveFileName( 0l, tr("Save Package"), dir,
                                                     tr("Packages (*.package);;All files (*.*)"));
    if (fileName.isEmpty()) return;

    savePackage( fileName );
}

QString SubPackage::pinEntry( Pin* pin, int pP, QString side )
{
    QString label = pin->getLabelText();
    QString id    = pin->pinId().split( "-" ).last().replace( " ", "" );
    QString paPin = QString::number( pP );
    
    QString pos = "";
    if     ( side == "left"   ) pos = QString::number( (int)pin->y()/8 );
    else if( side == "bottom" ) pos = QString::number( (int)pin->x()/8 );
    else if( side == "right"  ) pos = QString::number( (int)pin->y()/8 );
    else if( side == "top"    ) pos = QString::number( (int)pin->x()/8 );
    
    QString type = "";
    if( pin->inverted() )
    {
        type = "inverted";
        //if( !id.startsWith( "!" ) ) id = "!"+id;
    }
    else if( pin->unused() ) type = "unused";
    
    return "    <pin side=\""+side+"\" pos=\""+pos+"\"  type=\""+type+"\" id=\""+id+"\"  label=\""+label+"\" /><!-- packagePin"+paPin+" -->\n";
    //pP++;
}

void SubPackage::loadPackage()
{
    QDir pkgDir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    QString dir = pkgDir.absoluteFilePath( m_pkgeFile );
    
    QString fileName = QFileDialog::getOpenFileName( 0l, tr("Load Package File"), dir,
                       tr("Packages (*.package);;All files (*.*)"));

    if (fileName.isEmpty()) return; // User cancels loading

    setPackage( fileName );

    qSort( m_rigPin.begin(), m_rigPin.end(), lessPinY );
    qSort( m_topPin.begin(), m_topPin.end(), lessPinX );
    qSort( m_lefPin.begin(), m_lefPin.end(), lessPinY );
    qSort( m_botPin.begin(), m_botPin.end(), lessPinX );

    QDir pdir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();
    m_pkgeFile = pdir.relativeFilePath( fileName );
    m_lastPkg = fileName;

    Circuit::self()->saveState();
    Circuit::self()->update();
}

void SubPackage::savePackage( QString fileName )
{
    //qDebug() <<"SubPackage::savePackage fileName"<< fileName;
    if( !fileName.endsWith(".package") ) fileName.append(".package");

    QFile file( fileName );

    if( !file.open(QFile::WriteOnly | QFile::Text) )
    {
          QMessageBox::warning(0l, "Circuit::saveCircuit",
          tr("Cannot write file %1:\n%2.").arg(fileName).arg(file.errorString()));
          return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    QString name = m_id.split("-").first();
    int pins = m_topPin.size()+m_botPin.size()+m_lefPin.size()+m_rigPin.size();

    out << "<!DOCTYPE SimulIDE>\n\n";
    out << "<!-- This file was generated by SimulIDE -->\n\n";
    out << "<package name=\""+name+"\" pins=\""+QString::number(pins)+"\" width=\""+QString::number(m_width)+"\" height=\""+QString::number(m_height)+"\" >\n\n";
    
    int pP = 1;
    for( Pin* pin : m_lefPin )
    {
        out << pinEntry( pin, pP, "left" );
        pP++;
    }
    out << "    \n";
    for( Pin* pin : m_botPin )
    {
        out << pinEntry( pin, pP, "bottom" );
        pP++;
    }
    out << "    \n";
    for( Pin* pin : m_rigPin )
    {
        out << pinEntry( pin, pP, "right" );
        pP++;
    }
    out << "    \n";
    for( Pin* pin : m_topPin )
    {
        out << pinEntry( pin, pP, "top" );
        pP++;
    }
    out << "    \n";
    out << "</package>\n";

    file.close();
    QApplication::restoreOverrideCursor();

    QDir dir = QFileInfo( Circuit::self()->getFileName() ).absoluteDir();

    m_pkgeFile = dir.relativeFilePath( fileName );
    m_lastPkg = fileName;
    m_changed = false;
}

void SubPackage::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Chip::paint( p, option, widget );
    
    if( m_fakePin )
    {
        QPen pen = p->pen();
        pen.setWidth( 2 );
        pen.setColor( Qt::gray );
        p->setPen(pen);
        p->drawLine( m_p1X, m_p1Y, m_p2X, m_p2Y);
    }
}


//_____________________________________________________________
//_____________________________________________________________


EditDialog::EditDialog( SubPackage* pack, Pin* eventPin, QWidget* parent )
          : QDialog( parent, Qt::WindowCloseButtonHint )
{
    /*setWindowFlags( windowFlags() | Qt::CustomizeWindowHint );
    setWindowFlags( windowFlags() | Qt::WindowSystemMenuHint );
    setWindowFlags( windowFlags() | Qt::WindowCloseButtonHint );*/
    m_package = pack;

    m_nameLabel = new QLabel( tr("Pin Name:") );
    m_nameLineEdit = new QLineEdit;
    m_nameLineEdit->setText( eventPin->getLabelText() );
    m_nameLabel->setBuddy( m_nameLineEdit );
    QHBoxLayout* nameLayout = new QHBoxLayout;
    nameLayout->addWidget( m_nameLabel );
    nameLayout->addWidget( m_nameLineEdit );

    m_idLabel = new QLabel( tr("Pin Id:") );
    m_idLineEdit = new QLineEdit;
    m_idLineEdit->setText( eventPin->pinId().split( "-" ).last() );
    m_idLabel->setBuddy( m_idLineEdit );
    QHBoxLayout* idLayout = new QHBoxLayout;
    idLayout->addWidget( m_idLabel );
    idLayout->addWidget( m_idLineEdit );

    m_invertCheckBox = new QCheckBox(tr("Invert Pin"));
    m_invertCheckBox->setChecked( eventPin->inverted() );
    m_unuseCheckBox  = new QCheckBox(tr("Unused Pin"));
    m_unuseCheckBox->setChecked( eventPin->unused() );

    QDialogButtonBox* bb = new QDialogButtonBox( QDialogButtonBox::Ok );
    QPushButton* okBtn = bb->button(QDialogButtonBox::Ok);
    okBtn->setAutoDefault(true);
    okBtn->setDefault(true);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addLayout( nameLayout );
    layout->addLayout( idLayout );
    layout->addWidget( m_invertCheckBox );
    layout->addWidget( m_unuseCheckBox );
    layout->addWidget( bb );

    setLayout( layout );
    setWindowTitle( tr("Edit Pin ")+eventPin->getLabelText() );

    connect( bb, SIGNAL(accepted()), this, SLOT(accept()));

    connect( m_invertCheckBox, SIGNAL( toggled( bool ) ),
                         this, SLOT( invertPin( bool ) ) );

    connect( m_unuseCheckBox,  SIGNAL( toggled( bool ) ),
                        pack,  SLOT( unusePin( bool ) ) );

    connect( m_nameLineEdit, SIGNAL( textChanged( QString ) ),
                       pack, SLOT( setPinName( QString ) ) );

    connect( m_idLineEdit, SIGNAL( textEdited( QString ) ),
                     pack, SLOT( setPinId( QString ) ) );

    connect( m_idLineEdit, SIGNAL( textEdited( QString ) ),
                     pack, SLOT( setPinId( QString ) ) );
}

void EditDialog::invertPin( bool invert )
{
    QString id = m_idLineEdit->text();
    if( invert && !id.startsWith("!") ) id.prepend("!");
    else if( !invert && id.startsWith("!") ) id.remove( 0, 1 );

    m_idLineEdit->setText( id );
    m_package->invertPin( invert );
}

#include "moc_subpackage.cpp"

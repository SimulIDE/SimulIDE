/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "switchdip.h"
#include "itemlibrary.h"
#include "connector.h"
#include "circuit.h"
#include "pin.h"

static const char* SwitchDip_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Size")
};

Component* SwitchDip::construct( QObject* parent, QString type, QString id )
{ return new SwitchDip( parent, type, id ); }

LibraryItem* SwitchDip::libraryItem()
{
    return new LibraryItem(
            tr( "Switch Dip" ),
            tr( "Switches" ),
            "switchdip.png",
            "SwitchDip",
            SwitchDip::construct);
}

SwitchDip::SwitchDip( QObject* parent, QString type, QString id )
         : Component( parent, type, id )
         , eElement( id.toStdString() )
{
    Q_UNUSED( SwitchDip_properties );

    m_color = QColor( 50, 50, 70 );
    m_size = 0;
    m_state = 0;
    setSize( 8 );
    
    m_changed = true;
    
    setShowVal( false );
    setLabelPos(-16,-44, 0);
    
    Simulator::self()->addToUpdateList( this );
}
SwitchDip::~SwitchDip(){}

void SwitchDip::stamp()
{
    for( int i=0; i<m_size; i++ )
    {
        int pin1 = i*2;
        int pin2 = pin1+1;
        
        //qDebug()<<"SwitchDip::initialize"<< i<<m_size<<m_pin.size()<<pin1<<pin2;
        
        eNode* node0 = m_pin[pin1]->getEnode();
        eNode* node1 = m_pin[pin2]->getEnode();

        if( node0 ) node0->setSwitched( true );
        if( node1 ) node1->setSwitched( true );

        m_pin[pin1]->setEnodeComp( node1 );
        m_pin[pin2]->setEnodeComp( node0 );
    }
    m_changed = true;
    updateStep();
}

void SwitchDip::updateStep()
{
    if( !m_changed ) return;

    int i = 0;
    for( QPushButton* button : m_buttons ) 
    {
        double admit = 0;
        if( button->isChecked()  ) admit = 1e3;
        
        int pin = i*2;
        m_pin[pin]->stampAdmitance( admit );
        m_pin[pin+1]->stampAdmitance( admit );
       
       i++;
    }
    m_changed = false;
}

void SwitchDip::onbuttonclicked()
{
    m_changed = true;

    int i = 0;
    for( QPushButton* button : m_buttons ) 
    {
        if( button->isChecked()  ) 
        {
            button->setIcon(QIcon(":/switchbut.png"));
            m_state |= 1<<i;
        }
        else
        {
            button->setIcon(QIcon(":/stop.png"));
            m_state &= ~(1<<i);
        }
        i++;
    }
    update();
}

int SwitchDip::state()
{
    return m_state;
}

void SwitchDip::setState( int state )
{
    if( m_state == state ) return;
    m_state = state;
    
    int i = 0;
    for( QPushButton* button : m_buttons ) 
    {
        bool switchState = state&1;
        state >>= 1;
        
        button->setChecked( switchState );
        
        if( switchState  ) button->setIcon(QIcon(":/switchbut.png"));
        else               button->setIcon(QIcon(":/stop.png"));
        
        i++;
    }
    m_changed = true;
}

void SwitchDip::createSwitches( int c )
{
    int start = m_size;
    m_size = m_size+c;
    m_pin.resize( m_size*2 );

    for( int i=start; i<m_size; i++ )
    {
        int index = i*2;
        QString butId = m_id + QString("-switch"+QString::number(i));
        QPushButton* button = new QPushButton( );
        button->setMaximumSize( 6,6 );
        button->setGeometry(-6,-6,6,6);
        QFont font = button->font();
        font.setPixelSize(5);
        button->setFont(font);
        //button->setText( "O");
        button->setIcon(QIcon(":/stop.png"));
        button->setCheckable( true );
        button->setChecked( true );
        button->setIcon(QIcon(":/switchbut.png"));
        m_buttons.append( button );
        
        QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( button );
        proxy->setParentItem( this );
        proxy->setPos( QPoint( 3, -27+i*8 ) );
        m_proxys.append( proxy );
        connect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
        
        QPoint pinpos = QPoint(-8,-32+8+i*8 );
        Pin* pin = new Pin( 180, pinpos, butId+"-pinP", 0, this);
        m_pin[index] = pin;
        
        pinpos = QPoint( 16,-32+8+i*8 );
        pin = new Pin( 0, pinpos, butId+"-pinN", 0, this);
        m_pin[index+1] = pin;
        
        m_state |= 1<<i;                          // default state = on;
    }
    //update();
}

void SwitchDip::deleteSwitches( int d )
{
    if( d > m_size ) d = m_size;
    int start = m_size-d;

    for( int i=start*2; i<m_size*2; i++ )
    {
        Pin* pin = m_pin[i];
        if( pin->isConnected() ) pin->connector()->remove();
        
        delete pin;
    }
    for( int i=start; i<m_size; i++ )
    {
        QPushButton* button = m_buttons.takeLast();
        disconnect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
        delete button;
        
        m_proxys.removeLast();
    }
    m_size = m_size-d;

    m_pin.resize( m_size*2 );
    
    Circuit::self()->update();
}

int SwitchDip::size()
{
    return m_size;
}

void SwitchDip::setSize( int size )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    if( size == 0 ) size = 8;
    
    if     ( size < m_size ) deleteSwitches( m_size-size );
    else if( size > m_size ) createSwitches( size-m_size );
    
    m_area = QRect( -1, -26, 10, m_size*8-4 );
    
    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
}

void SwitchDip::remove()
{
    Simulator::self()->remFromUpdateList( this );
    
    deleteSwitches( m_size );

    Component::remove();
}

void SwitchDip::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    
    //p->setBrush( QColor( 80, 80, 80) );

    p->drawRoundRect( boundingRect(), 4, 4 );
}

#include "moc_switchdip.cpp"

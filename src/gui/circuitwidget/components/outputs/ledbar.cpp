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

#include "ledbar.h"
#include "connector.h"
#include "circuit.h"
#include "pin.h"


Component* LedBar::construct( QObject* parent, QString type, QString id )
{ return new LedBar( parent, type, id ); }

LibraryItem* LedBar::libraryItem()
{
    return new LibraryItem(
            tr( "LedBar" ),
            tr( "Outputs" ),
            "ledbar.png",
            "LedBar",
            LedBar::construct);
}

LedBar::LedBar( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
{
    m_area = QRect( -8, -28, 16, 64 );
    m_color = QColor(0,0,0);
    
    setLabelPos(-16,-44, 0);
    
    m_size = 0;
    setSize( 8 );
    
    setRes( 0.6 ); 
}
LedBar::~LedBar(){}

void LedBar::createLeds( int c )
{
    bool initialized = m_size > 0;
    int start = m_size;
    m_size = m_size+c;
    m_led.resize( m_size );
    m_pin.resize( m_size*2 );
    
    for( int i=start; i<m_size; i++ )
    {
        int index = i*2;
        
        /*QString reid = m_id;
        reid.append(QString("-resistor"+QString::number(i)));
        m_resistor[i] = new eResistor( reid.toStdString() );*/
        
        QString ledid = m_id;
        ledid.append(QString("-led"+QString::number(i)));
        m_led[i] = new LedSmd( this, "LEDSMD", ledid, QRectF(0, 0, 4, 4) );
        m_led[i]->setParentItem(this);
        m_led[i]->setPos( 0, -28+2+i*8 );
        //m_led[i]->setEnabled( false );
        m_led[i]->setFlag( QGraphicsItem::ItemIsSelectable, false );
        m_led[i]->setAcceptedMouseButtons(0);
        
        QPoint pinpos = QPoint(-16,-32+8+i*8 );
        Pin* pin = new Pin( 180, pinpos, ledid+"-pinP", 0, this);
        m_led[i]->setEpin( 0, pin );
        m_pin[index] = pin;
        
        pinpos = QPoint( 16,-32+8+i*8 );
        pin = new Pin( 0, pinpos, ledid+"-pinN", 0, this);
        m_led[i]->setEpin( 1, pin );
        m_pin[index+1] = pin;
        
        if( initialized ) 
        {
            m_led[i]->setGrounded( grounded() );
            m_led[i]->setRes( res() );
            m_led[i]->setMaxCurrent( maxCurrent() ); 
            m_led[i]->setThreshold( threshold() );
            m_led[i]->setColor( color() ); 
        }
    }
    //update();
}

void LedBar::deleteLeds( int d )
{
    if( d > m_size ) d = m_size;
    int start = m_size-d;
    
    if( grounded() )
    {
        for( int i=start; i<m_size; i++ ) m_led[i]->setGrounded( false );
    }

    for( int i=start*2; i<m_size*2; i++ )
    {
        Pin* pin = m_pin[i];
        if( pin->isConnected() ) pin->connector()->remove();
        
        delete pin;
    }
    for( int i=start; i<m_size; i++ )  Circuit::self()->removeComp( m_led[i] );
    m_size = m_size-d;
    m_led.resize( m_size );
    m_pin.resize( m_size*2 );
    //Circuit::self()->update();
}

void LedBar::setColor( LedBase::LedColor color ) 
{ 
    for( LedSmd* led : m_led ) led->setColor( color ); 
}

LedBase::LedColor LedBar::color() 
{ 
    return m_led[0]->color(); 
}

int LedBar::size()
{
    return m_size;
}

void LedBar::setSize( int size )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    if( size == 0 ) size = 8;
    
    if     ( size < m_size ) deleteLeds( m_size-size );
    else if( size > m_size ) createLeds( size-m_size );
    
    m_area = QRect( -8, -28, 16, m_size*8 );
    
    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
}

double LedBar::threshold()                     
{ 
    return m_led[0]->threshold(); 
}

void LedBar::setThreshold( double threshold ) 
{ 
    for( int i=0; i<m_size; i++ ) m_led[i]->setThreshold( threshold );
}

double LedBar::maxCurrent()                   
{ 
    return m_led[0]->maxCurrent(); 
}
void LedBar::setMaxCurrent( double current ) 
{ 
    for( int i=0; i<m_size; i++ ) m_led[i]->setMaxCurrent( current ); 
}

double LedBar::res() { return m_led[0]->res(); }

void LedBar::setRes( double resist )
{
    if( resist == 0 ) resist = 1e-14;

    for( int i=0; i<m_size; i++ ) m_led[i]->setRes( resist );
}

bool LedBar::grounded()
{
    if( m_size == 0 ) return false;
    return m_led[0]->grounded();
}

void LedBar::setGrounded( bool grounded )
{
    for( int i=0; i<m_size; i++ ) m_led[i]->setGrounded( grounded );
}

void LedBar::remove()
{
    for( int i=0; i<m_size; i++ ) Circuit::self()->removeComp( m_led[i] );

    Component::remove();
}
void LedBar::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRoundRect( m_area, 4, 4 );
}

#include "moc_ledbar.cpp"

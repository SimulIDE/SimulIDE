/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#include "ws2812.h"
#include "mcucomponent.h"
#include "simulator.h"
#include "circuit.h"
#include "pin.h"


Component* WS2812::construct( QObject* parent, QString type, QString id )
{
        return new WS2812( parent, type, id );
}

LibraryItem* WS2812::libraryItem()
{
    return new LibraryItem(
        tr( "WS2812 Led" ),
        tr( "Outputs" ),
        "ws2812.png",
        "WS2812",
        WS2812::construct );
}

WS2812::WS2812( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eLogicDevice( id.toStdString() )
{
    m_pin.resize( 2 );
    m_ePin.resize( 2 );
    m_rgb.resize( 3 );

    QString pinId = m_id;
    pinId.append(QString("-InPin"));
    QPoint pinPos = QPoint(-4-8,0);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-OutPin"));
    pinPos = QPoint(4+8,0);
    m_pin[1] = new Pin( 0, pinPos, pinId, 1, this);
    m_ePin[1] = m_pin[1];

    eLogicDevice::createClockPin( m_pin[0] );             // Input
    eLogicDevice::createOutput( m_pin[1] );               // Output

    setClockInv( false );                       //Don't Invert Clock pin

    m_cols = 1;
    setRows( 1 );

    Simulator::self()->addToUpdateList( this );
}
WS2812::~WS2812()
{
    Simulator::self()->remFromUpdateList( this );
}

void WS2812::updateStep()
{
    update();
}

void WS2812::initialize()
{
    m_stepsPerus = Simulator::self()->stepsPerus();
    if( McuComponent::self() )
    {
        double freq = McuComponent::self()->freq();
        double simurate = freq*1e6;
        if( Simulator::self()->simuRate() == simurate ) return;

        qDebug() << "WS2812::initialize: Changed Simulation Speed to:" << freq << "MHz";
        Simulator::self()->simuRateChanged( simurate );
    }
}

void WS2812::resetState()
{
    for( int i=0; i<m_leds; i++ ) m_led[i] = QColor( 0, 0, 0 );
    m_lastTime = 0;
    m_data = 0;
    m_word = 0;
    m_byte = 0;
    m_bit  = 7;
}
void WS2812::setVChanged()
{
    // Get Clk to don't miss any clock changes
    int  clkState = eLogicDevice::getClockState();

    uint64_t CircTime = Simulator::self()->circTime();
    uint64_t time = CircTime - m_lastTime;
    m_lastTime = CircTime;

    if( clkState  == Rising )                      // Input Rising edge
    {
        if( time > 50000 )                      // Time > 50uS -> Reset
        {
            m_data = 0;
            m_word = 0;
            m_byte = 0;
            m_bit  = 7;
        }
        else if(( time > 400 )&&( time < 951 ))   // Valid L State Time
        {
            if( m_word >= m_leds )
            {
                eLogicDevice::setOut( 0, true );
            }
            else if( m_newWord )
            {
                if( m_lastHstate  )
                {
                    if( time < 751 )  saveBit( 1 );    // Valid bit = 1
                }
                else if( time > 649 ) saveBit( 0 );    // Valid bit = 0
            }
        }
        m_newWord = true;
    }
    else if( clkState  == Falling )               // Input Falling edge
    {
        if( m_word >= m_leds )
        {
            eLogicDevice::setOut( 0, false );
        }
        else if(( time > 199 )&&( time < 851 ))   // Valid H State Time
        {
            if( time > 500 ) m_lastHstate = true;
            else             m_lastHstate = false;

            if(( m_byte == 2 )&&( m_bit == 0 )) // Low time for last bit can be anything?
            {                                   // so save it here and skip saving
                saveBit( m_lastHstate );        // at next Rising Edge Using m_newWord
                m_newWord = false;
            }
        }
    }
}

void WS2812::saveBit( bool bit )
{
    if( bit ) m_data |=   (1 << m_bit);
    else      m_data &=  ~(1 << m_bit);

    if( --m_bit < 0 )
    {
        m_rgb[ m_byte ] = m_data;

        m_data = 0;
        m_bit  = 7;

        if( ++m_byte > 2 )
        {
            m_led[ m_word ] = QColor( m_rgb[1], m_rgb[0], m_rgb[2] );

            m_byte = 0;

            m_word++;
        }
    }
}


int WS2812::rows()
{
    return m_rows;
}

void WS2812::setRows( int rows )
{
    if( rows == m_rows ) return;
    if( rows < 1 ) rows = 1;
    m_rows = rows;
    updateLeds();
}

int WS2812::cols()
{
    return m_cols;
}

void WS2812::setCols( int cols )
{
    if( cols == m_cols ) return;
    if( cols < 1 ) cols = 1;
    m_cols = cols;
    updateLeds();
}

void WS2812::updateLeds()
{
    m_leds = m_rows*m_cols;
    m_led.resize( m_leds );
    m_area = QRect( -6, -6, m_cols*12, m_rows*12 );
    m_pin[1]->setPos( 12*m_cols, 12*m_rows-12);
    Circuit::self()->update();
}

void WS2812::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush( QColor(Qt::black) );

    p->drawRect( m_area );

    QPointF points[3] = {
    QPointF( -8,-2 ),
    QPointF( -8+3,0 ),
    QPointF( -8, 2 )     };
    p->drawPolygon(points, 3);

    for( int row=0; row<m_rows; row++ )
    {
        for( int col=0; col<m_cols; col++ )
        {
            p->setBrush( m_led[row*m_cols+col] );
            p->drawEllipse( -6+col*12, -6+row*12, 12, 12 );
        }
    }
}

#include "moc_ws2812.cpp"


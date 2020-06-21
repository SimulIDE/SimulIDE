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

#include "sr04.h"
#include "pin.h"
#include "simulator.h"

Component* SR04::construct( QObject* parent, QString type, QString id )
{ return new SR04( parent, type, id ); }

LibraryItem* SR04::libraryItem()
{
    return new LibraryItem(
            tr( "HC-SR04" ),
            tr( "Sensors" ),
            "sr04.png",
            "SR04",
            SR04::construct);
}

SR04::SR04( QObject* parent, QString type, QString id )
    : Component( parent, type, id )
    , eElement( id.toStdString() )
{
    m_area = QRect( -10*8, -4*8, 21*8, 9*8 );
    setLabelPos(-16,-48, 0);
    m_BackGround = ":/sr04.png";
    
    m_pin.resize(5);
    
    QString pinid = id;
    pinid.append(QString("-inpin"));
    QPoint pinpos = QPoint(-11*8,-3*8);
    m_inpin = new Pin( 180, pinpos, pinid, 0, this);
    m_inpin->setLabelText( " In v=m" );
    m_pin[0] = m_inpin;
    
    pinid = id;
    pinid.append(QString("-vccpin"));
    pinpos = QPoint(-8,48);
    Pin* vccPin = new Pin( 270, pinpos, pinid, 0, this);
    vccPin->setLabelText( " Vcc" );
    vccPin->setUnused( true );
    m_pin[1] = vccPin;
    
    pinid = id;
    pinid.append(QString("-gndpin"));
    pinpos = QPoint(16,48);
    Pin* gndPin = new Pin( 270, pinpos, pinid, 0, this);
    gndPin->setLabelText( " Gnd" );
    gndPin->setUnused( true );
    m_pin[2] = gndPin;
    
    pinid = id;
    pinid.append(QString("-trigpin"));
    pinpos = QPoint(0,48);
    m_trigpin = new Pin( 270, pinpos, pinid, 0, this);
    m_trigpin->setLabelText( " Trig" );
    m_pin[3] = m_trigpin;
    
    pinid = id;
    pinid.append(QString("-outpin"));
    pinpos = QPoint(8,48);
    m_echopin = new Pin( 270, pinpos, pinid, 0, this);
    m_echopin->setLabelText( " Echo" );
    m_pin[4] = m_echopin;

    pinid.append(QString("-eSource"));
    m_echo = new eSource( pinid.toStdString(), m_echopin );
    m_echo->setVoltHigh( 5 );
    m_echo->setImp( 40 );
    
    resetState();
}
SR04::~SR04(){ Simulator::self()->remFromSimuClockList( this ); }

void SR04::stamp()
{
    eNode* enode = m_trigpin->getEnode(); // Register for Trigger Pin changes
    if( enode ) enode->addToChangedFast(this);
}

void SR04::resetState()
{
    m_lastStep = Simulator::self()->step();
    m_lastTrig = false;
    m_trigCount = 0;
    m_echouS = 0;
}

void SR04::setVChanged()              // Called when Trigger Pin changes
{
    bool trigState = m_trigpin->getVolt()>2.5;
    
    if( !m_lastTrig && trigState )                 // Rising trigger Pin
    {
        m_lastStep = Simulator::self()->step();
    }
    else if( m_lastTrig && !trigState )                     // Triggered
    {
        uint64_t step = Simulator::self()->step();
        double stepsPerus = Simulator::self()->stepsPerus();
        double time = (double)(step-m_lastStep)/stepsPerus;

        if( time >= 10 )                        // >=10 uS Trigger pulse
        {
            double count = 200*stepsPerus;
            m_trigCount = (int)count;
            
            double us = (m_inpin->getVolt()*2000/0.344+0.5)*stepsPerus;
            m_echouS = us;
            //qDebug() <<m_inpin->getVolt()<< us<<m_echouS;
            
            Simulator::self()->addToSimuClockList( this );
        }
    }
    m_lastTrig = trigState;
}

void SR04::simuClockStep()
{
    if( m_trigCount > 0 ) 
    {
        m_trigCount--;
        
        if( m_trigCount == 0 )                       // Start Echo pulse
        {
            m_echo->setOut( true );
            m_echo->stampOutput();
        }
    }
    else if( m_trigCount == 0 )
    {
        m_echouS--;
        
        if( m_echouS == 0 )                           // Stop Echo pulse
        {
            m_echo->setOut( false );
            m_echo->stampOutput();
            
            Simulator::self()->remFromSimuClockList( this );
        }
    }
}

void SR04::remove()
{
    delete m_echo;
    Component::remove();
}

void SR04::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    
    p->drawRoundedRect( m_area, 2, 2 );

    int ox = m_area.x();
    int oy = m_area.y();

    p->drawPixmap( ox, oy, QPixmap( m_BackGround ));
}

#include "moc_sr04.cpp"

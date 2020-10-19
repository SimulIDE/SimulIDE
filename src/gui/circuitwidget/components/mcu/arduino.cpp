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

#include "arduino.h"
#include "mainwindow.h"
#include "circuit.h"
#include "e-resistor.h"
#include "itemlibrary.h"
#include "utils.h"

#include "hd44780.h"

#include "avr_twi.h"

LibraryItem* Arduino::libraryItem()
{
    return new LibraryItem(
        tr("Arduino"),
        tr("Micro"),   
        "arduinoUnoIcon.png",
        "Arduino",
        Arduino::construct );
}

Component* Arduino::construct( QObject* parent, QString type, QString id )
{ 
    if( m_canCreate ) 
    {
        Arduino* ard = new Arduino( parent, type,  id );
        if( m_error > 0 )
        {
            Circuit::self()->compList()->removeOne( ard );
            ard->deleteLater();
            ard = 0l;
            m_error = 0;
            m_pSelf = 0l;
            m_canCreate = true;
        }
        return ard;
    }
    MessageBoxNB( tr("Error")
                , tr("Only 1 Mcu allowed\n to be in the Circuit.") );

    return 0l;
}

Arduino::Arduino( QObject* parent, QString type, QString id )
       : AvrCompBase( parent, type, id )
{
    m_pSelf = this;

    m_processor = AvrProcessor::self();
    
    setLabelPos( 100,-21, 0); // X, Y, Rot
    
    initChip();
    if( m_error == 0 )
    {
        initBoard();
        setFreq( 16 );
        //initBootloader();
        
        qDebug() <<"     ..."<<m_id<<"OK\n";
    }
    else
    {
        qDebug() <<"     ..."<<m_id<<"Error!!!\n";
    }
    setTransformOriginPoint( togrid( boundingRect().center()) );
}
Arduino::~Arduino() 
{
}

void Arduino::remove()
{
    m_pb5Pin->setEnode( 0l );
    ePin* ledPin0 = m_boardLed->getEpin(0);
    ledPin0->setEnode( 0l );
    delete ledPin0;

    ePin* ledPin1 = m_boardLed->getEpin(1);
    ledPin1->setEnode( 0l );
    delete ledPin1;

    delete m_groundEnode;
    delete m_boardLedBuffer;

    McuComponent::remove();
    if( m_boardLedEnode ) Simulator::self()->remFromEnodeList( m_boardLedEnode, true );
    Simulator::self()->remFromEnodeList( m_bufferEnode, true );
    Simulator::self()->remFromUpdateList( m_boardLed );
}

void Arduino::attach()
{
    eNode* enod = m_pb5Pin->getEnode();
    
    if( !enod )                        // Not connected: Create boardLed eNode
    {
        m_boardLedEnode = new eNode( m_id+"-boardLedeNode" );
        enod = m_boardLedEnode;
        m_pb5Pin->setEnode( m_boardLedEnode );
    }
    else if( enod != m_boardLedEnode ) // Connected to external eNode: Delete boardLed eNode
    {
        m_boardLedEnode = enod;
    }
    else return;                       // Already connected to boardLed eNode: Do nothing

    m_boardLedBuffer->getEpin( "input0" )->setEnode(enod);
}

void Arduino::initBoard()
{
    // Create Led eNodes
    m_groundEnode = new eNode( m_id+"-Gnod");
    m_groundEnode->setNodeNumber(0);
    Simulator::self()->remFromEnodeList( m_groundEnode, false );
    m_bufferEnode = new eNode( m_id+"-Lnod");

    // Create Led Buffer
    m_boardLedBuffer = new eGate( (m_id+"boardLedBuffer").toStdString(), 0 );
    m_boardLedBuffer->createPins( 1, 1 );
    m_boardLedBuffer->getEpin( "output0" )->setEnode( m_bufferEnode );

    // Create board led
    m_boardLed = new LedSmd( this, "LEDSMD", m_id+"boardled", QRectF(0, 0, 4, 3) );
    m_boardLed->setNumEpins(2);
    m_boardLed->setParentItem(this);
    m_boardLed->setEnabled(false);
    m_boardLed->setMaxCurrent( 0.003 );
    m_boardLed->setRes( 1000 );
    Circuit::self()->compList()->removeOne( m_boardLed );

    m_boardLed->getEpin(0)->setEnode( m_bufferEnode );
    m_boardLed->getEpin(1)->setEnode( m_groundEnode ); // Connect board led to ground

    if( objectName().contains("Mega") ) m_boardLed->setPos( 35+12, 125+105 );
    else                                m_boardLed->setPos( 35, 125 );

    for( int i=0; i<m_numpins; i++ )                      // Create Pins
    {
        McuComponentPin* mcuPin = m_pinList.at(i);

        if     ( mcuPin->angle() == 0   ) mcuPin->move(-16, 0 );
        else if( mcuPin->angle() == 180 ) mcuPin->move( 16, 0 );
        else if( mcuPin->angle() == 90  ) mcuPin->move( 0, 32 );
        else                              mcuPin->move( 0,-320 );

        Pin* pin = mcuPin->pin();
        pin->setLength(0);
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false );
        
        QString pinId = pin->pinId();
        QString type  = mcuPin->ptype();
        if     ( pinId.contains( "GND" ) )                   // Gnd Pins
        {    
            mcuPin->setImp( 0.01 ); 
        }
        else if( pinId.contains( "V3V" ) )                  // 3.3V Pins
        {
            mcuPin->setImp( 0.1 );
            mcuPin->setVoltHigh( 3.3 );
            mcuPin->setVoltLow( 3.3 );
        }
        else if( pinId.contains( "V5V" ) )                    // 5V Pins
        {
            mcuPin->setImp( 0.1 );
            mcuPin->setVoltHigh( 5 );
            mcuPin->setVoltLow( 5 );
        }
        else if( pinId.contains( "Vin" ) )          // Vin Pins ( 12 V )
        {
            mcuPin->setImp( 0.1 );
            mcuPin->setVoltHigh( 12 );
            mcuPin->setVoltLow( 12 );
        }
        else if( type.contains( "led" ) )                      // Pin 13
        {
            //pin->setEnode( m_boardLedEnode );
            m_pb5Pin = pin;
        }
        else if( pinId.toUpper().contains( "RST" ) )       // Reset Pins
        {
            mcuPin->setImp( 20000 );
            mcuPin->setVoltHigh( 5 );
            mcuPin->setVoltLow( 5 );
        }
    }
}

void Arduino::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    int ox = m_area.x();
    int oy = m_area.y();

    p->drawPixmap( ox, oy, QPixmap( m_BackGround ));
}

#include "moc_arduino.cpp"

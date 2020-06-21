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

#include "i2cram.h"
#include "pin.h"

static const char* I2CRam_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Control Code"),
    QT_TRANSLATE_NOOP("App::Property","Size bytes")
};

Component* I2CRam::construct( QObject* parent, QString type, QString id )
{
    return new I2CRam( parent, type, id );
}

LibraryItem* I2CRam::libraryItem()
{
    return new LibraryItem(
        tr( "I2C Ram" ),
        tr( "Logic/Memory" ),
        "2to3.png",
        "I2CRam",
        I2CRam::construct );
}

I2CRam::I2CRam( QObject* parent, QString type, QString id )
      : LogicComponent( parent, type, id )
      , eI2C( id.toStdString() )
      , MemData()
{
    Q_UNUSED( I2CRam_properties );
    
    m_width  = 4;
    m_height = 4;
    
    QStringList pinList;                              // Create Pin List

    pinList // Inputs:
            << "IL01 SDA"//type: Input, side: Left, pos: 01, label: "SDA"
            << "IL03 SCL"
            << "IR01 A0"
            << "IR02 A1"
            << "IR03 A2"
            // Outputs:
            ;
    init( pinList );                   // Create Pins Defined in pinList
    
    eLogicDevice::createInput( m_inPin[0] );                // Input SDA
    eLogicDevice::createClockPin( m_inPin[1] );             // Input SCL
    
    eLogicDevice::createInput( m_inPin[2] );                 // Input A0
    eLogicDevice::createInput( m_inPin[3] );                 // Input A1
    eLogicDevice::createInput( m_inPin[4] );                 // Input A2
    
    m_cCode = 0b01010000;
    m_size  = 65536;
    m_ram.resize( m_size );

    m_persistent = false;
    
    resetState();
}
I2CRam::~I2CRam(){}

void I2CRam::stamp()                     // Called at Simulation Start
{
    eI2C::stamp();
    
    for( int i=2; i<5; i++ )                  // Initialize address pins
    {
        eNode* enode =  m_inPin[i]->getEnode();
        if( enode ) enode->addToChangedFast( this );
    }
}

void I2CRam::resetState()
{
    eI2C::resetState();
    
    m_addrPtr = 0;
    m_phase = 3;
}

void I2CRam::setVChanged()             // Some Pin Changed State, Manage it
{
    bool A0 = eLogicDevice::getInputState( 1 );
    bool A1 = eLogicDevice::getInputState( 2 );
    bool A2 = eLogicDevice::getInputState( 3 );
    
    int  address = m_cCode;
    if( A0 ) address += 1;
    if( A1 ) address += 2;
    if( A2 ) address += 4;
    
    m_address = address;
    
    eI2C::setVChanged();                               // Run I2C Engine

    //qDebug() <<"I2CRam::setVChanged " << m_state ;
    
    if( m_state == I2C_STARTED )
    {
        if( m_size > 256 ) m_phase = 0;
        else               m_phase = 1;
    }
    if( m_state == I2C_STOPPED ) m_phase = 3;
}

void I2CRam::readByte() // Write to RAM
{
    if( m_phase == 0 )
    {
        m_phase++;
        m_addrPtr = m_rxReg<<8;
    }
    else if( m_phase == 1 )
    {
        m_phase++;

        if( m_size > 256 ) m_addrPtr += m_rxReg;
        else               m_addrPtr  = m_rxReg;
        
        //while( m_addrPtr >= m_size ) m_addrPtr -= m_size;
    }
    else
    {
        while( m_addrPtr >= m_size ) m_addrPtr -= m_size;
        //qDebug() << "I2CRam::readByte Address:"<<m_addrPtr<<" Value"<< m_rxReg;
        m_ram[ m_addrPtr ] = m_rxReg;
        m_addrPtr++;
        
        if( m_addrPtr >= m_size ) m_addrPtr = 0;
    }
    eI2C::readByte();
}

void I2CRam::writeByte() // Read from RAM
{
    while( m_addrPtr >= m_size ) m_addrPtr -= m_size;
    //qDebug() << "I2CRam::writeByte Address:"<<m_addrPtr<<" Value"<< m_txReg;
    m_txReg = m_ram[ m_addrPtr ];

    m_addrPtr++;
    
    if( m_addrPtr >= m_size ) m_addrPtr = 0;

    eI2C::writeByte();
}

void I2CRam::setMem( QVector<int> m )
{
    if( m.size() == 1 ) return;       // Avoid loading data if not saved
    m_ram = m;
}

QVector<int> I2CRam::mem()
{
    if( !m_persistent )
    {
        QVector<int> null;
        return null;
    }
    //qDebug() << m_ram.size() <<"Ram:\n" << m_ram;
    return m_ram;
}

int I2CRam::cCode()
{
    return m_cCode;
}

void I2CRam::setCcode( int code )
{
    m_cCode = code;
}

int I2CRam::rSize()
{
    return m_size;
}

void I2CRam::setRSize( int size )
{
    if( size > 65536 ) size = 65536;
    if( size < 1 ) size = 1;
    m_size = size;
    //m_ram.resize( size );
}

bool I2CRam::persistent()
{
    return m_persistent;
}

void I2CRam::setPersistent( bool p )
{
    m_persistent = p;
}

void I2CRam::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
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

void I2CRam::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load data") );
    connect( loadAction, SIGNAL(triggered()), this, SLOT(loadData()) );

    QAction* saveAction = menu->addAction(QIcon(":/save.png"), tr("Save data") );
    connect( saveAction, SIGNAL(triggered()), this, SLOT(saveData()) );

    menu->addSeparator();

    Component::contextMenu( event, menu );
}

void I2CRam::loadData()
{
    MemData::loadData( &m_ram );
}

void I2CRam::saveData()
{
    MemData::saveData( m_ram );
}
#include "moc_i2cram.cpp"

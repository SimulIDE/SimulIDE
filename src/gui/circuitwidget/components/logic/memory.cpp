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

#include <sstream>

#include "memory.h"
#include "circuit.h"
#include "pin.h"
#include "utils.h"

static const char* Memory_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Data Bits"),
    QT_TRANSLATE_NOOP("App::Property","Persistent")
};

Component* Memory::construct( QObject* parent, QString type, QString id )
{
    return new Memory( parent, type, id );
}

LibraryItem* Memory::libraryItem()
{
    return new LibraryItem(
        tr( "Ram/Rom" ),
        tr( "Logic/Memory" ),
        "2to3g.png",
        "Memory",
        Memory::construct );
}

Memory::Memory( QObject* parent, QString type, QString id )
      : LogicComponent( parent, type, id )
      , eMemory( id.toStdString() )
      , MemData()
{
    Q_UNUSED( Memory_properties );

    m_width  = 4;
    m_height = 11;
    
    m_WePin = new Pin( 180, QPoint( 0,0 ), m_id+"-Pin-We", 0, this );
    m_WePin->setLabelText( " WE" );
    m_WePin->setLabelColor( QColor( 0, 0, 0 ) );
    
    m_CsPin = new Pin(  0, QPoint( 0,0 ), m_id+"-Pin-Cs", 0, this );
    m_CsPin->setLabelText( "CS " );
    m_CsPin->setLabelColor( QColor( 0, 0, 0 ) );
    
    m_outEnPin = new Pin( 180, QPoint( 0,0 ), m_id+"-Pin-outEnable"  , 0, this );
    m_outEnPin->setLabelText( " OE" );
    m_outEnPin->setLabelColor( QColor( 0, 0, 0 ) );
    
    eLogicDevice::createInput( m_WePin );                          // WE
    eLogicDevice::createInput( m_CsPin );                          // CS
    eLogicDevice::createOutEnablePin( m_outEnPin );                // OE
    
    for( int i=0; i<2; i++ ) m_input[i]->setInverted( true ); // Invert control pins
    
    m_addrBits = 0;
    m_dataBits = 0;
    setAddrBits( 8 );
    setDataBits( 8 );
}
Memory::~Memory(){}

void Memory::updatePins()
{
    int h = m_addrBits+1;
    if( m_dataBits > h ) h = m_dataBits;
    
    m_height = h+2;
    int origY = -(m_height/2)*8;
    
    for( int i=0; i<m_addrBits; i++ )
    {
        m_inPin[i]->setPos( QPoint(-24,origY+8+i*8 ) );
        m_inPin[i]->setLabelPos();
        m_inPin[i]->isMoved();
    }
    
    for( int i=0; i<m_dataBits; i++ )
    {
        m_outPin[i]->setPos( QPoint(24,origY+8+i*8 ) ); 
        m_outPin[i]->setLabelPos();
        m_outPin[i]->isMoved();
    }
    
    m_WePin->setPos( QPoint(-24,origY+h*8 ) );          // WE
    m_WePin->isMoved();
    m_WePin->setLabelPos();
    
    m_CsPin->setPos( QPoint( 24,origY+8+h*8 ) );        // CS
    m_CsPin->isMoved();
    m_CsPin->setLabelPos();
    
    m_outEnPin->setPos( QPoint(-24,origY+8+h*8 ) );        // OE
    m_outEnPin->isMoved();
    m_outEnPin->setLabelPos();
    
    m_area   = QRect( -(m_width/2)*8, origY, m_width*8, m_height*8 );
}

void Memory::setAddrBits( int bits )
{
    if( bits == m_addrBits ) return;
    if( bits == 0 ) bits = 8;
    if( bits > 18 ) bits = 18;
    
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    if     ( bits < m_addrBits ) deleteAddrBits( m_addrBits-bits );
    else if( bits > m_addrBits ) createAddrBits( bits-m_addrBits );
    
    eMemory::setAddrBits( bits );

    updatePins();
    
    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
}

void Memory::createAddrBits( int bits )
{
    int chans = m_addrBits + bits;
    
    int origY = -(m_height/2)*8;
    
    m_inPin.resize( chans );
    m_numInPins = chans;
    
    for( int i=m_addrBits; i<chans; i++ )
    {
        QString number = QString::number(i);

        m_inPin[i] = new Pin( 180, QPoint(-24,origY+8+i*8 ), m_id+"-in"+number, i, this );
        m_inPin[i]->setLabelText( " A"+number );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        eLogicDevice::createInput( m_inPin[i] );
    }
}

void Memory::deleteAddrBits( int bits )
{
    eLogicDevice::deleteInputs( bits );
    LogicComponent::deleteInputs( bits );
}

void Memory::setDataBits( int bits )
{
    if( bits == m_dataBits ) return;
    if( bits == 0 ) bits = 8;
    if( bits > 32 ) bits = 32;
    
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    if     ( bits < m_dataBits ) deleteDataBits( m_dataBits-bits );
    else if( bits > m_dataBits ) createDataBits( bits-m_dataBits );
    
    m_dataBits = bits;
    
    updatePins();
    
    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
}

void Memory::createDataBits( int bits )
{
    int chans = m_dataBits + bits;
    
    int origY = -(m_height/2)*8;
    
    m_outPin.resize( chans );
    m_numOutPins = chans;
    
    for( int i=m_dataBits; i<chans; i++ )
    {
        QString number = QString::number(i);
        
        m_outPin[i] = new Pin( 0, QPoint(24,origY+8+i*8 ), m_id+"-out"+number, i, this );
        m_outPin[i]->setLabelText( "D"+number+" " );
        m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );
        eLogicDevice::createOutput( m_outPin[i] );
    }
}

void Memory::deleteDataBits( int bits )
{
    eLogicDevice::deleteOutputs( bits );
    LogicComponent::deleteOutputs( bits );
}

void Memory::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
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

void Memory::contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu )
{
    QAction* loadAction = menu->addAction( QIcon(":/load.png"),tr("Load data") );
    connect( loadAction, SIGNAL(triggered()), this, SLOT(loadData()) );

    QAction* saveAction = menu->addAction(QIcon(":/save.png"), tr("Save data") );
    connect( saveAction, SIGNAL(triggered()), this, SLOT(saveData()) );

    menu->addSeparator();

    Component::contextMenu( event, menu );
}

void Memory::loadData()
{
    MemData::loadData( &m_ram, false, m_dataBits );
}

void Memory::saveData()
{
    MemData::saveData( m_ram, m_dataBits );
}

void Memory::remove()
{
    if( m_CsPin->isConnected() )    m_CsPin->connector()->remove();
    if( m_WePin->isConnected() )    m_WePin->connector()->remove();
    if( m_outEnPin->isConnected() ) m_outEnPin->connector()->remove();
    
    LogicComponent::remove();
}

#include "moc_memory.cpp"

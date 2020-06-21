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

#include "connector.h"
#include "circuit.h"
#include "bus.h"

static const char* Bus_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Num Bits"),
    QT_TRANSLATE_NOOP("App::Property","Start Bit")
};

Component* Bus::construct( QObject* parent, QString type, QString id )
{
    return new Bus( parent, type, id );
}

LibraryItem* Bus::libraryItem()
{
    return new LibraryItem(
        tr( "Bus" ),
        tr( "Logic/Other Logic" ),
        "outbus.png",
        "Bus",
        Bus::construct );
}

Bus::Bus( QObject* parent, QString type, QString id )
   : Component( parent, type, id )
   , eBus( id.toStdString() )
{
    Q_UNUSED( Bus_properties );

    m_busPin1 = new Pin( 270, QPoint( 0, 0 ), m_id+"-busPinI", 1, this );
    m_busPin1->setLength( 1 );
    m_busPin1->setFlag( QGraphicsItem::ItemStacksBehindParent, false );


    setNumLines( 8 );                           // Create Input Pins

    m_busPin0 = new Pin( 90, QPoint( 0, 0 ), m_id+"-ePin0", 1, this );
    m_busPin0->setLength( 1 );
    m_busPin0->setFlag( QGraphicsItem::ItemStacksBehindParent, false );

    m_busPin1->setIsBus( true );
    m_busPin0->setIsBus( true );
    m_pin[0]  = m_busPin0;
    m_ePin[0] = m_busPin0;

    //m_pin[ m_numLines+1 ] = m_busPin1;
}
Bus::~Bus(){
}

void Bus::setNumLines( int lines )
{
    if( lines == m_numLines ) return;
    if( lines < 1 ) return;

    for( int i=1; i<=m_numLines; i++ )
    {
        if( m_pin[i]->isConnected() ) m_pin[i]->connector()->remove();
        if( m_pin[i]->scene() ) Circuit::self()->removeItem( m_pin[i] );
        delete m_pin[i];
    }
    m_numLines = lines;

    m_pin.resize( lines+2 );
    m_ePin.resize( lines+2 );
    
    for( int i=1; i<=lines; i++ )
    {
        QString pinId = m_id+"-ePin"+QString::number(i);
        Pin* pin = new Pin( 180, QPoint(-8, -8*lines+i*8 ), pinId, i, this );

        pin->setFontSize( 4 );
        pin->setLabelColor( QColor( 0, 0, 0 ) );
        pin->setLabelText( " "+QString::number( m_startBit+i-1 ) );
        m_pin[i]  = pin;
        m_ePin[i] = pin;
    }
    m_busPin1->setPos( QPoint( 0 ,-lines*8+8 ) );
    m_busPin1->isMoved();
    m_busPin1->setLabelPos();
    m_pin[ lines+1 ]  = m_busPin1;
    m_ePin[ lines+1 ] = m_busPin1;

    m_height = lines-1;
    m_area = QRect( -3,-m_height*8-2, 5, m_height*8+4 );
    Circuit::self()->update();
}

void Bus::setStartBit( int bit )
{
    eBus::setStartBit( bit );

    for( int i=1; i<=m_numLines; i++ )
    {
        m_pin[i]->setLabelText( " "+QString::number( m_startBit+i-1 ) );
    }
}

void Bus::initialize()
{
    if( !m_busPin0->isConnected() && !m_busPin1->isConnected() ) return;
    
    //qDebug() << "\nBus::initialize()"<< m_id << m_numLines;

    eNode* busEnode = m_busPin0->getEnode();
    if( !busEnode ) busEnode = m_busPin1->getEnode();
    //if( !busEnode ) return;

    for( int i=1; i<=m_numLines; i++ )
    {
        if( !m_pin[i]->isConnected() ) continue;
        //eNode* enode = m_pin[i]->getEnode();
        //if( !enode )
        eNode* enode = new eNode( m_id+"eNode"+QString::number( i ) );
        Pin* pin = m_pin[i];
        pin->registerPinsW( enode );

        if( busEnode )
        {
            QList<ePin*> epins = enode->getEpins();
            //qDebug() << "Registering pins line"<< i << epins;
            busEnode->addBusPinList( epins, m_startBit+i-1 );
            //for( ePin* epin : epins )epin->setEnode( 0l );
        }
    }
}

void Bus::inStateChanged( int msg )
{
    //qDebug() << "Bus::inStateChanged()"<< m_id << m_numLines;

    if( m_busPin0->isConnected() || m_busPin1->isConnected() )
    {
        eNode* enode = new eNode( m_id+"busNode" );
        enode->setIsBus( true );
        registerPins( enode );
        return;
    }
    if( msg == 3 ) // Called by m_busPin When disconnected
    {
        //qDebug() << "Bus::inStateChanged()" << m_numLines;

        for( int i=1; i<=m_numLines; i++ )
        {
            if( !m_pin[i]->isConnected() ) continue;

            eNode* enode = new eNode( m_id+"eNode"+QString::number( i ) );
            Pin* pin = m_pin[i];
            pin->registerPinsW( enode );
        }
    }
}

void Bus::registerPins( eNode* enode )
{
    if( !enode->isBus() ) return;

    if( m_busPin0->isConnected() ) m_busPin0->registerPinsW( enode );
    if( m_busPin1->isConnected() ) m_busPin1->registerPinsW( enode );
}

void Bus::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawRect( QRect( 0, -m_height*8, 0, m_height*8 ) );
              //QRect( -2, -m_height*8-4, 2, m_height*8+8 );
}

#include "moc_bus.cpp"

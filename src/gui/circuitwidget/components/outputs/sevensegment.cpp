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

#include "sevensegment.h"
#include "itemlibrary.h"
#include "connector.h"
#include "circuit.h"
#include "pin.h"

static const char* SevenSegment_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","NumDisplays"),
    QT_TRANSLATE_NOOP("App::Property","CommonCathode"),
    QT_TRANSLATE_NOOP("App::Property","Vertical Pins")
};

Component* SevenSegment::construct( QObject* parent, QString type, QString id )
{
    return new SevenSegment( parent, type, id );
}

LibraryItem* SevenSegment::libraryItem()
{
    return new LibraryItem(
        tr( "7 Segment" ),
        tr( "Outputs" ),
        "seven_segment.png",
        "Seven Segment",
        SevenSegment::construct );
}

SevenSegment::SevenSegment( QObject* parent, QString type, QString id )
            : Component( parent, type, id )
            , eElement( id.toStdString() )
{
    Q_UNUSED( SevenSegment_properties );

    setLabelPos( 20,-44, 0 );

    m_color = QColor(0,0,0);
    m_ledColor = LedBase::yellow;
    m_commonCathode = true;
    m_verticalPins  = false;
    m_numDisplays = 0;
    m_threshold  = 2.4;
    m_maxCurrent = 0.02;
    m_resistance = 1;
    m_area = QRect( -16, -24-1, 32, 48+2 );

    m_ePin.resize(8);
    m_pin.resize(8);

    QString nodid;
    QString pinid;

    // Create Pins & eNodes for 7 segments
    for( int i=0; i<7; i++ )
    {
        pinid = QString( 97+i ); // a..g

        nodid = m_id;
        nodid.append(QString("-pin_")).append( pinid );
        m_pin[i] = new Pin( 180, QPoint( -16-8, -24+i*8 ), nodid, 0, this );
        m_ePin[i] = m_pin[i];
    }
    // Pin dot
    nodid = m_id;
    nodid.append(QString("-pin_dot"));
    m_pin[7] = new Pin( 270, QPoint( -8, 24+8 ), nodid, 0, this );
    m_ePin[7] = m_pin[7];

    setNumDisplays(1);
}
SevenSegment::~SevenSegment() { }

int SevenSegment::numDisplays()
{
    return m_numDisplays;
}

void SevenSegment::setColor( LedBase::LedColor color ) 
{ 
    m_ledColor = color;
    
    for( LedSmd* segment : m_segment ) segment->setColor( color ); 
}

LedBase::LedColor SevenSegment::color() 
{ 
    return m_ledColor; 
}

void SevenSegment::setNumDisplays( int displays )
{
    if( displays < 1 ) displays = 1;
    if( displays == m_numDisplays ) return;

    if( m_verticalPins ) m_area = QRect( -18, -24-1, displays*32+4, 48+2 );
    else                 m_area = QRect( -16, -24-1, displays*32, 48+2 );

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    if( displays > m_numDisplays )
    {
        resizeData( displays );
        for( int i=m_numDisplays; i<displays; i++ ) createDisplay( i );
    }
    else
    {
        for( int i=displays; i<m_numDisplays; i++ ) deleteDisplay( i );
        resizeData( displays );
    }
    m_numDisplays = displays;
    setResistance( m_resistance );
    setThreshold( m_threshold );
    setMaxCurrent( m_maxCurrent );

    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
}

void SevenSegment::resizeData( int displays )
{
    m_commonPin.resize( displays );
    m_cathodePin.resize( displays*8 );
    m_anodePin.resize( displays*8 );
    m_segment.resize( displays*8 );
}

bool SevenSegment::isCommonCathode()
{
    return m_commonCathode;
}

void SevenSegment::setCommonCathode( bool isCommonCathode )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();

    m_commonCathode = isCommonCathode;

    if( pauseSim ) Simulator::self()->runContinuous();
}

bool SevenSegment::verticalPins()
{
    return m_verticalPins;
}

void SevenSegment::setVerticalPins( bool v )
{
    if( v == m_verticalPins ) return;
    m_verticalPins = v;
    
    if( v )
    {
        for( int i=0; i<5; i++ ) 
        {
            m_pin[i]->setPos( -16+8*i, -24-8 );
            m_pin[i]->setRotation( 90 );
        }
        for( int i=5; i<8; i++ ) 
        {
            m_pin[i]->setPos( -16+8*(i-5), 24+8 );
            m_pin[i]->setRotation( -90 );
        }
        m_area = QRect( -18, -24-1, 32*m_numDisplays+4, 48+2 );
    }
    else
    {
        for( int i=0; i<7; i++ )
        {
            m_pin[i]->setPos( -16-8, -24+i*8 );
            m_pin[i]->setRotation( 0 );
        }
        m_pin[7]->setPos( -8, 24+8 );
        m_pin[7]->setRotation( -90 );
        m_area = QRect( -16, -24-1, 32*m_numDisplays, 48+2 );
    }
    
    for( int i=0; i<8; i++ ) m_pin[i]->isMoved();
    Circuit::self()->update();
}
        
void SevenSegment::setResistance( double res )
{
    if( res < 1e-6 ) res = 1;
    m_resistance = res;
    
    for( uint i=0; i<m_segment.size(); i++ )
    {
        m_segment[i]->setRes( res );
    }
}

double SevenSegment::threshold()
{
    return m_threshold;
}

void SevenSegment::setThreshold( double threshold )
{
    if( threshold < 1e-6 ) threshold = 2.4;
    m_threshold = threshold;
    
    for( uint i=0; i<m_segment.size(); i++ )
    {
        m_segment[i]->setThreshold( threshold );
    }
}

double SevenSegment::maxCurrent()
{
    return m_maxCurrent;
}

void SevenSegment::setMaxCurrent( double current )
{
    if( current < 1e-6 ) current = 0.02;
    m_maxCurrent = current;
    
    for( uint i=0; i<m_segment.size(); i++ )
    {
        m_segment[i]->setMaxCurrent( current );
    }
}

void SevenSegment::attach()
{
    for( int i=0; i<8; i++ ) m_enode[i] = m_ePin[i]->getEnode(); // Get eNode of pin i

    for( int i=0; i<m_numDisplays; i++ )
    {
        eNode* commonEnode = m_commonPin[i]->getEnode();     // Get eNode of common

        int pin;
        if( m_commonCathode )
        {
            for( int j=0; j<8; j++ )
            {
                pin = i*8+j;
                m_cathodePin[pin]->setEnode( commonEnode );
                m_anodePin[pin]->setEnode( m_enode[j] );
            }
        }
        else
        {
            for( int j=0; j<8; j++ )
            {
                pin = i*8+j;
                m_anodePin[pin]->setEnode( commonEnode );
                m_cathodePin[pin]->setEnode( m_enode[j] );
            }
        }
    }
}

void SevenSegment::deleteDisplay( int dispNumber )
{
    Pin* pin = static_cast<Pin*>(m_commonPin[dispNumber]);
    if( pin->isConnected() ) pin->connector()->remove();
    pin->reset();
    delete pin;

    for( int i=0; i<8; i++ ) Circuit::self()->removeComp( m_segment[dispNumber*8+i] );
}

void SevenSegment::createDisplay( int dispNumber )
{
    int x = 32*dispNumber;
    QString nodid;
    QString pinid;
    //LedSmd* segment[8];

    // Pin common
    nodid = m_id;
    nodid.append(QString("-pin_common")).append( QString( 97+dispNumber ) );
    m_commonPin[dispNumber] = new Pin( 270, QPoint( x+8, 24+8 ), nodid, 0, this );

    // Create segments
    for( int i=0; i<8; i++ )
    {
        nodid = m_id;
        pinid = QString( 97+i );
        nodid.append(QString("-led_")).append( pinid );
        LedSmd* lsmd;
        if( i<7 ) lsmd = new LedSmd( this, "LEDSMD", nodid, QRectF(0, 0, 13.5, 1.5) ); // Segment
        else      lsmd = new LedSmd( this, "LEDSMD", nodid, QRectF(0, 0, 1.5, 1.5) );  // Point
        lsmd->setParentItem(this);
        //lsmd->setEnabled(false);
        lsmd->setFlag( QGraphicsItem::ItemIsSelectable, false );
        lsmd->setAcceptedMouseButtons(0);
        lsmd->setNumEpins(2);
        lsmd->setMaxCurrent( 0.02 );

        m_anodePin[dispNumber*8+i]= lsmd->getEpin(0);
        m_cathodePin[dispNumber*8+i] = lsmd->getEpin(1);

        m_segment[dispNumber*8+i] = lsmd;
    }
    m_segment[dispNumber*8+0]->setPos( x-5, -20 );
    m_segment[dispNumber*8+1]->setPos( x+11.5, -16 );
    m_segment[dispNumber*8+1]->setRotation(96);
    m_segment[dispNumber*8+2]->setPos( x+10, 3 );
    m_segment[dispNumber*8+2]->setRotation(96);
    m_segment[dispNumber*8+3]->setPos( x-8, 19 );
    m_segment[dispNumber*8+4]->setPos( x-9, 3 );
    m_segment[dispNumber*8+4]->setRotation(96);
    m_segment[dispNumber*8+5]->setPos( x-7.5, -16 );
    m_segment[dispNumber*8+5]->setRotation(96);
    m_segment[dispNumber*8+6]->setPos( x-6.5, 0 );
    m_segment[dispNumber*8+7]->setPos( x+12, 19 );
}

void SevenSegment::remove()
{
    for( int i=0; i<m_numDisplays; i++ ) deleteDisplay( i );

    Component::remove();
}

void SevenSegment::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED(option); Q_UNUSED(widget);

    Component::paint( p, option, widget );

    p->drawRect( boundingRect() );
}

#include "moc_sevensegment.cpp"


/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "sevensegment_bcd.h"
#include "simulator.h"
#include "connector.h"


Component* SevenSegmentBCD::construct( QObject* parent, QString type, QString id )
{
    return new SevenSegmentBCD( parent, type, id );
}

LibraryItem* SevenSegmentBCD::libraryItem()
{
    return new LibraryItem(
        tr( "7 Seg BCD" ),
        tr( "Logic/Other Logic" ),
        "7segbcd.png",
        "7-Seg BCD",
        SevenSegmentBCD::construct );
}

SevenSegmentBCD::SevenSegmentBCD( QObject* parent, QString type, QString id )
               : LogicComponent( parent, type, id )
               , eBcdTo7S( id.toStdString() )
{
    m_width  = 4;
    m_height = 6;

    QStringList pinList;

    pinList // Inputs:
            << "ID04  "
            << "ID03  "
            << "ID02  "
            << "ID01  "
            ;
    init( pinList );

    for( int i=0; i<m_numInPins; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    Simulator::self()->addToUpdateList( this );
    
    resetState();
}
SevenSegmentBCD::~SevenSegmentBCD(){}

void SevenSegmentBCD::resetState()
{
    for( int i=0; i<7; i++ ) m_outValue[i] = false;
}

void SevenSegmentBCD::stamp()
{
    eBcdTo7S::stamp();

    m_outValue[0] = true;
    m_outValue[1] = true;
    m_outValue[2] = true;
    m_outValue[3] = true;
    m_outValue[4] = true;
    m_outValue[5] = true;
    m_outValue[6] = false;
}

void SevenSegmentBCD::updateStep()
{
    if( m_changed )
    {
        update();
        m_changed = false;
    }
}

void SevenSegmentBCD::remove()
{
    Simulator::self()->remFromUpdateList( this );
    LogicComponent::remove();
}

void SevenSegmentBCD::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Q_UNUSED(option); Q_UNUSED(widget);

    QPen pen;
    pen.setWidth(3);
    pen.setCapStyle(Qt::RoundCap);

    p->setPen(pen);
    p->setBrush( QColor( 30, 30, 30 ) );
    p->drawRect( m_area );

    const int mg =  6;// Margin around number
    const int ds =  1; // "Slope"
    const int tk =  4; // Line thick
    const int x1 =  m_area.x()+mg;
    const int x2 = -m_area.x()-mg;
    const int y1 =  m_area.y()+mg;
    const int y2 = -m_area.y()-mg;


     pen.setWidth(tk);
     pen.setColor( QColor( 250, 250, 100));
     p->setPen(pen);

     if( m_outValue[0]) p->drawLine( x1+tk+ds, y1,    x2-tk+ds, y1    );
     if( m_outValue[1]) p->drawLine( x2+ds,    y1+tk, x2,      -tk    );
     if( m_outValue[2]) p->drawLine( x2,       tk,    x2-ds,    y2-tk );
     if( m_outValue[3]) p->drawLine( x2-tk-ds, y2,    x1+tk-ds, y2    );
     if( m_outValue[4]) p->drawLine( x1-ds,    y2-tk, x1,       tk    );
     if( m_outValue[5]) p->drawLine( x1,      -tk,    x1+ds,    y1+tk );
     if( m_outValue[6]) p->drawLine( x1+tk,    0,     x2-tk,    0     );

     /*if( m_point )
     {
         p->setPen( Qt::NoPen );
         p->setBrush( QColor( 250, 250, 100) );
         p->drawPie( x2+ds, y2-ds, tk, tk, 0, 16*360 );
         // Decimal pointn  p->drawPie( x2+ds, y3-ds, 6, 6, 0, 16*360 );
     }*/
}

#include "moc_sevensegment_bcd.cpp"

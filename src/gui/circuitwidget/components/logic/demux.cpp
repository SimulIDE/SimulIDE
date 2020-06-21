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

#include "demux.h"


Component* Demux::construct( QObject* parent, QString type, QString id )
{
        return new Demux( parent, type, id );
}

LibraryItem* Demux::libraryItem()
{
    return new LibraryItem(
        tr( "Demux" ),
        tr( "Logic/Converters" ),
        "demux.png",
        "Demux",
        Demux::construct );
}

Demux::Demux( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id ), eDemux( id.toStdString() )
{
    m_width  = 4;
    m_height = 10;

    QStringList pinList;

    pinList // Inputs:
            
            << "ID03S0"
            << "ID02 S1"
            << "ID01  S2"
            
            << "IL05 DI"
            
            << "IU01OE "
            
            // Outputs:
            << "OR01O0 "
            << "OR02O1 "
            << "OR03O2 "
            << "OR04O3 "
            << "OR05O4 "
            << "OR06O5 "
            << "OR07O6 "
            << "OR08O7 "
            ;
    init( pinList );
    m_area = QRect( -(m_width/2)*8-1, -(m_height/2)*8-8-1, m_width*8+2, m_height*8+16+2 );
    
    eLogicDevice::createOutEnablePin( m_inPin[4] );    // IOutput Enable
    
    for( int i=0; i<4; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    for( int i=0; i<8; i++ )
        eLogicDevice::createOutput( m_outPin[i] );
}
Demux::~Demux(){}

QPainterPath Demux::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-(m_width/2)*8,-(m_height/2)*8+2 )
           << QPointF(-(m_width/2)*8, (m_height/2)*8-2 )
           << QPointF( (m_width/2)*8, (m_height/2)*8+6 )
           << QPointF( (m_width/2)*8,-(m_height/2)*8-6 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void Demux::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    static const QPointF points[4] = {
        QPointF(-(m_width/2)*8,-(m_height/2)*8+2 ),
        QPointF(-(m_width/2)*8, (m_height/2)*8-2 ),
        QPointF( (m_width/2)*8, (m_height/2)*8+6 ),
        QPointF( (m_width/2)*8,-(m_height/2)*8-6 )};

    p->drawPolygon(points, 4);
}

#include "moc_demux.cpp"

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

#include "gate_xor.h"
#include "itemlibrary.h"


Component* XorGate::construct( QObject* parent, QString type, QString id )
{
        return new XorGate( parent, type, id );
}

LibraryItem* XorGate::libraryItem()
{
    return new LibraryItem(
        tr( "Xor Gate" ),
        tr( "Logic/Gates" ),
        "xorgate.png",
        "Xor Gate",
        XorGate::construct );
}

XorGate::XorGate( QObject* parent, QString type, QString id )
        : Gate( parent, type, id, 2 )
{
}
XorGate::~XorGate(){}

bool XorGate::calcOutput( int inputs )
{
    if( inputs == 1 )  return true;

    return false;
}

QPainterPath XorGate::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    int size = m_numInputs*4;
    
    points << QPointF(-15,-size+2 )
           << QPointF( -9,-8  )
           << QPointF( -9, 8  )
           << QPointF(-15, size+2 )
           << QPointF(  0, size )
           << QPointF( 10, 8  )
           << QPointF( 10,-8  )
           << QPointF(  0,-size );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void XorGate::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    int y_orig = m_area.y();
    int height = m_area.height();

    // Paint white background of gate
    Component::paint( p, option, widget );
    QPen pen = p->pen();
    
    p->setPen( Qt::NoPen );

    QPainterPath path;
    path.moveTo( -4, 0 );
    path.arcTo( -25, y_orig, 34, height, -90, 180 );

    path.moveTo( -4, 0 );
    path.arcTo( -12, y_orig, 8, height, -90, 180 );

    p->drawPath( path );

    // Draw curves
    p->setPen( pen );
    p->setBrush( Qt::NoBrush );

    // Output side arc
    p->drawArc( -21, y_orig, 30, height, -1520/*-16*95*/, 3040/*16*190*/ );

    // Input side arc
    p->drawArc( -12, y_orig, 8, height, -1440/*-16*90*/, 2880/*16*180*/ );

    // Input side arc close to pins
    p->drawArc( -16, y_orig, 8, height, -1440/*-16*90*/, 2880/*16*180*/ );
}

#include "moc_gate_xor.cpp"



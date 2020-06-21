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

#include "gate_and.h"
#include "itemlibrary.h"

Component* AndGate::construct( QObject* parent, QString type, QString id )
{
    return new AndGate( parent, type, id );
}

LibraryItem* AndGate::libraryItem()
{
    return new LibraryItem(
        tr( "And Gate" ),
        tr( "Logic/Gates" ),
        "andgate.png",
        "And Gate",
        AndGate::construct );
}

AndGate::AndGate( QObject* parent, QString type, QString id )
       : Gate( parent, type, id, 2 )
{
}
AndGate::~AndGate(){}

QPainterPath AndGate::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    int size = m_numInputs*4;
    
    points << QPointF( -9,-size )
           << QPointF( -9, size )
           << QPointF(  0, size-2 )
           << QPointF( 10, 8 )
           << QPointF( 10,-8 )
           << QPointF(  0,-size+2 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void AndGate::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->drawChord( -28, m_area.y(), 37, m_area.height(), -1440/*-16*90*/, 2880/*16*180*/ );
}

#include "moc_gate_and.cpp"

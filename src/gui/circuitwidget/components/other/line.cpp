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

#include "line.h"


Component* Line::construct( QObject* parent, QString type, QString id )
{ return new Line( parent, type, id ); }

LibraryItem* Line::libraryItem()
{
    return new LibraryItem(
            tr( "Line" ),
            tr( "Other" ),
            "line.png",
            "Line",
            Line::construct);
}

Line::Line( QObject* parent, QString type, QString id )
    : Shape( parent, type, id )
{
}
Line::~Line(){}

QPainterPath Line::shape() const
{
    QPainterPath path;
    QPolygon polygon;
    polygon << QPoint(-m_hSize/2+1, m_vSize/2   );
    polygon << QPoint( m_hSize/2,  -m_vSize/2+1 );
    polygon << QPoint( m_hSize/2-1,-m_vSize/2   );
    polygon << QPoint(-m_hSize/2,   m_vSize/2-1 );
    path.addPolygon(polygon);
    return path;
}

void Line::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    QPen pen( m_color, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if ( isSelected() ) pen.setColor( Qt::darkGray);

    p->setPen( pen );
    
    p->drawLine( -m_hSize/2, m_vSize/2, m_hSize/2, -m_vSize/2 );
}

#include "moc_line.cpp"

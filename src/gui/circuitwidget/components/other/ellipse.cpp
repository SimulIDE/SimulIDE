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

#include "ellipse.h"


Component* Ellipse::construct( QObject* parent, QString type, QString id )
{ return new Ellipse( parent, type, id ); }

LibraryItem* Ellipse::libraryItem()
{
    return new LibraryItem(
            tr( "Ellipse" ),
            tr( "Other" ),
            "ellipse.png",
            "Ellipse",
            Ellipse::construct);
}

Ellipse::Ellipse( QObject* parent, QString type, QString id )
       : Shape( parent, type, id )
{

}
Ellipse::~Ellipse(){}

QPainterPath Ellipse::shape() const
{
    QPainterPath path;
    path.addEllipse( Shape::boundingRect() );
    return path;
}

void Ellipse::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    QPen pen(Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if ( isSelected() ) pen.setColor( Qt::darkGray);

    p->setBrush( m_color );
    p->setPen( pen );
    
    p->drawEllipse( m_area );
}

#include "moc_ellipse.cpp"

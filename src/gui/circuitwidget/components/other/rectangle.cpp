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

#include "rectangle.h"


Component* Rectangle::construct( QObject* parent, QString type, QString id )
{ return new Rectangle( parent, type, id ); }

LibraryItem* Rectangle::libraryItem()
{
    return new LibraryItem(
            tr( "Rectangle" ),
            tr( "Graphical" ),
            "rectangle.png",
            "Rectangle",
            Rectangle::construct);
}

Rectangle::Rectangle( QObject* parent, QString type, QString id )
         : Shape( parent, type, id )
{
}
Rectangle::~Rectangle(){}

void Rectangle::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    QPen pen(Qt::black, m_border, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    if ( isSelected() ) pen.setColor( Qt::darkGray);

    p->setBrush( m_color );
    p->setPen( pen );
    
    if( m_border > 0 ) p->drawRect( m_area );
    else               p->fillRect( m_area, p->brush() );
}

#include "moc_rectangle.cpp"

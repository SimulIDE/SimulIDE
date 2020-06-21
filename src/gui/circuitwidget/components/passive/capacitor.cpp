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

#include "capacitor.h"


Component* Capacitor::construct( QObject* parent, QString type, QString id )
{ return new Capacitor( parent, type, id ); }

LibraryItem* Capacitor::libraryItem()
{
    return new LibraryItem(
            tr( "Capacitor" ),
            tr( "Passive" ),
            "capacitor.png",
            "Capacitor",
            Capacitor::construct);
}

Capacitor::Capacitor( QObject* parent, QString type, QString id )
         : CapacitorBase( parent, type, id )
{
}
Capacitor::~Capacitor(){}

void Capacitor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    p->drawLine(-3,-6,-3, 6 );
    p->drawLine( 3,-6, 3, 6 );
}

#include "moc_capacitor.cpp"

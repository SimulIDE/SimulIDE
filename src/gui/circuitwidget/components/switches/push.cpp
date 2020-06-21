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

#include "push.h"
#include "pin.h"
#include "itemlibrary.h"


Component* Push::construct( QObject* parent, QString type, QString id )
{ return new Push( parent, type, id ); }

LibraryItem* Push::libraryItem()
{
    return new LibraryItem(
            tr( "Push" ),
            tr( "Switches" ),
            "push.png",
            "Push",
            Push::construct);
}

Push::Push( QObject* parent, QString type, QString id )
    : PushBase( parent, type, id )
{
    m_area =  QRectF( -11, -9, 22, 11 );
    
    m_proxy->setPos( QPoint(-8, 4) );

    SetupSwitches( 1, 1 );
}
Push::~Push()
{
}

void Push::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    
    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    for( int i=0; i<m_numPoles; i++ )                           // Draw Switches
    {
        int offset = 16*i;

        if( m_closed ) p->drawLine(-9, -2-offset, 9, -2-offset );
        else           p->drawLine(-9, -8-offset, 9, -8-offset );
    }
    if( m_numPoles > 1 )
    {
        pen.setStyle(Qt::DashLine);
        pen.setWidth(1);
        p->setPen(pen);
        p->drawLine(-0, 4-4*m_pin0, 0, -3*m_pin0-16*m_numPoles+4 );
    }
}

#include "moc_push.cpp"

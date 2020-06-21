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

#include "led.h"
#include "pin.h"

Component* Led::construct( QObject* parent, QString type, QString id )
{ return new Led( parent, type, id ); }

LibraryItem* Led::libraryItem()
{
    return new LibraryItem(
            tr( "Led" ),
            tr( "Outputs" ),
            "led.png",
            "Led",
            Led::construct);
}

Led::Led( QObject* parent, QString type, QString id )
   : LedBase( parent, type, id )
{
    m_area = QRect( -8, -10, 20, 20 );
    m_pin.resize( 2 );
    
    QString nodid = m_id;
    nodid.append(QString("-lPin"));
    Pin* pin = new Pin( 180, QPoint(-16, 0 ), nodid, 0, this);
    pin->setLength( 10 );
    m_ePin[0] = pin;
    m_pin[0] = pin;

    nodid = m_id;
    nodid.append(QString("-rPin"));
    pin = new Pin( 0, QPoint( 16, 0 ), nodid, 1, this);
    m_ePin[1] = pin;
    m_pin[1] = pin;
}
Led::~Led(){}


void Led::drawBackground( QPainter *p )
{
    p->drawEllipse( -6, -8, 16, 16 );
    //p->drawLine( 11,-5, 11, 5 );
}

void Led::drawForeground( QPainter *p )
{
    static const QPointF points[3] = {
        QPointF( 8, 0 ),
        QPointF(-3,-6 ),
        QPointF(-3, 6 ) };

    p->drawPolygon( points, 3 );
    p->drawLine( 8,-4, 8, 4 );
    p->drawLine(-6, 0, 10, 0 );
}

#include "moc_led.cpp"

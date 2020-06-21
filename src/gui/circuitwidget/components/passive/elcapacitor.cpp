/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "elcapacitor.h"
#include "simulator.h"

Component* elCapacitor::construct( QObject* parent, QString type, QString id )
{ return new elCapacitor( parent, type, id ); }

LibraryItem* elCapacitor::libraryItem()
{
    return new LibraryItem(
            tr( "Electrolytic Capacitor" ),
            tr( "Passive" ),
            "elcapacitor.png",
            "elCapacitor",
            elCapacitor::construct);
}

elCapacitor::elCapacitor( QObject* parent, QString type, QString id )
           : CapacitorBase( parent, type, id )
{
    m_reversed = false;
    m_counter = 0;
    
    Simulator::self()->addToUpdateList( this );
}
elCapacitor::~elCapacitor(){}

void elCapacitor::resetState()
{
    eCapacitor::resetState();
    m_reversed = false;
    m_counter = 0;
    update();
}

void elCapacitor::updateStep()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();

    if( volt < -1e-6 )
    {
        m_counter++;
        
        if( m_counter > 4 )
        {
            m_counter = 0;
            m_reversed = !m_reversed;
            update();
        }
    }
    else if( m_reversed )
    {
        m_reversed = false;
        m_counter = 0;
        update();
    }
}

void elCapacitor::remove()
{
    Simulator::self()->remFromUpdateList( this ); 
    
    Component::remove();
}

void elCapacitor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);
    
    if( m_reversed )
    {
        pen.setColor( QColor( 255, 100, 100 ));
        pen.setWidth(2);
    }
    p->setPen(pen);

    p->drawLine( 3,-7, 3, 7 );
    p->drawLine(-3,-7, 3,-7 );
    p->drawLine(-3, 7, 3, 7 );
    p->drawLine(-3,-3,-3, 3 );
}

#include "moc_elcapacitor.cpp"


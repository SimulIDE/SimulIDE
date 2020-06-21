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

#include "switch.h"
#include "itemlibrary.h"
#include "simulator.h"
#include "e-node.h"
#include "pin.h"

Component* Switch::construct( QObject* parent, QString type, QString id )
{ return new Switch( parent, type, id ); }

LibraryItem* Switch::libraryItem()
{
    return new LibraryItem(
            tr( "Switch (all)" ),
            tr( "Switches" ),
            "switch.png",
            "Switch",
            Switch::construct);
}

Switch::Switch( QObject* parent, QString type, QString id )
      : SwitchBase( parent, type, id )
{
    m_area =  QRectF( -11, -9, 22, 11 );

    m_proxy->setPos( QPoint(-8, 4) );

    SetupSwitches( 1, 1 );

    connect( m_button, SIGNAL( clicked() ),
             this,     SLOT  ( onbuttonclicked() ));
}
Switch::~Switch(){}

void Switch::stamp()
{
    MechContact::stamp();

    onbuttonclicked();
}

void Switch::keyEvent( QString key, bool pressed )
{
    if( key == m_key )
    {
        if( !pressed )
        {
            m_button->setChecked( !m_button->isChecked() );
            SwitchBase::onbuttonclicked();
        }
    }
}

void Switch::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    MechContact::paint( p, option, widget );
}

#include "moc_switch.cpp"

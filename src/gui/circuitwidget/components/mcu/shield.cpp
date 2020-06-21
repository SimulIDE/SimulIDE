/***************************************************************************
 *   Copyright (C) 2020 by santiago González                               *
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

#include "shield.h"
#include "mcucomponent.h"
//#include "circuit.h"
//#include "e-resistor.h"
//#include "itemlibrary.h"
#include "utils.h"


Shield::Shield( QObject* parent, QString type, QString id )
       : SubCircuit( parent, type, id )
{
    qDebug()<<"Shield::Shield"<<id;
    
    setLabelPos( 100,-21, 0); // X, Y, Rot
    
    initChip();

    setTransformOriginPoint( togrid( boundingRect().center()) );
}
Shield::~Shield() 
{
}

void Shield::remove()
{
    SubCircuit::remove();
}

void Shield::attach()
{
    SubCircuit::attach();
}

void Shield::mousePressEvent( QGraphicsSceneMouseEvent* event )
{

}
void Shield::mouseDoubleClickEvent( QGraphicsSceneMouseEvent* event )
{
    McuComponent::self()->mouseDoubleClickEvent( event );
}
void Shield::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    McuComponent::self()->mouseMoveEvent( event );
}
void Shield::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    McuComponent::self()->mouseReleaseEvent( event );
}
void Shield::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    McuComponent::self()->contextMenuEvent( event );
}


void Shield::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    int ox = m_area.x();
    int oy = m_area.y();

    p->drawPixmap( ox, oy, QPixmap( m_BackGround ));
}

#include "moc_shield.cpp"

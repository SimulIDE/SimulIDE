/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "lm555.h"
#include "e-source.h"
#include "itemlibrary.h"
#include "connector.h"
#include "pin.h"


Component* Lm555::construct( QObject* parent, QString type, QString id )
{
        return new Lm555( parent, type, id );
}

LibraryItem* Lm555::libraryItem()
{
    return new LibraryItem(
        tr( "lm555" ),
        tr( "Logic/Other Logic" ),
        "ic2.png",
        "Lm555",
        Lm555::construct );
}

Lm555::Lm555( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eLm555( id.toStdString() )
{
    m_area = QRect( 0, 0, 8*4, 8*5 );
    m_color = QColor( 50, 50, 70 );
    
    m_pin.resize( 8 );
    
    QString newId = id;
    
    newId.append(QString("-ePin0"));
    m_pin[0] = new Pin( 180, QPoint(-8, 8*1), newId, 0, this );
    m_pin[0]->setLabelText( "Gnd" );
    m_ePin[0] = m_pin[0];
    
    newId = id;
    newId.append(QString("-ePin1"));
    m_pin[1] = new Pin( 180, QPoint(-8, 8*2), newId, 1, this );
    m_pin[1]->setLabelText( "Trg" );
    m_ePin[1] = m_pin[1];
    
    newId = id;
    newId.append(QString("-ePin2"));
    m_pin[2] = new Pin( 180, QPoint(-8, 8*3), newId, 2, this );
    m_pin[2]->setLabelText( "Out" );
    m_ePin[2] = m_pin[2];
    newId.append("-eSource");
    m_output = new eSource( newId.toStdString(), m_ePin[2] );
    m_output->setImp( 10 );
    m_output->setOut( true );
    
    newId = id;
    newId.append(QString("-ePin3"));
    m_pin[3] = new Pin( 180, QPoint(-8, 8*4), newId, 3, this );
    m_pin[3]->setLabelText( "Rst" );
    m_ePin[3] = m_pin[3];
    
    newId = id;
    newId.append(QString("-ePin4"));
    m_pin[4] = new Pin( 0, QPoint(4*8+8, 8*4), newId, 4, this );
    m_pin[4]->setLabelText( "CV" );
    m_ePin[4] = m_pin[4];
    newId.append("-eSource");
    m_cv = new eSource( newId.toStdString(), m_ePin[4] );
    m_cv->setImp( 10 );
    m_cv->setOut( true );
    
    newId = id;
    newId.append(QString("-ePin5"));
    m_pin[5] = new Pin( 0, QPoint(4*8+8, 8*3), newId, 5, this );
    m_pin[5]->setLabelText( "Thr" );
    m_ePin[5] = m_pin[5];
    
    newId = id;
    newId.append(QString("-ePin6"));
    m_pin[6] = new Pin( 0, QPoint(4*8+8, 8*2), newId, 6, this );
    m_pin[6]->setLabelText( "Dis" );
    m_ePin[6] = m_pin[6];
    newId.append("-eSource");
    m_dis = new eSource( newId.toStdString(), m_ePin[6] );
    m_dis->setImp( high_imp );
    m_dis->setOut( false );
    
    newId = id;
    newId.append(QString("-ePin7"));
    m_pin[7] = new Pin( 0, QPoint(4*8+8, 8*1), newId, 7, this );
    m_pin[7]->setLabelText( "Vcc" );
    m_ePin[7] = m_pin[7];
    
}
Lm555::~Lm555()
{
}

void Lm555::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area, 1, 1);
    
    p->setPen( QColor( 170, 170, 150 ) );
    p->drawArc( boundingRect().width()/2-6, -4, 8, 8, 0, -2880 /* -16*180 */ );
}

#include "moc_lm555.cpp"

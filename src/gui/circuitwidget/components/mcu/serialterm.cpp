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

#include "serialterm.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "mcucomponent.h"
#include "baseprocessor.h"
#include "utils.h"

static const char* SerialTerm_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Mcu Uart")
};

Component* SerialTerm::construct( QObject* parent, QString type, QString id )
{
    return new SerialTerm( parent, type, id );
}

LibraryItem* SerialTerm::libraryItem()
{
    return new LibraryItem(
        "SerialTerm",
        "",
        "SerialTerm.png",
        "SerialTerm",
        SerialTerm::construct );
}

SerialTerm::SerialTerm( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , eElement( (id+"-eElement").toStdString() )
          , m_serialWidget( CircuitWidget::self(), this )
{
    Q_UNUSED( SerialTerm_properties );

    m_area = QRect( 0, 0, 0, 0 );

    m_serialWidget.show();

    connect( McuComponent::self(), SIGNAL( closeSerials()),
                             this, SLOT(   slotClose()) );

    Simulator::self()->addToUpdateList( this );
}

SerialTerm::~SerialTerm()
{
    Simulator::self()->remFromUpdateList( this );
}

void SerialTerm::updateStep()
{
    m_serialWidget.step();
    m_serialWidget.update();
}

void SerialTerm::setUart( int uart )
{
    if( uart == m_serialWidget.uart() ) return;

    if      ( uart<1 ) uart = 1;
    else if ( uart>6 ) uart = 6;

    m_serialWidget.uartChanged( uart );
    m_serialWidget.update();
}

void SerialTerm::slotClose()
{
    Circuit::self()->removeComp( this );
}

void SerialTerm::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    return;
    //p->drawRoundedRect( QRectF( -114, -79, 345, 185 ), 3, 3 );
}

#include "moc_serialterm.cpp"



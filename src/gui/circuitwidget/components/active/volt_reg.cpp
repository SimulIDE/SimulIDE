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

#include "volt_reg.h"
#include "connector.h"
#include "simulator.h"
#include "pin.h"

static const char* VoltReg_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Volts")
};

Component* VoltReg::construct( QObject* parent, QString type, QString id )
{
        return new VoltReg( parent, type, id );
}

LibraryItem* VoltReg::libraryItem()
{
    return new LibraryItem(
        tr( "Volt. Regulator" ),
        tr( "Active" ),
        "voltreg.png",
        "VoltReg",
        VoltReg::construct );
}

VoltReg::VoltReg( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eVoltReg( id.toStdString() )
{
    Q_UNUSED( VoltReg_properties );
    
    m_area = QRect( -11, -8, 22, 19 );
    
    setVRef( 1.2 );
    m_voltPos = 0;
    m_voltNeg = 0;
    
    m_pin.resize( 3 );
    
    QString newId = id;
    
    newId.append(QString("-input"));
    m_pin[0] = new Pin( 180, QPoint( -16, 0 ), newId, 0, this );
    m_pin[0]->setLength(6);
    m_pin[0]->setLabelText( "I" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];
    
    newId = id;
    newId.append(QString("-output"));
    m_pin[1] = new Pin( 0, QPoint( 16, 0 ), newId, 1, this );
    m_pin[1]->setLength(6);
    m_pin[1]->setLabelText( "O" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[1] = m_pin[1];
    
    newId = id;
    newId.append(QString("-ref"));
    m_pin[2] = new Pin( 270, QPoint( 0, 16 ), newId, 2, this );
    m_pin[2]->setLength(6);
    m_pin[2]->setLabelText( "R" );
    m_pin[2]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[2] = m_pin[2];
}
VoltReg::~VoltReg()
{
}

void VoltReg::setVRef( double vref )
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();
    
    eVoltReg::setVRef( vref );
    
    if( pauseSim ) Simulator::self()->runContinuous();
}

void VoltReg::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRect( m_area );
}

#include "moc_volt_reg.cpp"

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

#include "capacitorbase.h"
#include "pin.h"

static const char* CapacitorBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Capacitance"),
    QT_TRANSLATE_NOOP("App::Property","Show Cap")
};


CapacitorBase::CapacitorBase( QObject* parent, QString type, QString id )
             : Component( parent, type, id )
             , eCapacitor( id.toStdString() )
{
    Q_UNUSED( CapacitorBase_properties );
    
    m_ePin.resize(2);
    m_pin.resize(2);
    
    m_area = QRectF( -10, -10, 20, 20 );

    QString nodid = m_id;
    nodid.append(QString("-lPin"));
    QPoint nodpos = QPoint(-16-8,0);
    m_pin[0] = new Pin( 180, nodpos, nodid, 0, this);
    m_pin[0]->setLength(12);
    m_pin[0]->setPos(-16, 0 );
    m_ePin[0] = m_pin[0];

    nodid = m_id;
    nodid.append( QString("-rPin") );
    nodpos = QPoint(16+8,0);
    m_pin[1] = new Pin( 0, nodpos, nodid, 1, this );
    m_pin[1]->setLength(12);
    m_pin[1]->setPos( 16, 0 );
    m_ePin[1] = m_pin[1];
    
    m_unit = "F";
    setCapac( m_cap );
    setValLabelPos(-16, 8, 0);
    setShowVal( true );

    setLabelPos(-16,-24, 0);
}
CapacitorBase::~CapacitorBase(){}

double CapacitorBase::capac() { return m_value; }

void CapacitorBase::setCapac( double c ) 
{ 
    if( c == 0 ) c = 0.001;
    
    Component::setValue( c );       // Takes care about units multiplier
    eCapacitor::setCap( m_value*m_unitMult );
}

void CapacitorBase::setUnit( QString un ) 
{
    Component::setUnit( un );
    eCapacitor::setCap( m_value*m_unitMult );
}

#include "moc_capacitorbase.cpp"

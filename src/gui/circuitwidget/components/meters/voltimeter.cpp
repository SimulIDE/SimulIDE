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

#include "voltimeter.h"
#include "simulator.h"
#include "connector.h"


Component* Voltimeter::construct( QObject* parent, QString type, QString id )
{ return new Voltimeter( parent, type, id ); }

LibraryItem* Voltimeter::libraryItem()
{
    return new LibraryItem(
            tr( "Voltimeter" ),
            tr( "Meters" ),
            "voltimeter.png",
            "Voltimeter",
            Voltimeter::construct);
}

Voltimeter::Voltimeter( QObject* parent, QString type, QString id )
          : Meter( parent, type, id )
{
    m_unit = "V";
    setRes( high_imp );
    Meter::updateStep();
}
Voltimeter::~Voltimeter(){}

void Voltimeter::updateStep()
{
    double volt = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();
    
    if( volt != m_dispValue )
    {
        setUnit("V");
        m_dispValue = volt;
        Meter::updateStep();
    }
}

#include "moc_voltimeter.cpp"

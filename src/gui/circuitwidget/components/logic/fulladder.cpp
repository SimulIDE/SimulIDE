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

#include "fulladder.h"

Component* FullAdder::construct(QObject *parent, QString type, QString id)
{
    return new FullAdder(parent, type, id);
}

LibraryItem* FullAdder::libraryItem()
{
    return new LibraryItem(
        tr( "Full Adder" ),
        tr ("Logic/Arithmetic"),
        "2to2.png",
        "FullAdder",
        FullAdder::construct );
}

FullAdder::FullAdder(QObject *parent, QString type, QString id) 
          : LogicComponent( parent, type, id ), eFullAdder( id.toStdString() )
{
    m_width  = 4;
    m_height = 4;

    QStringList pinList;
    pinList
        << "IU01 A"
        << "IU03 B"
        
        << "ID03 Ci "

        // Outputs:

        << "OD02 S" 
        << "OD01 Co"
        ;
    init( pinList );
    
    for( int i=0; i<m_numInPins; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    for( int i=0; i<m_numOutPins; i++ )
        eLogicDevice::createOutput( m_outPin[i] );
}

FullAdder::~FullAdder(){}

#include "moc_fulladder.cpp"

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

#include "flipflopjk.h"
#include "pin.h"


Component* FlipFlopJK::construct( QObject* parent, QString type, QString id )
{
        return new FlipFlopJK( parent, type, id );
}

LibraryItem* FlipFlopJK::libraryItem()
{
    return new LibraryItem(
        tr( "FlipFlop JK" ),
        tr( "Logic/Memory" ),
        "3to2.png",
        "FlipFlopJK",
        FlipFlopJK::construct );
}

FlipFlopJK::FlipFlopJK( QObject* parent, QString type, QString id )
          : LogicComponent( parent, type, id )
          , eFlipFlopJK( id.toStdString() )
{
    m_width  = 4;
    m_height = 6;
    
    QStringList pinList;

    pinList // Inputs:
            << "IL01 J"
            << "IL05 K"
            << "IU02S"
            << "ID02R"
            << "IL03>"
            
            // Outputs:
            << "OR01Q "
            << "OR05!Q "
            ;
    init( pinList );
    
    eLogicDevice::createInput( m_inPin[0] );                  // Input J

    eLogicDevice::createInput( m_inPin[1] );                  // Input K

    eLogicDevice::createInput( m_inPin[2] );                  // Input S

    eLogicDevice::createInput( m_inPin[3] );                  // Input R
    
    eLogicDevice::createClockPin( m_inPin[4] );           // Input Clock
    
    eLogicDevice::createOutput( m_outPin[0] );               // Output Q

    eLogicDevice::createOutput( m_outPin[1] );               // Output Q'

    setSrInv( true );                         // Invert Set & Reset pins
    setClockInv( false );                       //Don't Invert Clock pin

}
FlipFlopJK::~FlipFlopJK(){}


#include "moc_flipflopjk.cpp"

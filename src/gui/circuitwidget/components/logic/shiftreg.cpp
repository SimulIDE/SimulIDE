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

#include "shiftreg.h"
#include "pin.h"


Component* ShiftReg::construct( QObject* parent, QString type, QString id )
{
        return new ShiftReg( parent, type, id );
}

LibraryItem* ShiftReg::libraryItem()
{
    return new LibraryItem(
        tr( "Shift Reg." ),
        tr( "Logic/Arithmetic" ),
        "1to3.png",
        "ShiftReg",
        ShiftReg::construct );
}

ShiftReg::ShiftReg( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eShiftReg( id.toStdString() )
{
    m_width  = 4;
    m_height = 9;

    QStringList pinList;

    pinList // Inputs:
            << "IL03 DI"
            << "IL05>"
            << "IL07 Rst"

            << "IU01OE "
            
            // Outputs:
            << "OR01Q0 "
            << "OR02Q1 "
            << "OR03Q2 "
            << "OR04Q3 "
            << "OR05Q4 "
            << "OR06Q5 "
            << "OR07Q6 "
            << "OR08Q7 "
            ;
    init( pinList );

    eLogicDevice::createInput( m_inPin[0] );                 // Input DI
    
    eLogicDevice::createClockPin( m_inPin[1] );           // Input Clock

    eLogicDevice::createInput( m_inPin[2] );                // Input Rst

    eLogicDevice::createOutEnablePin( m_inPin[3] );    // IOutput Enable

    for( int i=0; i<m_numOutPins; i++ )
        eLogicDevice::createOutput( m_outPin[i] );

    setResetInv( true );                             // Invert Reset Pin
}
ShiftReg::~ShiftReg(){}

#include "moc_shiftreg.cpp"

/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#include "e-fulladder.h"


eFullAdder::eFullAdder( std::string id ) 
          : eLogicDevice( id )
{
}

void eFullAdder::stamp()
{
    for( int i=0; i<m_numInputs; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    
    eLogicDevice::stamp();
}

void eFullAdder::createPins()
{
    eLogicDevice::createPins( 3, 2 );          // Create Inputs, Outputs

    // Input 0 - A
    // Input 1 - B
    // Input 2 - Ci
    
    // Output 0 - S
    // Output 1 - Co
}

void eFullAdder::setVChanged()
{
    bool X  = eLogicDevice::getInputState( 0 );
    bool Y  = eLogicDevice::getInputState( 1 );
    bool Ci = eLogicDevice::getInputState( 2 );
    
    bool S  = (X ^ Y) ^ Ci;                                      // Suma
    bool Co = (X & Ci) | (Y & Ci) | (X & Y);                // Carry out
    
    eLogicDevice::setOut( 0, S );
    eLogicDevice::setOut( 1, Co );
}


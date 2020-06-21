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

#include <math.h>

#include "e-dectobcd.h"

eDecToBcd::eDecToBcd( std::string id )
         : eLogicDevice( id )
{
    m_16Bits = false;
}
eDecToBcd::~eDecToBcd()
{ 
}

void eDecToBcd::stamp()
{
    for( int i=0; i<15; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    m_address = -1;

    eLogicDevice::stamp();
}

void eDecToBcd::setVChanged()
{
    eLogicDevice::updateOutEnabled();
    
    int address = 0;

    int msBit = 9;
    if( m_16Bits ) msBit = 15;
    
    for( int i=0; i<msBit; i++ )
    {
        if( eLogicDevice::getInputState( i ) ) address = i+1;
    }
    if( address == m_address ) return;
    m_address = address;
    
    for( int i=0; i<4; i++ )
    {
        eLogicDevice::setOut( i, address & 1 );
        address >>= 1;
    }
}

void eDecToBcd::set_16bits( bool set )
{
    m_16Bits = set;
}

void eDecToBcd::createPins()
{
    eLogicDevice::createPins( 15, 4 );
}



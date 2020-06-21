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

#include "e-demux.h"

eDemux::eDemux( std::string id )
      : eLogicDevice( id )
{
}
eDemux::~eDemux()
{ 
}

void eDemux::stamp()
{
    for( int i=0; i<4; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    eLogicDevice::stamp();
}

void eDemux::setVChanged()
{
    eLogicDevice::updateOutEnabled();
    
    int address = 0;
    
    for( int i=0; i<3; i++ )
    {
        if( getInputState( i ) ) address += pow( 2, i );
    }
    
    for( int i=0; i<8; i++ )
    {
        eLogicDevice::setOut( i, false );
    }
    if( getInputState( 3 ) ) eLogicDevice::setOut( address, true );
}

void eDemux::createPins()
{
    eLogicDevice::createPins( 5, 8 );
}

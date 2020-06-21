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

#include "e-bcdto7s.h"

eBcdTo7S::eBcdTo7S( std::string id )
        : eLogicDevice( id )
{
    m_outValue.resize( 7 );
}
eBcdTo7S::~eBcdTo7S()
{ 
}

void eBcdTo7S::stamp()
{
    for( int i=0; i<4; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    m_changed = true;
    m_digit = -1;

    eLogicDevice::stamp();
}

void eBcdTo7S::setVChanged()
{
    eLogicDevice::updateOutEnabled();
    
    m_changed = true;
    
    bool a = eLogicDevice::getInputState( 0 );
    bool b = eLogicDevice::getInputState( 1 );
    bool c = eLogicDevice::getInputState( 2 );
    bool d = eLogicDevice::getInputState( 3 );

    int digit = a*1+b*2+c*4+d*8;
    if( digit == m_digit ) return;
    m_digit = digit;

    switch( digit)
    {
    case 0:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = false;
        break;
    case 1:
        m_outValue[0] = false;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = false;
        m_outValue[4] = false;
        m_outValue[5] = false;
        m_outValue[6] = false;
        break;
    case 2:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = false;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = false;
        m_outValue[6] = true;
        break;
    case 3:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = false;
        m_outValue[5] = false;
        m_outValue[6] = true;
        break;
    case 4:
        m_outValue[0] = false;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = false;
        m_outValue[4] = false;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 5:
        m_outValue[0] = true;
        m_outValue[1] = false;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = false;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 6:
        m_outValue[0] = true;
        m_outValue[1] = false;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 7:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = false;
        m_outValue[4] = false;
        m_outValue[5] = false;
        m_outValue[6] = false;
        break;
    case 8:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 9:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = false;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 10:
        m_outValue[0] = true;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = false;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 11:
        m_outValue[0] = false;
        m_outValue[1] = false;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 12:
        m_outValue[0] = true;
        m_outValue[1] = false;
        m_outValue[2] = false;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = false;
        break;
    case 13:
        m_outValue[0] = false;
        m_outValue[1] = true;
        m_outValue[2] = true;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = false;
        m_outValue[6] = true;
        break;
    case 14:
        m_outValue[0] = true;
        m_outValue[1] = false;
        m_outValue[2] = false;
        m_outValue[3] = true;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    case 15:
        m_outValue[0] = true;
        m_outValue[1] = false;
        m_outValue[2] = false;
        m_outValue[3] = false;
        m_outValue[4] = true;
        m_outValue[5] = true;
        m_outValue[6] = true;
        break;
    }
    for( int i=0; i<m_numOutputs; i++ ) setOut( i, m_outValue[i] );
}

void eBcdTo7S::createPins()
{
    eLogicDevice::createPins( 4, 8 );
}

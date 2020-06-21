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

#include <QDebug>

#include "e-flipflopd.h"
#include "circuit.h"

eFlipFlopD::eFlipFlopD( std::string id )
          : eLogicDevice( id )
{
}
eFlipFlopD::~eFlipFlopD()
{ 
}

void eFlipFlopD::createPins()
{
    createClockPin();
    eLogicDevice::createPins( 3, 2 );          // Create Inputs, Outputs
    
    // Input 0 - D
    // Input 1 - S
    // Input 2 - R
    // Input 3 - Clock
    
    // Output 1 - Q
    // Output 2 - !Q
}

void eFlipFlopD::stamp()
{
    eNode* enode = m_input[1]->getEpin()->getEnode();         // Set pin
    if( enode ) enode->addToChangedFast(this);
    
    enode = m_input[2]->getEpin()->getEnode();              // Reset pin
    if( enode ) enode->addToChangedFast(this);
    
    eLogicDevice::stamp();
}

void eFlipFlopD::setVChanged()
{
    // Get Clk to don't miss any clock changes
    bool clkRising = (eLogicDevice::getClockState() == Rising);

    bool set   = eLogicDevice::getInputState( 1 );
    bool reset = eLogicDevice::getInputState( 2 );

    //qDebug() << "eFlipFlopD::setVChanged()"<<clkRising;

    if(set && reset)
    {
        eLogicDevice::setOut( 0, true );                           // Q
        eLogicDevice::setOut( 1, true );                           // Q'
    }
    else if( set )          // Master Set
    {
        eLogicDevice::setOut( 0, true );                           // Q
        eLogicDevice::setOut( 1, false );                          // Q'
    }
    else if( reset )   // Master Reset
    {
        eLogicDevice::setOut( 0, false );                          // Q
        eLogicDevice::setOut( 1, true );                           // Q'
    }
    else if( clkRising )                             // Clk Rising edge
    {
        bool D = eLogicDevice::getInputState( 0 );

        eLogicDevice::setOut( 0, D );                              // Q
        eLogicDevice::setOut( 1, !D );                             // Q'
    }
}

void eFlipFlopD::setSrInv( bool inv )
{
    m_srInv = inv;
    m_input[1]->setInverted( inv );                           // Set
    m_input[2]->setInverted( inv );                           // Reset
    
    Circuit::self()->update();
}

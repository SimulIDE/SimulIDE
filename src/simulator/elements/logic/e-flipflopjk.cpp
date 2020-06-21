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

#include "e-flipflopjk.h"
#include "circuit.h"

eFlipFlopJK::eFlipFlopJK( std::string id )
           : eLogicDevice( id )
{
}
eFlipFlopJK::~eFlipFlopJK()
{ 
}

void eFlipFlopJK::createPins()
{
    createClockPin();
    eLogicDevice::createPins( 4, 2 );          // Create Inputs, Outputs
    
    // Input 0 - J
    // Input 1 - K
    // Input 2 - S
    // Input 3 - R
    // Input 4 - Clock
    
    // Output 1 - Q
    // Output 2 - !Q
}

void eFlipFlopJK::stamp()
{
    eNode* enode = m_input[2]->getEpin()->getEnode();         // Set pin
    if( enode ) enode->addToChangedFast(this);
    
    enode = m_input[3]->getEpin()->getEnode();              // Reset pin
    if( enode ) enode->addToChangedFast(this);
    
    eLogicDevice::stamp();
}

void eFlipFlopJK::setVChanged()
{
    // Get Clk to don't miss any clock changes
    bool clkRising = (eLogicDevice::getClockState() == Rising);

    //qDebug() << "eFlipFlopJK::setVChanged()"<<clkRising;

    if( eLogicDevice::getInputState( 2 )==true )          // Master Set
    {
        eLogicDevice::setOut( 0, true );                           // Q
        eLogicDevice::setOut( 1, false );                          // Q'
        //qDebug() << "eFlipFlopJK::setVChanged() set";
    }
    else if( eLogicDevice::getInputState( 3 )==true )   // Master Reset
    {
        eLogicDevice::setOut( 0, false );                          // Q
        eLogicDevice::setOut( 1, true );                           // Q'
        //qDebug() << "eFlipFlopJK::setVChanged() Reset";
    }
    else if( clkRising )                             // Clk Rising edge
    {
        bool J = eLogicDevice::getInputState( 0 );
        bool K = eLogicDevice::getInputState( 1 );
        bool Q = m_output[0]->out();
        
        bool state = (J && !Q) || (!K && Q) ;
        //qDebug() << "eFlipFlopJK::setVChanged() clk"<<J<<K<<state;

        eLogicDevice::setOut( 0, state );                          // Q
        eLogicDevice::setOut( 1, !state );                         // Q'
    }
}

void eFlipFlopJK::setSrInv( bool inv )
{
    m_srInv = inv;
    m_input[2]->setInverted( inv );                           // Set
    m_input[3]->setInverted( inv );                           // Reset
    
    Circuit::self()->update();
}

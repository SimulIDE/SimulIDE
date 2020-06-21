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

#include <sstream>

#include "e-shiftreg.h"

eShiftReg::eShiftReg( std::string id, int latchClk, int serOut ) 
         : eLogicDevice( id )
{
    // input0: DS    serial data input
    // input1: MR    master reset (active LOW)
    // packagePin15-QH' 
    
    m_latchClock = false;
    m_changed    = false;
    m_reset      = false;
    
    m_latchClockPin = 0l;
    m_serOutPin     = 0l;
    
    if( latchClk > 0 ) createLatchClockPin();
    if( serOut > 0 )   createSerOutPin();

    m_shiftReg.reset();
    m_latch.reset();
    resetState();
}
eShiftReg::~eShiftReg()
{
    delete m_latchClockPin;
    delete m_serOutPin;
}

void eShiftReg::initEpins()
{
    createPins( 2, 8 );                           // 2 Inputs, 8 Outputs
    createClockPin();

    m_input[1]->setInverted( true );// input1: master reset (active LOW)
}

void eShiftReg::stamp()
{
                                               // m_input[1] = Reset pin
    eNode* enode = m_input[1]->getEpin()->getEnode(); 
    if( enode ) enode->addToChangedFast(this);
    
    if( m_latchClockPin )
    {
        enode = m_latchClockPin->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    eLogicDevice::stamp();
}

void eShiftReg::resetState()
{
    m_shiftReg.reset();
    m_latch.reset();
    m_latchClock = false;
    m_changed = false;
    
    if( m_serOutPin )                              // Set Serial Out Pin
    {
        m_serOutPin->setOut( false );
        m_serOutPin->stampOutput();
    }
    eLogicDevice::resetState();
}

void eShiftReg::setVChanged()
{
    eLogicDevice::updateOutEnabled();
    // Get Clk to don't miss any clock changes
    bool clkRising = (eLogicDevice::getClockState() == Rising);

    double volt = m_input[1]->getEpin()->getVolt();   // Reset pin volt.
    
    bool reset = eLogicDevice::getInputState( 1 );// m_input[1] = Reset

    if( reset != m_reset  )           
    {
        m_reset = reset;
        if( reset )
        {
            m_shiftReg.reset();                  // Reset shift register  
            
            if( m_serOutPin )                    // Reset Serial Out Pin
            {
                m_serOutPin->setOut( false );
                m_serOutPin->stampOutput();
            }
        }
        m_changed = true;                      // Shift Register changed
    }
    else if( clkRising && !reset )                  // Clock rising edge
    {
        // Shift bits 7-1
        for( int i=7; i>0; i-- )m_shiftReg[i] = m_shiftReg[i-1];
        
        if( m_serOutPin )                          // Set Serial Out Pin
        {
            m_serOutPin->setOut( m_shiftReg[7] );
            m_serOutPin->stampOutput();
        }
        // Read data input pin & put in reg bit0
        volt = m_input[0]->getEpin()->getVolt();      // Reset pin volt.
        
        m_shiftReg[0] = (volt > m_inputHighV);     // input0: data input
        
        m_changed = true;                      // Shift Register changed
    }
    
    
    if( m_latchClockPin )                            // find rising edge
    {
        bool lastLcClock = m_latchClock;

        double volt = m_latchClockPin->getVolt();     // Clock pin volt.

        if     ( volt > m_inputHighV ) m_latchClock = true;
        else if( volt < m_inputLowV )  m_latchClock = false;

        if( m_latchClock & !lastLcClock & m_changed ) 
        {
            m_changed = false;
            
            for( int i=0; i<8; i++ )  // Store Shift-reg in Output Latch
            {
                bool data = m_shiftReg[i];
                m_latch[i] = data;
                eLogicDevice::setOut( i, data );
            }
        }
    }
    else
    {
        if( m_changed )
        {
            m_changed = false;
            
            for( int i=0; i<8; i++ ) 
                eLogicDevice::setOut( i, m_shiftReg[i] ); // Set outputs
        }
    }
}

void eShiftReg::createLatchClockPin()
{
    std::stringstream sspin;
    sspin << m_elmId << "latchClockPin";
    ePin* epin = new ePin( sspin.str(), 0 );

    std::stringstream ssesource;
    ssesource << m_elmId << "eSourceLatchClock";
    m_latchClockPin = new eSource( ssesource.str(), epin );
    m_latchClockPin->setImp( m_inputImp );
}

void eShiftReg::createSerOutPin()
{
    std::stringstream sspin;
    sspin << m_elmId << "serOutPin";
    ePin* epin = new ePin( sspin.str(), 0 );

    std::stringstream ssesource;
    ssesource << m_elmId << "eSourceSerOutPin";
    m_serOutPin = new eSource( ssesource.str(), epin );
    m_serOutPin->setVoltHigh( m_outHighV );
    m_serOutPin->setImp( m_outImp );
}
ePin* eShiftReg::getEpin( QString pinName )
{
    if( pinName.contains("latchClock") )return m_latchClockPin->getEpin();
    if( pinName.contains("serialOut") ) return m_serOutPin->getEpin();
    
    return eLogicDevice::getEpin( pinName );
}

void eShiftReg::setResetInv( bool inv )
{
    m_resetInv = inv;
    m_input[1]->setInverted( inv );
}

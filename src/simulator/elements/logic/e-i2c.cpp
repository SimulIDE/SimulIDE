/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
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

#include "e-i2c.h"
#include "simulator.h"
//#include <QDebug>

eI2C::eI2C( std::string id )
    : eLogicDevice( id )
{
    m_comp = 0l;
    m_address = 0x00;
    m_enabled = true;
    m_master  = false;
}
eI2C::~eI2C() { }

void eI2C::stamp()                    // Called at Simulation Start
{
    //if( !m_enabled ) return;
    if( m_input.size() == 0 ) return;

    eLogicDevice::stamp();   // Initialize Base Class ( Clock pin is managed in eLogicDevice )

    eNode* enode = m_input[0]->getEpin()->getEnode();
    if( enode ) enode->addToChangedFast( this );
}

void eI2C::resetState()
{
    eLogicDevice::resetState();

    double stepsPerS = Simulator::self()->stepsPerus()*1e6;
    m_stepsPe = stepsPerS/m_freq;
    m_nextCycle = 0;
    
    m_txReg = 0;
    m_rxReg = 0;
    m_bitPtr = 0;
    m_lastSDA = false;
    m_toggleClock = false;
    m_state = I2C_IDLE;
    m_addressBits = 7;

    //setEnabled( m_enabled ); // Restart Pin State

    /*if( m_input.size() == 0 ) return;
    if( !m_enabled ) return;
    m_input[0]->setVoltHigh( 0 );
    m_input[0]->setImp( high_imp );
    m_clockPin->setVoltHigh( 0 );
    m_clockPin->setImp( high_imp );*/
}

void eI2C::simuClockStep()       // We are in Mater mode, controlling Clock
{
    if( m_state == I2C_IDLE ) return;

    bool clkLow = m_clockPin->getVolt() < m_inputLowV;

    if( m_toggleClock )
    {
        if( clkLow ) m_clockPin->setImp( high_imp );
        else         m_clockPin->setImp( m_outImp );
        m_toggleClock = false;
    }

    if( m_nextCycle > 0 )                           // We are Clocking
    {
        if( --m_nextCycle > 0 ) return;
    }
    m_SDA = eLogicDevice::getInputState( 0 );       // State of SDA pin

    if( m_state == I2C_STARTED )                    // Send Start Condition
    {
        if( m_SDA )                            // Step 1: SDA is High, Lower it
        {
            m_input[0]->setOut( false );
            m_input[0]->setImp( m_outImp );
        }
        else if( !clkLow )                          // Step 2: SDA Already Low, Lower Clock
        {
            //m_clockPin->setOut( false );
            m_clockPin->setImp( m_outImp );
            if( m_comp ) m_comp->inStateChanged( 128 ); // Set TWINT
        }
    }
    else if( m_state == I2C_WRITTING )               // We are Writting data
    {
        m_toggleClock = true;                        // Keep Clocking
        m_nextCycle += m_stepsPe;
        if( clkLow ) writeBit();                      // Set SDA while clk is Low
    }
    else if( m_state == I2C_READING )                // We are Reading data
    {
        m_toggleClock = true;                        // Keep Clocking
        m_nextCycle += m_stepsPe;
        if( !clkLow )                                // Read bit while clk is high
        {
            readBit();
            if( m_bitPtr == 8 )
            {
                readByte();
                if( m_comp ) m_comp->inStateChanged( 128+I2C_READING );
            }
        }
    }
    else if( m_state == I2C_WAITACK )                // Read ACK
    {
        int ack = 257;                               //  ACK
        if( m_SDA ) ack = 256;                       // NACK
        if( m_comp ) m_comp->inStateChanged( ack );

        m_state = I2C_ACK;
        m_toggleClock = true;                        // Lower Clock afther ack
        m_nextCycle += m_stepsPe;
    }
    else if( m_state == I2C_STOPPED )                // Send Stop Condition
    {
        if     ( m_SDA  && clkLow ) m_input[0]->setImp( m_outImp ); // Step 1: Lower SDA
        else if( !m_SDA && clkLow ) m_clockPin->setImp( high_imp ); // Step 2: Raise Clock
        else if( !m_SDA && !clkLow) m_input[0]->setImp( high_imp ); // Step 3: Raise SDA
        else if( m_SDA && !clkLow )                                 // Step 4: Operation Finished
        {
            m_state = I2C_IDLE;
            if( m_comp ) m_comp->inStateChanged( 128+I2C_STOPPED ); // Set TWINT ( set to 0 )
        }
        m_toggleClock = false;                        // Stop Clocking
        m_nextCycle = 0;
    }
}

void eI2C::setVChanged()            // Some Pin Changed State, Manage it
{
    if( !m_enabled ) return;
    if( m_master ) return;

    int sclState = eLogicDevice::getClockState(); // Get Clk to don't miss any clock changes

    m_SDA = eLogicDevice::getInputState( 0 );        // State of SDA pin

    if(( sclState == CHigh )&&( m_state != I2C_ACK ))
    {
        if( !m_SDA  && m_lastSDA ) {     // We are in a Start Condition
            m_bitPtr = 0;
            m_rxReg = 0;
            m_state = I2C_STARTED;
        }
        else if( m_SDA && !m_lastSDA ) {   // We are in a Stop Condition
           slaveStop();
        }
    }
    else if( sclState == Rising )        // We are in a SCL Rissing edge
    {
        if( m_state == I2C_STARTED )             // Get Transaction Info
        {                                // TODO add support for 10 bits
            readBit();
            if( m_bitPtr > m_addressBits ) {
                bool rw = m_rxReg % 2;                 //Last bit is R/W
                m_rxReg >>= 1;
       //qDebug() << "eI2C::stepSlave address" << m_rxReg << m_address;
                if( m_rxReg == m_address ) {            // Address match
                    if( rw ) {                       // Master is Reading
                        m_state = I2C_READING;
                        writeByte();
                    } else {                        // Master is Writting
                        m_state = I2C_WRITTING;
                        m_bitPtr = 0;
                        startWrite();
                    }
                    ACK();
                    //qDebug() << "eI2C::stepSlave Reading" << rw;
                } else {
                    m_state = I2C_STOPPED;
                    m_rxReg = 0;
                }
            }
        }
        else if( m_state == I2C_WRITTING ){
            readBit();
            if( m_bitPtr == 8 ) readByte();
        }
        else if( m_state == I2C_WAITACK )      // We wait for Master ACK
        {
            if( !m_SDA ) {                      // ACK: Continue Sending
                m_state = m_lastState;
                writeByte();
            } else m_state = I2C_IDLE;
        }
    }
    else if( sclState == Falling )
    {
        if( m_state == I2C_ACK ) {                           // Send ACK
            m_input[0]->setImp( m_outImp );
            m_state = I2C_ENDACK;
        }
        else if( m_state == I2C_ENDACK ) {   // We sent ACK, release SDA
            m_input[0]->setImp( high_imp );
            m_state = m_lastState;
            m_rxReg = 0;
        }
        if( m_state == I2C_READING ) writeBit();
    }
    m_lastSDA = m_SDA;
}

void eI2C::masterStart( uint8_t addr )
{
    //qDebug() << "eI2C::masterStart"<<addr;

    m_input[0]->setOut( false );
    m_input[0]->setImp( high_imp );
    m_clockPin->setOut( false );
    m_clockPin->setImp( high_imp );

    m_state = I2C_STARTED;
}

void eI2C::masterWrite( uint8_t data )
{
    //qDebug() << "eI2C::masterWrite"<<data;
    m_state = I2C_WRITTING;
    m_txReg = data;
    writeByte();
}

void eI2C::masterRead()
{
    qDebug() << "eI2C::masterRead";

    m_input[0]->setOut( false );
    m_input[0]->setImp( high_imp );

    m_bitPtr = 0;

    m_state = I2C_READING;
}

void eI2C::masterStop()
{
    //qDebug() << "eI2C::masterStop";
    m_state = I2C_STOPPED;
}

void eI2C::slaveStop()
{
    m_state = I2C_STOPPED;
    //qDebug() << "eI2C::stepSlave I2C_STOPPED\n";
}

void eI2C::readBit()
{
    if( m_bitPtr > 0 ) m_rxReg <<= 1;
    m_rxReg += m_SDA;                            //Read one bit from sda
    m_bitPtr++;
}

void eI2C::writeBit()
{
    if( m_bitPtr < 0 ) 
    {
        waitACK();
        return;
    }
    bool bit = m_txReg>>m_bitPtr & 1;

    double imp = m_outImp;
    if( bit ) imp = high_imp;

    m_input[0]->setImp( imp );
    //qDebug() << "eI2C::writeBit()"<<m_bitPtr<<bit;
    m_bitPtr--;
}

void eI2C::readByte()
{
    //qDebug() << "eI2C::readByte()"<<m_rxReg;
    m_bitPtr = 0;
    ACK();
}

void eI2C::writeByte()
{
    m_bitPtr = 7;
}

void eI2C::ACK()
{
    m_lastState = m_state;
    m_state = I2C_ACK;
}

void eI2C::waitACK()
{
    m_input[0]->setImp( high_imp );
    
    m_lastState = m_state;
    m_state = I2C_WAITACK;
}

void eI2C::setEnabled( bool en )
{
    m_enabled = en;

    if( en )
    {
        m_input[0]->setOut( false );
        m_input[0]->setImp( high_imp );
        m_clockPin->setOut( false );
        m_clockPin->setImp( high_imp );
    }
    //qDebug() << "eI2C::setEnabled"<<m_enabled<<m_master;
}

void eI2C::setMaster( bool m )
{
    m_master = m;
    //qDebug() << "eI2C::setMaster"<<m_enabled<<m_master;
    if( m_master )
    {
        Simulator::self()->addToSimuClockList( this );
        eNode* enode = m_clockPin->getEpin()->getEnode();
        if( enode ) enode->remFromChangedFast(this);
    }
    else
    {
        Simulator::self()->remFromSimuClockList( this );
        eNode* enode = m_clockPin->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }

    if( m_enabled ) Simulator::self()->addToSimuClockList( this );
    else            Simulator::self()->remFromSimuClockList( this );
}

void eI2C::setAddress( int address )
{
    m_address = address;
}

void eI2C::setFreq( double f )
{
    m_freq = f;
    double stepsPerS = Simulator::self()->stepsPerus()*1e6;
    m_stepsPe = stepsPerS/m_freq/2;
    //qDebug() << "eI2C::setFreq" << f<< m_stepsPe;
}

void eI2C::createPins()  // Usually Called by Subcircuit to create ePins
{
    createClockPin();            // Clock pin is managed in eLogicDevice

    eLogicDevice::createPins( 1, 0 );        // Create input Pin for SDA
}

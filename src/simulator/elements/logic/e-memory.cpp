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

#include "e-memory.h"
#include "e-node.h"
#include <QDebug>

eMemory::eMemory( std::string id )
       : eLogicDevice( id )
{
    m_persistent = false;
    m_addrBits = 8;
    m_dataBits = 8;
    
    m_dataPinState.resize( 8 );
    m_ram.resize(256);
    
    resetState();
}
eMemory::~eMemory(){}

void eMemory::stamp()                 // Called at Simulation Start
{
    for( int i=0; i<2+m_addrBits; i++ )                 // Initialize control pins
    {
        eNode* enode =  m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast( this );
    }
    eLogicDevice::stamp();
}

void eMemory::resetState()
{
    m_we = true;
    m_cs = true;
    m_oe = false;
    
    double imp = 1e28;
    for( int i=0; i<m_numOutputs; i++ )
    {
        m_dataPinState[i] = false;
        m_output[i]->setImp( imp );
    }
    if( !m_persistent ) m_ram.fill( 0 );
    
    eLogicDevice::resetState();
}

void eMemory::setVChanged()        // Some Pin Changed State, Manage it
{
    bool CS = eLogicDevice::getInputState(1);
    //qDebug()<< QString::fromStdString( m_elmId ) << m_cs << CS;

    bool csTrig = false;

    if( CS != m_cs )
    {
        if( CS && !m_cs ) csTrig = true;
        m_cs = CS;
        
        if( !CS )
        {
            m_oe = false;
            double imp = 1e28;
            for( int i=0; i<m_numOutputs; i++ ) m_output[i]->setImp( imp );
        }
    }
    if( !CS ) return;

    bool WE = eLogicDevice::getInputState(0);
    bool oe = eLogicDevice::outputEnabled() && !WE;
    //qDebug() << WE << oe;
    
    if( oe != m_oe )
    {
        m_oe = oe;
        eLogicDevice::setOutputEnabled( oe );
    }
    int address = 0;
    for( int i=0; i<m_addrBits; i++ )                              // Get Address
    {
        bool  state = eLogicDevice::getInputState(i+2);
        
        if( state ) address += pow( 2, i );
        
    }
    //qDebug() << "address " << address;
    bool weTrig = false;
    if( WE && !m_we ) weTrig = true;
    m_we = WE;
    if( WE )                                                    // Write
    {
        if( csTrig || weTrig)
        {
            int value = 0;

            for( int i=0; i<m_numOutputs; i++ )
            {
                int volt = m_output[i]->getEpin()->getVolt();

                bool  state = m_dataPinState[i];

                if     ( volt > m_inputHighV ) state = true;
                else if( volt < m_inputLowV )  state = false;

                m_dataPinState[i] = state;
                //qDebug() << "Bit " << i << state;
                if( state ) value += pow( 2, i );
            }
            //qDebug()<< QString::fromStdString( m_elmId ) << "Writting " << address << value;
            m_ram[address] = value;
        }
    }
    else                                                         // Read
    {
        int value = m_ram[address];
        //qDebug()<< QString::fromStdString( m_elmId ) << "Reading " << address << value;
        for( int i=0; i<m_numOutputs; i++ )
        {
            bool pinState =  value & 1;
            m_output[i]->setOut( pinState );
            m_output[i]->stampOutput();
            //qDebug() << "Bit " << i << pinState;
            value >>= 1;
        }
    }
}

void eMemory::setMem( QVector<int> m )
{
    if( m.size() == 1 ) return;       // Avoid loading data if not saved
    m_ram = m;
}

QVector<int> eMemory::mem()
{
    if( !m_persistent ) 
    {
        QVector<int> null;
        return null;
    }
    //qDebug() << m_ram.size() <<"Ram:\n" << m_ram;
    return m_ram;
}

int eMemory::addrBits()
{
    return m_addrBits;
}

void eMemory::setAddrBits( int bits )
{
    if( bits == 0 ) bits = 8;
    m_addrBits = bits;
    
    m_ram.resize( pow( 2, bits ) );
}

int eMemory::dataBits()
{
    return m_dataBits;
}

void eMemory::setDataBits( int bits )
{
    if( bits == 0 ) bits = 8;
    m_dataBits = bits;
    
    m_dataPinState.resize( bits );
}

bool eMemory::persistent()
{
    return m_persistent;
}

void eMemory::setPersistent( bool p )
{
    m_persistent = p;
}

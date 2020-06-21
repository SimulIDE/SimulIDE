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

#include "e-gate.h"
#include "simulator.h"
//#include <QDebug>

bool eGate::m_oscCtrl = false;

eGate::eGate( std::string id, int inputs )
     : eLogicDevice( id )
{
    m_tristate = false;
    m_oscCtrl  = false;
    m_openCol  = false;
    m_oscCount = 0;
}
eGate::~eGate()
{
}

void eGate::stamp()
{
    eLogicDevice::stamp();
    
    for( int i=0; i<m_numInputs; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
}

void eGate::resetState()
{
    m_oscCtrl  = false;
    m_oscCount = 0;
    m_lastStep = Simulator::self()->step();
}

void eGate::setVChanged()
{
    uint64_t step = Simulator::self()->step();

    if( step-m_lastStep < 2 )                // Detect Oscillating gates
    {
        if(( m_oscCount < 10 ) && !m_oscCtrl )
        {
            m_oscCount++;
            if( m_oscCount == 10 ) 
            {
                m_oscCtrl = true;
                return;
            }
        }
    }
    else if( m_oscCount > 0 ) 
    {
        if( m_oscCount == 10 ) m_oscCtrl  = false;
        m_oscCount = 0;
    }
    m_lastStep = step;
    
    if( m_tristate ) eLogicDevice::updateOutEnabled();

    int  inputs = 0;

    for( int i=0; i<m_numInputs; i++ )
    {
        bool  state = eLogicDevice::getInputState( i );

        if( state ) inputs++;
    }
    //qDebug() << "eGate::setVChanged" << inputs <<m_output[0]->imp()<<m_outImp; 

    bool out = calcOutput( inputs );
    
    if( m_openCol ) 
    {
        double imp = m_outImp;
        bool oOut = out;
        if( m_output[0]->isInverted() ) oOut = !out;
        if( oOut || !eLogicDevice::outputEnabled() ) imp = high_imp;
        
        m_output[0]->setImp( imp );
    }
    
    eLogicDevice::setOut( 0, out );// In each gate type
}

bool eGate::calcOutput( int inputs ) 
{ 
    return (inputs==m_numInputs); // Default for: Buffer, Inverter, And, Nand
}

bool eGate::tristate()
{
    return m_tristate;
}
void eGate::setTristate( bool t )
{
    m_tristate = t;
}

bool eGate::openCol()
{
    return m_openCol;
}

void eGate::setOpenCol( bool op )
{
    m_openCol = op;
    
    double imp = m_outImp;
    
    if( op && m_output[0]->out() ) imp = high_imp;
    
    m_output[0]->setImp( imp );
}

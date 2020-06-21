/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "e-clock.h"
#include "simulator.h"
#include "e-source.h"

eClock::eClock( std::string id ) 
      : eElement( id )
{
    setNumEpins(1);
    
    std::stringstream ss;
    ss << m_elmId << "-eSource";
    m_output = new eSource( ss.str(), m_ePin[0] );
    m_output->setVoltHigh( 5 );
    m_output->setVoltLow( 0 );
    m_output->setOut( false );
    
    m_stepsPC = 0;
    m_step = 0;
    setFreq( 1000 );
    
    Simulator::self()->addToSimuClockList( this );
}
eClock::~eClock()
{ 
    delete m_output;
}

void eClock::resetState()
{
    m_step = 0;
}

void eClock::simuClockStep()
{
    m_step++;

    if ( m_step >= m_stepsPC/2 )
    {
        m_output->setOut( !m_output->out() );
        m_output->stampOutput();
        m_step = 0;
    }
}

void eClock::setFreq( double freq )
{
    m_stepsPC = 1e6/freq+0.5;
    
    if( m_stepsPC < 2 ) m_stepsPC = 2;
    
    m_freq = 1e6/m_stepsPC;
}

void eClock::setVolt( double v )
{
    m_output->setVoltHigh( v );
}

ePin* eClock::getEpin( QString pinName )
{
    return m_ePin[0];
}

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

// Inductor model using backward euler  approximation
// consists of a current source in parallel with a resistor.

#include "simulator.h"
#include "e-inductor.h"
#include "e-node.h"

#include <math.h>

eInductor::eInductor( std::string id ) : eResistor( id )
{
    m_ind = 1; // H
    m_tStep = (double)Simulator::self()->reaClock()/1e6;
    m_resist = m_ind/m_tStep;
    m_curSource = 0;
    m_volt = 0;
}
eInductor::~eInductor()
{
}

void eInductor::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToReactiveList(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToReactiveList(this);
    eResistor::stamp();
}

void eInductor::resetState()
{
    double reaClock   = (double)Simulator::self()->reaClock();
    m_tStep = reaClock/1e6;
    
    eResistor::setRes( m_ind/m_tStep );
    
    m_curSource = 0;
}

void eInductor::setVChanged()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();
    
    if( fabs(volt) < 1e-9 ) return;

    m_curSource += volt/m_resist;

    //qDebug() << "eInductor::setVChanged voltdiff " <<volt<<" m_resist "<<m_resist<< " m_curSource "<<m_curSource;

    m_ePin[0]->stampCurrent(-m_curSource );
    m_ePin[1]->stampCurrent( m_curSource );
}

double eInductor::indCurrent()
{
    return m_curSource;
}

double eInductor::ind()
{ 
    return m_ind; 
}
void  eInductor::setInd( double h ) 
{ 
    m_ind = h; 
    eResistor::setResSafe( m_ind/m_tStep ); 
}


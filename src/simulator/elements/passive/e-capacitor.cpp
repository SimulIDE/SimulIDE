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

// Capacitor model using backward euler  approximation
// consists of a current source in parallel with a resistor.

#include "e-capacitor.h"
#include "simulator.h"
#include "e-node.h"

#include <math.h>

eCapacitor::eCapacitor( std::string id ) 
          : eResistor( id )
{
    m_cap = 0.00001; // Farads
    m_resist = m_tStep/m_cap;
    m_curSource = 0;
    m_volt = 0;
}
eCapacitor::~eCapacitor()
{ 
}

void eCapacitor::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToReactiveList(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToReactiveList(this);
    eResistor::stamp();
}

void eCapacitor::resetState()
{
    double reaClock   = (double)Simulator::self()->reaClock();
    m_tStep = reaClock/1e6;
    
    eResistor::setRes( m_tStep/m_cap );
}

void eCapacitor::setVChanged()
{
    double volt = m_ePin[0]->getVolt() - m_ePin[1]->getVolt();
    
    if( fabs(volt) < 1e-9 ) return;

    m_curSource = volt/m_resist;

    //qDebug() << "eCapacitor::setVChanged voltdiff " <<volt<<" m_resist "<<m_resist<< " m_curSource "<<m_curSource;

    m_ePin[0]->stampCurrent( m_curSource );
    m_ePin[1]->stampCurrent(-m_curSource );
}

double eCapacitor::cap()             
{ 
    return m_cap; 
}
void  eCapacitor::setCap( double c ) 
{ 
    m_cap = c; 
    eResistor::setResSafe( m_tStep/m_cap ); 
    
    //qDebug() << "eCapacitor::setCap  m_tStep" << m_tStep << c <<" m_cap "<<m_cap<<" m_resist "<<m_resist;
}

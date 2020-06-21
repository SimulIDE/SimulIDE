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

#include <QDebug>
#include <math.h>   // fabs(x,y)

#include "e-pn.h"
#include "simulator.h"

ePN::ePN( std::string id ) 
   : eResistor(id )
{
    m_threshold = 0.7;
}
ePN::~ePN()
{ 
}

void ePN::stamp()
{
    if( m_ePin[0]->isConnected() )
    {
        eNode* node = m_ePin[0]->getEnode();
        node->addToNoLinList(this);
        node->setSwitched( true );
    }
    if( m_ePin[1]->isConnected() )
    {
        eNode* node = m_ePin[1]->getEnode();
        node->addToNoLinList(this);
        node->setSwitched( true );
    }
    eResistor::stamp();
}

void ePN::resetState()
{
    eResistor::setRes( 0.6 );
    m_accuracy = Simulator::self()->NLaccuracy();
    m_voltPN  = 0;
    m_deltaV  = 0;
    m_current = 0;
}

void ePN::setVChanged()
{
    m_voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    double deltaV = m_threshold;

    if( (m_threshold-m_voltPN) > 1e-6 )
    {
        eResistor::setAdmit( 0 );
        m_ePin[0]->stampCurrent( 0 );
        m_ePin[1]->stampCurrent( 0 );
        m_deltaV = m_voltPN;
        return;
    }
    if( m_admit != 1/m_resist ) eResistor::setAdmit( 1/m_resist );

    //qDebug() <<"ePN::setVChanged,  deltaR: "<< deltaR << "  deltaV" << deltaV << "m_voltPN" << m_voltPN ;

    if( fabs(deltaV-m_deltaV) < m_accuracy/10 )return;

    m_deltaV = deltaV;

    double current = deltaV/m_resist;

    eResistor::stampCurrent( current );
}

void ePN::setThreshold( double threshold )
{
    m_threshold = threshold;
}

void ePN::updateVI()
{
    m_current = 0;

    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
    {
        double volt = m_voltPN - m_deltaV;
        if( volt>0 )
        {
            m_current = volt/m_resist;
            //qDebug() << " current " <<m_current<<volt<<m_deltaV;
        }
    }
}

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

#include <math.h>   // fabs(x,y)
#include <sstream>

#include "e-op_amp.h"
#include "e-source.h"
#include "simulator.h"

eOpAmp::eOpAmp( std::string id )
      : eElement( id )
{
    m_ePin.resize(5);
    m_gain = 1000;
    
    resetState();
}
eOpAmp::~eOpAmp()
{
    delete m_output;
}

void eOpAmp::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToNoLinList(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToNoLinList(this);
    if( m_ePin[2]->isConnected() ) m_ePin[2]->getEnode()->addToNoLinList(this);
}

void eOpAmp::resetState()
{
    m_accuracy = Simulator::self()->NLaccuracy();
    
    m_lastOut = 0;
    m_lastIn  = 0;
    m_k = 1e-6/m_gain;
    m_converged = true;
}

void eOpAmp::setVChanged() // Called when input pins nodes change volt
{
    if( m_powerPins )
    {
        m_voltPos = m_ePin[3]->getVolt();
        m_voltNeg = m_ePin[4]->getVolt();
    }
    else
    {
        m_voltPos = 5;
        m_voltNeg = 0;
    }
    double vd = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();

    //qDebug() << "lastIn " << m_lastIn << "vd " << vd ;
    
    double out = vd * m_gain;
    if     ( out > m_voltPos ) out = m_voltPos;
    else if( out < m_voltNeg ) out = m_voltNeg;
    
    //qDebug() << "lastOut " << m_lastOut << "out " << out << abs(out-m_lastOut)<< "<1e-5 ??";

    if( fabs(out-m_lastOut) < m_accuracy )
    {
        m_converged = true;
        return;
    }

    if( m_converged )                  // First step after a convergence
    {
        double dOut = -1e-6;
        if( vd>0 ) dOut = 1e-6;
        
        out = m_lastOut + dOut;
        m_converged = false;
    }
    else
    {
        if( m_lastIn != vd ) // We problably are in a close loop configuration
        {
            double dIn  = fabs(m_lastIn-vd); // Input diff with last step
            
            // Guess next converging output:
            out = (m_lastOut*dIn + vd*1e-6)/(dIn + m_k);
        }
        m_converged = true;
    }
    
    if     ( out >= m_voltPos ) out = m_voltPos;
    else if( out <= m_voltNeg ) out = m_voltNeg;
    
    //qDebug()<< "lastOut " << m_lastOut << "out " << out << "dOut" << dOut  << "converged" << m_converged;
    
    m_lastIn = vd;
    m_lastOut = out;
    
    //m_output->setVoltHigh(out);
    //m_output->stampOutput();
    m_ePin[2]->stampCurrent( out/cero_doub );
}

double eOpAmp::gain()                {return m_gain;}
void   eOpAmp::setGain( double gain ){m_gain = gain;}

bool eOpAmp::hasPowerPins()          {return m_powerPins;}
void eOpAmp::setPowerPins( bool set ){m_powerPins = set;}

ePin* eOpAmp::getEpin( QString pinName )
{
    ePin* pin = 0l;
    if     ( pinName == "inputInv")  pin = m_ePin[0];
    else if( pinName == "inputNinv") pin = m_ePin[1];
    else if( pinName == "output")    pin = m_ePin[2];
    else if( pinName == "powerPos")  pin = m_ePin[3];
    else if( pinName == "powerNeg")  pin = m_ePin[4];
    return pin;
}

void eOpAmp::initEpins()
{
    setNumEpins(5); 
    
    std::stringstream ss;
    ss << m_elmId << "-eSource";
    m_output = new eSource( ss.str(), m_ePin[2] );
    //m_output->setImp( 40 );
    m_output->setOut( true );
}

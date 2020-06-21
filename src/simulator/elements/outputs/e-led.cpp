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

#include "e-led.h"
#include "simulator.h"

eLed::eLed( std::string id ) 
    : eDiode( id )
{
    m_threshold  = 2.4;
    m_maxCurrent = 0.03;
    resetState();
}
eLed::~eLed() {}

void eLed::initialize()
{
    eDiode::resetState();
}

void eLed::resetState()
{
    m_prevStep   = Simulator::self()->step();
    m_lastCurrent = 0.0;
    m_bright = 25;
    m_disp_brightness  = 0;
    m_avg_brightness   = 0;
    m_lastUpdatePeriod = 0;

    eDiode::resetState();
}

void eLed::setVChanged()
{
    eDiode::setVChanged();
    updateVI();
}
void eLed::updateVI()
{
    eDiode::updateVI();
    
    const uint64_t step = Simulator::self()->step();
    int period = step - m_prevStep;    

    m_prevStep = step;
    m_lastUpdatePeriod += period;

    if( m_lastCurrent > 0) m_avg_brightness += m_lastCurrent * period / m_maxCurrent;
    
    m_lastCurrent = m_current;

    //qDebug()<<"current"<< m_current<<m_lastCurrent<<period<< m_lastUpdatePeriod <<m_avg_brightness;
    //label->setText( QString("%1 A"). arg(double(int(m_current*1000))/1000) );
}

void eLed::updateBright()
{
    if( !Simulator::self()->isRunning() )
    {
        m_avg_brightness = 0;
        m_lastUpdatePeriod = 0;
        m_bright = 25;
        return;
    }
    updateVI();

    if( m_lastUpdatePeriod > Simulator::self()->circuitRate() )
    {
        m_disp_brightness = m_avg_brightness/m_lastUpdatePeriod;
        
        m_disp_brightness = pow( m_disp_brightness, 1.0/2.0 );

        m_avg_brightness   = 0;
        m_lastUpdatePeriod = 0;
        m_bright = uint(m_disp_brightness*255)+25;
    }
    //qDebug()<<"current"<< m_current<<m_lastCurrent<<m_lastUpdatePeriod;
    //qDebug() << m_bright;
}

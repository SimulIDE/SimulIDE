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

#include "clock-base.h"
#include "pin.h"
#include "simulator.h"

static const char* ClockBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Freq")
};

ClockBase::ClockBase( QObject* parent, QString type, QString id )
         : LogicInput( parent, type, id )
{
    Q_UNUSED( ClockBase_properties );
    
    m_area = QRect( -14, -8, 22, 16 );
    
    m_isRunning = false;

    m_stepsPC = 0;
    m_step = 0;
    setFreq( 1000 );

    Simulator::self()->addToUpdateList( this );
}
ClockBase::~ClockBase(){}

void ClockBase::stamp()
{
    setFreq( m_freq );
}

void ClockBase::updateStep()
{
    if( m_changed )
    {
        if( m_isRunning ) Simulator::self()->addToSimuClockList( this );
        else
        {
            m_out->setOut( false );
            Simulator::self()->remFromSimuClockList( this );
        }
        LogicInput::updateStep();
    }
}

double ClockBase::freq() { return m_freq; }

void ClockBase::setFreq( double freq )
{
    double stepsPerS = Simulator::self()->stepsPerus()*1e6;

    m_stepsPC = stepsPerS/freq+0.5;
    
    if( m_stepsPC < 2 ) m_stepsPC = 2;
    
    m_freq = stepsPerS/m_stepsPC;
    
    //qDebug() << "ClockBase::setFreq"<<freq<<m_freq<<m_stepsPC;
    
    emit freqChanged();
}

void ClockBase::rateChanged()
{
    setFreq( m_freq );
}

bool ClockBase::running() { return m_isRunning; }

void ClockBase::setRunning( bool running )
{
    m_isRunning = running;
    m_step = 0;
    m_changed = true;
    //updateStep();
    //qDebug() << m_stepsPC << m_isRunning ;
}

void ClockBase::onbuttonclicked()
{
    setRunning( !m_isRunning );
}

void ClockBase::remove()
{
    Simulator::self()->remFromSimuClockList( this );

    LogicInput::remove();
}

#include "moc_clock-base.cpp"


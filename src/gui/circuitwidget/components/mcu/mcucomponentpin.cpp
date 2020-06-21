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

#include "mcucomponentpin.h"
#include "baseprocessor.h"

McuComponentPin::McuComponentPin( McuComponent* mcuComponent, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
               : QObject( mcuComponent )
               , eSource( id.toStdString(), 0l )
{
    //qDebug()<<"McuComponentPin::McuComponentPin"<<id;
    m_mcuComponent = mcuComponent;

    m_id    = id;
    m_type  = type;
    m_angle = angle;
    m_pos   = pos;
    
    m_pinType = 0;
    
    m_attached = false;
    m_isInput  = true;
    m_openColl = false;

    m_inputImp = high_imp;

    Pin* pin = new Pin( angle, QPoint (xpos, ypos), mcuComponent->itemID()+"-"+id, pos, m_mcuComponent );
    pin->setLabelText( label );
    m_ePin[0] = pin;

    eSource::setImp( m_inputImp );
    eSource::setVoltHigh( 5 );
    eSource::setOut( false );
    
    type = type.toLower();
    if( type == "gnd" 
     || type == "vdd" 
     || type == "vcc" 
     || type == "unused" 
     || type == "nc" ) 
     pin->setUnused( true );
     
    resetState();
}

McuComponentPin::~McuComponentPin()
{
}

void McuComponentPin::stamp()
{
    //if( m_pinType == 1 ) eSource::setImp( m_inputImp );// All  IO Pins should be inputs at start-up

    if( m_ePin[0]->isConnected() && m_attached )
        m_ePin[0]->getEnode()->addToChangedFast(this);

    //if( m_pinType == 21 ) BaseProcessor::self()->hardReset( true );
    eSource::stamp();
}

void McuComponentPin::resetState()
{
    if( m_pinType == 1 )
    {
        m_inputImp = high_imp ;
        eSource::setImp( high_imp );// All  IO Pins should be inputs at start-up
        eSource::setOut( false );
        eSource::stamp();
    }
}

void McuComponentPin::terminate()
{
    m_attached = false;
}

void McuComponentPin::setDirection( bool out )
{
    //qDebug() << "McuComponentPin::setDirection "<< m_id << out;
    m_isInput = !out;

    if( out )                                     // Set Pin to Output
    {
        eSource::setImp( 40 );

        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->remFromChangedFast(this);

        if( m_state != m_out ) setOutput( m_state );
    }
    else                                          // Set Pin to Input
    {
        eSource::setImp( m_inputImp );

        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->addToChangedFast(this);
    }
    // Run Extra Step when current Processor Step is finished
    BaseProcessor::self()->p_runExtStep = true;
}

void McuComponentPin::setState( bool state )
{
    //qDebug() << "McuComponentPin::setState "<< m_id << state << m_openColl;

    if( state == m_state ) return;
    m_state = state;

    if( m_isInput )  return;

    setOutput( state );
}

void McuComponentPin::setOutput( bool state )
{
    if( m_openColl )
    {
        if( state ) eSource::setImp( m_inputImp );
        else        eSource::setImp( 40 );

        eSource::stamp();
    }
    else
    {
        eSource::setOut( state );
        eSource::stampOutput();
    }
    // Run Extra Step when current Processor Step is finished
    BaseProcessor::self()->p_runExtStep = true;
}

void McuComponentPin::setPullup( bool up )
{
    m_inputImp = up? 1e5:high_imp;

    if( !m_isInput ) return;

    //qDebug() << "McuComponentPin::setPullup "<< m_id << pullup;

    if( up )                               // Activate pullup
    {
        eSource::setImp( 1e5 );
        m_voltOut = m_voltHigh;
    }
    else                                 // Deactivate pullup
    {
        eSource::setImp( m_inputImp );
        m_voltOut = m_voltLow;
    }
    if( !(m_ePin[0]->isConnected()) )
    {
        pullupNotConnected( up );
        return;
    }
    m_ePin[0]->stampCurrent( m_voltOut/m_imp );

    // Run Extra Step when current Processor Step is finished
    BaseProcessor::self()->p_runExtStep = true;
}

void McuComponentPin::resetOutput()
{
    eSource::setOut(false );
    eSource::stampOutput();
}

void McuComponentPin::move( int dx, int dy )
{
    pin()->moveBy( dx, dy );
}

#include "moc_mcucomponentpin.cpp"

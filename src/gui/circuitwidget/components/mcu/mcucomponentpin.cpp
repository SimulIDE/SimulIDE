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
    m_isInput  = false;
    m_openColl = false;

    Pin* pin = new Pin( angle, QPoint( xpos, ypos ), mcuComponent->itemID()+"-"+id, pos, m_mcuComponent );
    pin->setLabelText( label );
    m_ePin[0] = pin;

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
    if( m_pinType == 1 )
    {
        if( m_isInput )
        {
            if( m_ePin[0]->isConnected() && m_attached )        // Receive voltage change notifications
                m_ePin[0]->getEnode()->addToChangedFast(this);

            if( (m_pupAdmit>0) && !(m_ePin[0]->isConnected()) ) // Pullup ?
                pullupNotConnected( true );

        }
        update();
    }
    else eSource::stamp();
}

void McuComponentPin::resetState()
{
    if( m_pinType == 1 )
    {
        m_state   = false;
        m_isInput = true;

        m_vddAdmit = 0;
        m_gndAdmit = cero_doub;
        m_vddAdmEx = 0;
        m_gndAdmEx = 0;
        m_pupAdmit = 0;

        eSource::setVoltHigh( 5 );
        update();
    }
}

void McuComponentPin::setDirection( bool out )
{
    //qDebug() << "McuComponentPin::setDirection "<< m_id << out;
    m_isInput = !out;

    if( out )       // Set Pin to Output
    {
        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->remFromChangedFast(this); // Don't Receive voltage change notifications

        eSource::setImp( 40 );
        setState( m_state );
    }
    else           // Set Pin to Input
    {
        if( m_ePin[0]->isConnected() && m_attached )
            m_ePin[0]->getEnode()->addToChangedFast(this); // Receive voltage change notifications

        m_vddAdmit = 0;
        m_gndAdmit = cero_doub;
        update();

        BaseProcessor::self()->setExtraStep(); // Run Extra Simulation Step
    }
}

void McuComponentPin::setState( bool state )
{
    //qDebug() << "McuComponentPin::setState "<< m_id << state << m_openColl;

    if( state == m_state ) return;
    m_state = state;

    if( m_isInput )  return;

    if( m_openColl )
    {
        if( state )
        {
            m_vddAdmit = 0;
            m_gndAdmit = cero_doub;
        }
        else
        {
            m_vddAdmit = 0;
            m_gndAdmit = 1./40.;
        }
        update();
    }
    else
    {
        eSource::setOut( state );
        eSource::stampOutput();
    }
    BaseProcessor::self()->setExtraStep(); // Run Extra Simulation Step
}

void McuComponentPin::update()
{
    double vddAdmit = m_vddAdmit+m_vddAdmEx+m_pupAdmit;
    double gndAdmit = m_gndAdmit+m_gndAdmEx;
    double Rth  = 1/(vddAdmit+gndAdmit);

    m_voltOut = 5*vddAdmit*Rth;

    eSource::setImp( Rth );
}

void McuComponentPin::setPullup( bool up )
{
    if( !m_isInput ) return;

    //qDebug() << "McuComponentPin::setPullup "<< m_id << pullup;

    if( up ) m_pupAdmit = 1/1e5; // Activate pullup
    else     m_pupAdmit = 0;     // Deactivate pullup

    if( !(m_ePin[0]->isConnected()) )
    {
        pullupNotConnected( up );
        return;
    }
    update();

    BaseProcessor::self()->setExtraStep(); // Run Extra Simulation Step
}

void McuComponentPin::setExtraSource( double vddAdmit, double gndAdmit )
{
    m_vddAdmEx = vddAdmit;
    m_gndAdmEx = gndAdmit;
    update();
}

void McuComponentPin::move( int dx, int dy )
{
    pin()->moveBy( dx, dy );
}

#include "moc_mcucomponentpin.cpp"

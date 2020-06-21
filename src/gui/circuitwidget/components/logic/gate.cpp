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

#include "connector.h"
#include "circuit.h"
#include "gate.h"


Gate::Gate( QObject* parent, QString type, QString id, int inputs )
    : Component( parent, type, id )
    , eGate( id.toStdString(), 0 )
{
    setNumInps( inputs );                           // Create Input Pins
    
    m_outputPin = new Pin( 0, QPoint( 16,-8+0*16+8 )
                          , m_id+"-out", 1, this );
                          
    eLogicDevice::createOutput( m_outputPin );
}
Gate::~Gate(){
}

void Gate::remove()
{
    for( int i=0; i<m_numInputs; i++ )
        if( m_inputPin[i]->isConnected() ) m_inputPin[i]->connector()->remove();

    if( m_outputPin->isConnected() ) m_outputPin->connector()->remove();
    
    Component::remove();
}

void Gate::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;

    for( int i=0; i<m_numInputs; i++ )
    {
        Pin* pin = m_inputPin[i];
        if( pin->isConnected() ) pin->connector()->remove();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        pin->reset();
        delete pin;
    }
    eLogicDevice::deleteInputs( m_numInputs );

    m_inputPin.resize( inputs );

    for( int i=0; i<inputs; i++ )
    {
        m_inputPin[i] = new Pin( 180, QPoint(-8-8,-4*inputs+i*8+4 )
                               , m_id+"-in"+QString::number(i), i, this );

        eLogicDevice::createInput( m_inputPin[i] );
    }
    m_area = QRect( -20, -4*m_numInputs, 32, 4*2*m_numInputs );
    
    Circuit::self()->update();
}

void Gate::setInverted( bool inverted )
{
    eLogicDevice::setInverted( inverted );
    Circuit::self()->update();
}

#include "moc_gate.cpp"

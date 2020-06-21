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

#include <math.h>
//#include <QDebug>

#include "e-dac.h"

eDAC::eDAC( std::string id )
    : eLogicDevice( id )
{
}
eDAC::~eDAC()
{ 
}

void eDAC::stamp()
{
    for( int i=0; i<m_numInputs; i++ )
    {
        eNode* enode = m_input[i]->getEpin()->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    m_output[0]->setOut( true );
    m_address = -1;
}

void eDAC::setVChanged()
{
    int address = 0;

    for( int i=0; i<m_numInputs; i++ )
    {
        if( getInputState( i ) ) address += pow( 2, m_numInputs-1-i );
    }
    if( address == m_address ) return;
    m_address = address;

    double v = m_maxVolt*address/m_maxAddr;

    qDebug() << "eDAC::setVChanged" << v << address<<m_maxAddr<<m_maxVolt;
    
    m_output[0]->setVoltHigh( v );
    m_output[0]->stampOutput();
}

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

#include <math.h>
#include <QDebug>

#include "e-bus.h"

eBus::eBus( std::string id )
    : eElement( id )
{
    m_numLines = 0;
    m_startBit = 0;
}
eBus::~eBus()
{ 
}

void eBus::setStartBit( int bit ) 
{ 
    if( bit < 0 ) bit = 0;
    m_startBit = bit; 
}

void eBus::initEpins()
{
    setNumEpins( m_numLines+1 );
}

void eBus::setNumLines( int lines )
{
    if( lines == m_numLines ) return;
    if( lines < 1 ) lines = 1;
    
    m_numLines = lines;
}

void eBus::setVChanged()
{

}

ePin* eBus::getEpin( QString pinName )
{
    //qDebug() << "eBus::getEpin" << pinName;
    if( pinName == "busPin1") return m_ePin[m_numLines+1];

    return eElement::getEpin( pinName );
}


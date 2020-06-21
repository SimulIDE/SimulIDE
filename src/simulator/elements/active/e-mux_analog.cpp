/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "e-mux_analog.h"
#include "e-resistor.h"
#include "e-node.h"
#include <QDebug>
#include <sstream>

eMuxAnalog::eMuxAnalog( std::string id )
          : eElement( id )
{
    m_channels = 0;
    m_addrBits = 0;
    m_address  = 0;
    m_admit = 1;
}
eMuxAnalog::~eMuxAnalog()
{ 
    for( eResistor* res : m_resistor ) delete res;
    for( ePin* pin : m_chanPin ) delete pin;
    for( ePin* pin : m_addrPin ) delete pin;
}

void eMuxAnalog::stamp()
{
    eNode* enode = m_inputPin->getEnode();
    if( enode ) enode->setSwitched( true );
    
    for( int i=0; i<m_channels; i++ ) 
    {
        m_ePin[i]->setEnode( enode );
        
       eNode* node = m_chanPin[i]->getEnode();
       if( node ) node->setSwitched( true );
    }
    
    enode = m_enablePin->getEnode();
    if( enode ) enode->addToChangedFast(this);
    
    for( int i=0; i<m_addrBits; i++ )
    {
        enode = m_addrPin[i]->getEnode();
        if( enode ) enode->addToChangedFast(this);
    }
    m_enabled = false;
}

void eMuxAnalog::setVChanged()
{
    bool enabled = m_enablePin->getVolt() < 2.5;
    
    if( enabled != m_enabled )
    {
        m_enabled = enabled;
        
        if( !enabled ) 
        {
            for( int i=0; i<m_channels; i++ ) 
            {
                if( m_resistor[i]->admit() != 0 )
                {
                    m_resistor[i]->setAdmit( 0 );
                    //qDebug() << "eMuxAnalog::setVChanged disabling:"<<i;
                }
            }
        }
    }
    if( !enabled ) return;
    
    int address = 0;
    for( int i=0; i<m_addrBits; i++ )
    {
        
        bool state = (m_addrPin[i]->getVolt()>2.5);
        if( state ) address += pow( 2, i );
    }
    //if( address == m_address ) return;
    m_address = address;
    
    //qDebug() << "eMuxAnalog::setVChanged"<<address<<m_admit;
    for( int i=0; i<m_channels; i++ )
    {
        if( i == address )                
        {
            if( m_resistor[i]->admit() == 0 ) 
            {
                m_resistor[i]->setAdmit( m_admit );
                //qDebug() << "eMuxAnalog::setVChanged connecting:"<<i;
            }
        }
        else if( m_resistor[i]->admit() != 0 ) 
        {
            m_resistor[i]->setAdmit( 0 );
            //qDebug() << "eMuxAnalog::setVChanged disconnecting:"<<i;
        }
    }
}

double eMuxAnalog::resist() 
{ 
    return 1/m_admit; 
}

void eMuxAnalog::setResist( double r )
{
    m_admit = 1/r;
}

void eMuxAnalog::setBits( int bits )
{ 
    std::stringstream si;
    si << m_elmId << "-PinInput";
    m_inputPin = new ePin( si.str(), 0 );
    std::stringstream se;
    se << m_elmId << "-PinEnable";
    m_enablePin = new ePin( se.str(), 0 );
    
    if( bits < 1 ) bits = 1;
    
    m_addrBits = bits;
    m_addrPin.resize( bits );

    for( int i=0; i<m_addrBits; i++ )
    {
        std::stringstream ss;
        ss << m_elmId << "-pinAddr" << i;
        m_addrPin[i] = new ePin( ss.str(), i );
    }
    m_channels = pow( 2, bits );
    m_resistor.resize( m_channels );
    m_chanPin.resize( m_channels );
    //m_ePin.resize( m_channels );
    setNumEpins( m_channels );

    for( int i=0; i<m_channels; i++ )
    {
        std::stringstream ss;
        ss << m_elmId << "-resistor" << i;
        m_resistor[i] = new eResistor( ss.str() );
        
        m_resistor[i]->setEpin( 0, m_ePin[i] );
        
        std::stringstream sl;
        sl<< m_elmId << "-pinY" << i;
        m_chanPin[i] = new ePin( sl.str(), i );
        m_resistor[i]->setEpin( 1, m_chanPin[i] );

        m_resistor[i]->setAdmit( 0 );
    }
    //qDebug() << "eMuxAnalog::setBits"<<m_ePin.size();
}

ePin* eMuxAnalog::getEpin( QString pinName )
{
    //qDebug() << "eElement::getEpin" << pinName;
    if     ( pinName == "PinInput")  return m_inputPin;
    else if( pinName == "PinEnable") return m_enablePin;
    
    else if( pinName.contains("pinY") )
    {
        int pin = pinName.remove("pinY").toInt();
        if( pin < m_channels ) return m_chanPin[pin];
    }
    else if( pinName.contains("pinAddr") ) 
    {
        int pin = pinName.remove("pinAddr").toInt();
        if( pin < m_addrBits ) return m_addrPin[pin];
    }
    return 0l;
}


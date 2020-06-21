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

#include <sstream>

#include "e-source.h"
#include "simulator.h"

eSource::eSource( std::string id, ePin* epin )
       : eElement( id )
{
    m_ePin.resize(1);
    m_ePin[0] = epin;
    m_out     = false;
    m_inverted = false;

    m_voltHigh = cero_doub;
    m_voltLow  = cero_doub;
    m_voltOut  = cero_doub;
    m_imp      = cero_doub;
    m_admit    = 1/m_imp;

    QString nodId = QString::fromStdString(id);

    m_scrEnode = new eNode( nodId+"scr" );
    m_scrEnode->setNodeNumber(0);

    Simulator::self()->remFromEnodeList( m_scrEnode, /*delete=*/ false );
}
eSource::~eSource(){ delete m_scrEnode; }

/*void eSource::setEpin( ePin* epin )
{
    m_ePin[0] = epin;
}*/

void eSource::createPin()
{
    std::stringstream sspin;
    sspin << m_elmId << "-ePin0";
    m_ePin[0] = new ePin( sspin.str(), 0 );
}

void eSource::initialize()
{
    m_ePin[0]->setEnodeComp( m_scrEnode );
    stamp();
}

void eSource::stamp()
{
    //qDebug() <<"eSource::stamp"<<QString::fromStdString(m_elmId)<< m_out;
    m_ePin[0]->stampAdmitance( m_admit );
    stampOutput();
}

void eSource::stampOutput()                               // Stamp Output
{
    //qDebug() <<"eSource::stampOutput"<<QString::fromStdString(m_elmId)<< m_out<<m_voltOut<<m_imp ;
    m_scrEnode->setVolt( m_voltOut );

    m_ePin[0]->stampCurrent( m_voltOut/m_imp );
}

void eSource::setVoltHigh( double v )
{
    m_voltHigh = v;
    if( m_out ) m_voltOut = v;
}
double eSource::voltHight() { return m_voltHigh; }

void eSource::setVoltLow( double v )
{
    m_voltLow = v;
    if( !m_out ) m_voltOut = v;
}
double eSource::voltLow() { return m_voltLow; }

void eSource::setOut( bool out )           // Set Output to Hight or Low
{
    if( m_inverted ) m_out = !out;
    else             m_out =  out;

    if( m_out ) m_voltOut = m_voltHigh;
    else        m_voltOut = m_voltLow;
    
    //qDebug() << "eSource::setOut"<<QString::fromStdString(m_elmId)<<m_voltOut;
}
bool  eSource::out() { return m_out; }

void eSource::setInverted( bool inverted )
{
    if( inverted == m_inverted ) return;

    m_inverted = inverted;
    m_ePin[0]->setInverted( inverted );
    
    m_out = !m_out;
    if( m_out ) m_voltOut = m_voltHigh;
    else        m_voltOut = m_voltLow;
    
    stampOutput();
}
bool eSource::isInverted() { return m_inverted; }

void eSource::setImp( double imp )
{
    m_imp = imp;
    m_admit = 1/m_imp;
    eSource::stamp();
}

double eSource::imp() { return m_imp; }

ePin* eSource::getEpin()
{
    return m_ePin[0];
}

ePin* eSource::getEpin( QString pinName )
{
    pinName ="";
    return m_ePin[0];
}

double eSource::getVolt()
{
    //qDebug() << "eSource::getVolt()"<<QString::fromStdString(m_elmId)<< m_ePin[0]->isConnected() <<m_voltOut;
    
    if( m_ePin[0]->isConnected() ) return m_ePin[0]->getVolt();
    else                           return m_voltOut;
}

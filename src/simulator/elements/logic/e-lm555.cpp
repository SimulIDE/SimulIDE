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

//#include <QDebug>
#include <sstream>

#include "e-lm555.h"
#include "e-source.h"

eLm555::eLm555( std::string id )
      : eElement( id )
{
    setNumEpins(8);
    resetState();
}
eLm555::~eLm555()
{ 
    delete m_output;
    delete m_cv;
    delete m_dis;
}

void eLm555::stamp()
{
    for( int i=0; i<8; i++ )
    {
        if( i == 2 ) continue; // Output
        if( i == 6 ) continue; // Discharge
        
        if( m_ePin[i]->isConnected() ) m_ePin[i]->getEnode()->addToNoLinList(this);
    }
}

void eLm555::resetState()
{
    m_outState = false;
    m_volt = 0;
}

void eLm555::setVChanged()
{
    double voltPos = m_ePin[7]->getVolt();
    double voltNeg = m_ePin[0]->getVolt();
    double volt = voltPos - voltNeg;
    
    double reftTh = m_ePin[4]->getVolt();
    double reftTr = reftTh/2;
    
    if( volt != m_volt )
    {
        m_volt = volt;
        m_cv->setVoltHigh( volt*2/3 );
        m_cv->stampOutput();
    }
    double voltTr = m_ePin[1]->getVolt();
    double voltTh = m_ePin[5]->getVolt();
    
    double voltRst = m_ePin[3]->getVolt();
    
    bool reset = ( voltRst < (voltNeg+0.7) );
    bool th    = ( voltTh > reftTh );
    bool tr    = ( reftTr > voltTr );
    
    bool outState = m_outState;
    
    if( reset )
    {
        outState = false;
    }
    else if( tr )
    {
        outState =  true;
    }
    else if( !tr && th )
    {
        outState =  false;
    }
    //qDebug() << "eLm555::setVChanged" << outState<<"th"<<th<<"tr"<<tr;
    if( outState != m_outState )
    {
        m_outState = outState;
        
        double voltHight = voltNeg;
        if( outState ) 
        {
            voltHight = voltPos - 1.7;
            if( voltHight < voltNeg ) voltHight = voltNeg;
            m_dis->setVoltHigh( voltNeg );
            m_dis->setImp( high_imp );
        }
        else
        {
            m_dis->setVoltHigh( voltNeg );
            m_dis->setImp( 1 );
        }
        m_output->setVoltHigh( voltHight );
        m_output->stampOutput();
        //qDebug() << "eLm555::setVChanged" << outState<<reset<<th<<tr;
    }
}

void eLm555::initEpins()
{//qDebug() << "eLm555::initEpins";
    setNumEpins(8); 
    
    std::stringstream ss;
    ss << m_elmId << "-out-eSource";
    m_output = new eSource( ss.str(), m_ePin[2] );
    m_output->setImp( 10 );
    m_output->setOut( true );
    
    std::stringstream ss1;
    ss1 << m_elmId << "-cv-eSource";
    m_cv = new eSource( ss1.str(), m_ePin[4] );
    m_cv->setImp( 10 );
    m_cv->setOut( true );
    
    std::stringstream ss2;
    ss2 << m_elmId << "-dis-eSource";
    m_dis = new eSource( ss2.str(), m_ePin[6] );
    m_dis->setImp( high_imp );
    m_dis->setOut( false );
}

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

#include <math.h>   // fabs(x,y)
#include <sstream>
#include <QDebug>
#include <ctime>

#include "e-bjt.h"
#include "e-node.h"
#include "simulator.h"

eBJT::eBJT( std::string id )
    : eResistor( id )
{
    m_ePin.resize(3);
    
    m_gain = 100;
    m_PNP = false;
    m_BCdiodeOn = false;
    
    std::stringstream ssa;
    ssa << m_elmId << "-BEdiode";
    m_BEdiode = new ePN( ssa.str() );
    m_BEdiode->initEpins();
    setBEthr( 0.7 );
    
    std::stringstream ssb;
    ssb << m_elmId << "-BCdiode";
    m_BCdiode = new eDiode( ssb.str() );
    m_BCdiode->initEpins();
}
eBJT::~eBJT()
{ 
    delete m_BEdiode;
    delete m_BCdiode;
}

void eBJT::stamp()
{
     // C
    {
        eNode* enod0 = m_ePin[0]->getEnode();
        if( enod0 )
        {
            enod0->addToNoLinList(this);
            enod0->setSwitched( true );
        }
    }
     // E
    {
        eNode* enod1 = m_ePin[1]->getEnode();
        if( enod1 )
        {
            enod1->addToNoLinList(this);
            enod1->setSwitched( true );
        }
    }
     // B
    {
        eNode* enod2 = m_ePin[2]->getEnode();
        if( enod2 ) enod2->addToNoLinList(this);
    }
    eResistor::stamp();
}

void eBJT::attach()
{
     // C
    {
        eNode* enod0 = m_ePin[0]->getEnode();
        if( m_BCdiodeOn )
        {
            if( m_PNP ) m_BCdiode->getEpin( 0 )->setEnode( enod0 );
            else        m_BCdiode->getEpin( 1 )->setEnode( enod0 );
        }
    }
     // E
    {
        eNode* enod1 = m_ePin[1]->getEnode();
        if( m_PNP ) m_BEdiode->getEpin( 0 )->setEnode( enod1 );
        else        m_BEdiode->getEpin( 1 )->setEnode( enod1 );
    }
     // B
    {
        eNode* enod2 = m_ePin[2]->getEnode();
        if( m_PNP )
        {
            m_BEdiode->getEpin( 1 )->setEnode( enod2 );
            if( m_BCdiodeOn ) m_BCdiode->getEpin( 1 )->setEnode( enod2 );
        }
        else
        {
            m_BEdiode->getEpin( 0 )->setEnode( enod2 );
            if( m_BCdiodeOn ) m_BCdiode->getEpin( 0 )->setEnode( enod2 );
        }
    }
}

void eBJT::resetState()
{
    //eResistor::setRes( 400/m_gain );
    eResistor::setAdmit( 0 );
    eResistor::stamp();

    m_BEdiode->resetState();
    if( m_BCdiodeOn ) m_BCdiode->resetState();

    m_accuracy = Simulator::self()->NLaccuracy();
    //m_stage = 0;
    m_lastOut = 0;
    m_baseCurr = 0;
    m_voltE = 0;
    m_Efollow = false;

    srand(time(NULL));
}

void eBJT::setVChanged() 
{
    double voltCE;
    double voltBE;
    double voltC = m_ePin[0]->getVolt();
    double voltE = m_ePin[1]->getVolt();
    double voltB = m_ePin[2]->getVolt();
    //qDebug()<<voltC<<voltE<<voltB;

    if( m_PNP )
    {
        voltCE = voltE-voltC;
        voltBE = voltE-voltB;
    }
    else
    {
        voltCE = voltC-voltE;
        voltBE = voltB-voltE;
    }
    if( voltCE < cero_doub ) voltCE = cero_doub;
    
    double maxCurrCE = voltCE/m_resist;
    double current = maxCurrCE;
    
    if( !m_Efollow )
    {
        if( m_PNP )
        {
            if( fabs( voltE-m_voltE )<0 ){ m_Efollow = true; }
        }
        else if(( fabs(m_voltE) > 1e-3 )&&( m_voltE != voltE ))
        { 
            m_Efollow = true; 
            eResistor::setRes( 400/m_gain );
            eResistor::stamp();
        }
        m_voltE = voltE;
    }
    double satK = 0;

    if( voltCE < voltBE )          
    {
        satK = voltCE/voltBE-1;
        satK = pow( satK, 2 );
    }
    m_baseCurr = m_BEdiode->current();

    double currentCE = m_baseCurr*m_gain*(1+voltCE/75);
    currentCE -= currentCE*satK;
    
    //qDebug()<<"m_baseCurr"<<m_baseCurr<<"    currentCE"<<currentCE<<"     maxCurrCE"<<maxCurrCE<<"     voltBE"<<voltBE <<"    m_Efollow"<<m_Efollow;

    if( m_Efollow )
    {
        current = maxCurrCE-currentCE;

        //double r = (double)(std::rand() %10+1);
        
        current = m_lastOut+(current-m_lastOut)/2;//(3.5+1.5/r);

        if( current < 0 ) current = 0;
        
        //qDebug()<<"current"<<current<<"  m_baseCurr"<<m_baseCurr<<"  currentCE"<<currentCE<<"  maxCurrCE"<<maxCurrCE<<"  voltBE"<<voltBE <<" m_Efollow"<<m_Efollow;

        if( m_PNP ) current = -current;

        double accuracy = m_accuracy/5;

        if( fabs(current-m_lastOut) < accuracy ) return;
        
        m_lastOut = current;
        m_ePin[0]->stampCurrent( current );
        m_ePin[1]->stampCurrent(-current );
    }
    else
    {
        double admit = 0;
        if( currentCE!=0 && voltCE!=0 ) admit = currentCE/voltCE;
        if( admit < 0 ) admit = 0;
        //admit = m_lastOut+(admit-m_lastOut)/2;

        double r = (double)(std::rand() %10+1);

        admit = m_lastOut + (admit-m_lastOut)/r;

        //admit *= 1.1;
        
        if( fabs(admit-m_lastOut) < m_accuracy ) return;
        
        eResistor::setAdmit( admit );
        eResistor::stamp();
        m_lastOut = admit;
        //qDebug()<< QString::fromStdString( m_elmId )<<" voltBE"<<voltBE << " admit" <<admit;
    }
}

double eBJT::BEthr()
{
    return m_BEthr;
}

void eBJT::setBEthr( double thr )
{
    if( thr == 0 ) thr = 0.7;
    m_BEdiode->setThreshold( thr );
    m_BEthr = thr;
}

void eBJT::setBCd( bool bcd ) 
{ 
    if( !bcd )
    {
        m_BCdiode->getEpin( 0 )->setEnode( 0l );
        m_BCdiode->getEpin( 1 )->setEnode( 0l );
    }
    m_BCdiodeOn = bcd; 
}

void eBJT::initEpins()
{
    std::stringstream sd;
    sd << m_elmId << "-collector";
    m_ePin[0] = new ePin( sd.str(), 0 );
    
    std::stringstream ss;
    ss << m_elmId << "-emiter";
    m_ePin[1] = new ePin( ss.str(), 1 ); 
    
    std::stringstream sg;
    sg << m_elmId << "-base";
    m_ePin[2] = new ePin( sg.str(), 2 );  
}

ePin* eBJT::getEpin( QString pinName )
{
    ePin* pin = 0l;
    if     ( pinName == "collector") pin = m_ePin[0];
    else if( pinName == "emiter")    pin = m_ePin[1];
    else if( pinName == "base")      pin = m_ePin[2];
    return pin;
}



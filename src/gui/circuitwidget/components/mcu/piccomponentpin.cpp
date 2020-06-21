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

#include <qstring.h>
#include <stdint.h>

#include "piccomponentpin.h"
#include "piccomponent.h"
#include "simulator.h"

//#include "stimuli.h"
//#include "ioports.h"
#include "pic-processor.h"
//#include "gpsim_time.h"

PICComponentPin::PICComponentPin( McuComponent* mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
               : McuComponentPin( mcu, id, type, label, pos, xpos, ypos, angle )
{
    m_PicProcessor  = 0l;
    m_pIOPIN        = 0l;
}
PICComponentPin::~PICComponentPin(){}

void PICComponentPin::attach( pic_processor *PicProcessor )
{
    if( m_PicProcessor ) return;
    
    m_PicProcessor = PicProcessor;
    
    IOPIN* iopin = m_PicProcessor->get_pin( m_pos );

    if( m_id.startsWith("R") || m_id.startsWith("GP") )
    {
        m_pinType = 1;
        
        m_port = m_id.at(1).toLatin1();
        m_pinN = m_id.mid(2,1).toInt();

        if( !iopin )
        {
            qDebug() << "PICComponentPin::attach : iopin is NULL: "<< m_id << endl;
            return;
        }
        if( m_pIOPIN )
        {
            qDebug() << "PICComponentPin::attach :Already have an iopin" << endl;
            return;
        }
        m_pIOPIN = iopin;
        m_pIOPIN->setPicPin( this );

        if( m_pIOPIN->getType() == OPEN_COLLECTOR )
        {
            m_openColl = true;
            eSource::setVoltHigh( 0 );
        }
    }
    else if( m_id.startsWith("MCLR") )
    {
        m_pinType = 21;
    }
    m_attached = true;
}

void PICComponentPin::setVChanged()
{
    if( !m_isInput ) return;      // Nothing to do if pin is output

    double volt = m_ePin[0]->getVolt();
    //qDebug() << "PICComponentPin::setVChanged "<< m_id <<volt;
    
    if( m_pinType == 1 )                                 // Is an IO Pin
    {
        m_pIOPIN->set_nodeVoltage(volt);
    }
    else if( m_pinType == 21 ) // reset
    {
        if( volt < 3 )  BaseProcessor::self()->hardReset( true );
        else            BaseProcessor::self()->hardReset( false );
    }
}

void PICComponentPin::pullupNotConnected( bool up )
{
    m_pIOPIN->set_nodeVoltage( up? 5:0);
}


#include "moc_piccomponentpin.cpp"

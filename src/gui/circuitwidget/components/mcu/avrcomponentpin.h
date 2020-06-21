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

#ifndef AVRCOMPONENTPIN_H
#define AVRCOMPONENTPIN_H

#include <stdint.h>

#include "mcucomponentpin.h"

//simavr includes
#include "sim_avr.h"
#include "sim_irq.h"
#include "sim_io.h"
#include "avr_adc.h"
#include "avr_ioport.h"
#include "avr_timer.h"


class AVRComponentPin : public McuComponentPin
{
    Q_OBJECT
    public:
        AVRComponentPin( McuComponent *mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        ~AVRComponentPin();

        virtual void attach( avr_t* AvrProcessor );
        virtual void setVChanged();
        virtual void resetState();

        virtual void pullupNotConnected( bool up );
        //virtual void resetOutput();

        void adcread();
        void enableIO( bool en );

        static void ddr_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);

            ptrAVRComponentPin->setDirection( value>0 );
        }

        static void port_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);

            ptrAVRComponentPin->setState( value>0 );
        }
        
        static void port_reg_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            // get the pointer out of param and asign it to AVRComponentPin*
            AVRComponentPin* ptrAVRComponentPin = reinterpret_cast<AVRComponentPin*> (param);

            ptrAVRComponentPin->setPullup( value>0 );
        }

    protected:
        //void setPullup( uint32_t value );

        int  m_channel;

        //from simavr
        avr_t*     m_avrProcessor;
        avr_irq_t* m_PortChangeIrq;
        avr_irq_t* m_PortRegChangeIrq;
        avr_irq_t* m_DdrRegChangeIrq;
        avr_irq_t* m_Write_stat_irq;
        avr_irq_t* m_Write_adc_irq;
};

#endif

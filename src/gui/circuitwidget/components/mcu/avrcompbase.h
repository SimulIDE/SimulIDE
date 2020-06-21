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

#ifndef AVRCOMPBASE_H
#define AVRCOMPBASE_H

#include "avrcomponentpin.h"
#include "mcucomponent.h"
#include "avrprocessor.h"
#include "e-i2c.h"


class AvrCompBase : public McuComponent
{
    Q_OBJECT
    Q_PROPERTY( bool Init_gdb READ initGdb WRITE setInitGdb DESIGNABLE true  USER true )

    public:
        AvrCompBase( QObject* parent, QString type, QString id );
        ~AvrCompBase();

        bool initGdb();
        void setInitGdb( bool init );

        //int getRamValue( int address );
        
        void adcread( int channel );
        void i2cOut( uint32_t value );
        void twenChanged( uint32_t value );

        virtual void inStateChanged( int value );

 static void adc_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            AvrCompBase* ptrAvr = reinterpret_cast<AvrCompBase*> (param);

            int channel = int( value/524288 );
            ptrAvr->adcread( channel );
        }

 static void i2c_out_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            AvrCompBase* ptrAvr = reinterpret_cast<AvrCompBase*> (param);

            ptrAvr->i2cOut( value );
        }

 static void twen_hook( struct avr_irq_t* irq, uint32_t value, void* param )
        {
            Q_UNUSED(irq);
            AvrCompBase* ptrAvr = reinterpret_cast<AvrCompBase*> (param);

            ptrAvr->twenChanged( value );
        }

    public slots:
        //virtual void remove();

    private:
        void attachPins();
        void addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        QString getType( QString type , QString t );

        QHash<int, AVRComponentPin*> m_ADCpinList;

        eI2C m_avrI2C;
        AVRComponentPin* m_sda;
        AVRComponentPin* m_scl;

        uint8_t m_slvAddr;

        avr_irq_t* m_i2cInIrq;

        AvrProcessor m_avr;
};

#endif

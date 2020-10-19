/***************************************************************************
 *   Copyright (C) 2017 by Santiago González                               *
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

#ifndef EI2C_H
#define EI2C_H

#define I2C_IDLE      0
#define I2C_STARTED   1
#define I2C_READING   2
#define I2C_WRITTING  3
#define I2C_STOPPED   4
#define I2C_ACK       5
#define I2C_ENDACK    6
#define I2C_WAITACK   7


#include "e-logic_device.h"
#include "component.h"


class MAINMODULE_EXPORT eI2C : public eLogicDevice
{
    public:

        eI2C( std::string id );
        ~eI2C();

        virtual void stamp();
        virtual void resetState();
        virtual void simuClockStep();
        virtual void setVChanged();

        virtual void startWrite(){;}
        virtual void writeByte();
        virtual void readByte();
        virtual void slaveStop();
        //virtual bool isEnabled() { return m_enabled; }
        virtual void setEnabled( bool en );
        virtual void setMaster( bool m );
        virtual void setAddress( int address );
        virtual void setFreq( double f );
        virtual void setComponent( Component* comp ) { m_comp = comp; }

        virtual void masterStart( uint8_t addr );
        virtual void masterWrite( uint8_t data );
        virtual void masterRead();
        virtual void masterStop();

        virtual int byteReceived() { return m_rxReg; }

        void createPins();

    protected:
        void readBit();
        void writeBit();
        void ACK();
        void waitACK();
        
        int m_address;                                 // Device Address
        
        int m_txReg;                                     // Byte to Send
        int m_rxReg;                                    // Byte Received
        int m_state;                             // Current State of i2c
        int m_lastState;                            // Last State of i2c
        int m_addressBits;                   // Number of m_address bits
        int m_bitPtr;                                     // Bit Pointer

        double m_freq;
        double m_stepsPe;
        double m_nextCycle;

        bool m_SDA;
        bool m_lastSDA;
        bool m_master;
        bool m_enabled;
        bool m_toggleClock;

        Component* m_comp;
};


#endif

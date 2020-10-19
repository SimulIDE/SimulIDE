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

#ifndef PROCESSOR_H
#define PROCESSOR_H

//#include <QtGui>

#include "ramtable.h"
#include "terminalwidget.h"

class RamTable;

class MAINMODULE_EXPORT BaseProcessor : public QObject
{
    Q_OBJECT
    public:
        BaseProcessor( QObject* parent=0 );
        ~BaseProcessor();
        
 static BaseProcessor* self() { return m_pSelf; }
 
        QString getFileName();

        virtual void    setDevice( QString device );
        virtual QString getDevice();
        
        virtual void setDataFile( QString datafile );

        virtual bool loadFirmware( QString file )=0;
        virtual bool getLoadStatus() { return m_loadStatus; }
        virtual void terminate();

        virtual void setSteps( double steps );
        virtual void step();
        virtual void stepOne()=0;
        virtual void stepCpu()=0;
        virtual void reset()=0;
        virtual int  pc()=0;
        virtual uint64_t cycle()=0;
        
        virtual void hardReset( bool reset );
        virtual int getRamValue( QString name );
        virtual int getRamValue( int address )=0;
        virtual int getRegAddress( QString name );
        virtual void addWatchVar( QString name, int address, QString type );
        virtual void updateRamValue( QString name );

        virtual void uartOut( int uart, uint32_t value );
        virtual void uartIn( int uart, uint32_t value );
        
        virtual void initialized();
        virtual QStringList getRegList() { return m_regList; }
        
        virtual RamTable* getRamTable() { return m_ramTable; }

        virtual QVector<int> eeprom()=0;
        virtual void setEeprom( QVector<int> eep )=0;
        
        virtual void setRegisters();

        void setExtraStep();

    signals:
        void uartDataOut( int uart, int value );
        void uartDataIn(  int uart, int value );
    
    protected:
 static BaseProcessor* m_pSelf;
        
        virtual int  validate( int address )=0;
        
        void runSimuStep();

        QString m_symbolFile;
        QString m_dataFile;
        QString m_device;
        
        double   m_nextCycle;
        double   m_mcuStepsPT;
        int      m_msimStep;
        uint64_t m_extraCycle;

        RamTable* m_ramTable;
        QStringList m_regList;
        QHash<QString, int> m_regsTable;     // int max 32 bits
        QHash<QString, float> m_floatTable;  // float 32 bits
        QHash<QString, QString> m_typeTable;

        QVector<int> m_eeprom;

        bool m_resetStatus;
        bool m_loadStatus;
};


#endif


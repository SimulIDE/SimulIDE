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

#ifndef MCUCOMPONENT_H
#define MCUCOMPONENT_H

#include <QtWidgets>

#include "chip.h"
#include "memdata.h"
#include <QtSerialPort/QSerialPort>

#include "shield.h"

class BaseProcessor;
class McuComponentPin;

class MAINMODULE_EXPORT McuComponent : public Chip, public MemData
{
    Q_OBJECT
    Q_PROPERTY( QVector<int> eeprom  READ eeprom   WRITE setEeprom )
    Q_PROPERTY( QString  Program     READ program  WRITE setProgram  DESIGNABLE true  USER true )
    Q_PROPERTY( double   Mhz         READ freq     WRITE setFreq     DESIGNABLE true  USER true )
    Q_PROPERTY( bool     Auto_Load   READ autoLoad WRITE setAutoLoad DESIGNABLE true  USER true )

    public:

        McuComponent( QObject* parent, QString type, QString id );
        ~McuComponent();
        
 static McuComponent* self() { return m_pSelf; }

        virtual void updateStep();
        virtual void runAutoLoad();

        QString program()   const { return  m_symbolFile; }
        void setProgram( QString pro );

        double freq();
        virtual void setFreq( double freq );

        bool autoLoad() { return m_autoLoad; }
        void setAutoLoad( bool al ) { m_autoLoad = al; }
        
        QString device() { return m_device; }

        virtual void initChip();

        void setEeprom(QVector<int> eep );
        QVector<int> eeprom();

        virtual void setLogicSymbol( bool ls );
        
        QList<McuComponentPin*> getPinList() { return m_pinList; }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    signals:
        void closeSerials();
        void openSerials();
  
    public slots:
        virtual void terminate();
        virtual void remove();
        virtual void reset();
        virtual void load( QString fileName );
        void slotLoad();
        void slotReload();
        void slotOpenTerm();
        void slotOpenSerial();

        void loadData();
        void saveData();
        
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );
        
    protected:
 static McuComponent* m_pSelf;
 static bool m_canCreate;
        
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

        virtual void addPin( QString id, QString type, QString label,
                             int pos, int xpos, int ypos, int angle )=0;

        virtual void updatePin( QString id, QString type, QString label,
                                int pos, int xpos, int ypos, int angle );

        virtual void attachPins()=0;

        BaseProcessor* m_processor;

        double m_freq;           // Clock Frequency Mhz
        
        bool m_attached;
        bool m_autoLoad;

        QString m_device;       // Name of device
        QString m_symbolFile;   // firmware file loaded
        QString m_lastFirmDir;  // Last firmware folder used

        QList<McuComponentPin*> m_pinList;

        Shield* m_shield;

    friend class Shield;
};
#endif

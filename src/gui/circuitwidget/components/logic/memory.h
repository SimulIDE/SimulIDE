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

#ifndef MEMORY_H
#define MEMORY_H

#include "itemlibrary.h"
#include "logiccomponent.h"
#include "e-memory.h"
#include "memdata.h"


class MAINMODULE_EXPORT Memory : public LogicComponent, public eMemory, public MemData
{
    Q_OBJECT

    Q_PROPERTY( QVector<int> Mem  READ mem        WRITE setMem )
    Q_PROPERTY( int  Address_Bits READ addrBits   WRITE setAddrBits   DESIGNABLE true USER true )
    Q_PROPERTY( int  Data_Bits    READ dataBits   WRITE setDataBits   DESIGNABLE true USER true )
    Q_PROPERTY( bool Persistent   READ persistent WRITE setPersistent DESIGNABLE true USER true )

    public:
        Memory( QObject* parent, QString type, QString id );
        ~Memory();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        void setAddrBits( int bits );
        void deleteAddrBits( int bits );
        void createAddrBits( int bits );

        void setDataBits( int bits );
        void deleteDataBits( int bits );
        void createDataBits( int bits );

        void updatePins();

    public slots:
        virtual void remove();
        void loadData();
        void saveData();
        void contextMenu( QGraphicsSceneContextMenuEvent* event, QMenu* menu );

    protected:
        virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
        
    private:
        Pin* m_CsPin;
        Pin* m_WePin;
        Pin* m_outEnPin;
};

#endif


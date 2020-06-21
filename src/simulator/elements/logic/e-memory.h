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

#ifndef EMEMORY_H
#define EMEMORY_H

#include <QVector>

#include "e-logic_device.h"

class MAINMODULE_EXPORT eMemory : public eLogicDevice
{
    public:
        eMemory( std::string id );
        ~eMemory();
        
        virtual void stamp();
        virtual void resetState();
        virtual void setVChanged();
        
        void setMem( QVector<int> m );
        QVector<int> mem();
        
        virtual int addrBits();
        virtual void setAddrBits( int bits );
        
        virtual int dataBits();
        virtual void setDataBits( int bits );
        
        bool persistent();
        void setPersistent( bool p );
        
    protected:
        int m_addrBits;
        int m_dataBits;
        
        QVector<int> m_ram;
        std::vector<bool> m_dataPinState;
        
        bool m_we;
        bool m_cs;
        bool m_oe;
        bool m_persistent;
};

#endif

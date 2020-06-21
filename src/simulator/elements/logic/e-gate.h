/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef EGATE_H
#define EGATE_H

#include "e-logic_device.h"

class MAINMODULE_EXPORT eGate : public eLogicDevice
{
    public:

        eGate( std::string id, int inputs );
        ~eGate();

        virtual void stamp();
        virtual void resetState();
        virtual void setVChanged();
        
        bool tristate();
        void setTristate( bool t );
        
        bool openCol();
        void setOpenCol( bool op );

    protected:           
        virtual bool calcOutput( int inputs );

        bool m_tristate;
        bool m_openCol;
 static bool m_oscCtrl;
        int  m_oscCount;
        
        uint64_t m_lastStep;
};


#endif

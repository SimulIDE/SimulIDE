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

#ifndef EBINCOUNTER_H
#define EBINCOUNTER_H

#include "e-logic_device.h"


class MAINMODULE_EXPORT eBinCounter : public eLogicDevice
{
    public:

        eBinCounter( std::string id );
        ~eBinCounter();

        int  TopValue() const;
        void setTopValue( int );
        
        virtual void stamp();
        virtual void resetState();

        void createPins();
        virtual void setVChanged();

        bool resetInv() { return m_resetInv; }
        void setResetInv( bool inv );

    protected:
        int m_Counter;
        int m_TopValue;

        bool m_resetInv;
};

#endif

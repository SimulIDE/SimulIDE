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

#ifndef EBCDTODEC_H
#define EBCDTODEC_H

#include "e-logic_device.h"

class MAINMODULE_EXPORT eBcdToDec : public eLogicDevice
{
    public:

        eBcdToDec( std::string id  );
        ~eBcdToDec();

        virtual void stamp();
        virtual void resetState();
        virtual void setVChanged();

        virtual void set_16bits( bool set );
        
        void createPins();
                             
    protected:           
        bool m_16Bits;

        int m_address;
};


#endif

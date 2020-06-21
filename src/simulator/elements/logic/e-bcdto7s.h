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

#ifndef EBCDTO7S_H
#define EBCDTO7S_H

#include "e-logic_device.h"

class MAINMODULE_EXPORT eBcdTo7S : public eLogicDevice
{
    public:

        eBcdTo7S( std::string id  );
        ~eBcdTo7S();

        virtual void stamp();
        virtual void setVChanged();
        
        void createPins();

    protected:
        std::vector<bool> m_outValue;
        
        bool m_changed;

        int m_digit;
};


#endif

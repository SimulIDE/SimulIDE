/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

#ifndef ERESISTORDIP_H
#define ERESISTORDIP_H

#include "e-resistor.h"

class MAINMODULE_EXPORT eResistorDip : public eElement
{
    public:
        eResistorDip( std::string id );
        ~eResistorDip();

        virtual void setRes( double resist );
        
        virtual void setSize( int size ) { m_size = size; }
        
        virtual void initEpins();

    protected:
        double m_resist;
        
        int m_size;
        
        std::vector<eResistor*> m_resistor;
};

#endif

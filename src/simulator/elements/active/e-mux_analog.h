/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#ifndef EMUXANALOG_H
#define EMUXANALOG_H

#include "e-element.h"

class ePin;
class eResistor;

class MAINMODULE_EXPORT eMuxAnalog : public eElement
{
    public:

        eMuxAnalog( std::string id );
        ~eMuxAnalog();

        virtual void stamp();
        virtual void setVChanged();
        
        double resist();
        void setResist( double r );

        void setBits( int bits );
        
        virtual void initEpins(){;}
        
        virtual ePin* getEpin( QString pinName );
                             
    protected:
        int m_addrBits;
        int m_channels;
        int m_address;
        
        double m_admit;
        
        bool m_enabled;
        
        ePin* m_inputPin;
        ePin* m_enablePin;
        
        std::vector<eResistor*> m_resistor;
        std::vector<ePin*> m_addrPin;
        std::vector<ePin*> m_chanPin;
};

#endif

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

#ifndef EBUS_H
#define EBUS_H

#include "e-element.h"

class MAINMODULE_EXPORT eBus : public eElement
{
    public:

        eBus( std::string id  );
        ~eBus();

        virtual void setVChanged();
        
        int numLines() { return m_numLines; }
        virtual void initEpins();
        
        virtual void setNumLines( int lines );
        
        int startBit() { return m_startBit; }
        void setStartBit( int bit );

        ePin* getEpin( QString pinName );
                             
    protected:
        int m_numLines;
        int m_startBit;
};

#endif

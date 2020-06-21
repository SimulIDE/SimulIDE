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

#ifndef EELEMENT_H
#define EELEMENT_H

#include <string>
#include <math.h>
#include <QPointer>
#include <QDebug>
#include "e-pin.h"


// The following provides compatibility with gcc compiler v5 and up
// (i.e. c++11 standard complience)
//#if __GNUC__ >= 5
 #define GNU_CONST_STATIC_FLOAT_DECLARATION constexpr
//#else
// #define GNU_CONST_STATIC_FLOAT_DECLARATION const
//#endif

class MAINMODULE_EXPORT eElement
{
    public:
        eElement( std::string id=0 );
        virtual ~eElement();

        virtual void initEpins();
        virtual void setNumEpins( int n );

        virtual ePin* getEpin( int pin );
        virtual ePin* getEpin( QString pinName );
        
        virtual void setEpin( int num, ePin* pin );

        std::string getId(){ return m_elmId; }

        virtual void initialize(){;}
        virtual void resetState(){;}
        virtual void attach(){;}
        virtual void stamp(){;}

        virtual void simuClockStep(){;}
        virtual void updateStep(){;}
        virtual void setVChanged(){;}

        static GNU_CONST_STATIC_FLOAT_DECLARATION double cero_doub         = 1e-14;
        static GNU_CONST_STATIC_FLOAT_DECLARATION double high_imp          = 1e14;
        static GNU_CONST_STATIC_FLOAT_DECLARATION double digital_high      = 5.0;
        static GNU_CONST_STATIC_FLOAT_DECLARATION double digital_low       = 0.0;
        static GNU_CONST_STATIC_FLOAT_DECLARATION double digital_threshold = 2.5;

    protected:
        std::vector<ePin*> m_ePin;

        std::string m_elmId;
};

#endif


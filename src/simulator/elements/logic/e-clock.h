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

#ifndef ECLOCK_H
#define ECLOCK_H

#include "e-element.h"

class eSource;

class MAINMODULE_EXPORT eClock : public eElement
{
    public:
        eClock( std::string id );
        ~eClock();

        virtual void resetState();
        virtual void simuClockStep();
        
        void setFreq( double freq );
        void setVolt( double v );
        
        virtual void initEpins(){;}
        virtual ePin* getEpin( QString pinName );

    protected:
        eSource* m_output;
        
        double m_freq;
        
        int m_step;
        int m_stepsPC;
};

#endif

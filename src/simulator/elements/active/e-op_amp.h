/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef EOPAMP_H
#define EOPAMP_H

#include "e-element.h"

class eSource;

class MAINMODULE_EXPORT eOpAmp : public eElement
{
    public:

        eOpAmp( std::string id );
        virtual ~eOpAmp();

        virtual void stamp();
        virtual void resetState();
        virtual void setVChanged();
        
        virtual double gain();
        virtual void setGain( double gain );
        
        virtual bool hasPowerPins();
        virtual void setPowerPins( bool set );
        
        virtual ePin* getEpin( QString pinName );
        virtual void initEpins();
        
    protected:
        eSource* m_output;
        
        bool m_converged;
        bool m_powerPins;
        
        double m_accuracy;
        double m_gain;
        double m_k;
        double m_voltPos;
        double m_voltNeg;
        double m_lastOut;
        double m_lastIn;
};


#endif

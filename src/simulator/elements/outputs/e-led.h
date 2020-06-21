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

#ifndef ELED_H
#define ELED_H

#include "e-diode.h"

class MAINMODULE_EXPORT eLed : public eDiode
{
    public:
        eLed( std::string id );
        ~eLed();

        virtual double maxCurrent() const             { return m_maxCurrent; }
        virtual void  setMaxCurrent( double current ) { m_maxCurrent = current; }

        void setVChanged();

        virtual void initialize();
        virtual void resetState();

    protected:
        void updateBright();
        virtual void updateVI();

        uint64_t m_prevStep;

        uint  m_bright;

        double m_maxCurrent;
        double m_lastCurrent;
        double m_lastUpdatePeriod;
        double m_avg_brightness;
        double m_disp_brightness;
};

#endif


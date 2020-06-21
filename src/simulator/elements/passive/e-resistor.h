/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
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

#ifndef ERESISTOR_H
#define ERESISTOR_H

#include "e-element.h"

class MAINMODULE_EXPORT eResistor : public eElement
{
    public:
        eResistor( std::string id );
        ~eResistor();

        virtual double res();
        virtual void setRes( double resist );
        virtual void setResSafe( double resist );
        
        virtual double admit();
        virtual void setAdmit( double admit );
        void stampAdmit();
        void stampCurrent( double current );

        double current();

        virtual void stamp();

        virtual void initEpins();

    protected:
        virtual void updateVI();

        double m_resist;
        double m_admit;
        double m_current;
};

#endif

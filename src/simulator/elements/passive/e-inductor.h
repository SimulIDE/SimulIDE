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

#ifndef EINDUCTOR_H
#define EINDUCTOR_H

#include "e-resistor.h"

class LibraryItem;

class MAINMODULE_EXPORT eInductor : public eResistor
{
    public:
        eInductor( std::string id );
        ~eInductor();

        virtual void stamp();
        virtual void resetState();
        virtual void setVChanged();

        double indCurrent();

        double ind();
        void   setInd( double h );

    protected:
        double m_ind;
        double m_curSource;
        double m_tStep;
        double m_volt;
};

#endif

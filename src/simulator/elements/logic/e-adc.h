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

#ifndef EADC_H
#define EADC_H

#include "e-logic_device.h"

class MAINMODULE_EXPORT eADC : public eLogicDevice
{
    public:
        eADC( std::string id  );
        ~eADC();

        double maxVolt()               { return m_maxVolt; }
        void setMaxVolt( double volt ) { m_maxVolt = volt; }

        double maxAddr()               { return m_maxAddr; }
        void setMaxAddr( double volt ) { m_maxAddr = volt; }

        virtual void stamp();
        virtual void setVChanged();
                             
    protected:
        double m_maxVolt;
        double m_maxAddr;
};

#endif

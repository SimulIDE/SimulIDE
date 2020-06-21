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

#ifndef ELM555_H
#define ELM555_H

#include "e-element.h"

class eSource;

class MAINMODULE_EXPORT eLm555 : public eElement
{
    public:

        eLm555( std::string id );
        ~eLm555();
        
        virtual void stamp();
        virtual void resetState();

        virtual void setVChanged();
        
        virtual void initEpins();

    protected:
        
        eSource* m_output;
        eSource* m_cv;
        eSource* m_dis;
        
        double m_volt;
        
        bool m_outState;
};


#endif

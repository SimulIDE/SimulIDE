/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef EFUNCTION_H
#define EFUNCTION_H

#include <QScriptEngine>

#include "e-logic_device.h"

class MAINMODULE_EXPORT eFunction : public eLogicDevice
{
    public:

        eFunction( std::string id );
        ~eFunction();

        virtual void stamp();
        virtual void setVChanged();
        
        QString functions();
        void setFunctions( QString f );

    protected:
        QScriptEngine m_engine;
        
        QString m_functions;
        QStringList m_funcList;
};


#endif

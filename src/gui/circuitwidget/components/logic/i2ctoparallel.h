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

#ifndef I2CTOPARALLEL_H
#define I2CTOPARALLEL_H

#include "e-i2c.h"
#include "itemlibrary.h"
#include "logiccomponent.h"

class MAINMODULE_EXPORT I2CToParallel : public LogicComponent, public eI2C
{
    Q_OBJECT
    Q_PROPERTY( int Control_Code READ cCode WRITE setCcode DESIGNABLE true USER true )

    public:
        I2CToParallel( QObject* parent, QString type, QString id );
        ~I2CToParallel();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        int cCode();
        void setCcode( int code );
        
        virtual void stamp();
        virtual void setVChanged();
        //virtual void writeByte();
        virtual void readByte();
        
    private:
        int m_cCode;
        //int m_phase;
};

#endif


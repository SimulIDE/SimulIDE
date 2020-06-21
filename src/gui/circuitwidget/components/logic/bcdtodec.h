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

#ifndef BCDTODEC_H
#define BCDTODEC_H

#include "e-bcdtodec.h"
#include "itemlibrary.h"
#include "logiccomponent.h"

class MAINMODULE_EXPORT BcdToDec : public LogicComponent, public eBcdToDec
{
    Q_OBJECT
    Q_PROPERTY( double Input_High_V READ inputHighV WRITE setInputHighV DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Low_V  READ inputLowV  WRITE setInputLowV  DESIGNABLE true USER true )
    Q_PROPERTY( double Input_Imped  READ inputImp   WRITE setInputImp   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_High_V   READ outHighV   WRITE setOutHighV   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Low_V    READ outLowV    WRITE setOutLowV    DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Imped    READ outImp     WRITE setOutImp     DESIGNABLE true USER true )
    Q_PROPERTY( bool   Inverted     READ inverted   WRITE setInverted   DESIGNABLE true USER true )
    Q_PROPERTY( bool   Tristate     READ tristate   USER true )
    Q_PROPERTY( bool   _16_Bits     READ _16bits    WRITE set_16bits    DESIGNABLE true USER true )

    public:
        BcdToDec( QObject* parent, QString type, QString id );
        ~BcdToDec();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        bool tristate() { return true; }

        bool _16bits();
        void set_16bits( bool set );

   private:

};

#endif


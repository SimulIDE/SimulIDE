/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#ifndef ADC_H
#define ADC_H

#include "logiccomponent.h"
#include "e-adc.h"

class LibraryItem;

class MAINMODULE_EXPORT ADC : public LogicComponent, public eADC
{
    Q_OBJECT
    Q_PROPERTY( double Out_High_V  READ outHighV   WRITE setOutHighV   DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Low_V   READ outLowV    WRITE setOutLowV    DESIGNABLE true USER true )
    Q_PROPERTY( double Out_Imped   READ outImp     WRITE setOutImp     DESIGNABLE true USER true )
    Q_PROPERTY( double Vref        READ maxVolt    WRITE setMaxVolt    DESIGNABLE true USER true )
    Q_PROPERTY( int    Num_Bits    READ numOuts    WRITE setNumOuts    DESIGNABLE true USER true )

    public:
        ADC( QObject* parent, QString type, QString id );
        ~ADC();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        virtual void setNumOuts( int outs );
        
    protected:
};

#endif

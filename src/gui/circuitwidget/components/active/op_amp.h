/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OPAMP_H
#define OPAMP_H

#include "e-op_amp.h"
#include "component.h"

#include <QObject>

class LibraryItem;

class MAINMODULE_EXPORT OpAmp : public Component, public eOpAmp
{
    Q_OBJECT
    Q_PROPERTY( double Gain       READ gain          WRITE setGain      DESIGNABLE true USER true )
    Q_PROPERTY( bool   Power_Pins READ hasPowerPins  WRITE setPowerPins DESIGNABLE true USER true )
    
    public:

        OpAmp( QObject* parent, QString type, QString id );
        ~OpAmp();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void setPowerPins( bool set );

        virtual QPainterPath shape() const;
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
};

#endif

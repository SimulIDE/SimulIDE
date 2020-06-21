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

#ifndef DIODE_H
#define DIODE_H

#include "e-diode.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT Diode : public Component, public eDiode
{
    Q_OBJECT
    Q_PROPERTY( double Threshold  READ threshold WRITE setThreshold DESIGNABLE true USER true )
    Q_PROPERTY( double Zener_Volt READ zenerV    WRITE setZenerV    DESIGNABLE true USER true )
    

    public:
        Diode( QObject* parent, QString type, QString id );
        ~Diode();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
};

#endif

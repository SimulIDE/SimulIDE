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

#ifndef BJT_H
#define BJT_H

#include "e-bjt.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT BJT : public Component, public eBJT
{
    Q_OBJECT
    Q_PROPERTY( double Gain      READ gain  WRITE setGain  DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold READ BEthr WRITE setBEthr DESIGNABLE true USER true )
    Q_PROPERTY( bool   PNP       READ pnp   WRITE setPnp   DESIGNABLE true USER true )
    Q_PROPERTY( bool   BC_diode  READ BCd   WRITE setBCd   DESIGNABLE true USER true )
    
    public:

        BJT( QObject* parent, QString type, QString id );
        ~BJT();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void updateStep();
        
        void setPnp( bool pnp );
        void setBCd( bool bcd );

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
};

#endif

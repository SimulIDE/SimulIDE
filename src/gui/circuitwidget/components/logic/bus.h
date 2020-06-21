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

#ifndef BUS_H
#define BUS_H

#include "itemlibrary.h"
#include "component.h"
#include "e-bus.h"

class MAINMODULE_EXPORT Bus : public Component, public eBus
{
    Q_OBJECT
    Q_PROPERTY( int Num_Bits  READ numLines    WRITE setNumLines    DESIGNABLE true USER true )
    Q_PROPERTY( int Start_Bit READ startBit    WRITE setStartBit    DESIGNABLE true USER true )

    public:
        Bus( QObject* parent, QString type, QString id );
        ~Bus();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void setNumLines( int lines );

        void setStartBit( int bit );
        
        virtual void initialize();
        void inStateChanged( int msg );

        void registerPins( eNode* enode );

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    protected:
        int m_height;
        
        Pin* m_busPin0;
        Pin* m_busPin1;
};

#endif


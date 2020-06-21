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

#ifndef LED_H
#define LED_H

#include "ledbase.h"
#include "itemlibrary.h"

class MAINMODULE_EXPORT Led : public LedBase
{
    Q_OBJECT

    public:
        Led( QObject* parent, QString type, QString id );
        ~Led();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

    protected:
        void drawBackground( QPainter *p );
        void drawForeground( QPainter *p );
};

#endif

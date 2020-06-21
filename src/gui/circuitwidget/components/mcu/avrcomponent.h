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

#ifndef AVRCOMPONENT_H
#define AVRCOMPONENT_H


#include "avrcompbase.h"
#include "itemlibrary.h"

class AVRComponent : public AvrCompBase
{
    Q_OBJECT

    public:

        AVRComponent( QObject* parent, QString type, QString id );
        ~AVRComponent();
    
 static Component*   construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

};

#endif


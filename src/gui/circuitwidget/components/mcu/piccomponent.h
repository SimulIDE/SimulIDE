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

#ifndef PICCOMPONENT_H
#define PICCOMPONENT_H

#include "mcucomponent.h"
#include "picprocessor.h"
#include "itemlibrary.h"


class PICComponent : public McuComponent
{
    Q_OBJECT

    public:

        PICComponent( QObject* parent, QString type, QString id );
        ~PICComponent();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem * libraryItem();

        int getRamValue( int address );

    private:
        void attachPins();
        void addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        
        PicProcessor m_pic;
};

#endif


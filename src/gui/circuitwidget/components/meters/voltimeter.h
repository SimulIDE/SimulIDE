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

#ifndef VOLTIMETER_H
#define VOLTIMETER_H

#include "itemlibrary.h"
#include "meter.h"


class MAINMODULE_EXPORT Voltimeter : public Meter
{
    Q_OBJECT

    public:

        Voltimeter( QObject* parent, QString type, QString id );
        ~Voltimeter();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        void updateStep();
};

#endif

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

#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "itemlibrary.h"
#include "shape.h"

class MAINMODULE_EXPORT Ellipse : public Shape
{
    Q_OBJECT
    
    public:

        Ellipse( QObject* parent, QString type, QString id );
        ~Ellipse();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem *libraryItem();

        virtual QPainterPath shape() const;
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

};

#endif

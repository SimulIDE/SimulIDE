/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef BUFFER_H
#define BUFFER_H

#include "gate.h"
#include "component.h"

class LibraryItem;

class MAINMODULE_EXPORT Buffer : public Gate
{
    Q_OBJECT
    Q_PROPERTY( bool Tristate READ tristate WRITE setTristate DESIGNABLE true USER true )
    
    public:
        QRectF boundingRect() const { return m_area; }
    
        Buffer( QObject* parent, QString type, QString id );
        ~Buffer();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void setTristate( bool t );
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
    
    public slots:
        virtual void remove();
        
    private:
        Pin* m_outEnPin;
};

#endif

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

#ifndef OSCOPE_H
#define OSCOPE_H

#include "component.h"
#include "e-element.h"
#include "oscopewidget.h"
#include "topwidget.h"
#include "pin.h"

class LibraryItem;

class MAINMODULE_EXPORT Oscope : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double Filter READ filter  WRITE setFilter DESIGNABLE true USER true )

    public:

        Oscope( QObject* parent, QString type, QString id );
        ~Oscope();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void updateStep();
        
        double getVolt();
        
        double filter()                 { return m_oscopeW->filter(); }
        void setFilter( double filter ) { m_oscopeW->setFilter( filter ); }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void remove();

    private:
        
        OscopeWidget* m_oscopeW;
        TopWidget    m_topW;
        QGraphicsProxyWidget* m_proxy;
};

#endif


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

#ifndef SerialTerm_H
#define SerialTerm_H

#include "component.h"
#include "e-element.h"
#include "terminalwidget.h"

class LibraryItem;

class MAINMODULE_EXPORT SerialTerm : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int Mcu_Uart READ uart  WRITE setUart )
    Q_PROPERTY( int posx     READ posX  WRITE setPosX )
    Q_PROPERTY( int posy     READ posY  WRITE setPosY )
    Q_PROPERTY( int sizex    READ sizeX WRITE resizeX )
    Q_PROPERTY( int sizey    READ sizeY WRITE resizeY )

    public:

        SerialTerm( QObject* parent, QString type, QString id );
        ~SerialTerm();
        
 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();

        void updateStep();

        int uart() { return m_serialWidget.uart(); }

        int posX() { return m_serialWidget.x(); }
        void setPosX( int x ) { m_serialWidget.move( x, m_serialWidget.y() );}

        int posY() { return m_serialWidget.y(); }
        void setPosY( int y ) { m_serialWidget.move( m_serialWidget.x(), y );}

        int sizeX() { return m_serialWidget.size().width(); }
        void resizeX( int x ) {m_serialWidget.resize( QSize( x, m_serialWidget.size().height()) ); }

        int sizeY() { return m_serialWidget.size().height(); }
        void resizeY( int y ) {m_serialWidget.resize( QSize( m_serialWidget.size().width(), y) ); }

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void slotClose();
        void setUart( int uart );

    private:
        TerminalWidget m_serialWidget;
};

#endif


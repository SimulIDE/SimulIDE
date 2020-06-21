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

#ifndef SWITCHDIP_H
#define SWITCHDIP_H

#include "component.h"
#include "e-element.h"

class LibraryItem;

class MAINMODULE_EXPORT SwitchDip : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int    Size   READ size   WRITE setSize   DESIGNABLE true USER true )
    Q_PROPERTY( int    State  READ state  WRITE setState  )

    public:
        SwitchDip( QObject* parent, QString type, QString id );
        ~SwitchDip();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        int  size();
        void setSize( int size );

        int  state();
        void setState( int state );

        virtual void stamp();
        virtual void updateStep();

        void createSwitches( int c );
        void deleteSwitches( int d );

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    public slots:
        virtual void remove();
        void onbuttonclicked();

    private:
        QList<QPushButton*> m_buttons;
        QList<QGraphicsProxyWidget*> m_proxys;
        std::vector<Pin*> m_pin;

        bool m_changed;

        int m_size;
        int m_state;
};

#endif

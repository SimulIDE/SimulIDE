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

#ifndef KEYPAD_H
#define KEYPAD_H

#include "itemlibrary.h"
#include "e-element.h"
#include "push_base.h"


class MAINMODULE_EXPORT KeyPad : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int      Rows       READ rows WRITE setRows  DESIGNABLE true USER true )
    Q_PROPERTY( int      Cols       READ cols WRITE setCols  DESIGNABLE true USER true )
    Q_PROPERTY( QString  Key_Labels READ keyLabels WRITE setKeyLabels  DESIGNABLE true USER true )

    public:
        KeyPad( QObject* parent, QString type, QString id );
        ~KeyPad();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();

        double rows();
        void setRows( double rows );
        
        double cols();
        void setCols( double cols );
        
        QString keyLabels();
        void setKeyLabels( QString keyLabels );
        
        virtual void attach();

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    public slots:
        virtual void remove();

    private:
        void setupButtons();
        int m_rows;
        int m_cols;
        
        QString m_keyLabels;
        
        QList<PushBase*> m_buttons;
};

#endif

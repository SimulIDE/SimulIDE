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

#ifndef SWITCH_BASE_H
#define SWITCH_BASE_H

#include "mech_contact.h"

class MAINMODULE_EXPORT SwitchBase : public MechContact
{
    Q_OBJECT
    Q_PROPERTY( QString Key READ key WRITE setKey DESIGNABLE true USER true )

    public:

        SwitchBase( QObject* parent, QString type, QString id );
        ~SwitchBase();

        virtual void updateStep();
        
        void setButtonText( QString text );

        QString key();
        void setKey( QString key );
        
        QPushButton* button() { return m_button; }

    public slots:
        void remove();
        virtual void onbuttonclicked();
        virtual void keyEvent( QString key, bool pressed ){;}

    protected:
        bool m_changed;

        QString m_key;
        
        QPushButton*          m_button;
        QGraphicsProxyWidget* m_proxy;
};

#endif

/***************************************************************************
 *   Copyright (C) 2016 by santiago González                               *
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

#ifndef PUSHBASE_H
#define PUSHBASE_H

#include "switch_base.h"


class MAINMODULE_EXPORT PushBase : public SwitchBase
{
    Q_OBJECT
    
    public:
    
        PushBase( QObject* parent, QString type, QString id );
        ~PushBase();

    public slots:
        void onbuttonPressed();
        void onbuttonReleased();
        virtual void keyEvent( QString key, bool pressed );
};

#endif

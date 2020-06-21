/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#ifndef SERVO_H
#define SERVO_H

#include "e-logic_device.h"
#include "itemlibrary.h"
#include "logiccomponent.h"


class MAINMODULE_EXPORT Servo : public LogicComponent, public eLogicDevice
{
    Q_OBJECT
    Q_PROPERTY( double Speed   READ speed    WRITE setSpeed    DESIGNABLE true USER true )

    public:
        Servo( QObject* parent, QString type, QString id );
        ~Servo();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();

        double speed() { return m_speed; }
        void setSpeed( double speed ) { m_speed = speed; }

        void resetState();
        void stamp();

        void setVChanged();
        void updateStep();
        
        virtual QPainterPath shape() const;
        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void remove();
        
    private:
        int m_pos;                      // Actual Angular position 0-180
        int m_targetPos;                // Target Angular position 0-180

        double m_speed;                         // Angular speed sec/60ª
        int    m_minAngle;                 // Angle to move evry repaint

        uint64_t m_pulseStart;              // Simulation step
        uint64_t m_lastUpdate;              // Simulation step
};


#endif


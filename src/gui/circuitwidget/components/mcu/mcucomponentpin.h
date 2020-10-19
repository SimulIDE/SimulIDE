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

#ifndef MCUCOMPONENTPIN_H
#define MCUCOMPONENTPIN_H

#include "mcucomponent.h"
#include "e-source.h"
#include "pin.h"


class MAINMODULE_EXPORT McuComponentPin : public QObject, public eSource
{
    Q_OBJECT
    public:
        McuComponentPin( McuComponent *mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle );
        ~McuComponentPin();

        Pin* pin() const { return ( static_cast<Pin*>(m_ePin[0]) ); }

        virtual void stamp();
        virtual void resetState();

        void setDirection( bool out );
        void setState( bool state );
        void setPullup(bool up );
        void setExtraSource( double vddAdmit, double gndAdmit );
        void update();

        virtual void pullupNotConnected( bool up )=0;

        void move( int dx, int dy );
        int angle() { return m_angle; }
        
        QString ptype() { return m_type; }
        QString id() { return m_id; }

    protected:
        McuComponent* m_mcuComponent;

        double m_vddAdmit;  // Out stage
        double m_gndAdmit;  // Out Stage
        double m_vddAdmEx;  // Extra Source (vref out)
        double m_gndAdmEx;  // Extra Source (vref out)
        double m_pupAdmit;  // Internal Pullup
        
        bool m_attached;
        bool m_isInput;
        bool m_openColl;
        bool m_state;
        
        char m_port;
        int  m_pinN;
        int  m_pos;

        int m_pinType;
        int m_angle;

        QString m_type;
        QString m_id;
};

#endif


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

#ifndef STEPPER_H
#define STEPPER_H

#include "itemlibrary.h"
#include "component.h"
#include "e-element.h"
#include "e-resistor.h"
#include "pin.h"

class MAINMODULE_EXPORT Stepper : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int     Steps      READ steps WRITE setSteps DESIGNABLE true USER true )
    Q_PROPERTY( double  Resistance READ res   WRITE setRes   DESIGNABLE true USER true )
    Q_PROPERTY( QString Unit       READ unit  WRITE setUnit  DESIGNABLE true USER true )
    Q_PROPERTY( bool    Bipolar    READ bipolar  WRITE setBipolar  DESIGNABLE true USER true )
    
    public:
        Stepper( QObject* parent, QString type, QString id );
        ~Stepper();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        int steps();
        void setSteps( int steps ); //" 4, 8,16,32"
        
        double res();
        void setRes( double rows );
        
        void setUnit( QString un );

        bool bipolar() { return m_bipolar; }
        void setBipolar( bool bi );
        
        virtual void attach();
        virtual void stamp();

        virtual void setVChanged();
        virtual void updateStep();
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    public slots:
        virtual void remove();

    protected:
        int m_steps;
        double m_res;
        int m_ang;
        int m_stpang;
        int m_Ppos;

        bool m_bipolar;
        
        eResistor m_resA1;
        eResistor m_resA2;
        eResistor m_resB1;
        eResistor m_resB2;
        
        Pin m_pinA1;
        Pin m_pinA2;
        Pin m_pinCo;
        Pin m_pinB1;
        Pin m_pinB2;
        
        ePin m_ePinA1Co;
        ePin m_ePinA2Co;
        ePin m_ePinB1Co;
        ePin m_ePinB2Co;
};

#endif


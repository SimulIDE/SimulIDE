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

#ifndef SEVENSEGMENT_H
#define SEVENSEGMENT_H

#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class LibraryItem;

class MAINMODULE_EXPORT SevenSegment : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( LedBase::LedColor Color READ color      WRITE setColor         DESIGNABLE true USER true )
    Q_PROPERTY( int  NumDisplays   READ numDisplays     WRITE setNumDisplays   DESIGNABLE true USER true )
    Q_PROPERTY( bool CommonCathode READ isCommonCathode WRITE setCommonCathode DESIGNABLE true USER true )
    Q_PROPERTY( bool Vertical_Pins READ verticalPins    WRITE setVerticalPins DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold   READ threshold       WRITE setThreshold     DESIGNABLE true USER true )
    Q_PROPERTY( double MaxCurrent  READ maxCurrent      WRITE setMaxCurrent    DESIGNABLE true USER true )
    Q_PROPERTY( double Resistance  READ resistance      WRITE setResistance    DESIGNABLE true USER true )

    public:
        SevenSegment( QObject* parent, QString type, QString id );
        ~SevenSegment();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        
        LedBase::LedColor color();
        void setColor( LedBase::LedColor color );

        int numDisplays();
        void setNumDisplays( int dispNumber );

        bool verticalPins();
        void setVerticalPins( bool v );
        
        bool isCommonCathode();
        void setCommonCathode( bool isCommonCathode );
        
        double threshold();
        void   setThreshold( double threshold );
        
        double maxCurrent();
        void   setMaxCurrent( double current );

        double resistance() { return m_resistance; }
        void   setResistance( double res );

        virtual void attach();
        
        virtual void remove();

        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );

    private:
        void createDisplay( int dispNumber );
        void deleteDisplay( int dispNumber );
        void resizeData( int displays );

        bool m_commonCathode;
        bool m_verticalPins;
        
        int  m_numDisplays;
        double m_threshold;
        double m_maxCurrent;
        double m_resistance;
        
        LedBase::LedColor m_ledColor;

        std::vector<ePin*>  m_commonPin;
        std::vector<ePin*>  m_cathodePin;
        std::vector<ePin*>  m_anodePin;
        std::vector<LedSmd*> m_segment;
        eNode* m_enode[8];
        //eNode* m_virtGnd;
};

#endif


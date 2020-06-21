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

#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include "itemlibrary.h"
#include "component.h"
#include "e-element.h"
#include "ledsmd.h"

class MAINMODULE_EXPORT LedMatrix : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( LedBase::LedColor Color READ color WRITE setColor     DESIGNABLE true USER true )
    Q_PROPERTY( int    Rows       READ rows       WRITE setRows       DESIGNABLE true USER true )
    Q_PROPERTY( int    Cols       READ cols       WRITE setCols       DESIGNABLE true USER true )
    Q_PROPERTY( bool Vertical_Pins READ verticalPins    WRITE setVerticalPins DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold  READ threshold  WRITE setThreshold  DESIGNABLE true USER true )
    Q_PROPERTY( double MaxCurrent READ maxCurrent WRITE setMaxCurrent DESIGNABLE true USER true )
    Q_PROPERTY( double Resistance READ res        WRITE setRes        DESIGNABLE true USER true )

    public:
        LedMatrix( QObject* parent, QString type, QString id );
        ~LedMatrix();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void setColor( LedBase::LedColor color );
        LedBase::LedColor color();
        
        int  rows();
        void setRows( int rows );
        
        int  cols();
        void setCols( int cols );
        
        bool verticalPins();
        void setVerticalPins( bool v );
        
        double threshold();
        void   setThreshold( double threshold );

        double maxCurrent();
        void   setMaxCurrent( double current );
        
        double res();
        void  setRes( double resist );

        virtual void attach();
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    public slots:
        virtual void remove();

    private:
        void setupMatrix( int rows, int cols );
        void createMatrix();
        void deleteMatrix();
        
        std::vector<std::vector<LedSmd*>> m_led;
        std::vector<Pin*>    m_rowPin;
        std::vector<Pin*>    m_colPin;
        
        bool m_verticalPins;
        
        LedBase::LedColor m_ledColor;
        
        double m_resist;
        double m_maxCurr;
        double m_threshold;
        
        int m_rows;
        int m_cols;
};

#endif

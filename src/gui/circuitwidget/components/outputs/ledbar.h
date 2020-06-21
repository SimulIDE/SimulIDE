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

#ifndef LEDBAR_H
#define LEDBAR_H

#include "itemlibrary.h"
#include "component.h"
#include "ledsmd.h"

class MAINMODULE_EXPORT LedBar : public Component
{
    Q_OBJECT
    Q_PROPERTY( LedBase::LedColor Color READ color WRITE setColor     DESIGNABLE true USER true )
    Q_PROPERTY( int    Size       READ size       WRITE setSize    DESIGNABLE true USER true )
    Q_PROPERTY( double Threshold  READ threshold  WRITE setThreshold  DESIGNABLE true USER true )
    Q_PROPERTY( double MaxCurrent READ maxCurrent WRITE setMaxCurrent DESIGNABLE true USER true )
    Q_PROPERTY( double Resistance READ res        WRITE setRes        DESIGNABLE true USER true )
    Q_PROPERTY( bool   Grounded   READ grounded   WRITE setGrounded   DESIGNABLE true USER true )

    public:

        LedBar( QObject* parent, QString type, QString id );
        ~LedBar();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem *libraryItem();
        
        void setColor( LedBase::LedColor color );
        LedBase::LedColor color();
        
        int  size();
        void setSize( int size );
        
        double threshold();
        void  setThreshold( double threshold );

        double maxCurrent();
        void  setMaxCurrent( double current );
        
        double res();
        void  setRes( double resist );
        
        bool grounded();
        void setGrounded( bool grounded );
        
        void createLeds( int c );
        void deleteLeds( int d );
        
        virtual void paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget );
        
    public slots:
        virtual void remove();

    private:
        std::vector<LedSmd*> m_led;
        
        int m_size;
};

#endif

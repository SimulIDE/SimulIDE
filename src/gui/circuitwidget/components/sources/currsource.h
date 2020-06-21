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

#ifndef CURRSOURCE_H
#define CURRSOURCE_H

#include "component.h"
#include "e-source.h"
#include "voltwidget.h"
#include "pin.h"

class LibraryItem;
//class DialWidget;

class MAINMODULE_EXPORT CurrSource : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double  Current   READ current WRITE setCurrent DESIGNABLE true USER true )
    Q_PROPERTY( QString Unit      READ unit    WRITE setUnit    DESIGNABLE true USER true )
    Q_PROPERTY( bool    Show_Amp  READ showVal WRITE setShowVal DESIGNABLE true USER true )
    //Q_PROPERTY( double  Max_Volt  READ volt    WRITE setVolt    DESIGNABLE true USER true )

    public:
        CurrSource( QObject* parent, QString type, QString id );
        ~CurrSource();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void initialize();
        virtual void updateStep();

        double current() const      { return m_value; }
        void setCurrent( double c );
        
        void setUnit( QString un );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void currChanged( int volt );
        virtual void remove();

    private:
        void updateButton();
        
        bool m_changed;
        
        double m_current;
        double m_maxCurrent;
        
        Pin* outpin;
        
        VoltWidget m_voltw;

        QPushButton* m_button;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif


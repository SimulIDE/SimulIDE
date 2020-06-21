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

#ifndef VOLTSOURCE_H
#define VOLTSOURCE_H

#include "component.h"
#include "e-source.h"
#include "voltwidget.h"
#include "pin.h"

class LibraryItem;
//class DialWidget;

class MAINMODULE_EXPORT VoltSource : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( double  Voltage   READ volt    WRITE setVolt    DESIGNABLE true USER true )
    Q_PROPERTY( QString Unit      READ unit    WRITE setUnit    DESIGNABLE true USER true )
    Q_PROPERTY( bool    Show_Volt READ showVal WRITE setShowVal DESIGNABLE true USER true )

    public:
        VoltSource( QObject* parent, QString type, QString id );
        ~VoltSource();

        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        virtual void updateStep();

        double volt() const      { return m_value; }
        void setVolt( double v );
        
        void setUnit( QString un );

        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void onbuttonclicked();
        void voltChanged( int volt );
        virtual void remove();

    private:
        void updateButton();
        
        bool m_changed;
        
        double m_voltHight;
        double m_voltOut;
        Pin* outpin;
        eSource* m_out;
        
        VoltWidget m_voltw;

        QPushButton* m_button;
        QDial* m_dial;
        QGraphicsProxyWidget* m_proxy;
};

#endif


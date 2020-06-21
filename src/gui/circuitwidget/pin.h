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

#ifndef PIN_H
#define PIN_H

#include "component.h"
#include "connector.h"
#include "e-pin.h"

class MAINMODULE_EXPORT Pin : public QObject, public QGraphicsItem, public ePin
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    public:
        QRectF boundingRect() const { return m_area; }

        Pin( int angle, const QPoint &pos, QString id, int index, Component* parent = 0 );
        ~Pin();

        enum { Type = UserType + 3 };
        int type() const { return Type; }

        QString pinId();
        
        bool unused() {return m_unused; }
        void setUnused( bool unused );

        void setLength( int length );
        
        void setColor( QColor color ) { m_color = color; }
        void setPinAngle( int angle );
        int pinAngle() { return m_angle; }

        void setBoundingRect( QRect area );
        
        Component* component() { return m_component; }

        Connector* connector();
        void setConnector( Connector* c );

        void setConPin( Pin* pin );
        Pin* conPin();

        QString getLabelText();
        void setLabelPos();
        void setLabelColor( QColor color );
        void setFontSize( int size );

        void setVisible( bool visible );

        void moveBy( int dx, int dy );

        void reset();

        void registerPins( eNode* enode );
        void registerPinsW( eNode* enode );
        
        void setIsBus( bool bus );
        bool isBus();

        virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void isMoved();
        void setLabelText( QString label );
        void setPinId( QString id );

    protected:
        void mousePressEvent(QGraphicsSceneMouseEvent* event);

    private:
        int m_angle;
        int m_length;

        bool m_blocked;
        bool m_isBus;
        bool m_unused;

        QString m_labelText;
        
        QColor m_color;
        QRect      m_area;
        Connector* my_connector;
        Component* m_component;
        Pin*       m_conPin;          // Pin at the other side of connector

        QGraphicsSimpleTextItem m_label;
};

#endif

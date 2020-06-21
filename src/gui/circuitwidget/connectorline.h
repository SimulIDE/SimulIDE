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
 
#ifndef CONNECTORLINE_H
#define CONNECTORLINE_H

#include <QtWidgets>

class Connector;
 
class MAINMODULE_EXPORT ConnectorLine : public QGraphicsObject
{
    Q_OBJECT

    public:
        ConnectorLine( int x1, int y1, int x2, int y2, Connector*  connector );
        ~ConnectorLine();

        virtual QRectF boundingRect() const;

        void setConnector( Connector* con );
        Connector* connector();
        
        void setPrevLine( ConnectorLine* prevLine );
        void setNextLine( ConnectorLine* nextLine );

        void setP1( QPoint );
        void setP2( QPoint );

        QPoint p1();
        QPoint p2();

        int dx();
        int dy();
        
        bool isDiagonal();

        void move( QPointF delta );
        void moveLine( QPoint delta );
        void moveSimple( QPointF delta );

        void updatePos();
        void updateLines();
        void updatePrev();
        void updateNext();
        
        void setIsBus( bool bus );

        void mousePressEvent( QGraphicsSceneMouseEvent* event );
        void mouseMoveEvent( QGraphicsSceneMouseEvent* event );
        void mouseReleaseEvent( QGraphicsSceneMouseEvent* event );

        void contextMenuEvent( QGraphicsSceneContextMenuEvent* event );
        
        virtual QPainterPath shape() const;
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    signals:
        //void moved();
        //void yourP1changed( QPoint );
        //void yourP2changed( QPoint );

    public slots:
        void sSetP1( QPoint );
        void sSetP2( QPoint );
        void remove();

    private:
        int myIndex();
        int m_p1X;
        int m_p1Y;
        int m_p2X;
        int m_p2Y;
        
        bool m_isBus;
        bool m_moveP1;
        bool m_moveP2;
        bool m_moving;

        Connector* m_pConnector;
        ConnectorLine* m_prevLine;
        ConnectorLine* m_nextLine;
};

#endif


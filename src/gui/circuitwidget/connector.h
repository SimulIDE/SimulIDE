/***************************************************************************
 *   Copyright (C) 2012 by Santiago González                               *
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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "component.h"

class ConnectorLine;
class eNode;

class MAINMODULE_EXPORT Connector : public Component
{
    Q_OBJECT
    Q_PROPERTY( QStringList pointList  READ pointList  WRITE setPointList )
    Q_PROPERTY( QString     startpinid READ startPinId WRITE setStartPinId )
    Q_PROPERTY( QString     endpinid   READ endPinId   WRITE setEndPinId )
    Q_PROPERTY( QString     enodeid    READ enodId     WRITE setEnodId )

    public:

        QRectF boundingRect() const { return QRect( 0, 0, 1, 1 ); }

        Connector( QObject* parent, QString type, QString id, Pin* startpin, Pin* endpin = 0l );
        ~Connector();

        // PROPERTIES-----------------------------------
        QStringList pointList();
        void setPointList( QStringList pl );

        QString startPinId();
        void setStartPinId( QString pinid);
        QString endPinId();
        void setEndPinId( QString pinid);

        QString enodId();
        void setEnodId( QString /*nodid*/ ){;}
        // END PROPERTIES-------------------------------

        void refreshPointList();

        Pin* startPin();
        void setStartPin( Pin* pin);
        Pin* endPin();
        void setEndPin( Pin* pin);

        eNode* enode();
        void setEnode( eNode* enode );

        double getVolt();

        QList<ConnectorLine*>* lineList();

        void incActLine();

        ConnectorLine* addConLine( int x1, int y1, int x2, int y2, int index );

        void addConLine( ConnectorLine* line, int index );
        void remConLine( ConnectorLine* line  );
        void remNullLines();
        void remLines();

        void updateConRoute( Pin* nod, QPointF this_point );

        /**
        * Finish the creation of this connector,
        * adding the end-pin
        */
        void closeCon( Pin* endpin, bool connect=false );

        /**
        * Split this connector in two, the line at index will be the first of new connector,
        * pin1 will be endpin of this connector,
        * pin2 will be se startpin of the new connector,
        * and this connector endpin (previous) will be endpin of the new connector
        */
        void splitCon( int index, Pin* pin1, Pin* pin2 );

        void updateLines();
        
        void setIsBus( bool bus );
        bool isBus();
        
        bool m_freeLine;

    signals:
        void selected(bool yes);

    public slots:
        virtual void remove();
        virtual void move( QPointF delta );
        virtual void setSelected( bool selected );

    private:
        void updateCon();
        /**
        * Connect signals to update lines positions of lines at inex1 and index2,
        * note that the lines are connected in the given order,
        * this is: end point of line1 to start point of line2.
        */
        void connectLines( int index1, int index2 );
        void disconnectLines( int index1, int index2 );

        int m_actLine;
        int m_lastindex;
        
        bool m_isBus;
        
        QString m_startpinid;
        QString m_endpinid;

        Pin*    m_startPin;
        Pin*    m_endPin;

        QStringList  m_pointList;

        QList<ConnectorLine*> m_conLineList;
};

#endif

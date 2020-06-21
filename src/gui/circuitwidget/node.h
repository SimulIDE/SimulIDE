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

#ifndef NODE_H
#define NODE_H

#include "component.h"
#include "pin.h"

class MAINMODULE_EXPORT Node : public Component
{
    Q_OBJECT
    public:
        QRectF boundingRect() const { return QRect( -4, -4, 8, 8 ); }

        Node( QObject* parent, QString type, QString id );
        ~Node();

        Pin* getPin( int pin ) const { return m_pin[pin]; }

        void registerPins( eNode* enode );

        //virtual void setChanged( bool changed );

        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void inStateChanged( int rem=1 );
        void remove();
        
    protected:
        void contextMenuEvent(QGraphicsSceneContextMenuEvent* event){;}

    private:
        void joinConns( int co0, int c1);

        Pin* m_pin[3];
        
        bool m_isBus;
};
#endif

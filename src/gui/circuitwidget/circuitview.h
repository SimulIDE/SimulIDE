/***************************************************************************
 *   Copyright (C) 2010 by santiago González                               *
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

#ifndef CIRCUITVIEW_H
#define CIRCUITVIEW_H

#include <QtWidgets>


class Component;
class Circuit;

class CircuitView : public QGraphicsView
{
    Q_OBJECT

    public:
        CircuitView( QWidget *parent );
        ~CircuitView();

 static CircuitView* self() { return m_pSelf; }
 
        void clear();
        
        void wheelEvent( QWheelEvent *event );
        void dragMoveEvent( QDragMoveEvent* event );
        void dragEnterEvent( QDragEnterEvent* event );
        void dragLeaveEvent( QDragLeaveEvent* event );

        void mousePressEvent( QMouseEvent* event );
        void mouseReleaseEvent( QMouseEvent* event );

        void resizeEvent( QResizeEvent* event );
        
        void setCircTime( uint64_t step);

    public slots:
        void saveImage();
        void slotPaste();
        void importCirc();
        
    protected:
        void contextMenuEvent( QContextMenuEvent* event );

    private:
 static CircuitView*  m_pSelf;
 
        QPlainTextEdit* m_info;
 
        Component*  m_enterItem;
        Circuit*    m_circuit;

        QPointF m_eventpoint;
};

#endif

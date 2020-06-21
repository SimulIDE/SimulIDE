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

#ifndef RENDERAREA_H
#define RENDERAREA_H

#include <QBrush>
#include <QPen>
#include <QPixmap>
#include <QtWidgets>

class RenderArea : public QWidget
{
    Q_OBJECT

    public:
        RenderArea( int width, int height, QWidget *parent = 0 );

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        void setData(const int channel, double data );
        void printData();
        void drawVmark();
         
        void setTick( int tickUs );
        
        void setZero( int zero );
        void setXScale( int scale );

    public slots:
        void setPen( const int channel, const QPen &pen );
        void setBrush( const QBrush &brush );
        void setAntialiased( const bool antialiased );

    protected:
        void paintEvent( QPaintEvent *event );

    private:
        QPen m_pen[4];
        QBrush brush;
        bool antialiased;
        QPixmap pixmap;

        int m_data[4];
        int m_dataP[4];
        int m_width;
        int m_height;
        int m_sec;
        int m_zero0;
        int m_zero1;
        int m_xScale;
         
        QString m_tick;
 };

 #endif


/***************************************************************************
*   Copyright (C) 2016 by santiago González                               *
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

#ifndef RENDEROSCOPE_H
#define RENDEROSCOPE_H

#include <QPixmap>
#include <QtWidgets>

class RenderOscope : public QWidget
{
    Q_OBJECT

    public:
        RenderOscope( int width, int height, QWidget *parent = 0 );

        QSize minimumSizeHint() const;
        QSize sizeHint() const;

        void setData( int data[] );
        void setMaxMin( double max, double min );

    protected:
        void paintEvent( QPaintEvent *event );

    private:
        int m_width;
        int m_height;
        int* m_data;
        
        double m_hCenter;
        double m_vCenter;
        double m_vMax;
        double m_vMin;
        double m_margin;
        double m_scale;
        
        QPointF lastPoint;
        
        int m_sec;
};

#endif


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

#ifndef PLOTTERWIDGET_H
#define PLOTTERWIDGET_H

#include <QtWidgets>

class RenderArea;

class MAINMODULE_EXPORT PlotterWidget : public QWidget
{
    Q_OBJECT
    
    Q_PROPERTY( QString  itemtype  READ itemType )
    Q_PROPERTY( double MaxVolt READ maxVolt  WRITE setMaxVolt )
    Q_PROPERTY( double MinVolt READ minVolt  WRITE setMinVolt )
    Q_PROPERTY( int    Scale   READ xScale    WRITE xScaleChanged )
    Q_PROPERTY( int    Tracks  READ tracks   WRITE setTracks )

    public:
        PlotterWidget( QWidget *parent );
        ~PlotterWidget();

 static PlotterWidget* self() { return m_pSelf; }
 
        QString itemType(){ return "Plotter"; }

        //int  getChannel();
        bool addChannel( int channel );
        void remChannel( int channel );

        QColor getColor( int channel );

        void clear();
        void step();
        void setData(int channel, double data );
        void setTicksPs( int tps );
        void setPlotterTick( int tickUs );
        
        double maxVolt();
        void setMaxVolt( double volt );
        
        double minVolt();
        void setMinVolt( double volt );
        
        int xScale() { return m_xScale; }
        
        int tracks();

    public slots:
        void maxChanged( double value );
        void minChanged( double value );
        void xScaleChanged( int scale );
        void setTracks( int tracks );
        
    private:
 static PlotterWidget* m_pSelf;

        void setupWidget();
        void setRenderData(int channel, double data );
        void setScale();

        QHBoxLayout* m_horizontalLayout;
        QVBoxLayout* m_verticalLayout;
        QLineEdit*   m_chanLabel[4];
        QDoubleSpinBox* m_maxValue;
        QDoubleSpinBox* m_minValue;
        QSpinBox*       m_XScale;
        QSpinBox*       m_tracks;

        RenderArea*  m_rArea;

        QColor m_color[4];

        bool m_channel[4];

        int  m_numchan;
        int  m_counter;
        int  m_ticksPs;
        int  m_xScale;
        int  m_numTracks;
        
        double m_data[4];
        double m_maxVolt;
        double m_minVolt;
        double m_offset;
};

#endif


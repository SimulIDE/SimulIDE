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

 #include "renderoscope.h"
 
#include <QBrush>
#include <QPen>

RenderOscope::RenderOscope( int width, int height, QWidget *parent )
            : QWidget( parent )
{
    m_width  = width;
    m_height = height;
    m_hCenter = (double)width/2;
    m_vCenter = (double)height/2;
    m_scale = ((double)width)/180;
    m_margin = 15*m_scale;
    m_scale = ((double)width-30*m_scale)/140;
    m_vMax = 0;
    m_vMin = 0;
    m_data = 0l;
}

QSize RenderOscope::minimumSizeHint() const  {  return QSize( m_width, m_height );  }

QSize RenderOscope::sizeHint() const  { return QSize( m_width, m_height ); }

void RenderOscope::setMaxMin( double max, double min )
{
    m_vMax = max;
    m_vMin = min;
}
void RenderOscope::setData( int data[] )
{
    m_data = data;
}

void RenderOscope::paintEvent( QPaintEvent* /* event */ )
{
    QPainter p( this );

    p.setRenderHint( QPainter::Antialiasing, true );
    
    p.setBrush( QColor( 10, 15, 50 ) );
    p.drawRoundRect(0, 0, m_width, m_height );

    QPen pen( QColor( 90, 90, 180 ), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p.setPen( pen );
    
    double cero = m_margin;
    double end = m_width-m_margin;
    p.drawLine( cero, m_vCenter, end, m_vCenter );
    p.drawLine( m_hCenter, cero, m_hCenter, end );
    
    p.setPen( QColor( 70, 70, 140 ) );
    
    for( double i=cero; i<end+1; i+=20*m_scale )
    {
        p.drawLine( i, cero, i, end );
        p.drawLine( cero, i, end, i );
    }
    
    if( m_data )
    {
        QPen pen2( QColor( 240, 240, 100 ), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        
        p.setPen( pen2 );
        
        QPointF lastP = QPointF( m_margin, end-(double)m_data[0]*m_scale );
        for( int i=1; i<140; i++ )
        {
            QPointF thisP = QPointF( (double)i*m_scale+m_margin, end-(double)m_data[i]*m_scale );
            p.drawLine( lastP, thisP );
            lastP = thisP;
        }
    }
    
    QPen pen3( QColor( 200, 200, 200 ), 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p.setPen( pen3 );
    
    QFont font = p.font();
    font.setPixelSize(9);
    font.setBold(true);
    p.setFont( font );
    
    p.drawText( cero, 1,     end, m_margin, Qt::AlignCenter, "Max: "+QString::number(m_vMax,'f', 2)+" V" );
    p.drawText( cero, end+1, end, m_margin, Qt::AlignCenter, "Min: "+QString::number(m_vMin,'f', 2)+" V" );
    
    p.end();
}

#include "moc_renderoscope.cpp"

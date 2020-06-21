/***************************************************************************
 *   Copyright ( C ) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   ( at your option ) any later version.                                   *
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

 #include "renderarea.h"

 RenderArea::RenderArea( int width, int height, QWidget *parent )
           : QWidget( parent )
 {
     antialiased = false;

     setBackgroundRole( QPalette::Base );
     setAutoFillBackground( true );

     m_width  = width; //1000;
     m_height = height; //180;
     setZero( 0 );
     setXScale( 1 );

     QFont font;
     font.setPixelSize(9);
     pixmap = QPixmap( m_width, m_height );
     QPainter p( &pixmap );
     p.setFont( font );
     p.end();

     //drawBackground();
     for( int i=0; i<4; i++ )
     {
         m_data[i] = 0;
         m_dataP[i] = 0;
     }

     m_sec = 0;
 }

void RenderArea::drawVmark()
{
    m_sec++;

    int last = m_width-m_xScale;
    QString sec = "";

    QPainter p( &pixmap );
    /*if( m_sec == 10 )
     {
         m_sec = 0;
         p.setPen( QColor( 90, 90, 150 ) );
     }
     else p.setPen( QColor( 50, 50, 100 ) );*/
    if( m_sec == 10 )
    {
        m_sec = 0;
        QPen pen( QColor( 170, 170, 255 ), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen );
    }
    else 
    {
        QPen pen( QColor( 100, 100, 200 ), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        p.setPen( pen );
    }
    sec.setNum(m_sec);

    //int base = m_height-20;

    p.drawLine( last, 0, last, m_height );
    //p.drawText( last-10, base-5, 10, 20, Qt::AlignHCenter, sec );
    p.end();
}

 QSize RenderArea::minimumSizeHint() const  {  return QSize( 100, 150 );  }

 QSize RenderArea::sizeHint() const  { return QSize( 400, 200 ); }

 void RenderArea::setPen( int channel, const QPen &pen )
 {
     this->m_pen[channel] = pen;
     //update();
 }

 void RenderArea::setBrush( const QBrush &brush )
 {
     this->brush = brush;
     //update();
 }

 void RenderArea::setAntialiased( const bool antialiased )
 {
     this->antialiased = antialiased;
     //update();
 }

void RenderArea::setData( const int channel, double data )
{
    data = (data/2+250)*m_height/520;
    data = m_height-data-4;
    m_dataP[channel] = m_data[channel];
    m_data[channel]  = data;
}
 
void RenderArea::setZero( int zero )
{
    int zero0 = zero/2-250;
    m_zero0 = (zero0/2+250)*m_height/520;
    m_zero0 = m_height-m_zero0-4;
    
    int zero1 = zero/2+250;
    m_zero1 = (zero1/2+250)*m_height/520;
    m_zero1 = m_height-m_zero1-4;
}

void RenderArea::setXScale( int scale )
{
    m_xScale = scale;
}

void RenderArea::printData()
{
    pixmap.scroll( -m_xScale, 0, pixmap.rect() );

    QPainter p( &pixmap );
    
    int last = m_width-m_xScale;
    
    //p.setPen( QColor( 10, 15, 50 ) );
    QPen pen( QColor( 5, 10, 30 ), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p.setPen( pen );
    
    for( int i=0; i<m_xScale; i++ )
    {
        p.drawLine( last+i, 0, last+i, m_height ); // erase previous line

        /*p.setPen( QColor( 150, 150, 170 ) );
        p.drawLine( last-m_xScale, m_zero0, last, m_zero0 ); // Linea de base ( 0 ch 0-1)
        p.drawLine( last-m_xScale, m_zero1, last, m_zero1 ); // Linea de base ( 0 ch 2-3)
        //p.drawLine( last-1, m_height-33, last, m_height-33 );*/
    }
    if( antialiased ) p.setRenderHint( QPainter::Antialiasing, true );

    for( int i=0; i<4; i++ )
    {
        int data  = m_data[i];
        int dataP = m_dataP[i];
        if(( data > 60000 ) || ( dataP > 60000 )) continue;
        
        p.setPen( m_pen[i] );
        p.drawLine( last-m_xScale, dataP, last, data );
        m_dataP[i] = data;
    }
    p.end();
    update();
}
 
void RenderArea::setTick( int tickUs )
{
    QString unit = " S";
    
    double temp = tickUs/1e6;
    
    if( temp < 1 )
    {
        unit = " mS";
        temp = tickUs/1e3;
        if( temp < 1 )
        {
            unit = " uS";
            temp = tickUs;
        }
    }
    m_tick.setNum(temp);
    m_tick += unit;
    //qDebug() << "RenderArea::setTick" << m_tick;
}

void RenderArea::paintEvent( QPaintEvent * /* event */ )
{
    QPainter painter( this );

    //painter.drawPixmap( 0, 0, pixmap );
    int origX = width()-m_width;
    //int origy = 0; //height(),
    painter.drawPixmap( origX, 0, /*m_width, height(),*/ pixmap );
    painter.setPen( QColor( 255, 255, 255 ) );

    painter.drawText( 0, 5, 100, 20, Qt::AlignHCenter, "Tick: "+m_tick );

    painter.end();
}

#include "moc_renderarea.cpp"

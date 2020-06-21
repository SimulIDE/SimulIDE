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

#include "plotterwidget.h"
#include "renderarea.h"
#include "mainwindow.h"
#include "simulator.h"

PlotterWidget* PlotterWidget::m_pSelf = 0l;

PlotterWidget::PlotterWidget(  QWidget *parent  )
             : QWidget( parent )
{
    m_pSelf = this;
    setObjectName( "PlotterWidget" );

    this->setVisible( false );

    setMinimumSize(QSize(200, 200));
    setMaximumSize(QSize(1000, 200));

    m_color[0] = QColor( 190, 190, 0 );
    m_color[1] = QColor( 255, 110, 50 );
    m_color[2] = QColor( 100, 120, 255 );
    m_color[3] = QColor( 0, 230, 100 );

    setupWidget();
    
    m_maxVolt = 500;
    m_minVolt = -500;
    m_offset  = 0;
    m_numTracks = 1;
    m_xScale = -1;

    m_rArea->setAntialiased(true);

    m_ticksPs = 20;
    m_counter = 0;
    m_numchan = 0;

    for( int i=0; i<4; i++ )
    {
        m_data[i] = -1000;
        m_channel[i] = false;
    }
    xScaleChanged( 1 );
    clear();
}
PlotterWidget::~PlotterWidget(){ }

void PlotterWidget::clear()
{
    int numchan = m_numchan;
    m_numchan = 1;
    for( int i=0; i<1000; i++ ) step();
    m_numchan = numchan;
}

/*int PlotterWidget::getChannel()
{
    for( int i=0; i<4; i++ )                   // Find available channel
    {
        if( m_channel[i] == true ) continue;

        addChannel( i+1 );
        return i+1;                    // return channel number assigned
    }
    return 0;                              // -1 = not channel available
}*/

bool PlotterWidget::addChannel( int channel )
{
    channel--;
    
    if( m_channel[channel] ) return false;
    m_numchan++;                                    // Inc used channels
    m_channel[channel] = true;                    // Set channel to busy
    m_chanLabel[channel]->setEnabled( true );
    m_chanLabel[channel]->setText( " 0.00 V" );
    if( m_numchan > 0 ) setVisible( true );
    return true;
}

void PlotterWidget::remChannel( int channel )
{
    channel--;
    if( channel < 0 || channel > 3 || m_channel[channel] == false ) return; // Nothing to do
    
    setData( channel+1, 0 );

    m_numchan--;                                // Decrease used channel
    m_channel[channel] = false;               // Set channel to not busy
    m_chanLabel[channel]->setEnabled( false );  // Disable channel label
    //m_data[channel] = 0;                                 // reset data
    m_chanLabel[channel]->setText( "--.-- V" );
    m_rArea->setData( channel, 0 );
    
    if( m_numchan == 0 )               // Hide this if no channel active
    {
        Simulator::self()->setTimerScale( 1 );
        setVisible( false );
        clear();
    }
}

QColor PlotterWidget::getColor( int channel )
{
    channel--;
    return m_color[channel];
}

void PlotterWidget::step()
{
    if( m_numchan == 0 ) return; // No data to plot
    //qDebug() << "PlotterWidget::step" << m_numchan;

    if( ++m_counter >= m_ticksPs )
    {
        m_counter = 0;
        m_rArea->drawVmark();
    }
    for( int i=0; i<4; i++ )                   // Find available channel
    {
        if( m_channel[i] == false ) setRenderData( i, 1e9 ); // Don't plot
        else                        setRenderData( i, m_data[i] );
    }
    m_rArea->printData();
}

void PlotterWidget::setData( int channel, double data )
{
    channel--;
    if( data == m_data[channel] ) return;

    float vf = data;
    vf = vf/100;
    QString volt;
    volt.setNum( vf );

    if     ( !volt.contains(".") )                  volt.append(".00");
    else if( volt.split(".").last().length() == 1 ) volt.append("0");

    volt.append( " V" );
    m_chanLabel[channel]->setText( volt );   // Update volt Label

    //if( data>m_maxVolt || data<m_minVolt ) data = 1e9;
    m_data[channel] = data;
    //setRenderData( channel, data );
}

void PlotterWidget::setRenderData( int channel, double data )
{
    double renderData = data*1000/(m_maxVolt-m_minVolt)-m_offset;
    renderData /= m_numTracks;

    if( m_numTracks == 2 )
    {
        if( channel > 1 ) renderData -= 250;
        else              renderData += 250;
    }
    else if( m_numTracks == 4 )
    {
        renderData += 375-channel*250;
    }
    m_rArea->setData( channel, renderData );
}

void PlotterWidget::setTicksPs( int tps )
{
    m_ticksPs = tps;
}

void PlotterWidget::setPlotterTick( int tickUs )
{
    m_rArea->setTick( tickUs );
}

double PlotterWidget::maxVolt() 
{ 
    return m_maxVolt; 
}

void PlotterWidget::setMaxVolt( double volt ) 
{ 
    m_maxValue->setValue( volt/100 );
}

double PlotterWidget::minVolt() 
{ 
    return m_minVolt; 
}
void PlotterWidget::setMinVolt( double volt ) 
{ 
     m_minValue->setValue( volt/100 ); 
}
        
void PlotterWidget::maxChanged( double value )
{
    m_maxVolt = value*100;
    if( m_maxVolt <= m_minVolt ) 
    {
        m_maxVolt = m_minVolt+1;
        m_maxValue->setValue( m_maxVolt/100 );
    }
    setScale();
}

void PlotterWidget::minChanged( double value )
{
    m_minVolt = value*100;
    if( m_maxVolt <= m_minVolt ) 
    {
        m_minVolt = m_maxVolt-1;
        m_minValue->setValue( m_minVolt/100 );
    }
    setScale();
}

void  PlotterWidget::xScaleChanged( int scale )
{
    if( m_numchan == 0 ) scale = 1;
    if( scale == m_xScale ) return;
    
    if( scale == 3 )
    {
        if( m_xScale == 2 ) scale = 4;
        else                scale = 2;
    }
    m_xScale = scale;
    m_XScale->setValue( scale );
    
    m_ticksPs = 20;
    int timSc = 1;
    
    if( scale > 1) 
    {
        timSc = 2;
        m_ticksPs = 40;
    }
    Simulator::self()->setTimerScale( timSc );
    if( m_numchan > 0 )Simulator::self()->simuRateChanged( Simulator::self()->simuRate() );
    
    if( scale > 2 ) scale = 2;
    else            scale = 1;
    m_rArea->setXScale( scale );
}

void PlotterWidget::setScale()
{
    m_offset = (m_maxVolt+m_minVolt)*500/(m_maxVolt-m_minVolt);
    m_rArea->setZero( -m_offset );
    
    for( int i=0; i<4; i++ ) 
        if( m_channel[i] ) setRenderData( i, m_data[i] );
}

int PlotterWidget::tracks()
{
    return m_numTracks;
}

void PlotterWidget::setTracks( int tracks )
{
    if( tracks == m_numTracks ) return;
    if( tracks == 3 )
    {
        if( m_numTracks == 2 ) tracks = 4;
        else                   tracks = 2;
    }
    m_numTracks = tracks;
    m_tracks->setValue( tracks );
}

void PlotterWidget::setupWidget()
{
    m_horizontalLayout = new QHBoxLayout( this );
    m_horizontalLayout->setObjectName( "horizontalLayout");
    //m_horizontalLayout.setContentsMargins(0, 0, 0, 0);
    //m_horizontalLayout.setSpacing(0);
    m_verticalLayout = new QVBoxLayout();
    m_verticalLayout->setObjectName( "verticalLayout");
    m_verticalLayout->setContentsMargins(0, 0, 0, 0);
    m_verticalLayout->setSpacing(1);

    QFont font;
    font.setPixelSize( 14*MainWindow::self()->fontScale() );

    for( int i=0; i<4; i++ )    // Create volt Labels
    {
        m_chanLabel[i] = new QLineEdit( this );
        m_chanLabel[i]->setObjectName(QString::fromUtf8("voltLabel"+i));
        m_chanLabel[i]->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        m_chanLabel[i]->setMaxLength(7);
        m_chanLabel[i]->setFont(font);
        m_chanLabel[i]->setAcceptDrops(false);
        m_chanLabel[i]->setReadOnly(true);
        m_chanLabel[i]->setText("--.-- V");
        m_chanLabel[i]->setFixedHeight(20);
        m_chanLabel[i]->setFixedWidth(85);
        m_chanLabel[i]->setVisible( true );
        m_chanLabel[i]->setEnabled( false );

        QPalette p = m_chanLabel[i]->palette();
        p.setColor( QPalette::Active, QPalette::Base, m_color[i] );
        m_chanLabel[i]->setPalette(p);
        m_verticalLayout->addWidget( m_chanLabel[i] );
    }
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_verticalLayout->addWidget(line);
    
    m_maxValue = new QDoubleSpinBox( this );
    m_maxValue->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    m_maxValue->setMaximum( 1000000 );
    m_maxValue->setMinimum(-1000000 );
    m_maxValue->setPrefix( tr("Max ") );
    m_maxValue->setValue( 5 );
    m_verticalLayout->addWidget( m_maxValue );
    connect( m_maxValue, SIGNAL( valueChanged(double) ),
                   this, SLOT( maxChanged(double) ));
    
    m_minValue = new QDoubleSpinBox( this );
    m_minValue->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    m_minValue->setMaximum( 1000000 );
    m_minValue->setMinimum(-1000000 );
    m_minValue->setPrefix( tr("Min  ") );
    m_minValue->setValue( -5 );
    m_verticalLayout->addWidget( m_minValue );
    connect( m_minValue, SIGNAL( valueChanged(double) ),
                   this, SLOT( minChanged(double) ));
                   
    m_XScale = new QSpinBox( this );
    m_XScale->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    m_XScale->setMaximum( 4 );
    m_XScale->setMinimum( 1 );
    m_XScale->setPrefix( tr("Scale:   ") );
    m_XScale->setValue( 1 );
    m_verticalLayout->addWidget( m_XScale );
    connect( m_XScale, SIGNAL( valueChanged(int) ),
                 this, SLOT( xScaleChanged(int) ));
                   
    m_tracks = new QSpinBox( this );
    m_tracks->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    m_tracks->setMaximum( 4 );
    m_tracks->setMinimum( 1 );
    m_tracks->setPrefix( tr("Tracks: ") );
    m_tracks->setValue( 1 );
    m_verticalLayout->addWidget( m_tracks );
    connect( m_tracks, SIGNAL( valueChanged(int) ),
                 this, SLOT( setTracks(int) ));
        
    m_horizontalLayout->addLayout( m_verticalLayout );

    m_rArea = new RenderArea( 1000, 180, this );
    m_rArea->setObjectName( "oscope" );

    QPen pen( m_color[0], 2.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );

    for( int i=0; i<4; i++ )
    {
        pen.setColor( m_color[i] );
        m_rArea->setPen( i, pen );
    }
    m_horizontalLayout->addWidget(m_rArea);
}

#include "moc_plotterwidget.cpp"

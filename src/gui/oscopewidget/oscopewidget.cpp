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

#include "oscopewidget.h"
#include "simulator.h"
#include "oscope.h"

#include <math.h>   // fabs(x,y)

OscopeWidget::OscopeWidget(  QWidget *parent  )
            : QWidget( parent )
            , eElement( "oscope" )
{
    this->setVisible( false );

    setMaximumSize(QSize(300, 200));
    setMinimumSize(QSize(300, 200));

    m_prevHscale = 10;
    m_Hscale     = 10;
    m_prevHpos = 0;
    m_Hpos     = 0;
    
    m_prevVscale = 1;
    m_Vscale     = 1;
    m_prevVpos = 0;
    m_Vpos     = 0;
    
    m_ampli = 0;
    m_filter = 0.3;

    m_newRead = true;
    m_auto = true;
    
    m_oscope  = 0l;
    //clear();
}
OscopeWidget::~OscopeWidget()
{ 
}

void OscopeWidget::resetState()
{
    clear();
}

void OscopeWidget::setOscope( Oscope* oscope )
{
    m_oscope = oscope;
    
    if( oscope  ) Simulator::self()->addToSimuClockList( this );
    else          Simulator::self()->remFromSimuClockList( this );
}

void OscopeWidget::clear()
{
    for( int i=0; i<140; i++ ) m_data[i] = 2.5*28;
    m_display->setData( m_data );
    
    m_newRead = true;
    m_rising   = false;
    m_falling  = false;
    
    m_step = 0;
    m_totalP = 0;
    m_numCycles = 0;
    m_numMax = 0;
    m_lastData =0;
    m_counter = 0;
    m_ampli = 0;
    m_tick = 0;
    m_max =-1e12;
    m_min = 1e12;
    Hpos = 0;
    m_freq = 500;
    
    m_freqLabel->setText( "Frq: 000 Hz" );
    m_ampLabel->setText( "Amp: 0.00 V" );
}

void OscopeWidget::read()
{
    m_stepsPerS = Simulator::self()->stepsPerus()*1e6;

    if( m_step > m_stepsPerS ) clear();
    if( m_numMax < 2 ) return;
    m_numMax--;
    
    double freq = m_stepsPerS/(double)(m_totalP/m_numMax);
    //qDebug() <<"Frequencimeter::simuClockStep"<<m_totalP<<m_numMax<<m_totalP/m_numMax;
    if( m_freq != freq )
    {
        m_freq = freq;
        if     ( freq >= 10000 ) m_freqLabel->setText( "Frq: "+QString::number( freq, 'f', 0 )+" Hz" );
        else if( freq >= 1000 )  m_freqLabel->setText( "Frq: "+QString::number( freq, 'f', 1 )+" Hz" );
        else                     m_freqLabel->setText( "Frq: "+QString::number( freq, 'f', 2 )+" Hz" );
    }
    m_totalP = 0;
    m_numMax = 0;
    m_lastMax = 0;
    m_step   = 0;
    
        m_reading = true;
        
    double tick = 20*m_Hscale;
    double val = tick/m_stepsPerS;
    QString unit = " S";
    
    if( val < 1 )
    {
        unit = " mS";
        val = 1e3*tick/m_stepsPerS;
        if( val < 1 )
        {
            unit = " uS";
            val = tick;
        }
    }
    m_tickLabel->setText( "Div:  "+QString::number( val,'f', 2)+unit );
    m_ampLabel->setText(  "Amp: " +QString::number( m_ampli,'f', 2)+" V" );
    m_display->update();
}

void OscopeWidget::simuClockStep()
{
    m_step++;

    double data = m_oscope->getVolt();
    double delta = data-m_lastData;
    
    if( data > m_max ) m_max = data;
    if( data < m_min ) m_min = data;
    
    if( delta > m_filter )                                     // Rising 
    {
        if( m_falling && !m_rising )                        // Min Found
        {
            if( m_numMax > 0 ) m_totalP += m_step-m_lastMax;

            m_lastMax = m_step;
            m_numMax++;
            m_numCycles++;

            m_ampli = m_max-m_min;
            m_display->setMaxMin( m_max, m_min );
            m_falling = false;
            m_max = -1e12;
        }
        m_rising = true;
        m_lastData = data;

        if( m_reading && m_newRead && ( m_numCycles > 1 ))// Wait for a full wave
        {
            double mid = m_min + m_ampli/2;

            if( data >= mid )                             // Rising edge
            {
                m_numCycles = 0;
                int per = m_stepsPerS/m_freq;
                if( per > 1 )
                {
                    if( m_auto ) 
                    {
                        m_Vpos = mid;
                        m_Hpos = 0;
                        m_Vscale = 5/m_ampli;
                        if     ( m_Vscale > 1000 )  m_Vscale = 1000;
                        else if( m_Vscale < 0.001 ) m_Vscale = 0.001;
                        
                        m_Hscale = abs(per/70)+1;
                        if( m_Hscale > 100000 ) m_Hscale = 100000;
                    }
                    Hpos = 0;
                    m_tick = 0;
                    m_counter = 0;
                    m_newRead = false;
                }
            }
        }
    }
    else if( delta < -m_filter )                              // Falling
    {
        if( m_rising && !m_falling )                        // Max Found
        {
            //m_ampli = m_max-m_min;
            //m_display->setMaxMin( m_max, m_min );
            m_rising = false;
            m_min = 1e12;
        }
        m_falling = true;
        m_lastData = data;
    }
    if( m_counter == 140 )                   // DataSet Ready to display
    {
        m_display->setData( m_data );
        m_newRead = true;
        m_reading = false;
        m_numCycles = 0;
        m_counter = 0;
        return; 
    }
    if( m_newRead == false )                          // Data Set saving
    {
        if( m_counter == 140 ) return;          // Done, Wait for update

        if( Hpos < m_Hpos ) Hpos++;                 // Wait for H offset
        else                                                // Save data 
        {
            if(( m_counter == 0 )||( ++m_tick == m_Hscale ))
            {
                m_data[m_counter] = ((data-m_Vpos)*m_Vscale+2.5)*28;

                m_counter++;
                m_tick = 0;
            }
        }
    }
}

void OscopeWidget::HscaleChanged( int Hscale )
{
    if( m_auto ) return;
    
    if( Hscale < m_prevHscale ) 
    {
        m_Hscale++;
        if( m_Hscale > 10000 ) m_Hscale = 10000;
    }
    else
    {
        m_Hscale--;
        if( m_Hscale < 1 ) m_Hscale = 1;
    }
    m_prevHscale = Hscale;
}
void OscopeWidget::VscaleChanged( int Vscale )
{
    if( m_auto ) return;
    
    double vscale = (double)Vscale;
    if( vscale > m_prevVscale ) 
    {
        m_Vscale *= 1.01;
        if( m_Vscale > 1000 ) m_Vscale = 1000;
    }
    else
    {
        m_Vscale /= 1.01;
        if( m_Vscale < 0.001 ) m_Vscale = 0.001;
    }
    m_prevVscale = vscale;
}

void OscopeWidget::HposChanged( int hPos )
{
    if( m_auto ) return;
    
    if( hPos < m_prevHpos ) 
    {
        m_Hpos += m_Hscale;
        if( m_Hpos > 200*m_Hscale ) m_Hpos = 200*m_Hscale;
    }
    else
    {
        m_Hpos -= m_Hscale;
        if( m_Hpos < 0 ) m_Hpos = 0;
    }
    m_prevHpos = hPos;
}

void OscopeWidget::VposChanged( int Vpos )
{
    if( m_auto ) return;
    
    double vpos = (double)Vpos;
    
    if( vpos < m_prevVpos ) 
    {
        m_Vpos += 0.02*m_Vscale;
    }
    else
    {
        m_Vpos -= 0.02*m_Vscale;
    }
    m_prevVpos = vpos;
}

void OscopeWidget::autoChanged( int au )
{
    m_auto = au;
}

void OscopeWidget::setupWidget(  int size  )
{
    m_horizontalLayout = new QHBoxLayout( this );
    m_horizontalLayout->setObjectName( "horizontalLayout" );
    m_horizontalLayout->setContentsMargins(2, 2, 2, 2);
    //m_horizontalLayout.setSpacing(0);
    
    m_verticalLayout = new QVBoxLayout();
    m_verticalLayout->setObjectName( "verticalLayout" );
    m_verticalLayout->setContentsMargins(0, 0, 0, 0);
    m_verticalLayout->setSpacing(0);
    
    m_freqLabel = new QLabel( "Freq: 0 Hz", this );
    
    QFont font = m_freqLabel->font();
    font.setPixelSize(10);
    font.setBold(true);
    
    m_freqLabel->setFont(font);
    m_freqLabel->setFixedSize( 85, 14 );
    m_verticalLayout->addWidget(m_freqLabel);
    
    m_ampLabel  = new QLabel( "Amp: 0.00 V", this );
    m_ampLabel->setFont(font);
    m_ampLabel->setFixedSize( 85, 14 );
    m_verticalLayout->addWidget(m_ampLabel);
    
    m_tickLabel  = new QLabel( "Div:  0 S", this );
    m_tickLabel->setFont(font);
    m_tickLabel->setFixedSize( 85, 14 );
    m_verticalLayout->addWidget(m_tickLabel);
    
    QHBoxLayout* row2Layout = new QHBoxLayout();
    row2Layout->setObjectName( "row2Layout" );
    row2Layout->setContentsMargins(0, 0, 0, 0);
    row2Layout->setSpacing(0);
    
    m_autoCheck = new QCheckBox( "Auto", this );
    //m_autoCheck->setLayoutDirection(Qt::RightToLeft);
    m_autoCheck->setChecked( true );
    m_autoCheck->setFixedSize( 38, 16 );
    font.setPixelSize(9);
    m_autoCheck->setFont( font );
    row2Layout->addWidget( m_autoCheck );
    
    QLabel* HLabel = new QLabel( "H", this );
    HLabel->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    HLabel->setFixedSize( 14, 16 );
    font.setPixelSize(10);
    HLabel->setFont( font );
    row2Layout->addWidget( HLabel );
    
    QLabel* VLabel = new QLabel( "V", this );
    VLabel->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    VLabel->setFixedSize( 30, 16 );
    VLabel->setFont( font );
    row2Layout->addWidget( VLabel );
    
    m_verticalLayout->addLayout( row2Layout );
    
    /*QFrame* line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    m_verticalLayout->addWidget(line);*/
    
    QHBoxLayout* row0Layout = new QHBoxLayout();
    row0Layout->setObjectName( "row0Layout" );
    row0Layout->setContentsMargins(0, 0, 0, 0);
    row0Layout->setSpacing(0);
    
    QLabel* scaleLabel = new QLabel( "Scale", this );
    scaleLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    scaleLabel->setFixedSize( 32, 29 );
    scaleLabel->setFont( font );
    row0Layout->addWidget( scaleLabel );
    
    m_HscaleDial = new QDial(this);
    m_HscaleDial->setObjectName( "HscaleDial" );
    m_HscaleDial->setFixedSize( 32, 29 );
    m_HscaleDial->setNotchesVisible( true );
    m_HscaleDial->setWrapping(true);
    m_HscaleDial->setMinimum( 1 );
    m_HscaleDial->setMaximum( 20 );
    //m_HscaleDial->setValue( 10 );
    m_HscaleDial->setSingleStep( 1 );
    row0Layout->addWidget(m_HscaleDial );
    
    m_VscaleDial = new QDial(this);
    m_VscaleDial->setObjectName( "VscaleDial" );
    m_VscaleDial->setFixedSize( 32, 29 );
    m_VscaleDial->setNotchesVisible( true );
    m_VscaleDial->setWrapping( true );
    m_VscaleDial->setMinimum( 1 );
    m_VscaleDial->setMaximum( 20 );
    //m_VscaleDial->setValue( 25 );
    m_VscaleDial->setSingleStep( 1 );
    row0Layout->addWidget( m_VscaleDial );    
    
    m_verticalLayout->addLayout( row0Layout );
    
    QHBoxLayout* row1Layout = new QHBoxLayout();
    row1Layout->setObjectName( "row1Layout" );
    row1Layout->setContentsMargins(0, 0, 0, 0);
    row1Layout->setSpacing(0);
    
    QLabel* posLabel = new QLabel( "Pos", this );
    posLabel->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
    posLabel->setFixedSize( 32, 29 );
    posLabel->setFont(font);
    row1Layout->addWidget( posLabel );
    
    m_HposDial = new QDial(this);
    m_HposDial->setObjectName( "HposDial" );
    m_HposDial->setFixedSize( 32, 29 );
    m_HposDial->setNotchesVisible( true );
    m_HposDial->setWrapping( true );
    m_HposDial->setMinimum( 1 );
    m_HposDial->setMaximum( 20 );
    m_HposDial->setSingleStep( 1 );
    row1Layout->addWidget( m_HposDial );
    
    m_VposDial = new QDial(this);
    m_VposDial->setObjectName( "VposDial" );
    m_VposDial->setFixedSize( 32, 29 );
    m_VposDial->setNotchesVisible( true );
    m_VposDial->setWrapping( true );
    m_VposDial->setMinimum( 1 );
    m_VposDial->setMaximum( 20 );
    m_VposDial->setSingleStep( 1 );
    row1Layout->addWidget( m_VposDial );
    
    m_verticalLayout->addLayout( row1Layout );

    m_horizontalLayout->addLayout( m_verticalLayout );

    m_display = new RenderOscope( size, size, this );
    m_display->setObjectName( "oscope" );
    
    m_horizontalLayout->addWidget( m_display );
    m_horizontalLayout->setAlignment( m_display, Qt::AlignRight );
    
    
    connect(m_HscaleDial, SIGNAL( valueChanged(int) ),
             this,        SLOT  ( HscaleChanged(int)) );
             
    connect( m_VscaleDial, SIGNAL( valueChanged(int) ),
             this,         SLOT  ( VscaleChanged(int)) );
             
    connect( m_HposDial, SIGNAL( valueChanged(int) ),
             this,       SLOT  ( HposChanged(int)) );
             
    connect( m_VposDial, SIGNAL( valueChanged(int) ),
             this,       SLOT  ( VposChanged(int)) );
             
    connect( m_autoCheck, SIGNAL( stateChanged(int) ),
             this,        SLOT  ( autoChanged(int)) );
}

#include "moc_oscopewidget.cpp"

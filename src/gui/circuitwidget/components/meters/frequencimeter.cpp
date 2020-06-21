/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "frequencimeter.h"
#include "simulator.h"
#include "pin.h"

Component* Frequencimeter::construct( QObject* parent, QString type, QString id )
{ return new Frequencimeter( parent, type, id ); }

LibraryItem* Frequencimeter::libraryItem()
{
    return new LibraryItem(
            tr( "Frequencimeter" ),
            tr( "Meters" ),
            "frequencimeter.png",
            "Frequencimeter",
            Frequencimeter::construct);
}

Frequencimeter::Frequencimeter( QObject* parent, QString type, QString id )
              : Component( parent, type, id )
              , eElement( id.toStdString() )
              , m_display( this )
{
    m_area = QRectF( -32, -10, 75, 20 );
    m_color = Qt::black;

    m_ePin.resize( 1 );
    m_pin.resize( 1 );
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-40, 0);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_ePin[0] = m_pin[0];
    
    m_idLabel->setPos(-12,-24);
    setLabelPos(-32,-24, 0);

    QFont f( "Helvetica [Cronyx]", 10, QFont::Bold );
    f.setPixelSize(12);
    m_display.setFont(f);
    m_display.setBrush(  Qt::yellow );
    m_display.setPos( -30, -6 );
    m_display.setVisible( true );
    
    Simulator::self()->addToSimuClockList( this );
    Simulator::self()->addToUpdateList( this );
    
    resetState();
}
Frequencimeter::~Frequencimeter()
{
    Simulator::self()->remFromSimuClockList( this );
    Simulator::self()->remFromUpdateList( this );
}

void Frequencimeter::resetState()
{
    m_rising  = false;
    m_falling = false;
    
    m_filter = 0.01;
    m_lastData = 0;
    m_max = 0;
    m_min = 0;
    m_freq = 0;
    
    m_numMax = 0;
    
    m_step = 0;
    m_lastMax = 0;
    m_totalP = 0;
    
    m_display.setText( "0 Hz" );
}

void Frequencimeter::updateStep()
{
    double spus = Simulator::self()->stepsPerus();
    uint64_t stepsPerS = spus*1e6;

    if( m_step > stepsPerS ) resetState();
    if( m_numMax < 2 ) return;
    m_numMax--;
    
    double freq = stepsPerS/(double)(m_totalP/m_numMax);
    //qDebug() <<"Frequencimeter::simuClockStep"<<m_totalP<<m_numMax<<m_totalP/m_numMax;
    if( m_freq != freq )
    {
        m_freq = freq;

        int Fdecs = 1;
        QString unit = "  Hz";

        if( freq > 999 )
        {
            freq /= 1e3;
            unit = " KHz";

            if( freq > 999 )
            {
                freq /= 1e3;
                unit = " MHz";
            }
        }
        if     ( freq < 10 )  Fdecs = 3;
        else if( freq < 100 ) Fdecs = 2;
        m_display.setText(QString::number( freq, 'f', Fdecs )+unit );
    }
    m_totalP = 0;
    m_numMax = 0;
    m_step   = 0;
}

void Frequencimeter::simuClockStep()
{
    m_step++;
    
    double data = m_ePin[0]->getVolt();
    
    if( (data-m_lastData) > m_filter )                         // Rising 
    {
        if( m_falling && !m_rising )                        // Min Found
        {
            m_falling = false;
            m_max = -1e12;
        }
        m_rising = true;
        m_lastData = data;
    }
    else if( (data-m_lastData) < -m_filter )                  // Falling
    {
        if( m_rising && !m_falling )                        // Max Found
        {
            if( m_numMax > 0 ) m_totalP += m_step-m_lastMax;

            m_lastMax = m_step;
            m_numMax++;
            m_rising = false;
            m_min = 1e12;
        }
        m_falling = true;
        m_lastData = data;
    }
}

void Frequencimeter::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    p->setBrush( Qt::black);

    p->drawRect( m_area );
}

//#include "moc_frequencimeter.cpp"

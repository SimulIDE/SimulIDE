/***************************************************************************
 *   Copyright (C) 2018 by santiago González                               *
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

#include "audio_out.h"
#include "simulator.h"
#include "connector.h"
#include "itemlibrary.h"
#include "pin.h"

static const char* AudioOut_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Impedance"),
    QT_TRANSLATE_NOOP("App::Property","Buzzer")

};

Component* AudioOut::construct( QObject* parent, QString type, QString id )
{ return new AudioOut( parent, type, id ); }

LibraryItem* AudioOut::libraryItem()
{
    return new LibraryItem(
            tr( "Audio Out" ),
            tr( "Outputs" ),
            "audio_out.png",
            "AudioOut",
            AudioOut::construct);
}

AudioOut::AudioOut( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eResistor( id.toStdString() )
{
    Q_UNUSED( AudioOut_properties );
    
    m_area = QRect( -12, -24, 24, 40 );
    
    m_pin.resize( 2 );
    
    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-8-8,-8);
    m_pin[0] = new Pin( 180, pinPos, pinId, 0, this);
    m_pin[0]->setLabelText( "+" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-rPin"));
    pinPos = QPoint(-8-8,0);
    m_pin[1] = new Pin( 180, pinPos, pinId, 1, this);
    m_pin[1]->setLabelText( " -" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[1] = m_pin[1];

    //m_idLabel->setText( QString("") );
    m_idLabel->setPos(-12,-24);
    setLabelPos(-20,-36, 0);
    
    m_resist = 8;
    m_buzzer = false;
    
    int refreshPeriod = 10; // mS
    int sampleRate    = 40000; // samples/S
    
    m_deviceinfo = QAudioDeviceInfo::defaultOutputDevice(); 
    if( m_deviceinfo.isNull() ) 
    {
        qDebug() <<"No defaulf Audio Output Device Found" ;
        return;
    }
    m_format.setSampleRate( sampleRate );  
    m_format.setChannelCount(1);
    m_format.setSampleSize(8);
    m_format.setCodec( "audio/pcm" );  
    m_format.setByteOrder( QAudioFormat::LittleEndian );  
    m_format.setSampleType( QAudioFormat::UnSignedInt );  
    
    if( !m_deviceinfo.isFormatSupported( m_format )) 
    {  
        qDebug() << "Default format not supported - trying to use nearest";  
        m_format = m_deviceinfo.nearestFormat( m_format );  
        
        qDebug() << m_format.sampleRate() << m_format.channelCount()<<m_format.sampleSize();
    }  
    m_audioOutput = new QAudioOutput( m_deviceinfo, m_format );   
    
    m_dataSize = 2*refreshPeriod*sampleRate/1000;
    
    m_dataBuffer = new char[ m_dataSize ];
    m_audioOutput->setBufferSize( 2*m_dataSize );

    //qDebug() << "AudioOut::AudioOut" << m_audioOutput->notifyInterval();
    
    //m_audioOutput->setNotifyInterval( refreshPeriod );
    
    //connect( m_audioOutput, SIGNAL( notify() ),
    //         this,          SLOT(   OnAudioNotify() ));

    resetState();
}

AudioOut::~AudioOut()
{
    delete m_dataBuffer;
    //qDebug() << "AudioOut::~AudioOut deleting" << QString::fromStdString( m_elmId );
}

void AudioOut::stamp()
{
    if( m_deviceinfo.isNull() ) return;
    
    if( m_ePin[0]->isConnected() && m_ePin[1]->isConnected() )
        Simulator::self()->addToSimuClockList( this );
    
    eResistor::stamp();
}

void AudioOut::resetState()
{
    if( m_deviceinfo.isNull() ) return;
    m_counter = 0;
    m_dataCount = 0;
    
    m_auIObuffer = m_audioOutput->start();
}

void AudioOut::simuClockStep()
{
    m_counter++;
    double stepsPerUs = Simulator::self()->stepsPerus();
    if( m_counter == 25*stepsPerUs )
    {
        m_counter = 0;

        double voltPN = m_ePin[0]->getVolt()-m_ePin[1]->getVolt();
        if( voltPN > 5 ) voltPN = 5;

        char outVal = 0;

        if( m_buzzer)
        {
            if( voltPN> 2.5 )
            {
                int stepsPC = stepsPerUs*1e6/600;
                double time = Simulator::self()->step();
                time = remainder( time, stepsPC );
                time = qDegreesToRadians( time*360/stepsPC );

                outVal = sin( time )*51/2;
            }
        }
        else outVal = voltPN*51;

        m_dataBuffer[ m_dataCount ] = outVal;
        m_dataCount++;

        if( m_dataCount == m_dataSize )
        {
            //qDebug() << m_dataCount;
            m_dataCount = 0;
            m_auIObuffer->write( (const char*)m_dataBuffer, m_dataSize );
        }
    }
}

void AudioOut::OnAudioNotify()
{
    //qDebug() << "AudioOut::OnAudioNotify()"<<m_dataCount;
    m_auIObuffer->write( (const char*)m_dataBuffer, m_dataCount );
    m_dataCount = 0;
}

void AudioOut::remove()
{
    Simulator::self()->remFromSimuClockList( this );
    
    if( m_ePin[0]->isConnected() ) (static_cast<Pin*>(m_ePin[0]))->connector()->remove();
    if( m_ePin[1]->isConnected() ) (static_cast<Pin*>(m_ePin[1]))->connector()->remove();
    Component::remove();
}

QPainterPath AudioOut::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-10,-12 )
           << QPointF(-10, 4 )
           << QPointF( 0, 4 )
           << QPointF( 10, 16 )
           << QPointF( 10, -24 )
           << QPointF( 0, -12 )
           << QPointF(-10, -12 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void AudioOut::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    //p->drawRect( -10.5, -12, 12, 24 );
    static const QPointF points[7] = {
        QPointF(-10,-12 ),
        QPointF(-10, 4 ),
        QPointF( 0, 4 ),
        QPointF( 10, 16 ),
        QPointF( 10, -24 ),
        QPointF( 0, -12 ),
        QPointF(-10, -12 )
        };

    p->drawPolygon(points, 7);
    
    if( m_deviceinfo.isNull() )
    {
        p->drawLine(0,-8, 7, 0 );
        p->drawLine( 7,-8,0, 0 );
    }
}

#include "moc_audio_out.cpp"

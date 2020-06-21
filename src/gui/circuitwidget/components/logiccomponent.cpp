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

#include "connector.h"
#include "circuit.h"
#include "logiccomponent.h"

static const char* LogicComponent_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Input High V"),
    QT_TRANSLATE_NOOP("App::Property","Input Low V"),
    QT_TRANSLATE_NOOP("App::Property","Input Imped"),
    QT_TRANSLATE_NOOP("App::Property","Out High V"),
    QT_TRANSLATE_NOOP("App::Property","Out Low V"),
    QT_TRANSLATE_NOOP("App::Property","Out Imped"),
    QT_TRANSLATE_NOOP("App::Property","Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Tristate"),
    QT_TRANSLATE_NOOP("App::Property","Clock Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Reset Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Invert Inputs"),
    QT_TRANSLATE_NOOP("App::Property","S R Inverted"),
    QT_TRANSLATE_NOOP("App::Property","Num Inputs"),
    QT_TRANSLATE_NOOP("App::Property","Num Outputs"),
    QT_TRANSLATE_NOOP("App::Property","Num Bits"),
    QT_TRANSLATE_NOOP("App::Property","Channels"),
    QT_TRANSLATE_NOOP("App::Property","Open Collector"),
    QT_TRANSLATE_NOOP("App::Property","Trigger"),
    QT_TRANSLATE_NOOP("App::Property"," 16 Bits")
};

LogicComponent::LogicComponent( QObject* parent, QString type, QString id )
              : Component( parent, type, id )
{
    Q_UNUSED( LogicComponent_properties );
    
    m_numInPins  = 0;
    m_numOutPins = 0;
}
LogicComponent::~LogicComponent()
{
}

void LogicComponent::init( QStringList pins )
{
    m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, m_height*8 );
    
    QStringList inputs;                                    // Input Pins
    QStringList outputs;                                  // Output Pins

    // Example: pin = "IL02Name" => input, left, number 2, label = "Name"
    
    for( QString pin : pins )    
    {
             if( pin.startsWith( "I" ) ) inputs.append(  pin.remove(0,1) );
        else if( pin.startsWith( "O" ) ) outputs.append( pin.remove(0,1) );
        else qDebug() << " LogicComponent::init: pin name error ";
    }
    //qDebug() << inputs << outputs;

    // configure Input Pins............................................
    setNumInps( inputs.length() );
    int i = 0;
    for( QString input : inputs )
    {
        // Example input = "L02Name"
        QString pin = input.left(3);        // Pin position
        QString label = input.remove(0,3);  // Pin name

        m_inPin[i] = createPin( pin, m_id+"-in"+QString::number(i) );
        m_inPin[i]->setLabelText( label );
        i++;
    }

    // configure Output Pins............................................
    setNumOuts( outputs.length() );
    i = 0;
    for( QString output : outputs )
    {
        // Example output = "L02Name"
        QString pin = output.left(3);        // Pin position
        QString label = output.remove(0,3);  // Pin name

        m_outPin[i] = createPin( pin, m_id+"-out"+QString::number(i) );
        m_outPin[i]->setLabelText( label );
        i++;
    }
    //label->setPos( m_area.x(), m_area.y()-20 );
}

Pin* LogicComponent::createPin( QString data, QString pinId )
{
    // Example pin = "L02" => left side, number 2

    QString pos = data.left(1);
    int     num = data.remove(0,1).toInt();

    int angle = 0;
    int x = 0;
    int y = 0;

    if( pos == "U" )        // Up
    {
        angle = 90;
        x = m_area.x() + num*8;
        y = m_area.y() - 8;
    }
    else if( pos == "L")    // Left
    {
        angle = 180;
        x = m_area.x() - 8;
        y = m_area.y() + num*8;
    }
    if( pos == "D" )        // Down
    {
        angle = 270;
        x = m_area.x() + num*8;
        y = m_area.height()/2 + 8;
    }
    else if( pos == "R")    // Right
    {
        x = m_area.width()/2 + 8;
        y = m_area.y() + num*8;
    }
    Pin* pin = new Pin( angle, QPoint( x, y ), pinId, 0, this );
    pin->setLabelColor( QColor( 0, 0, 0 ) );
    return pin;
}

void LogicComponent::remove()
{
    for( int i=0; i<m_numInPins; i++ )
        if( m_inPin[i]->isConnected() ) m_inPin[i]->connector()->remove();

    for( int i=0; i<m_numOutPins; i++ )
        if( m_outPin[i]->isConnected() ) m_outPin[i]->connector()->remove();
    
    Component::remove();
}

void LogicComponent::setNumInps( int inPins )
{
    //qDebug()<< m_id << "LogicComponent::setNumInps" << inPins;
    if( inPins == m_numInPins ) return;
    if( inPins < 1 ) return;
    
    for( int i=0; i<m_numInPins; i++ )
    {
        if( m_inPin[i]->isConnected() ) m_inPin[i]->connector()->remove();
        if( m_inPin[i]->scene() ) Circuit::self()->removeItem( m_inPin[i] );
        m_inPin[i]->reset();
        delete m_inPin[i];
    }
    m_inPin.resize( inPins );

    m_numInPins = inPins;
}

void LogicComponent::setNumOuts( int outPins )
{
    //qDebug()<< m_id << "LogicComponent::setNumOuts" << outPins;
    if( outPins == m_numOutPins ) return;
    if( outPins < 1 ) return;
    
    for( int i=0; i<m_numOutPins; i++ )
    {
        if( m_outPin[i]->isConnected() ) m_outPin[i]->connector()->remove();
        if( m_outPin[i]->scene() ) Circuit::self()->removeItem( m_outPin[i] );
        m_outPin[i]->reset();
        delete m_outPin[i];
    }
    m_outPin.resize( outPins );
    
    m_numOutPins = outPins;
}

void LogicComponent::deleteInputs( int inputs )
{
    if( m_numInPins-inputs < 0 ) inputs = m_numInPins;

    for( int i=m_numInPins-1; i>m_numInPins-inputs-1; i-- )
    {
        if( m_inPin[i]->isConnected() ) m_inPin[i]->connector()->remove();
        if( m_inPin[i]->scene() ) Circuit::self()->removeItem( m_inPin[i] );
        m_inPin[i]->reset();
        delete m_inPin[i];
    }
    m_numInPins -= inputs;
    m_inPin.resize( m_numInPins );
}

void LogicComponent::deleteOutputs( int outputs )
{
    for( int i=m_numOutPins-1; i>m_numOutPins-outputs-1; i-- )
    {
        if( m_outPin[i]->isConnected() ) m_outPin[i]->connector()->remove();
        //else                             m_outPin[i]->reset();

        if( m_outPin[i]->scene() ) Circuit::self()->removeItem( m_outPin[i] );

        delete m_outPin[i];
    }
    m_numOutPins -= outputs;
    m_outPin.resize( m_numOutPins );
}

void LogicComponent::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRect( m_area );
}

#include "moc_logiccomponent.cpp"

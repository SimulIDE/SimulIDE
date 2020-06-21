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

#include "meter.h"
#include "simulator.h"
#include "e-source.h"
#include "pin.h"
#include "utils.h"


#include <math.h>   // fabs(x,y)

Meter::Meter( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eResistor( id.toStdString() )
     , m_display( this )
{
    m_area = QRectF( -24, -24, 48, 32 );

    m_pin.resize( 3 );

    QString pinId = m_id;
    pinId.append(QString("-lPin"));
    QPoint pinPos = QPoint(-8, 16);
    m_pin[0] = new Pin( 270, pinPos, pinId, 0, this);
    //m_pin[0]->setLabelText( "+" );
    m_pin[0]->setColor( Qt::red );
    m_ePin[0] = m_pin[0];

    pinId = m_id;
    pinId.append(QString("-rPin"));
    pinPos = QPoint(8, 16);
    m_pin[1] = new Pin( 270, pinPos, pinId, 1, this);
    //m_pin[1]->setLabelText( "|" );
    m_ePin[1] = m_pin[1];

    pinId = id;
    pinId.append(QString("-outnod"));
    pinPos = QPoint(32,-8);
    m_pin[2] = new Pin( 0, pinPos, pinId, 0, this);
    m_outpin = m_pin[2];

    pinId.append(QString("-eSource"));
    m_out = new eSource( pinId.toStdString(), m_outpin );
    m_out->setOut( true );
    m_out->setVoltHigh( 0 );

    m_idLabel->setPos(-12,-24);
    setLabelPos(-24,-40, 0);

    QFont f( "Helvetica [Cronyx]", 10, QFont::Bold );
    f.setPixelSize(12);
    m_display.setFont(f);
    m_display.setText( "Freq: 0 Hz" );
    m_display.setBrush(  Qt::yellow );
    m_display.setPos( -22, -22 );
    m_display.setVisible( true );

    setShowVal( false );

    Simulator::self()->addToUpdateList( this );
}
Meter::~Meter(){}

void Meter::updateStep()
{
    int dispVal = 0;

    QString sign = " ";

    double dispValue = fabs(m_dispValue);

    if( dispValue > 1e-6 )
    {
        if( m_dispValue < 0 ) sign = "-";

        setValue( dispValue );
        dispVal = int( m_value*10+0.5 );

        if( dispVal > 999 )
        {
            setValue( dispVal/10 );
            dispVal = int( m_value*10 );
        }
        //qDebug() <<"Meter::updateStep"<<m_dispValue<< m_value<<dispVal<<m_unitMult;
    }
    m_display.setText( sign+decToBase( dispVal/10, 10, 3 )
                       +"."+decToBase( dispVal%10, 10, 1 )
                       +"\n"+m_mult+m_unit );

    m_out->setVoltHigh( m_dispValue );
    m_out->stampOutput();
}

void Meter::remove()
{
    Simulator::self()->remFromUpdateList( this );

    delete m_out;

    Component::remove();
}

void Meter::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );
    p->setBrush( Qt::black);

    p->drawRect( m_area );

    QPointF points[3] = {
    QPointF( 27,-12 ),
    QPointF( 32, -8 ),
    QPointF( 27, -4 )     };
    p->drawPolygon(points, 3);
}

#include "moc_meter.cpp"

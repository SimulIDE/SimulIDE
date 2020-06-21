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

#include "relay_base.h"
#include "simulator.h"
#include "circuit.h"

#include <math.h>

static const char* RelayBase_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Rcoil"),
    QT_TRANSLATE_NOOP("App::Property","IOn"),
    QT_TRANSLATE_NOOP("App::Property","IOff"),
    QT_TRANSLATE_NOOP("App::Property","Inductance")
};

RelayBase::RelayBase( QObject* parent, QString type, QString id )
         : MechContact( parent, type, id )
{
    Q_UNUSED( RelayBase_properties );
    
    // This represents a coil, so is an Inductor in series with a Resistor: -Ind-Nod-Res-
    // We need to create the resistor, internal eNode and do the connections.

    m_ePin.resize(4);
    m_pin.resize(2);

    m_pin0 = 4;

    QString nodid = m_id;            // External Left pin to inductor
    nodid.append(QString("-lPin"));
    QPoint nodpos = QPoint(-16-8,0);
    m_pin[0] = new Pin( 180, nodpos, nodid, 0, this);
    m_pin[0]->setLength(4.5);
    m_pin[0]->setPos(-16, 0 );
    m_ePin[0] = m_pin[0];

    nodid = m_id;                    // External Right pin to resistor
    nodid.append(QString("-rPin"));
    nodpos = QPoint(16+8,0);
    m_pin[1] = new Pin( 0, nodpos, nodid, 1, this);
    m_pin[1]->setLength(4.5);
    m_pin[1]->setPos( 16, 0 );
    m_ePin[3] = m_pin[1];

    QString reid = m_id;
    nodid = reid;                  // Internal Left pin to inductor
    nodid.append(QString("-lIntPin"));
    m_ePin[1] = new ePin( reid.toStdString(), 1 );

    nodid = reid;                 // Internal right pin to resistor
    nodid.append(QString("-rIntPin"));
    m_ePin[2] = new ePin( reid.toStdString(), 2 );

    reid = m_id;
    reid.append(QString("-resistor"));
    m_resistor = new eResistor( reid.toStdString() );
    m_resistor->setRes( 100 );
    m_resistor->setEpin( 0, m_ePin[2] );
    m_resistor->setEpin( 1, m_ePin[3] );

    reid = m_id;
    reid.append(QString("-inductor"));
    m_inductor = new eInductor( reid.toStdString() );
    m_inductor->setInd( 0.1 );  // 100 mH
    m_inductor->setEpin( 0, m_ePin[0] );
    m_inductor->setEpin( 1, m_ePin[1] );

    m_trigCurrent = 0.02;
    m_relCurrent  = 0.01;

    m_internalEnode = new eNode( m_id+"-internaleNode" );
    m_ePin[1]->setEnode( m_internalEnode );
    m_ePin[2]->setEnode( m_internalEnode );

    setValLabelPos(-16, 6, 0);
    setLabelPos(-16, 8, 0);
    setShowVal( true );

    SetupSwitches( 1, 1 );
}
RelayBase::~RelayBase()
{
}

void RelayBase::stamp()
{
    if( m_ePin[0]->isConnected() ) m_ePin[0]->getEnode()->addToChangedFast(this);
    if( m_ePin[1]->isConnected() ) m_ePin[1]->getEnode()->addToChangedFast(this);
    MechContact::stamp();
}

void RelayBase::setVChanged()
{
    double indCurr = fabs( m_inductor->indCurrent() );
    bool closed;

    if( m_closed ) closed = ( indCurr > m_relCurrent );
    else           closed = ( indCurr > m_trigCurrent );

    if( m_nClose ) closed = !closed;
    if( closed != m_closed ) setSwitch( closed );
}

void RelayBase::remove()
{
    Simulator::self()->remFromEnodeList( m_internalEnode, true );

    delete m_resistor;
    delete m_inductor;

    MechContact::remove();
}

double RelayBase::rCoil() const
{ return m_resistor->res(); }

void RelayBase::setRCoil( double res )
{
    if( res > 0.0 ) m_resistor->setResSafe(res);
}

double RelayBase::iTrig() const { return m_trigCurrent; }

void RelayBase::setITrig( double current )
{
    if( current > 0.0 ) m_trigCurrent = current;
}

double RelayBase::iRel() const
{
    return m_relCurrent;
}

void RelayBase::setIRel( double current )
{
    m_relCurrent = current;
}

double RelayBase::induc() { return m_inductor->ind(); }

void RelayBase::setInduc( double i )
{
    m_inductor->setInd( i );
}

void RelayBase::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    p->setBrush(Qt::white);
    p->drawRect( m_area );

    QPen pen = p->pen();                                       // Draw Coil
    pen.setWidth(2.8);
    p->setPen(pen);

    QRectF rectangle(-12,-4.5, 10, 10 );
    int startAngle = -45 * 16;
    int spanAngle = 220 * 16;
    p->drawArc(rectangle, startAngle, spanAngle);

    QRectF rectangle2(-5,-4.5, 10, 10 );
    startAngle = 225 * 16;
    spanAngle = -270 * 16;
    p->drawArc(rectangle2, startAngle, spanAngle);

    QRectF rectangle3(2,-4.5, 10, 10 );
    startAngle = 225 * 16;
    spanAngle = -220 * 16;
    p->drawArc(rectangle3, startAngle, spanAngle);

    MechContact::paint( p, option, widget );
}

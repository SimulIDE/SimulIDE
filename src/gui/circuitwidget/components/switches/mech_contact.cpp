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

#include "mech_contact.h"
#include "simulator.h"
#include "circuit.h"

#include <math.h>

static const char* MechContact_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Poles"),
    QT_TRANSLATE_NOOP("App::Property","DT"),
    QT_TRANSLATE_NOOP("App::Property","Norm Close")
};

MechContact::MechContact( QObject* parent, QString type, QString id )
           : Component( parent, type, id )
           , eElement(  id.toStdString() )
{
    Q_UNUSED( MechContact_properties );

    m_numthrows = 0;
    m_numPoles  = 0;
    m_pin0      = 0;

    m_nClose = false;
    m_closed = false;
    m_hidden = false;

    //resetState();
}
MechContact::~MechContact()
{
}

void MechContact::attach()
{
    if( m_hidden ) return;
    for( int i=0; i<m_numPoles; i++ )
    {
        eNode* enode = m_pin[m_pin0/2+i*(1+m_numthrows)]->getEnode();

        int epinN = m_pin0+i*m_numthrows*2;
        m_ePin[ epinN ]->setEnode( enode );

        if( m_numthrows > 1 ) m_ePin[ epinN+2 ]->setEnode( enode );
    }
}

void MechContact::stamp()
{
    for( uint i=m_pin0; i<m_ePin.size(); i++ )
    {
        eNode* enode = m_ePin[i]->getEnode();

        if( enode ) enode->setSwitched( true );
    }
    setSwitch( m_nClose );
}


void MechContact::setSwitch( bool closed )
{
    //qDebug() << "MechContact::setSwitch" << closed;
    m_closed = closed;

    for( int i=0; i<m_numPoles; i++ )
    {
        int switchN = i*m_numthrows;

        if( closed ) m_switches[ switchN ]->setAdmit( 1e3 );
        else         m_switches[ switchN ]->setAdmit( 0 );

        if( m_numthrows == 2 )
        {
            switchN++;

            if( !closed ) m_switches[ switchN ]->setAdmit( 1e3 );
            else          m_switches[ switchN ]->setAdmit( 0 );
        }
    }
    update();
}

void MechContact::remove()
{
    for( eResistor* res : m_switches ) delete res;
    Component::remove();
}

void MechContact::SetupButton()
{
    initEpins();
    m_switches.resize( 1 );
    m_switches[ 0 ] = new eResistor( "res0" );
    m_switches[ 0 ]->setEpin( 0, m_ePin[0] );
    m_switches[ 0 ]->setEpin( 1, m_ePin[1] );
    m_numPoles = 1;
    m_numthrows = 1;
    m_hidden = true;
}

void MechContact::SetupSwitches( int poles, int throws )
{
    if( m_pin0  == 0 ) m_area = QRectF( -13,-16*poles, 26, 16*poles );
    else               m_area = QRectF( -13, -8-16*poles-4, 26, 8+16*poles+8+4 );
    int start = m_pin0/2;

    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();

    for( uint i=0; i<m_switches.size(); i++ )
        delete m_switches[i];
        
    //qDebug() << "MechContact::SetupSwitches Pins:"<<poles<<throws<<m_numPoles<<m_numthrows;
    for( int i=0; i<m_numPoles; i++ )
    {
        int epinN = m_pin0+i*m_numthrows*2;
        delete m_ePin[ epinN ];

        if( m_numthrows > 1 ) delete m_ePin[ epinN+2 ];
    }

    for( uint i=start; i<m_pin.size(); i++ )
    {
        Pin* pin = m_pin[i];
        if( pin->isConnected() ) pin->connector()->remove();
        pin->reset();
        delete pin;
    }

    m_numPoles = poles;
    m_numthrows = throws;

    m_switches.resize( poles*throws );
    m_pin.resize( start+poles+poles*throws);

    m_ePin.resize(m_pin0+2*poles*throws);
//qDebug() << "MechContact::SetupSwitches" << poles+poles*throws;

    int cont = 0;
    for( int i=0; i<poles; i++ )              // Create Resistors
    {
        Pin* pin;
        int pinN = start+cont;
        int ePinN = m_pin0+cont;
        QString reid = m_id;

        QPoint pinpos = QPoint(-16,-4*m_pin0-16*i );
        pin = new Pin( 180, pinpos, reid+"-pinP"+QString::number(pinN), 0, this);
        pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top
        m_pin[pinN] = pin;

        for( int j=0; j<throws; j++ )
        {
            reid = m_id;

            cont++;
            int tN = i*throws+j;

            reid.append( QString( "-switch"+QString::number(tN)) );
            m_switches[ tN ] = new eResistor( reid.toStdString() );

            ePinN = m_pin0+tN*2;
            QString pinp = reid+"pinP";
            m_ePin[ ePinN ] = new ePin( pinp.toStdString(), 1 );

            pinpos = QPoint( 16,-4*m_pin0-16*i-8*j);
            pin = new Pin( 0, pinpos, reid+"pinN", 1, this);
            pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top

            m_pin[ start+cont ] = pin;
            m_ePin[ ePinN+1 ] = pin;

            m_switches[ tN ]->setEpin( 0, m_ePin[ePinN] );
            m_switches[ tN ]->setEpin( 1, pin );
        }
        cont++;
    }
    if( pauseSim ) Simulator::self()->runContinuous();
    Circuit::self()->update();
    //for( Pin* pin : m_pin )
    //    pin->setFlag( QGraphicsItem::ItemStacksBehindParent, false ); // draw Pins on top
}

int MechContact::poles() const
{ return m_numPoles; }

void MechContact::setPoles( int poles )
{
    if( poles < 1 ) poles = 1;

    if( poles != m_numPoles )
        SetupSwitches( poles, m_numthrows );
}

bool MechContact::dt() const
{ return (m_numthrows>1); }

void MechContact::setDt( bool dt )
{
    int throws = 1;
    if( dt ) throws = 2;

    if( throws != m_numthrows )
        SetupSwitches( m_numPoles, throws );
}

bool MechContact::nClose() const
{
    return m_nClose;
}

void MechContact::setNClose( bool nc )
{
    m_nClose = nc;
    setSwitch( m_nClose );
}


void MechContact::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    if( m_hidden ) return;

    QPen pen = p->pen();
    pen.setWidth(3);
    p->setPen(pen);

    for( int i=0; i<m_numPoles; i++ )                           // Draw Switches
    {
        int offset = 16*i-16+4*m_pin0;

        if( m_closed )                                          // switch is closed
            p->drawLine(-10, -16-offset, 10, -18-offset );
        else                                                    // Switch is oppened
            p->drawLine(-10.5, -16-offset, 8, -24-offset );
    }
    if( m_numPoles > 1 )
    {
        pen.setStyle(Qt::DashLine);
        pen.setWidth(1);
        p->setPen(pen);
        p->drawLine(-0, 4-4*m_pin0, 0, -3*m_pin0-16*m_numPoles+4 );
    }
}

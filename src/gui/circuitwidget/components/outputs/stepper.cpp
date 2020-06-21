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

#include <cmath>

#include "stepper.h"
#include "simulator.h"

static const char* Stepper_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Steps"),
    QT_TRANSLATE_NOOP("App::Property","Bipolar")
};

Component*  Stepper::construct( QObject* parent, QString type, QString id )
{
    return new Stepper( parent, type, id );
}

LibraryItem* Stepper::libraryItem()
{
    return new LibraryItem(
        tr("Stepper"),
        tr("Outputs"),
        "steeper.png",
        "Stepper",
        Stepper::construct );
}

Stepper::Stepper( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eElement( (id+"-eElement").toStdString() )
        , m_resA1( (id+"-eEresistorA1").toStdString() )
        , m_resA2( (id+"-eEresistorA2").toStdString() )
        , m_resB1( (id+"-eEresistorB1").toStdString() )
        , m_resB2( (id+"-eEresistorB2").toStdString() )
        , m_pinA1( 180, QPoint(-72,-32), id+"-PinA1", 0, this )
        , m_pinA2( 180, QPoint(-72, 16), id+"-PinA2", 0, this )
        , m_pinCo( 180, QPoint(-72, 0 ), id+"-PinCo", 0, this )
        , m_pinB1( 180, QPoint(-72,-16), id+"-PinB1", 0, this )
        , m_pinB2( 180, QPoint(-72, 32), id+"-PinB2", 0, this )
        , m_ePinA1Co( (id+"-ePinA1Co").toStdString(), 0 )
        , m_ePinA2Co( (id+"-ePinA2Co").toStdString(), 0 )
        , m_ePinB1Co( (id+"-ePinB1Co").toStdString(), 0 )
        , m_ePinB2Co( (id+"-ePinB2Co").toStdString(), 0 )
{
    Q_UNUSED( Stepper_properties );
    
    m_area = QRectF( -64, -50, 114, 100 );
    m_color = QColor( 50, 50, 70 );
    m_unit = "Ω";

    m_bipolar = false;
    
    m_ang  = 0;
    m_Ppos = 4;
    m_steps = 32;
    //m_res = 1e3;
    m_stpang = 360*8/m_steps;
        
    m_pinA1.setLabelText( " A+" );
    m_pinA2.setLabelText( " A-" );
    m_pinCo.setLabelText( " Co" );
    m_pinB1.setLabelText( " B+" );
    m_pinB2.setLabelText( " B-" );
    
    m_resA1.setEpin( 0, &m_pinA1 );
    m_resA1.setEpin( 1, &m_ePinA1Co );
    m_resA2.setEpin( 0, &m_pinA2 );
    m_resA2.setEpin( 1, &m_ePinA2Co );
    m_resB1.setEpin( 0, &m_pinB1 );
    m_resB1.setEpin( 1, &m_ePinB1Co );
    m_resB2.setEpin( 0, &m_pinB2 );
    m_resB2.setEpin( 1, &m_ePinB2Co );
    
    setRes( 100 );
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos(-32,-62, 0);
    setShowId( true );
}

Stepper::~Stepper()
{
}

void Stepper::setVChanged()
{
    double voltCom = m_pinCo.getVolt();
    double phaseA = ( m_pinA1.getVolt()-voltCom )-( m_pinA2.getVolt()-voltCom );
    double phaseB = ( m_pinB1.getVolt()-voltCom )-( m_pinB2.getVolt()-voltCom );
    
    if     ( phaseA > 1 ) phaseA = 1;
    else if( phaseA <-1 ) phaseA =-1;
    else                  phaseA = 0;
    
    if     ( phaseB > 1 ) phaseB = 1;
    else if( phaseB <-1 ) phaseB =-1;
    else                  phaseB = 0;

    int delta = 0;
    if( (abs(phaseA)+abs(phaseB)) > 0 ) // nosense algoritm.. just works
    {
        int ca = 4; 
        int cb =-1;
        if( phaseA ==-1 ) ca = 0;
        if( phaseA == 0 ) cb = 2;
        if( phaseA == 1 ) cb = 1;
        
        int newPos = ca+cb*phaseB;
        
        delta = newPos-m_Ppos;
        if( delta > 4 ) delta = delta-8;
        if( delta <-4 ) delta = delta+8;
        
        m_Ppos += delta;
    }
    else                        // avoid keep in half step when no input
    {
        delta = m_Ppos;
        m_Ppos = (m_Ppos/2)*2;
        delta = m_Ppos-delta;
    }
    m_ang  += delta*m_stpang;

    if (m_ang < 0)      m_ang += 360*16;
    if (m_ang > 360*16) m_ang -= 360*16;
    if (m_Ppos < 0)     m_Ppos += 8;
    if (m_Ppos > 7)     m_Ppos -= 8;
}

int Stepper::steps()
{
    return m_steps;
}

void Stepper::setSteps( int steps ) //" 4, 8,16,32"
{
    m_steps = steps/4;
    m_steps *= 4;
    if( m_steps < 4 ) m_steps = 4;
    m_stpang = 360*8/m_steps;
    m_ang  = 0;
    m_Ppos = 4;
    update();
}

double Stepper::res()
{
    return m_value;
}

void Stepper::setRes( double res )
{
    m_res = res;
    Component::setValue( res );       // Takes care about units multiplier
    m_resA1.setResSafe( m_value*m_unitMult );
    m_resA2.setResSafe( m_value*m_unitMult );
    m_resB1.setResSafe( m_value*m_unitMult );
    m_resB2.setResSafe( m_value*m_unitMult );
}

void Stepper::setUnit( QString un )
{
    Component::setUnit( un );
    m_resA1.setResSafe( m_value*m_unitMult );
    m_resA2.setResSafe( m_value*m_unitMult );
    m_resB1.setResSafe( m_value*m_unitMult );
    m_resB2.setResSafe( m_value*m_unitMult );
}

void Stepper::setBipolar( bool bi )
{
    m_bipolar = bi;
    if( m_pinCo.isConnected() ) m_pinCo.connector()->remove();
    m_pinCo.setVisible( !bi );
}

void Stepper::attach()
{
    eNode* enode = m_pinCo.getEnode();// Register for changes callback
    if( enode )
    {
        m_ePinA1Co.setEnode( enode );
        m_ePinA2Co.setEnode( enode );
        m_ePinB1Co.setEnode( enode );
        m_ePinB2Co.setEnode( enode );
    }
}

void Stepper::stamp()
{
    eNode* enode = m_pinA1.getEnode();// Register for changes callback
    if( enode ) enode->addToChangedFast(this);

    enode = m_pinA2.getEnode();// Register for changes callback
    if( enode ) enode->addToChangedFast(this);

    enode = m_pinB1.getEnode();// Register for changes callback
    if( enode ) enode->addToChangedFast(this);

    enode = m_pinB2.getEnode();// Register for changes callback
    if( enode ) enode->addToChangedFast(this);
    
    enode = m_pinCo.getEnode();// Register for changes callback
    if( enode ) enode->addToChangedFast(this);
}

void Stepper::updateStep()
{
    update();
}

void Stepper::remove()
{
    if( m_pinA1.isConnected() ) m_pinA1.connector()->remove();
    if( m_pinA2.isConnected() ) m_pinA2.connector()->remove();
    if( m_pinCo.isConnected() ) m_pinCo.connector()->remove();
    if( m_pinB1.isConnected() ) m_pinB1.connector()->remove();
    if( m_pinB2.isConnected() ) m_pinB2.connector()->remove();
    
    Simulator::self()->remFromUpdateList( this ); 
    
    Component::remove();
}

void Stepper::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    //p->setBrush( QColor(250, 210, 230) );
    p->drawRoundRect(-64,-40, 25, 80 );

    p->setBrush( QColor(50, 70, 100) ); 
    p->drawRoundRect(-48,-48, 96, 96 );

    p->setPen( QColor(0, 0, 0) );
    p->setBrush( QColor(255, 255, 255) );
    p->drawEllipse(-37,-37, 74, 74 );

    p->setPen ( QColor(255, 255, 255) );
    for ( int i = 0; i< 360*16; i += m_stpang*2 )
        p->drawPie(-42,-42, 84, 84, i+m_stpang*5/6, m_stpang/3 );

    p->setPen ( QColor(0, 0, 0) );
    p->setBrush( QColor(50, 70, 100) );
    p->drawPie(-33,-33, 66, 66, m_ang-m_stpang*3/4, m_stpang*3/2 );

    //p->setBrush( QColor(50, 70, 100) );
    p->drawEllipse(-25,-25, 50, 50);
}
#include "moc_stepper.cpp"

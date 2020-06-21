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

#include <cmath>

#include "inductor.h"
#include "connector.h"
#include "itemlibrary.h"

static const char* Inductor_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Inductance"),
    QT_TRANSLATE_NOOP("App::Property","Show Ind")
};


Component* Inductor::construct( QObject* parent, QString type, QString id )
{ return new Inductor( parent, type, id ); }

LibraryItem* Inductor::libraryItem()
{
    return new LibraryItem(
            tr( "Inductor" ),
            tr( "Passive" ),
            "inductor.png",
            "Inductor",
            Inductor::construct);
}

Inductor::Inductor( QObject* parent, QString type, QString id )
        : Component( parent, type, id )
        , eInductor( id.toStdString() )
{
    Q_UNUSED( Inductor_properties );
    
    m_ePin.resize(2);
    
    m_area = QRectF( -10, -10, 20, 20 );

    QString nodid = m_id;
    nodid.append(QString("-lPin"));
    QPoint nodpos = QPoint(-16-8,0);
    Pin* pin = new Pin( 180, nodpos, nodid, 0, this);
    pin->setLength(4.5);
    pin->setPos(-16, 0 );
    m_ePin[0] = pin;

    nodid = m_id;
    nodid.append(QString("-rPin"));
    nodpos = QPoint(16+8,0);
    pin = new Pin( 0, nodpos, nodid, 1, this);
    pin->setLength(4.5);
    pin->setPos( 16, 0 );
    m_ePin[1] = pin;
    
    m_unit = "H";
    setInduc( m_ind );
    setValLabelPos(-16, 6, 0);
    setShowVal( true );

    setLabelPos(-16,-24, 0);
}
Inductor::~Inductor(){}

double Inductor::induc() { return m_value; }

void Inductor::setInduc( double i ) 
{
    if( i == 0 ) i = 0.001;
    Component::setValue( i );       // Takes care about units multiplier
    eInductor::setInd( m_value*m_unitMult );
}

void Inductor::setUnit( QString un ) 
{
    Component::setUnit( un );
    eInductor::setInd( m_value*m_unitMult );
}
void Inductor::remove()
{
    if( m_ePin[0]->isConnected() ) (static_cast<Pin*>(m_ePin[0]))->connector()->remove();
    if( m_ePin[1]->isConnected() ) (static_cast<Pin*>(m_ePin[1]))->connector()->remove();
    Component::remove();
}

void Inductor::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
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
}


#include "moc_inductor.cpp"

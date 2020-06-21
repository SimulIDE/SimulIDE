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

#include "oscope.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "oscopewidget.h"

static const char* Oscope_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Filter")
};

Component* Oscope::construct( QObject* parent, QString type, QString id )
{
    return new Oscope( parent, type, id );
}

LibraryItem* Oscope::libraryItem()
{
    return new LibraryItem(
        tr( "Oscope" ),
        tr( "Meters" ),
        "oscope.png",
        "Oscope",
        Oscope::construct );
}

Oscope::Oscope( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( (id+"-eElement").toStdString() )
      , m_topW( )
{
    Q_UNUSED( Oscope_properties );
    
    m_area = QRectF( -115, -65, 230, 130 );
    setLabelPos(-100,-80, 0);
    
    m_pin.resize(2);
    m_ePin.resize(2);
    m_pin[0] = new Pin( 180, QPoint(-120,0 ), id+"-PinP", 0, this );
    m_pin[1] = new Pin( 180, QPoint(-120,16), id+"-PinN", 0, this );
    m_ePin[0] = m_pin[0];
    m_ePin[1] = m_pin[1];

    m_pin[0]->setLabelText( "+" );
    m_pin[1]->setLabelText( "_" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_pin[0]->setLength( 5 );
    m_pin[1]->setLength( 5 );
    
    m_oscopeW = new OscopeWidget( &m_topW );
    m_oscopeW->setupWidget( 116 );
    m_oscopeW->setFixedSize( 220, 120 );
    m_oscopeW->setVisible( true );
    m_oscopeW->setOscope( this );
    m_topW.setupWidget( m_oscopeW );
    
    m_proxy = Circuit::self()->addWidget( &m_topW);
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint( -110, -60) );
    //m_proxy->setFlag(QGraphicsItem::ItemNegativeZStacksBehindParent, true );
    
    Simulator::self()->addToUpdateList( this );
}

Oscope::~Oscope() 
{
}

void Oscope::updateStep()
{
    m_oscopeW->read();
    update();
}

double Oscope::getVolt()
{
    //qDebug() <<m_pin[0]->getVolt() - m_pin[1]->getVolt();
    return m_pin[0]->getVolt() - m_pin[1]->getVolt();
}

void Oscope::remove()
{    
    Simulator::self()->remFromUpdateList( this );
    m_oscopeW->setOscope( 0l );
    
    Component::remove();
}

void Oscope::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    
    p->setBrush( Qt::darkGray );
    p->drawRoundedRect( m_area, 4, 4 );
    
    p->setBrush( Qt::white );
    QPen pen = p->pen();
    pen.setWidth( 0 );
    pen.setColor( Qt::white );
    p->setPen(pen);
    
    p->drawRoundedRect( QRectF( -114, -64, 225, 125 ), 3, 3 );

}

#include "moc_oscope.cpp"



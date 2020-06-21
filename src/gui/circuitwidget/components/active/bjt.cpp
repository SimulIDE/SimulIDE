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

#include "bjt.h"
#include "itemlibrary.h"
#include "connector.h"
#include "circuit.h"
#include "pin.h"

static const char* BJT_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Gain"),
    QT_TRANSLATE_NOOP("App::Property","Threshold"),
    QT_TRANSLATE_NOOP("App::Property","PNP"),
    QT_TRANSLATE_NOOP("App::Property","BC diode")
};


Component* BJT::construct( QObject* parent, QString type, QString id )
{
        return new BJT( parent, type, id );
}

LibraryItem* BJT::libraryItem()
{
    return new LibraryItem(
        tr( "BJT" ),
        tr( "Active" ),
        "bjt.png",
        "BJT",
        BJT::construct );
}

BJT::BJT( QObject* parent, QString type, QString id )
   : Component( parent, type, id )
   , eBJT( id.toStdString() )
{
    Q_UNUSED( BJT_properties );
    
    m_area =  QRectF( -12, -14, 28, 28 );
    setLabelPos(18, 0, 0);

    QString newId = id;
    
    m_pin.resize( 3 );
    newId.append(QString("-collector"));
    m_pin[0] = new Pin( 90, QPoint(8,-16), newId, 0, this );
    m_pin[0]->setLabelText( "" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];
    
    newId = id;
    newId.append(QString("-emiter"));
    m_pin[1] = new Pin( 270, QPoint(8, 16), newId, 1, this );
    m_pin[1]->setLabelText( "" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[1] = m_pin[1];
    
    newId = id;
    newId.append(QString("-base"));
    m_pin[2] = new Pin( 180, QPoint(-16, 0), newId, 0, this );
    m_pin[2]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[2] = m_pin[2];

    Simulator::self()->addToUpdateList( this );
    
    resetState();
}
BJT::~BJT()
{
    Simulator::self()->remFromUpdateList( this );
}

void BJT::updateStep()
{
    update();
}

void BJT::setPnp( bool pnp ) 
{
    m_PNP = pnp;
    update();
}

void BJT::setBCd( bool bcd ) 
{ 
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim )  Simulator::self()->pauseSim();
    
    eBJT::setBCd( bcd );
    
    if( pauseSim ) Simulator::self()->runContinuous();
}

void BJT::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );
    
    if( Circuit::self()->animate() && m_baseCurr > 1e-4 )  p->setBrush( Qt::yellow );
    else                                                   p->setBrush( Qt::white );

    p->drawEllipse( m_area );
    
    p->drawLine( -12, 0,-4, 0 );
    p->drawLine(  -4,-8,-4, 8 );
    
    p->drawLine( -4,-4, 8,-12 );
    p->drawLine( -4, 4, 8, 12 );
    
    p->setBrush( Qt::black );
    if( m_PNP )
    {
        QPointF points[3] = {
        QPointF( 0.1, 6.8 ),
        QPointF( 2.4, 10 ),
        QPointF( 4, 7.5 ) };
        p->drawPolygon(points, 3);
    }
    else
    {
        QPointF points[3] = {
        QPointF( 6, 10.7 ),
        QPointF( 2.4, 10 ),
        QPointF( 4, 7.5 ) };
        p->drawPolygon(points, 3);
    }
}

#include "moc_bjt.cpp"


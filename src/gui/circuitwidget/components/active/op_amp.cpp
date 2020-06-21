/***************************************************************************
 *                                                                         *
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
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "op_amp.h"
#include "e-source.h"
#include "itemlibrary.h"
#include "connector.h"
#include "pin.h"

static const char* OpAmp_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Power Pins")
};

Component* OpAmp::construct( QObject* parent, QString type, QString id )
{
        return new OpAmp( parent, type, id );
}

LibraryItem* OpAmp::libraryItem()
{
    return new LibraryItem(
        tr( "OpAmp" ),
        tr( "Active" ),
        "opamp.png",
        "opAmp",
        OpAmp::construct );
}

OpAmp::OpAmp( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eOpAmp( id.toStdString() )
{
    Q_UNUSED( OpAmp_properties );
    
    m_area = QRect( -18, -8*2, 36, 8*2*2 );
    setLabelPos(-16,-32, 0);
    
    setGain( 1000 );
    m_voltPos = 5;
    m_voltNeg = 0;
    
    m_pin.resize( 5 );
    
    QString newId = id;
    
    newId.append(QString("-inputNinv"));
    m_pin[0] = new Pin( 180, QPoint(-16-8,-8), newId, 0, this );
    m_pin[0]->setLabelText( "+" );
    m_pin[0]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[0] = m_pin[0];
    
    newId = id;
    newId.append(QString("-inputInv"));
    m_pin[1] = new Pin( 180, QPoint(-16-8,8), newId, 1, this );
    m_pin[1]->setLabelText( " -" );
    m_pin[1]->setLabelColor( QColor( 0, 0, 0 ) );
    m_ePin[1] = m_pin[1];
    
    newId = id;
    newId.append(QString("-output"));
    m_pin[2] = new Pin( 0, QPoint(16+8,0), newId, 2, this );
    m_ePin[2] = m_pin[2];
    newId.append("-eSource");
    m_output = new eSource( newId.toStdString(), m_ePin[2] );
    //m_output->setImp( 40 );
    m_output->setOut( true );
    
    newId = id;
    newId.append(QString("powerPos"));
    m_pin[3] = new Pin( 90, QPoint(0,-16), newId, 3, this );
    m_ePin[3] = m_pin[3];
    //newId.append("-eSource");
    //m_powerPos = new eSource( newId.toStdString(), m_ePin[2] );
    
    newId = id;
    newId.append(QString("powerNeg"));
    m_pin[4] = new Pin( 270, QPoint(0, 16), newId, 4, this );
    m_ePin[4] = m_pin[4];
    //newId.append("-eSource");
    //m_powerNeg = new eSource( newId.toStdString(), m_ePin[3] );
    
    setPowerPins( false );
}
OpAmp::~OpAmp()
{
}

void OpAmp::setPowerPins( bool set )
{
    m_pin[3]->setEnabled( set );
    m_pin[3]->setVisible( set );
    m_pin[4]->setEnabled( set );
    m_pin[4]->setVisible( set );
    
    m_powerPins = set;
}

QPainterPath OpAmp::shape() const
{
    QPainterPath path;
    
    QVector<QPointF> points;
    
    points << QPointF(-16,-16 )
           << QPointF(-16, 16 )
           << QPointF( 16,  1 )
           << QPointF( 16, -1 );
        
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void OpAmp::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    QPen pen = p->pen();
    pen.setWidth(2);
    p->setPen(pen);

    static const QPointF points[4] = {
        QPointF(-16,-16 ),
        QPointF(-16, 16 ),
        QPointF( 16,  1 ),
        QPointF( 16, -1 )            };

    p->drawPolygon(points, 4);
}

#include "moc_op_amp.cpp"

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

#include "connectorline.h"
#include "connector.h"
#include "circuitview.h"
#include "circuit.h"
#include "node.h"
#include "utils.h"
 
ConnectorLine::ConnectorLine( int x1, int y1, int x2, int y2, Connector* connector )
             : QGraphicsObject()
{
    setParent( connector );
    m_pConnector = connector;
    
    m_prevLine = 0l;
    m_nextLine = 0l;

    m_p1X = x1;
    m_p1Y = y1;
    m_p2X = x2;
    m_p2Y = y2;

    m_isBus  = false;
    m_moveP1 = false;
    m_moveP2 = false;
    m_moving = false;

    this->setFlag( QGraphicsItem::ItemIsSelectable, true );

    setCursor( Qt::CrossCursor );

    //setZValue( -0.5 );
    setZValue( 1 );

    updatePos();
}
ConnectorLine::~ConnectorLine(){}

QRectF ConnectorLine::boundingRect() const
{
    int dy = m_p2Y-m_p1Y;
    int dx = m_p2X-m_p1X;
    int p =-1;
    int d = 2;

    if( dx != 0
     && dy != 0 )     return QRect( 0   , 0   , dx  , dy );
    else if( dx > 0 ) return QRect(-1   ,-2   , dx+d, 4 );
    else if( dx < 0 ) return QRect( dx+p,-2   ,-dx+d, 4 );
    else if( dy > 0 ) return QRect(-2   ,-1   , 4   , dy+d );
    else if( dy < 0 ) return QRect(-2   , dy+p, 4   ,-dy+d );
    else              return QRect( 0   , 0   , 0   , 0 );
}

void ConnectorLine::sSetP1( QPoint point )
{
    prepareGeometryChange();
    m_p1X = point.x();
    m_p1Y = point.y();
    updatePos();
}

void ConnectorLine::sSetP2( QPoint point )
{
    prepareGeometryChange();
    m_p2X = point.x();
    m_p2Y = point.y();
    updatePos();
}

void ConnectorLine::setP1( QPoint point )
{
    if( m_prevLine ) m_prevLine->sSetP2( point );
    sSetP1( point );
}

void ConnectorLine::setP2( QPoint point )
{
    if( m_nextLine ) m_nextLine->sSetP1( point );
    sSetP2( point );
}

void ConnectorLine::moveSimple( QPointF delta )
{
    bool deltaH  = fabs( delta.x() )> 0;
    bool deltaV  = fabs( delta.y() )> 0;
    
    prepareGeometryChange();

    m_p1X = m_p1X + delta.x();
    m_p1Y = m_p1Y + delta.y();
    m_p2Y = m_p2Y + delta.y();
    m_p2X = m_p2X + delta.x();

    bool isHoriz = ( dy() == 0 ) && ( dx() != 0 );
    
    if( m_prevLine && !(m_prevLine->isSelected()) )
    {
        m_prevLine->moveLine( delta.toPoint() );
        
        if( (  isHoriz && deltaV )
          ||( !isHoriz && deltaH ))
            m_prevLine->sSetP2( QPoint( m_p1X, m_p1Y) );
            
        m_prevLine->updatePos();
        m_prevLine->updatePrev();
    }
    if( m_nextLine && !(m_nextLine->isSelected()) ) 
    {
        m_nextLine->moveLine( delta.toPoint() ); 
        
        if( (  isHoriz && deltaV )
          ||( !isHoriz && deltaH ))
            m_nextLine->sSetP1( QPoint( m_p2X, m_p2Y) );
            
        m_nextLine->updatePos();
        m_nextLine->updateNext();
    }
    updatePos();
}

void ConnectorLine::move( QPointF delta )
{
    // If contiguous lines are also selected, just move line.
    bool moveSimple = false;
    /* !( ( m_prevLine && !(m_prevLine->isSelected()) )
                       ||( m_nextLine && !(m_nextLine->isSelected()) ));*/
                       
    if( Circuit::self()->pasting() || moveSimple )
    {
        prepareGeometryChange();
        m_p1Y = m_p1Y + delta.y();
        m_p2Y = m_p2Y + delta.y();
        m_p1X = m_p1X + delta.x();
        m_p2X = m_p2X + delta.x();
        updatePos();

        return;
    }
    int myindex = m_pConnector->lineList()->indexOf( this );
    if( ( myindex == 0 ) || ( myindex == m_pConnector->lineList()->size()-1 ) )
        return;    //avoid moving first or last line

    moveLine( delta.toPoint() );
    updatePos();
    updateLines();
    m_pConnector->refreshPointList();
}

void ConnectorLine::moveLine( QPoint delta )
{
    prepareGeometryChange();

    if( /*( dy() == 0 ) &&*/ ( dx() != 0 ) )
    {
       m_p1Y = m_p1Y + delta.y();
       m_p2Y = m_p2Y + delta.y();
    }
    if( /*( dx() == 0 ) &&*/ ( dy() != 0 ) )
    {
       m_p1X = m_p1X + delta.x();
       m_p2X = m_p2X + delta.x();
    }
    //else return;                     //line is "0"
}

void ConnectorLine::updateLines()
{
    updatePrev();
    updateNext();
}

void ConnectorLine::updatePrev()
{
    if( m_prevLine ) m_prevLine->sSetP2( QPoint( m_p1X, m_p1Y) );
}

void ConnectorLine::updateNext()
{
    if( m_nextLine ) 
    {
        m_nextLine->sSetP1( QPoint( m_p2X, m_p2Y) );
        m_nextLine->updatePos();
    }
}

void ConnectorLine::updatePos()
{
    setPos( m_p1X, m_p1Y );
    
    update();
}

void ConnectorLine::setPrevLine( ConnectorLine* prevLine )
{
    m_prevLine = prevLine;
}

void ConnectorLine::setNextLine( ConnectorLine* nextLine )
{
    m_nextLine = nextLine;
}

void ConnectorLine::remove() 
{ 
    if( !isSelected() ) Circuit::self()->clearSelection();
    setSelected( true );
    Circuit::self()->removeItems();

    //m_pConnector->remove(); 
}

void ConnectorLine::mousePressEvent( QGraphicsSceneMouseEvent* event )
{
    if( event->button() == Qt::MidButton )                      // Move Line
    {
        event->accept();

        if     ( dy() == 0 ) CircuitView::self()->viewport()->setCursor( Qt::SplitVCursor );
        else if( dx() == 0 ) CircuitView::self()->viewport()->setCursor( Qt::SplitHCursor );
        else                 CircuitView::self()->viewport()->setCursor( Qt::SizeAllCursor );
        m_moving = true;
    }
    else if( event->button() == Qt::LeftButton )
    {
        if( event->modifiers() == Qt::ControlModifier ) setSelected( !isSelected() ); // Select - Deselect
        
        else if( event->modifiers() & Qt::ShiftModifier ) // Move Corner
        {
            QPoint evPoint = togrid( event->scenePos() ).toPoint();
            
            if     ( evPoint==p1() ) m_moveP1 = true;
            else if( evPoint==p2() ) m_moveP2 = true;
            m_moving = true;
        }
        else                                   // Connecting a wire here
        {   
           if( Circuit::self()->is_constarted() )       
           {
               Connector* con = Circuit::self()->getNewConnector();
               
               if( con->isBus() != m_isBus ) // Avoid connect Bus with no-Bus
               {
                   event->ignore();
                   return;
               }
               eNode* eNode1 = con->enode();
               eNode* eNode2 = m_pConnector->enode();
               
               if( eNode1 == eNode2 )     // Avoid connect to same eNode
               {
                   event->ignore();
                   return;
               }
           }
           int index;
           int myindex = m_pConnector->lineList()->indexOf( this );
           QPoint point1 = togrid(event->scenePos()).toPoint();

           ConnectorLine* line;

           if(( ( (dy() == 0) && ( abs( point1.x()-m_p2X ) < 8 ) ) // point near the p2 corner
             || ( (dx() == 0) && ( abs( point1.y()-m_p2Y ) < 8 ) ) )
             && ( myindex != m_pConnector->lineList()->size()-1 ) )
           {
               if( myindex == m_pConnector->lineList()->size()-1 )
               {
                   event->ignore();
                   return;
               }
               event->accept();
               point1 = p2();
               index = myindex+1;
               line = m_pConnector->lineList()->at( index );
           }
           else if(( ( (dy() == 0) && ( abs( point1.x()-m_p1X ) < 8 ) ) // point near the p1 corner
                  || ( (dx() == 0) && ( abs( point1.y()-m_p1Y ) < 8 ) ) )
                  && ( myindex != 0 ) )
           {
               if( myindex == 0 )
               {
                   event->ignore();
                   return;
               }
               event->accept();
               point1 = p1();
               line = this;
               index = myindex;
           }
           else                                // split this line in two
           {
               event->accept();

               if( dy() == 0 ) point1.setY( m_p1Y );
               else            point1.setX( m_p1X );

               index = myindex+1;

               line = new ConnectorLine( point1.x(), point1.y(), m_p2X, p2().y(), m_pConnector );
               m_pConnector->addConLine( line, index );
           }

           QString type = QString("Node");
           QString id = type;
           id.append( "-" );
           id.append( Circuit::self()->newSceneId() );

           Node* node = new Node( 0, type, id );     // Now add the Node
           node->setPos( point1.x(), point1.y());
           Circuit::self()->addItem( node );

           bool pauseSim = Simulator::self()->isRunning();
           if( pauseSim )  Simulator::self()->pauseSim();

           //qDebug() << "line constarted" << Circuit::self()->is_constarted() << Circuit::self();

           m_pConnector->splitCon( index, node->getPin(0), node->getPin(2) );
           eNode* enode = m_pConnector->enode();    // get the eNode from my connector
           node->getPin(1)->setEnode( enode );

           if( Circuit::self()->is_constarted() )   // A Connector wants to connect here (ends in a node)
               Circuit::self()->closeconnector( node->getPin(1) );
           else                                     // A new Connector created here (starts in a node)
               Circuit::self()->newconnector( node->getPin(1) );      // start a new connector

           if( pauseSim ) Simulator::self()->runContinuous();
        }
    }
    //else setSelected( true );
}

void ConnectorLine::mouseMoveEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();

    QPoint delta = togrid( event->scenePos() ).toPoint() - togrid(event->lastScenePos()).toPoint();

    if( event->modifiers() & Qt::ShiftModifier )          // Move Corner
    {
        //qDebug() << "ConnectorLine::mousePressEvent"<<event->scenePos()<<evPoint<<p1()<<p2();
        //qDebug() << "ConnectorLine::mousePressEvent corner"<<delta;
        if     ( m_moveP1 ) setP1( p1()+delta );
        else if( m_moveP2 ) setP2( p2()+delta );
    }
    else
    {
        int myindex = m_pConnector->lineList()->indexOf( this );

        if( myindex == 0 )
            m_pConnector->addConLine( p1().x(), p1().y(), p1().x(), p1().y(), myindex );

        else if( myindex == m_pConnector->lineList()->size()-1 )
            m_pConnector->addConLine( p2().x(), p2().y(), p2().x(), p2().y(), myindex + 1 );

        moveLine( delta );
    }
    updatePos();
    updateLines();
    //Circuit::self()->update();
}

void ConnectorLine::mouseReleaseEvent( QGraphicsSceneMouseEvent* event )
{
    event->accept();
    m_moveP1 = false;
    m_moveP2 = false;
    m_pConnector->remNullLines();

    if( m_moving )
    {
        m_moving = false;
        Circuit::self()->setChanged();
    }
}

void ConnectorLine::contextMenuEvent( QGraphicsSceneContextMenuEvent* event )
{
    if( Circuit::self()->is_constarted() ) return;

    if( m_pConnector->endPin() )
    {
       event->accept();
       QMenu menu;

       QAction* removeAction = menu.addAction( tr("Remove") );
       connect(removeAction, SIGNAL(triggered()), this, SLOT(remove()));

       menu.exec(event->screenPos());

       //qDebug() << "ConnectorLine::contextMenuEvent\n" << m_pConnector->pointList();
    }
}

void ConnectorLine::setIsBus( bool bus )
{
    m_isBus = bus;
}

void ConnectorLine::setConnector( Connector* con ) { m_pConnector = con; }

QPoint ConnectorLine::p1() { return QPoint( m_p1X, m_p1Y ); }
QPoint ConnectorLine::p2() { return QPoint( m_p2X, m_p2Y ); }

int ConnectorLine::dx() { return (m_p2X - m_p1X);}
int ConnectorLine::dy() { return (m_p2Y - m_p1Y);}

bool ConnectorLine::isDiagonal()
{
    return ( abs(m_p2X - m_p1X)>0 && abs(m_p2Y - m_p1Y)>0 );
}

Connector* ConnectorLine::connector(){ return m_pConnector; }

QPainterPath ConnectorLine::shape() const
{
    int dy = m_p2Y-m_p1Y;
    int dx = m_p2X-m_p1X;
    int q = 0;
    int i = 0;

    if( this->cursor().shape() == Qt::ArrowCursor ) // New Connector
    {
        if     ( dx > 0 ) q = -3;
        else if( dx < 0 ) q =  3;
        if     ( dy > 0 ) i = -3;
        else if( dy < 0 ) i =  3;
    }

    QPainterPath path;
    
    QVector<QPointF> points;
    
    if( abs(m_p2X - m_p1X) > abs(m_p2Y - m_p1Y) )
    {
        points << mapFromScene( QPointF( m_p1X  , m_p1Y-2 ) )
               << mapFromScene( QPointF( m_p1X  , m_p1Y+2 ) )
               << mapFromScene( QPointF( m_p2X+q, m_p2Y+2 ) )
               << mapFromScene( QPointF( m_p2X+q, m_p2Y-2 ) );
    }
    else
    {
        points << mapFromScene( QPointF( m_p1X-2, m_p1Y   ) )
               << mapFromScene( QPointF( m_p1X+2, m_p1Y   ) )
               << mapFromScene( QPointF( m_p2X+2, m_p2Y+i ) )
               << mapFromScene( QPointF( m_p2X-2, m_p2Y+i ) );
    }
    path.addPolygon( QPolygonF(points) );
    path.closeSubpath();
    return path;
}

void ConnectorLine::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //pen.setColor( Qt::darkGray);
    //p->setPen( pen );

    QColor color;
    if( isSelected() ) color = QColor( Qt::darkGray );
    else if( !m_isBus  && Circuit::self()->animate() )               //color = QColor( 40, 40, 60 /*Qt::black*/ );
    {
        if( m_pConnector->getVolt() > 2.5 ) color = QColor( 200, 50, 50 );
        else                                color = QColor( 50, 50, 200 );
        //int volt = 50*int( m_pConnector->getVolt() );
        //if( volt > 250 )volt = 250;
        //if( volt < 0 ) volt = 0;

        /*if( m_pConnector->endPin()
        && (m_pConnector->startPin()->changed()
        ||  m_pConnector->endPin()->changed()) )
        { pen.setWidth(3); }*/

        //color = QColor( volt, 50, 250-volt);
    }
    else if( m_isBus ) color =  Qt::darkGreen;
    else color = QColor( 40, 40, 60 /*Qt::black*/ );

    QPen pen( color, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    //p->setBrush( Qt::green );
    //p->drawRect( boundingRect() );
    //p->setBrush( Qt::blue );
    //p->drawPath( shape() );
    
    if( m_isBus ) 
    {
        pen.setWidth( 3 );
    }
    p->setPen( pen );
    p->drawLine( 0, 0, dx(), dy());
}


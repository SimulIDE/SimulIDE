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

#include "probe.h"
#include "connector.h"
#include "connectorline.h"
#include "e-source.h"
#include "simulator.h"
#include "itemlibrary.h"
#include "circuitwidget.h"
#include "pin.h"

#include <math.h>

static const char* Probe_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Show volt"),
    QT_TRANSLATE_NOOP("App::Property","Threshold"),
    QT_TRANSLATE_NOOP("App::Property","PlotterCh")
};

Component* Probe::construct( QObject* parent, QString type, QString id )
{ return new Probe( parent, type, id ); }

LibraryItem* Probe::libraryItem()
{
    return new LibraryItem(
        tr( "Probe" ),
        tr( "Meters" ),
        "probe.png",
        "Probe",
        Probe::construct );
}

Probe::Probe( QObject* parent, QString type, QString id )
     : Component( parent, type, id )
     , eElement( id.toStdString() )
{
    Q_UNUSED( Probe_properties );
    
    m_area = QRect( -8, -8, 16, 16 );
    m_readPin = 0l;
    m_readConn = 0l;
    m_voltTrig = 2.5;
    m_plotterLine = 0;
    m_plotterColor = QColor( 255, 255, 255 );

    // Create Input Pin
    m_ePin.resize(1);
    QString nodid = id;
    nodid.append(QString("-inpin"));
    QPoint nodpos = QPoint(-22,0);
    m_inputpin = new Pin( 180, nodpos, nodid, 0, this);
    m_inputpin->setLength( 20 );
    m_inputpin->setBoundingRect( QRect(-2, -2, 6, 4) );
    
    nodid.append( QString("-eSource") );
    m_inSource = new eSource( nodid.toStdString(), m_inputpin );
    m_inSource->setOut(false);
    m_inSource->setImp( 1e9 );

    setRotation( rotation() - 45 );
    
    m_unit = " ";
    m_valLabel->setDefaultTextColor( Qt::darkRed );
    m_valLabel->setPlainText( "0" );
    setValLabelPos( 16, 0 , 45 ); // x, y, rot 
    setVolt( 0 );
    setShowVal( true );
    
    setLabelPos( 16, -16 , 45 );

    Simulator::self()->addToUpdateList( this );
}
Probe::~Probe()
{
    delete m_inSource;
}

void Probe::updateStep()
{
    m_readPin = 0l;
    m_readConn = 0l;
    
    if( !Simulator::self()->isRunning() )
    {
        setVolt( 0.0 );
        return;
    }

    if( m_inputpin->isConnected() )// Voltage from connected pin
    {
         setVolt( m_inputpin->getVolt() );
         return;
    }

    // Voltage from connector or Pin behind inputPin
    QList<QGraphicsItem*> list = m_inputpin->collidingItems();

    if( list.isEmpty() )
    {
        setVolt( 0.0 );
        return;
    }
    for( QGraphicsItem* it : list )
    {
        if( it->type() == 65536 )                           // Component
        {
            ConnectorLine* line =  qgraphicsitem_cast<ConnectorLine*>( it );

            Connector* con = line->connector();

            if( con->objectName().startsWith("Connector") ) // Connector found
            {
                setVolt( con->getVolt() ); //startPin()->volt();
                m_readConn = con;
                break;
            }
        }
        else if( it->type() == 65536+3 )                    // Pin found
        {
            m_readPin =  qgraphicsitem_cast<Pin *>( it );
            setVolt( m_readPin->getVolt() );
            //qDebug() << " probe: Pin found" << volt;
            break;
        }
    }
    //qDebug() << " probe: " /*<< item->type()*/ << UserType;//con->objectName();
}

void Probe::setVolt( double volt )
{
    if( m_voltIn == volt ) return;

    m_voltIn = volt;

    if( fabs(volt) < 0.01 ) volt = 0;
    int dispVolt = int( volt*100+0.5 );
    
    m_valLabel->setPlainText( QString("%1 V").arg(double(dispVolt)/100) );

    if( m_plotterLine > 0 ) PlotterWidget::self()->setData( m_plotterLine, m_voltIn*100 );

    update();       // Repaint
}

double Probe::getVolt()
{
    double volt = 0;
    if     ( m_inputpin->isConnected() ) volt = m_inputpin->getVolt();
    else if( m_readConn != 0l )          volt = m_readConn->getVolt();
    else if( m_readPin != 0l )           volt = m_readPin->getVolt();
    return volt;
}

void Probe::remove()
{
    if( m_inputpin->isConnected() ) m_inputpin->connector()->remove();

    slotPlotterRem();
    
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

int Probe::plotter()
{
    return m_plotterLine ;
}

void Probe::setPlotter( int channel )
{
    if( channel == 0 ) return;
    
    if( PlotterWidget::self()->addChannel( channel ) )
    {
        slotPlotterRem(); 
        m_plotterLine = channel;
        PlotterWidget::self()->setData( m_plotterLine, int(m_voltIn*100) );
        m_plotterColor = PlotterWidget::self()->getColor( m_plotterLine );
        update();       // Repaint
    }
}

void Probe::slotPlotter1() { setPlotter( 1 ); }
void Probe::slotPlotter2() { setPlotter( 2 ); }
void Probe::slotPlotter3() { setPlotter( 3 ); }
void Probe::slotPlotter4() { setPlotter( 4 ); }

/*void Probe::slotPlotterAdd()
{
    if( m_plotterLine != 0 ) return;            // Already have plotter
    
    m_plotterLine = PlotterWidget::self()->getChannel();
    if( m_plotterLine < 1 ) return;

    PlotterWidget::self()->setData( m_plotterLine, int(m_voltIn*100) );
    m_plotterColor = PlotterWidget::self()->getColor( m_plotterLine );
    update();       // Repaint
}*/

void Probe::slotPlotterRem()
{
    //qDebug() << m_plotterLine;
    if( m_plotterLine == 0 ) return;              // No plotter to remove

    PlotterWidget::self()->remChannel( m_plotterLine );
    m_plotterLine = 0;
    update();       // Repaint
}

void Probe::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    event->accept();
    QMenu* menu  = new QMenu();
    QMenu *pmenu = menu->addMenu(QIcon(":/fileopen.png"),tr("Plotter Channel"));

    QAction* plotter1Action = pmenu->addAction(QIcon(":/fileopen.png"),tr("Channel 1"));
    connect(plotter1Action, SIGNAL(triggered()), this, SLOT(slotPlotter1()));
    
    QAction* plotter2Action = pmenu->addAction(QIcon(":/fileopen.png"),tr("Channel 2"));
    connect(plotter2Action, SIGNAL(triggered()), this, SLOT(slotPlotter2()));
    
    QAction* plotter3Action = pmenu->addAction(QIcon(":/fileopen.png"),tr("Channel 3"));
    connect(plotter3Action, SIGNAL(triggered()), this, SLOT(slotPlotter3()));
    
    QAction* plotter4Action = pmenu->addAction(QIcon(":/fileopen.png"),tr("Channel 4"));
    connect(plotter4Action, SIGNAL(triggered()), this, SLOT(slotPlotter4()));

    QAction* plotterRemAction = pmenu->addAction(QIcon(":/fileopen.png"),tr("Remove from Plotter"));
    connect(plotterRemAction, SIGNAL(triggered()), this, SLOT(slotPlotterRem()));
    
    menu->addSeparator();

    Component::contextMenu( event, menu );
    menu->deleteLater();
}

QPainterPath Probe::shape() const
{
    QPainterPath path;
    path.addEllipse( m_area );
    return path;
}

void Probe::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    if( m_plotterLine > 0 )           p->setBrush( m_plotterColor );
    else if ( m_voltIn > m_voltTrig)  p->setBrush( QColor( 255, 166, 0 ) );
    else if ( m_voltIn < -m_voltTrig) p->setBrush( QColor( 0, 100, 255 ) );
    else                              p->setBrush( QColor( 230, 230, 255 ) );

    p->drawEllipse( m_area );
    
    if( m_plotterLine > 0 )
    {
        //p->drawLine(-4,-7,-5,-1 );
        p->drawLine(-5,-1, 1,-3 );
        p->drawLine( 1,-3,-1, 3 );
        p->drawLine(-1, 3, 5, 1 );
        //p->drawLine( 6, 1, 4, 7 );
        //p->drawLine( 5, 3, 8,  0 );
    }
}

#include "moc_probe.cpp"


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

#include "currsource.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"

static const char* CurrSource_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Current"),
    QT_TRANSLATE_NOOP("App::Property","Show Amp")
};

Component* CurrSource::construct( QObject* parent, QString type, QString id )
{
    return new CurrSource( parent, type, id );
}

LibraryItem* CurrSource::libraryItem()
{
    return new LibraryItem(
        tr( "Current Source" ),
        tr( "Sources" ),
        "voltsource.png",
        "Current Source",
        CurrSource::construct );
}

CurrSource::CurrSource( QObject* parent, QString type, QString id )
          : Component( parent, type, id )
          , eElement( id.toStdString() )
{
    Q_UNUSED( CurrSource_properties );

    m_area = QRect( -44, -68, 56, 80 );
    
    m_changed = false;

    m_voltw.setFixedSize( 46,70 );
    
    m_proxy = Circuit::self()->addWidget( &m_voltw );
    m_proxy->setParentItem( this );
    m_proxy->setPos( QPoint(-39, -63) );

    m_button = m_voltw.pushButton;
    m_dial   = m_voltw.dial;
    m_dial->setMaximum( 1000 );

    m_button->setText( QString("-- A") );

    QString nodid = id;
    nodid.append(QString("-outPin"));
    QPoint nodpos = QPoint(16,0);
    outpin = new Pin( 0, nodpos, nodid, 0, this );
    
    m_unit = "A";
    //setVolt(50.0);
    setCurrent( 1 );
    currChanged( 0 );
    
    setValLabelPos(-26, 10 , 0 ); // x, y, rot 
    setShowVal( true );
    
    setLabelPos(-40,-80, 0);
    
    Simulator::self()->addToUpdateList( this );

    connect( m_button, SIGNAL( clicked()),
             this,     SLOT  ( onbuttonclicked()) );

    connect( m_dial,   SIGNAL( valueChanged(int) ),
             this,     SLOT  ( currChanged(int)) );
}

CurrSource::~CurrSource() 
{ 
}

void CurrSource::initialize()
{
    m_changed = true;
}

void CurrSource::updateStep()
{
    if( m_changed ) 
    {
        double current = 0;
        if( m_button->isChecked() ) current = m_current;

        outpin->stampCurrent( current );

        updateButton();
        m_changed = false;
    }
}

void CurrSource::updateButton()
{
    QString msg;
    bool checked = m_button->isChecked();
    
    if( checked )
        msg = QString("%1 A").arg(float(int(m_current*100))/100);
    else
        msg = QString("-- A");
        
    m_button->setText( msg );
}

void CurrSource::onbuttonclicked()
{
    m_changed = true;
}

void CurrSource::currChanged( int val )
{
    m_current = double( m_maxCurrent*val/1000 );
    m_changed = true;
}

/*void CurrSource::setVolt( double v )            // Sets the Maximum Volt
{
    m_voltHight = v;
}*/

void CurrSource::setCurrent( double c )      
{
    Component::setValue( c );       // Takes care about units multiplier
    m_maxCurrent = m_value*m_unitMult;
    currChanged( m_dial->value() );
}

void CurrSource::setUnit( QString un ) 
{
    Component::setUnit( un );
    m_maxCurrent = m_value*m_unitMult;
    currChanged( m_dial->value() );
}

void CurrSource::remove()
{
    if ( outpin->isConnected() ) outpin->connector()->remove();
    
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

void CurrSource::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    p->setBrush(Qt::white);
    p->drawRoundedRect( QRect( -42, -66, 52, 76 ), 1, 1 );

    p->fillRect( QRect( -39, -63, 49, 73 ), Qt::darkGray );

    Component::paint( p, option, widget );
}

#include "moc_currsource.cpp"

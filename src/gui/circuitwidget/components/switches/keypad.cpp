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

#include "keypad.h"
#include "circuit.h"
#include "itemlibrary.h"

static const char* KeyPad_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Key Labels")
};

Component* KeyPad::construct( QObject* parent, QString type, QString id )
{ return new KeyPad( parent, type, id ); }

LibraryItem* KeyPad::libraryItem()
{
    return new LibraryItem(
            tr( "KeyPad" ),
            tr( "Switches" ),
            "keypad.png",
            "KeyPad",
            KeyPad::construct);
}

KeyPad::KeyPad( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( id.toStdString() )
{
    Q_UNUSED( KeyPad_properties );
    
    setLabelPos(-8,-16, 0);
    
    m_keyLabels = "123456789*0#";
    m_rows = 4;
    m_cols = 3;
    setupButtons();
}
KeyPad::~KeyPad(){}

void KeyPad::attach()
{
    for( int row=0; row<m_rows; row++ )
    {
        Pin* rowPin = m_pin[row];
        eNode* rowNode = rowPin->getEnode();
        
        for( int col=0; col<m_cols; col++ )
        {
            Pin* colPin = m_pin[m_rows+col];
            eNode* colNode = colPin->getEnode();
            
            PushBase* button = m_buttons.at( row*m_cols+col );
            
            ePin* epin0 = button->getEpin( 0 );
            epin0->setEnode( rowNode );
            
            ePin* epin1 = button->getEpin( 1 );
            epin1->setEnode( colNode );
        }
    }
}

void KeyPad::setupButtons()
{
    bool pauseSim = Simulator::self()->isRunning();
    if( pauseSim ) Simulator::self()->pauseSim();
    
    m_area = QRectF( -12, -4, 16*m_cols+8, 16*m_rows+8 );
    
    for( PushBase* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       Circuit::self()->removeComp( button );
    }
    
    for( Pin* pin : m_pin ) 
    {
        if( pin->isConnected() ) pin->connector()->remove();
        if( pin->scene() ) Circuit::self()->removeItem( pin );
        delete pin;
    }
    m_pin.resize( m_rows + m_cols );
    
    int labelMax = m_keyLabels.size()-1;
    
    for( int row=0; row<m_rows; row++ )
    {
        QString pinId = m_id;
        pinId.append( QString("-Pin")+QString::number(row)) ;
        QPoint pinPos = QPoint(m_cols*16, 8+row*16);
        m_pin[row] = new Pin( 0, pinPos, pinId, 0, this);
        
        for( int col=0; col<m_cols; col++ )
        {
            QString butId = m_id+"button"+QString::number(row)+QString::number(col);
            //qDebug()<<butId;
            PushBase* button = new PushBase( this, "PushBase", butId );
            button->SetupButton();
            button->setParentItem( this );
            button->setPos( QPointF(col*16+12, 16+row*16 ) );
            button->setFlag( QGraphicsItem::ItemIsSelectable, false );
            m_buttons.append( button );
            
            int pos = row*m_cols+col;
            QString buttonLabel = "";
            if( pos <= labelMax ) buttonLabel = m_keyLabels.mid( pos, 1 );
            button->setKey( buttonLabel );

            if( row == 0 )
            {
                QString pinId = m_id;
                pinId.append( QString("-Pin")+QString::number(m_rows+col)) ;
                QPoint pinPos = QPoint( col*16, m_rows*16+8);
                m_pin[m_rows+col] = new Pin( 270, pinPos, pinId, 0, this);
            }
        }
    }
    if( pauseSim ) Simulator::self()->resumeSim();
    Circuit::self()->update();
}

double KeyPad::rows()
{
    return m_rows;
}

void KeyPad::setRows( double rows )
{
    m_rows = rows;
    setupButtons();
}

double KeyPad::cols()
{
    return m_cols;
}

void KeyPad::setCols( double cols )
{
    m_cols = cols;
    setupButtons();
}

QString KeyPad::keyLabels()
{
    return m_keyLabels;
}
void KeyPad::setKeyLabels( QString keyLabels )
{
    m_keyLabels = keyLabels;
    
    int labelMax = m_keyLabels.size()-1;
    
    for( int row=0; row<m_rows; row++ )
    {
        for( int col=0; col<m_cols; col++ )
        {
            PushBase* button = m_buttons.at( row*m_cols+col );
            
            int pos = row*m_cols+col;
            QString buttonLabel = "";
            if( pos <= labelMax ) buttonLabel = m_keyLabels.mid( pos, 1 );
            button->setKey( buttonLabel );
        }
    }
}

void KeyPad::remove()
{
    for( PushBase* button : m_buttons ) 
        Circuit::self()->removeComp( button );

    Component::remove();
}

void KeyPad::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    Component::paint( p, option, widget );

    p->drawRoundedRect( m_area,2,2 );
}

#include "moc_keypad.cpp"

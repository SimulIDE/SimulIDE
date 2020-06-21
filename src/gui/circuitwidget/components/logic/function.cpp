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

#include "function.h"
#include "connector.h"
#include "circuit.h"
#include "itemlibrary.h"

static const char* Function_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Functions")
};

Component* Function::construct( QObject* parent, QString type, QString id )
{
        return new Function( parent, type, id );
}

LibraryItem* Function::libraryItem()
{
    return new LibraryItem(
        tr( "Function" ),
        tr( "Logic/Arithmetic" ),
        "subc.png",
        "Function",
        Function::construct );
}

Function::Function( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eFunction( id.toStdString() )
{
    Q_UNUSED( Function_properties );
    
    setNumInps( 2 );                           // Create Input Pins
    setNumOuts( 1 );
    
    setFunctions( "i0 | i1" );
    
    //Simulator::self()->addToUpdateList( this );
}
Function::~Function(){
}

void Function::remove()
{
    for( QPushButton* button : m_buttons ) 
    {
       m_buttons.removeOne( button );
       delete button;
    }
    LogicComponent::remove();
}

void Function::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;
    
    if( inputs < m_numInputs ) 
    {
        int dif = m_numInputs-inputs;
        
        eLogicDevice::deleteInputs( dif );
        LogicComponent::deleteInputs( dif );
    }
    else
    {
        m_inPin.resize( inputs );
        m_numInPins = inputs;
    
        for( int i=m_numInputs; i<inputs; i++ )
        {
            QString num = QString::number(i);
            m_inPin[i] = new Pin( 180, QPoint(-24, i*8+8 ), m_id+"-in"+num, i, this );
                                   
            m_inPin[i]->setLabelText( " I"+num );
            m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

            eLogicDevice::createInput( m_inPin[i] );
        }
    }
    m_height = m_numOutputs*2-1;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );
    
    Circuit::self()->update();
}

void Function::setNumOuts( int outs )
{
    if( outs == m_numOutputs ) return;
    if( outs < 1 ) return;
    
    if( outs < m_numOutputs ) 
    {
        int dif = m_numOutputs-outs;
        
        eLogicDevice::deleteOutputs( dif );
        LogicComponent::deleteOutputs( dif );
    
        for( int i=0; i<dif; i++ )
        {
            QPushButton* button = m_buttons.takeLast();
            disconnect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
            delete button;
            
            m_proxys.removeLast();
            m_funcList.removeLast();
        }
    }
    else
    {
        m_outPin.resize( outs );
        m_numOutPins = outs;
        
        for( int i=m_numOutputs; i<outs; i++ )
        {
            QString num = QString::number(i);
            m_outPin[i] = new Pin( 0, QPoint(24, i*8*2+8 ), m_id+"-out"+num, i, this );

            eLogicDevice::createOutput( m_outPin[i] );
            
            QPushButton* button = new QPushButton( );
            button->setMaximumSize( 14,14 );
            button->setGeometry(-14,-14,14,14);
            QFont font = button->font();
            font.setPixelSize(7);
            button->setFont(font);
            button->setText( "O"+num );
            button->setCheckable( true );
            m_buttons.append( button );

            QGraphicsProxyWidget* proxy = Circuit::self()->addWidget( button );
            proxy->setParentItem( this );
            proxy->setPos( QPoint( 0, i*8*2+1 ) );
            
            m_proxys.append( proxy );
            m_funcList.append( "" );
            
            connect( button, SIGNAL( released() ), this, SLOT  ( onbuttonclicked() ));
        }
    }
    m_height = m_numOutputs*2-1;
    if( m_numInputs > m_height ) m_height = m_numInputs;
    m_area = QRect( -16, 0, 32, 8*m_height+8 );
    
    m_functions = m_funcList.join(",");
    
    Circuit::self()->update();
}

void Function::onbuttonclicked()
{
    int i = 0;
    for( QPushButton* button : m_buttons ) 
    {
       if( button->isChecked()  )
       {
           button->setChecked( false );
           break;
       }
       i++;
    }
    bool ok;
    QString text = QInputDialog::getText(0l, tr("Set Function"),
                                             "Output "+QString::number(i)+tr(" Function:"), 
                                             QLineEdit::Normal,
                                             m_funcList[i], &ok);
    if( ok && !text.isEmpty() )
    {
        m_funcList[i] = text;
        m_functions = m_funcList.join(",");
    }
}

#include "moc_function.cpp"

/***************************************************************************
 *   Copyright (C) 2017 by santiago González                               *
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

#include "adc.h"
#include "connector.h"
#include "itemlibrary.h"

static const char* ADC_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Vref")
};

Component* ADC::construct( QObject* parent, QString type, QString id )
{
        return new ADC( parent, type, id );
}

LibraryItem* ADC::libraryItem()
{
    return new LibraryItem(
        tr( "ADC" ),
        tr( "Logic/Other Logic" ),
        "1to3.png",
        "ADC",
        ADC::construct );
}

ADC::ADC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
   , eADC( id.toStdString() )
{
    Q_UNUSED( ADC_properties );
    
    m_width  = 4;
    m_height = 9;

    setNumOuts( 8 );                           // Create Output Pins
    setMaxVolt( 5 );

    LogicComponent::setNumInps( 1 );
    
    m_inPin[0] = new Pin( 180, QPoint( -24, -8 ), m_id+"-in", 1, this );
    m_inPin[0]->setLabelText( " In" );
    m_inPin[0]->setLabelColor( QColor( 0, 0, 0 ) );
                          
    eLogicDevice::createInput( m_inPin[0] );
}
ADC::~ADC(){
}

void ADC::setNumOuts( int outs )
{
    if( outs == m_numOutputs ) return;
    if( outs < 1 ) return;

    LogicComponent::setNumOuts( outs );
    eLogicDevice::deleteOutputs( m_numOutputs );

    for( int i=0; i<outs; i++ )
    {
        QString num = QString::number(outs-i-1);
        m_outPin[i] = new Pin( 0, QPoint(24,-8*outs+i*8+8 ), m_id+"-out"+num, i, this );

        m_outPin[i]->setLabelText( "D"+num+" " );
        m_outPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

        eLogicDevice::createOutput( m_outPin[i] );
    }
    m_maxAddr = pow( 2, m_numOutputs )-1;

    m_height = outs+1;
    m_area = QRect( -(m_width/2)*8, -m_height*8+8, m_width*8, m_height*8 );
}

#include "moc_adc.cpp"

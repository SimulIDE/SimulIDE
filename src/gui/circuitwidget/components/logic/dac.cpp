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

#include "dac.h"

Component* DAC::construct( QObject* parent, QString type, QString id )
{
        return new DAC( parent, type, id );
}

LibraryItem* DAC::libraryItem()
{
    return new LibraryItem(
        tr( "DAC" ),
        tr( "Logic/Other Logic" ),
        "3to1.png",
        "DAC",
        DAC::construct );
}

DAC::DAC( QObject* parent, QString type, QString id )
   : LogicComponent( parent, type, id )
   , eDAC( id.toStdString() )
{    
    m_width  = 4;
    m_height = 9;

    setNumInps( 8 );                           // Create Input Pins
    setMaxVolt( 5 );

    LogicComponent::setNumOuts( 1 );
    
    m_outPin[0] = new Pin( 0, QPoint( 24, -8 ), m_id+"-out", 1, this );
    m_outPin[0] ->setLabelText( "Out " );
    m_outPin[0] ->setLabelColor( QColor( 0, 0, 0 ) );
                          
    eLogicDevice::createOutput( m_outPin[0] );
}
DAC::~DAC(){
}

void DAC::setNumInps( int inputs )
{
    if( inputs == m_numInputs ) return;
    if( inputs < 1 ) return;

    LogicComponent::setNumInps( inputs );
    eLogicDevice::deleteInputs( m_numInputs );

    for( int i=0; i<inputs; i++ )
    {
        QString num = QString::number( inputs-i-1 );
        m_inPin[i] = new Pin( 180, QPoint(-24,-8*inputs+i*8+8 ), m_id+"-in"+num, i, this );

        m_inPin[i]->setLabelText( "D"+num+" " );
        m_inPin[i]->setLabelColor( QColor( 0, 0, 0 ) );

        eLogicDevice::createInput( m_inPin[i] );
    }
    m_maxAddr = pow( 2, m_numInputs )-1;

    m_height = inputs+1;
    m_area = QRect( -(m_width/2)*8, -m_height*8+8, m_width*8, m_height*8 );
}

#include "moc_dac.cpp"

/***************************************************************************
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
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "bcdtodec.h"
#include "circuit.h"


Component* BcdToDec::construct( QObject* parent, QString type, QString id )
{
        return new BcdToDec( parent, type, id );
}

LibraryItem* BcdToDec::libraryItem()
{
    return new LibraryItem(
        tr( "Decoder(4 to 10/16)" ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "BcdToDec",
        BcdToDec::construct );
}

BcdToDec::BcdToDec( QObject* parent, QString type, QString id )
        : LogicComponent( parent, type, id )
        , eBcdToDec( id.toStdString() )
{
    m_width  = 4;
    m_height = 11;

    m_16Bits = false;

    QStringList pinList;

    pinList // Inputs:
            
            << "IL04 S0"
            << "IL05 S1"
            << "IL06 S2"
            << "IL07 S3"

            << "IU01OE "
            
            // Outputs:
            << "OR01O0 "
            << "OR02O1 "
            << "OR03O2 "
            << "OR04O3 "
            << "OR05O4 "
            << "OR06O5 "
            << "OR07O6 "
            << "OR08O7 "
            << "OR09O8 "
            << "OR10O9 "
            << "OR1110 "
            << "OR1211 "
            << "OR1312 "
            << "OR1413 "
            << "OR1514 "
            << "OR1615 "
            ;
    init( pinList );

    eLogicDevice::createOutEnablePin( m_inPin[4] );    // IOutput Enable

    for( int i=0; i<4; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    for( int i=0; i<16; i++ )
    {
        eLogicDevice::createOutput( m_outPin[i] );
    }
    for( int i=10; i<16; i++ )
    {
        m_outPin[i]->setVisible( false );
    }
}
BcdToDec::~BcdToDec(){}

bool BcdToDec::_16bits()
{
    return m_16Bits;
}

void BcdToDec::set_16bits( bool set )
{
    m_16Bits = set;

    if( set )
    {
        for( int i=10; i<16; i++ )
        {
            m_outPin[i]->setVisible( true );
        }
        m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, 17*8 );
    }
    else
    {
        for( int i=10; i<16; i++ )
        {
            m_outPin[i]->setVisible( false );
            if( m_outPin[i]->isConnected() ) m_outPin[i]->connector()->remove();
        }
        m_area = QRect( -(m_width/2)*8, -(m_height/2)*8, m_width*8, m_height*8 );
    }
    Circuit::self()->update();
}

#include "moc_bcdtodec.cpp"

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

#include "bcdto7s.h"
#include "pin.h"


Component* BcdTo7S::construct( QObject* parent, QString type, QString id )
{
        return new BcdTo7S( parent, type, id );
}

LibraryItem* BcdTo7S::libraryItem()
{
    return new LibraryItem(
        tr( "Bcd To 7S." ),
        tr( "Logic/Converters" ),
        "2to3g.png",
        "BcdTo7S",
        BcdTo7S::construct );
}

BcdTo7S::BcdTo7S( QObject* parent, QString type, QString id )
       : LogicComponent( parent, type, id )
       , eBcdTo7S( id.toStdString() )
{
    m_width  = 4;
    m_height = 8;

    QStringList pinList;

    pinList // Inputs:
            
            << "IL03 S0"
            << "IL04 S1"
            << "IL05 S2"
            << "IL06 S3"

            << "IU01OE "
            
            // Outputs:
            << "OR01a  "
            << "OR02b  "
            << "OR03c  "
            << "OR04d "
            << "OR05e  "
            << "OR06f  "
            << "OR07g "
            ;
    init( pinList );

    eLogicDevice::createOutEnablePin( m_inPin[4] );    // IOutput Enable

    for( int i=0; i<4; i++ )
        eLogicDevice::createInput( m_inPin[i] );
        
    for( int i=0; i<7; i++ )
    {
        eLogicDevice::createOutput( m_outPin[i] );
    }
}
BcdTo7S::~BcdTo7S(){}

void BcdTo7S::stamp()
{
    eBcdTo7S::stamp();

    m_outValue[0] = true;
    m_outValue[1] = true;
    m_outValue[2] = true;
    m_outValue[3] = true;
    m_outValue[4] = true;
    m_outValue[5] = true;
    m_outValue[6] = false;
    for( int i=0; i<m_numOutputs; i++ ) setOut( i, m_outValue[i] );
}

#include "moc_bcdto7s.cpp"

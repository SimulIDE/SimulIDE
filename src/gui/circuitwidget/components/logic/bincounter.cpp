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

#include "bincounter.h"
#include "pin.h"

static const char* BinCounter_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Max Value")
};

Component *BinCounter::construct(QObject *parent, QString type, QString id)
{
    return new BinCounter(parent, type, id);
}

LibraryItem* BinCounter::libraryItem()
{
    return new LibraryItem(
        tr( "Counter" ),
        tr ("Logic/Arithmetic"),
        "2to1.png",
        "Counter",
        BinCounter::construct );
}

BinCounter::BinCounter(QObject *parent, QString type, QString id) 
          : LogicComponent( parent, type, id )
          , eBinCounter( id.toStdString() )
{
    Q_UNUSED( BinCounter_properties );
    
    m_width  = 4;
    m_height = 4;

    QStringList pinList;
    pinList
      << "IL01>"
      << "IL03 R"
      << "OR02Q "
    ;
    init( pinList );
    
    eLogicDevice::createClockPin( m_inPin[0] );           // Input Clock
    
    eLogicDevice::createInput( m_inPin[1] );              // Input Reset
    
    eLogicDevice::createOutput( m_outPin[0] );               // Output Q

    setResetInv( true );                             // Invert Reset Pin
}

BinCounter::~BinCounter(){}

#include "moc_bincounter.cpp"

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

#include "avrcomponent.h"
#include "circuit.h"
#include "utils.h"


LibraryItem* AVRComponent::libraryItem()
{
    return new LibraryItem(
        tr("AVR"),
        tr("Micro"),
        "ic2.png",
        "AVR",
        AVRComponent::construct );
}

Component* AVRComponent::construct( QObject* parent, QString type, QString id )
{ 
    if( m_canCreate ) 
    {
        AVRComponent* avr = new AVRComponent( parent, type,  id );
        if( m_error > 0 )
        {
            Circuit::self()->compList()->removeOne( avr );
            avr->deleteLater();
            avr = 0l;
            m_error = 0;
            m_pSelf = 0l;
            m_canCreate = true;
        }
        return avr;
    }
    MessageBoxNB( tr("Error")
                , tr("Only 1 Mcu allowed\n to be in the Circuit.") );

    return 0l;
}

AVRComponent::AVRComponent( QObject* parent, QString type, QString id )
            : AvrCompBase( parent, type, id )
{
    m_pSelf = this;
    m_processor = AvrProcessor::self();

    initChip();
    if( m_error == 0 )
    {
        setFreq( 16 );
        qDebug() <<"     ..."<<m_id<<"OK\n";
    }
    else
    {
        qDebug() <<"     ..."<<m_id<<"Error!!!\n";
    }
}
AVRComponent::~AVRComponent() { }


#include "moc_avrcomponent.cpp"

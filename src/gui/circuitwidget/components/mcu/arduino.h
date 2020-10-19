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

#ifndef ARDUINO_H
#define ARDUINO_H

#include "avrcompbase.h"
#include "ledsmd.h"
#include "e-source.h"
#include "e-node.h"
#include "e-gate.h"
#include "itemlibrary.h"


class Arduino : public AvrCompBase
{
    Q_OBJECT

    public:
        Arduino( QObject* parent, QString type, QString id );
        ~Arduino();

 static Component* construct( QObject* parent, QString type, QString id );
 static LibraryItem* libraryItem();
 
        void attach();

        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );
        
    public slots:
        virtual void remove();

    private:
        void initBoard();

        eNode*   m_groundEnode;
        eNode*   m_bufferEnode;
        LedSmd*  m_boardLed;
        eNode*   m_boardLedEnode;
        eGate*   m_boardLedBuffer;
        Pin*     m_pb5Pin;
};

#endif

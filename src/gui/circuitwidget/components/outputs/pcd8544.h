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
// Based on:

// C++ Interface: pcd8544
//
// Description: This component emulates a graphic LCD module based on the
//              PCD8544 controller.
//
// Author: Roland Elek <elek.roland@gmail.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution


#ifndef PCD8544_H
#define PCD8544_H

#include "component.h"
#include "itemlibrary.h"
#include "e-element.h"
#include "pin.h"

class MAINMODULE_EXPORT Pcd8544 : public Component, public eElement
{
    Q_OBJECT
    
    public:
        Pcd8544( QObject* parent, QString type, QString id );
        ~Pcd8544();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        void stamp();
        void resetState();
        void setVChanged();
        
        void updateStep();
        
        void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void remove();
        
    protected:
        void initPins();

        virtual void clearLcd();

        void incrementPointer();

        void reset();
        
        void clearDDRAM();
        
        QImage *m_pdisplayImg;        //Visual representation of the LCD

        unsigned char m_aDispRam[6][84];                   //84x48 DDRAM

        //Controller state
        bool m_bPD;
        bool m_bV;
        bool m_bH;
        bool m_bD;
        bool m_bE;
        bool m_lastScl;
        int m_addrX;                                    // X RAM address
        int m_addrY;                                    // Y RAM address
        int m_inBit;        //How many bits have we read since last byte
        unsigned char m_cinBuf;     //Buffer where we keep incoming bits

        //Inputs
        Pin m_pRst;
        Pin m_pCs;
        Pin m_pDc;
        Pin m_pSi;
        Pin m_pScl;
};

#endif


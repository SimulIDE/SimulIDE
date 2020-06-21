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

#ifndef KS0108_H
#define KS0108_H

#include "component.h"
#include "itemlibrary.h"
#include "e-source.h"
#include "pin.h"

class MAINMODULE_EXPORT Ks0108 : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( bool CS_Active_Low   READ csActLow    WRITE setCsActLow   DESIGNABLE true USER true )
    
    public:
        Ks0108( QObject* parent, QString type, QString id );
        ~Ks0108();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        void stamp();
        void resetState();
        
        void setVChanged();
        
        void updateStep();
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void remove();
        void setCsActLow( bool low ) { m_csActLow = low; }
        bool csActLow() { return m_csActLow; }
        
    protected:
        void initPins();
        
        void writeData( int data );
        void proccessCommand( int command );
        void ReadData();
        void ReadStatus();
        void dispOn( int state );
        void setYaddr( int addr );
        void setXaddr( int addr );
        void startLin( int line );
        void clearLcd();

        void incrementPointer();

        void reset();
        
        void clearDDRAM();
        
        QImage *m_pdisplayImg;        //Visual representation of the LCD

        unsigned char m_aDispRam[8][128];                 //128x64 DDRAM

        
        int m_input;
        int m_addrX1;                                   // X RAM address
        int m_addrY1;                                   // Y RAM address
        int m_addrX2;                                   // X RAM address
        int m_addrY2;                                   // Y RAM address
        int m_startLin;
        
        bool m_csActLow;
        bool m_Cs1;
        bool m_Cs2;
        bool m_dispOn;
        bool m_lastScl;
        bool m_reset;
        bool m_Write;

        //Inputs
        Pin m_pinRst;
        Pin m_pinCs2;
        Pin m_pinCs1;
        Pin m_pinEn;
        Pin m_pinRW;
        Pin m_pinDC;
        
        std::vector<Pin*> m_dataPin;
        std::vector<eSource*> m_dataeSource;
};

#endif


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

#ifndef HD44780_H
#define HD44780_H

#include "itemlibrary.h"
#include "component.h"
#include "e-element.h"
#include "pin.h"

class MAINMODULE_EXPORT Hd44780 : public Component, public eElement
{
    Q_OBJECT
    Q_PROPERTY( int Cols   READ cols    WRITE setCols   DESIGNABLE true USER true )
    Q_PROPERTY( int Rows   READ rows    WRITE setRows   DESIGNABLE true USER true )
    
    public:
        Hd44780( QObject* parent, QString type, QString id );
        ~Hd44780();
        
        static Component* construct( QObject* parent, QString type, QString id );
        static LibraryItem* libraryItem();
        
        int cols();
        void setCols( int cols );
        
        int rows();
        void setRows( int rows );
        
        void stamp();
        void resetState();
        void setVChanged();
        void updateStep();
        void showPins( bool show );
        
        ePin* getEpin( QString pinName );
        
        virtual void paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget );

    public slots:
        void remove();
        
    private:
        void clearDDRAM();
        void clearLcd();
        void writeData( int data );
        void proccessCommand( int command );
        void functionSet( int data );
        void C_D_Shift( int data );
        void dispControl( int data );
        void entryMode( int data );
        void cursorHome();
        void setDDaddr( int addr );
        void setCGaddr( int addr );
        
        QImage m_fontImg;                  //Characters image

        int m_DDram[80];                   //80 DDRAM
        int m_CGram[64];                   //64 CGRAM
        
        int m_rows;
        int m_cols;
        //int m_lineWidth;
        int m_imgWidth;
        int m_imgHeight;
        
        int m_cursPos;
        int m_shiftPos;
        int m_direction;
        int m_shiftDisp;
        int m_dispOn;
        int m_cursorOn;
        int m_cursorBlink;
        int m_dataLength;
        //int m_dispLines;
        int m_lineLength;
        int m_DDaddr;
        int m_CGaddr;
        int m_nibble;
        int m_input;
        
        int m_blinkStep;
        
        bool m_lastClock;
        bool m_writeDDRAM;

        //Inputs
        Pin* m_pinRS;
        Pin* m_pinRW;
        Pin* m_pinEn;
        std::vector<Pin*> m_dataPin;
};

#endif


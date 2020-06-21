/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "ssd1306.h"

static const char* Ssd1306_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Color")
};


Component* Ssd1306::construct( QObject* parent, QString type, QString id )
{
    return new Ssd1306( parent, type, id );
}

LibraryItem* Ssd1306::libraryItem()
{
    return new LibraryItem(
        tr( "Ssd1306" ),
        tr( "Outputs" ),
        "ssd1306.png",
        "Ssd1306",
        Ssd1306::construct );
}

Ssd1306::Ssd1306( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eI2C( id.toStdString() )
       , m_pinSck( 270, QPoint(-48, 48), id+"-PinSck" , 0, this )
       , m_pinSda( 270, QPoint(-40, 48), id+"-PinSda" , 0, this )
       //, m_pinRst( 270, QPoint(-32, 48), id+"-PinRst" , 0, this )
       //, m_pinDC ( 270, QPoint(-24, 48), id+"-PinDC"  , 0, this )
       //, m_pinCS ( 270, QPoint(-16, 48), id+"-PinCS"  , 0, this )
{
    Q_UNUSED( Ssd1306_properties );
    
    m_area = QRectF( -70, -48, 140, 92 );

    m_address = 0b00111100; // 60
    
    m_pinSck.setLabelText( " SCK" );
    m_pinSda.setLabelText( " SDA" );
    //m_pinRst.setLabelText( " Res" );
    //m_pinDC.setLabelText(  " DC" );
    //m_pinCS.setLabelText(  " CS" );

    eLogicDevice::createInput( &m_pinSda );                // Input SDA
    eLogicDevice::createClockPin( &m_pinSck );             // Input SCL

    m_pdisplayImg = new QImage( 128, 64, QImage::Format_MonoLSB );
    m_pdisplayImg->setColor( 0, qRgb(0, 0, 0));
    m_pdisplayImg->setColor( 1, qRgb(255, 255, 255) );
    m_dColor = White;
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos( -32,-60, 0);
    setShowId( true );
    
    resetState();
}

Ssd1306::~Ssd1306(){}

/*void Ssd1306::stamp()
{
    eI2C::stamp();

    eNode* enode = m_pinCS.getEnode();// Register for CS changes callback
    if( enode ) enode->addToChangedFast(this); 
    
    enode = m_pinRst.getEnode();       // Register for Rst changes callback
    if( enode ) enode->addToChangedFast(this); 
}*/

void Ssd1306::resetState()
{
    eI2C::resetState();

    m_continue = false;
    m_command = false;
    m_data = false;
    m_addrMode = 0;

    clearDDRAM();
    clearLcd();
    reset() ;
    updateStep();
}

/*void Ssd1306::setVChanged()                 // Called when En Pin changes
{
    if( m_pinRst.getVolt()<2.5 ) reset();            // Reset Pin is Low
    else                         m_reset = false;

    bool sa0 = (m_pinDC.getVolt()>2.5);

    m_address = 0b00111100; // 60
    if( sa0 ) m_address++;

    eI2C::setVChanged();

    //m_input = 0;
}*/

void Ssd1306::slaveStop()
{
    eI2C::slaveStop();

    m_command = false;
    m_data    = false;
    m_continue = false;
    //m_readBytes = 0;
}

void Ssd1306::readByte()
{
    eI2C::readByte();

    //if( m_reset ) return;

    //if( !m_command && ((m_rxReg & 0b00111111) == 0) )   // Control Byte
    //if( m_byte == 1 ) // First Byte is Control Byte
    if( !m_command && !m_data )
    {
        if( (m_rxReg & 0b00111111) == 0 ) // Control Byte
        {
            int co = m_rxReg & 0b10000000;
            if( co == 0 ) m_continue = true;
            else          m_continue = false;

            int cd = m_rxReg & 0b01111111;
            if( cd == 0 ) m_command = true;// 0 Command Byte
            else          m_data    = true;// 64 Data Byte
        }
        //else            m_command = true;// Command Byte
        //m_readBytes = 0;
    }
    else                                // Data Byte
    {
        if     ( m_command ) proccessCommand();
        else if( m_data )    writeData();

        if( !m_continue )
        {
            m_command = false;
            m_data    = false;
        }
    }
}

void Ssd1306::writeData()
{
    //qDebug() << "Ssd1306::writeData"<<m_addrX1 <<m_addrY1<<data;

    m_aDispRam[m_addrX][m_addrY] = m_rxReg;

    incrementPointer();
}

void Ssd1306::proccessCommand()
{
    if( m_readBytes > 0 )
    {
        if( m_lastCommand == 0x20 ) m_addrMode = m_rxReg;
        else if( m_lastCommand == 0x21 ) // 21 33 Set Column Address (Start-End)
        {
            if( m_readBytes == 2 ) m_startX = m_rxReg & 0x7F; // 0b01111111
            else                   m_endX   = m_rxReg & 0x7F; // 0b01111111
        }
        else if( m_lastCommand == 0x22 ) // 22 34 Set Page Address (Start-End)
        {
            if( m_readBytes == 2 ) m_startY = m_rxReg & 0x07; // 0b00000111
            else                   m_endY   = m_rxReg & 0x07; // 0b00000111
        }
        else if( m_lastCommand == 0x26   // 26 36 Continuous Horizontal Scroll Setup
              || m_lastCommand == 0x27
              || m_lastCommand == 0x29   // 29-2A 38 39 Continuous Vertical and Horizontal Scroll Setup
              || m_lastCommand == 0x2A )
        {
            int byte = 6-m_readBytes;
            int value = m_rxReg & 0x07; // 0b00000111

            m_scrollV = false;

            if     ( m_lastCommand == 0x26 ) m_scrollR = true;
            else if( m_lastCommand == 0x27 ) m_scrollR = false;
            else if( m_lastCommand == 0x29 )
            {
                byte = 5-m_readBytes;
                m_scrollV = true;
                m_scrollR = true;
            }
            else if( m_lastCommand == 0x2A )
            {
                byte = 5-m_readBytes;
                m_scrollV = true;
                m_scrollR = false;
            }

            if     ( byte == 1 ) m_scrollStartPage =  value;
            else if( byte == 2 )
            {
                if     ( value == 0 ) m_scrollInterval = 5;
                else if( value == 1 ) m_scrollInterval = 64;
                else if( value == 2 ) m_scrollInterval = 128;
                else if( value == 3 ) m_scrollInterval = 256;
                else if( value == 4 ) m_scrollInterval = 3;
                else if( value == 5 ) m_scrollInterval = 4;
                else if( value == 6 ) m_scrollInterval = 25;
                else if( value == 7 ) m_scrollInterval = 2;
            }
            else if( byte == 3 ) m_scrollEndPage =  value;
            else if( byte == 4 )
            {
                m_scrollVertOffset = m_rxReg & 0x3F; // 0b00111111
            }
            qDebug() << m_lastCommand << byte << m_scrollR << m_scrollV << m_scrollInterval << m_scrollStartPage << m_scrollEndPage<<m_scrollVertOffset;
        }
        else if( m_lastCommand == 0xA3 ) // A3 163 Set Vertical Scroll Area
        {
            ;
        }

        m_readBytes--;
        return;
    }
    m_lastCommand = m_rxReg;

    if( m_rxReg < 0x10 ) // 00-0F 0-15 Set Lower Colum Start Address for Page Addresing mode
    {
        m_addrX = (m_addrX & ~0xF) | (m_rxReg & 0xF);
    }
    else if( m_rxReg < 0x20 ) // 10-1F 16-31 Set Higher Colum Start Address for Page Addresing mode
    {
        m_addrX = (m_addrX & 0xF) | ((m_rxReg & 0xF) << 4);
    }
    else if( m_rxReg == 0x20 ) m_readBytes = 1; // 20 32 Set Memory Addressing Mode
    else if( m_rxReg == 0x21 ) m_readBytes = 2; // 21 33 Set Column Address (Start-End)
    else if( m_rxReg == 0x22 ) m_readBytes = 2; // 22 34 Set Page Address (Start-End)

    else if( m_rxReg == 0x26 ) m_readBytes = 6; // 26 36 Continuous Horizontal Right Scroll Setup
    else if( m_rxReg == 0x27 ) m_readBytes = 6; // 27 37 Continuous Horizontal Left Scroll Setup
    else if( m_rxReg == 0x29 ) m_readBytes = 5; // 29-2A 38 39 Continuous Vertical and Horizontal Scroll Setup
    else if( m_rxReg == 0x2A ) m_readBytes = 5;

    else if( m_rxReg == 0x2E ) m_scroll = false; // 0b00101110 // 0x2E 46    Deactivate scroll
    else if( m_rxReg == 0x2F )
    {
        m_scroll = true;  // 0b00101111 // 0x2F 47    Activate scroll
        m_scrollCount = m_scrollInterval/5;
        qDebug() << "Activate Scroll" << m_scrollCount<<"\n";
    }

    else if( (m_rxReg>=0x40) && (m_rxReg<=0x7F) ) // 0b01xxxxxx 40-7F 64-127 Set Display Start Line
    {
        m_startLin = m_rxReg & 0x3F; // 0b00111111
    }

    else if( m_rxReg == 0x81 ) m_readBytes = 1; // 81 129 Set Contrast Control

    else if( m_rxReg == 0x8D ) m_readBytes = 1; // 8D 141 Charge Pump

    // A0-A1 160-161 Set Segment Re-map

    else if( m_rxReg == 0xA3 ) m_readBytes = 2;     // A3 163 Set Vertical Scroll Area
    else if( m_rxReg == 0xA4 ) m_dispFull = false;  // A4-A5 164-165 Entire Display ON
    else if( m_rxReg == 0xA5 ) m_dispFull = true;
    else if( m_rxReg == 0xA6 ) m_dispInv = false;   // A6-A7 166-167 Set Normal/inverse Display
    else if( m_rxReg == 0xA7 ) m_dispInv = true;
    else if( m_rxReg == 0xA8 ) m_readBytes = 1;     // A8 168 Set Multiplex Ratio

    else if( m_rxReg == 0xAE ) reset();             // 174 // AE-AF Set Display ON/OFF
    else if( m_rxReg == 0xAF ) m_dispOn = true;     // 175

    else if( (m_rxReg>=0xB0) && (m_rxReg<=0xB7) )   // B0-B7 176-183 Set Page Start Address for Page Addresing mode
    {
        m_addrY = m_rxReg & 0x07; // 0b00000111
    }

    // C0-C8 192-200 Set COM Output Scan Direction

    else if( m_rxReg == 0xD3 ) m_readBytes = 1; // D3 211 Set Display Offset

    else if( m_rxReg == 0xD5 ) m_readBytes = 1; // D5 213 Set Display Clock Divide Ratio/Oscillator Frequency

    else if( m_rxReg == 0xD9 ) m_readBytes = 1; // D9 217 Set Precharge
    else if( m_rxReg == 0xDA ) m_readBytes = 1; // DA 218 Set COM Pins Hardware Configuration
    else if( m_rxReg == 0xDB ) m_readBytes = 1; // DB 219 SET VCOM DETECT


    //qDebug() << "Ssd1306::proccessCommand: " << command;
}

void Ssd1306::clearLcd() 
{
    m_pdisplayImg->fill(0);
}

void Ssd1306::clearDDRAM() 
{
    for(int row=0;row<8;row++) 
        for( int col=0;col<128;col++ ) 
            m_aDispRam[col][row] = 0;
}

void Ssd1306::incrementPointer() 
{
    if( m_addrMode == VERT_ADDR_MODE )
    {
        m_addrY++;
        if( m_addrY > m_endY )
        {
            m_addrY = m_startY;
            m_addrX++;
        }
        if( m_addrX > m_endX )
        {
            m_addrX = m_startX;
        }
    }
    else
    {
        m_addrX++;
        if( m_addrX > m_endX )
        {
            m_addrX = m_startX;
            if( m_addrMode == HORI_ADDR_MODE ) m_addrY++;
        }
        if( m_addrMode == HORI_ADDR_MODE )
        {
            if( m_addrY > m_endY )
            {
                m_addrY = m_startY;
            }
        }
    }
}

void Ssd1306::reset() 
{
    m_cdr = 1;
    m_mr  = 63;
    m_fosc = 370000;
    m_frm = m_fosc/(m_cdr*54*m_mr);

    m_addrX  = 0;
    m_addrY  = 0;
    m_startX = 0;
    m_endX   = 127;
    m_startY = 0;
    m_endY   = 7;

    m_scrollStartPage  = 0;
    m_scrollEndPage    = 7;
    m_scrollInterval   = 5;
    m_scrollVertOffset = 0;

    m_startLin = 0;
    m_readBytes = 0;

    m_dispOn   = false;
    m_dispFull = false;
    m_dispInv  = false;

    m_scroll  = false;
    m_scrollR = false;
    m_scrollV = false;

    //m_reset = true;

    clearLcd();
}

void Ssd1306::remove()
{
    if( m_pinSck.isConnected()) m_pinSck.connector()->remove();
    if( m_pinSda.isConnected()) m_pinSda.connector()->remove();
    //if( m_pinRst.isConnected()) m_pinRst.connector()->remove();
    //if( m_pinDC.isConnected() ) m_pinDC.connector()->remove();
    //if( m_pinCS.isConnected() ) m_pinCS.connector()->remove();
    
    delete m_pdisplayImg;
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

void Ssd1306::updateStep()
{
    if( !m_dispOn ) m_pdisplayImg->fill(0);               // Display Off
    else
    {
        if( m_scroll )
        {
            m_scrollCount--;
            if( m_scrollCount <= 0 )
            {
                m_scrollCount = m_scrollInterval;

                for( int row=m_scrollStartPage; row<=m_scrollEndPage; row++ )
                {
                    unsigned char start = m_aDispRam[0][row];
                    unsigned char end   = m_aDispRam[127][row];

                    for( int col=0; col<128; col++ )
                    {

                        if( m_scrollR )
                        {
                            int c = 127-col;

                            if( c < 127 ) m_aDispRam[c][row] = m_aDispRam[c-1][row];
                            if( col == 0 )  m_aDispRam[0][row]   = end;
                        }
                        else
                        {
                            if( col < 127 )  m_aDispRam[col][row] = m_aDispRam[col+1][row];
                            if( col == 127 ) m_aDispRam[col][row] = start;
                        }
                    }

                }
            }

        }
        for( int row=0; row<8; row++ )
        {
            for( int col=0; col<128; col++ )
            {
                unsigned char abyte;
                if( m_dispFull ) abyte = 255;           // Display fully On
                else abyte = m_aDispRam[col][row];

                if( m_dispInv ) abyte = ~abyte;         // Display Inverted

                for( int bit=0; bit<8; bit++ ) 
                {
                    m_pdisplayImg->setPixel(col,row*8+bit,(abyte & 1) );
                    abyte >>= 1;
                }
            }
        }
    }
    update();
}

void Ssd1306::setColor( dispColor color )
{
    m_dColor = color;

    if( m_dColor == White )  m_pdisplayImg->setColor( 1, qRgb(245, 245, 245) );
    if( m_dColor == Blue  )  m_pdisplayImg->setColor( 1, qRgb(200, 200, 255) );
    if( m_dColor == Yellow ) m_pdisplayImg->setColor( 1, qRgb(245, 245, 100) );
}

void Ssd1306::paint( QPainter* p, const QStyleOptionGraphicsItem* option, QWidget* widget )
{
    QPen pen( Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );
    p->drawImage(-64,-42,*m_pdisplayImg );
}

#include "moc_ssd1306.cpp"

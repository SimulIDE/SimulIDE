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


#include "connector.h"
#include "simulator.h"
#include "hd44780.h"
#include "utils.h"

static const char* Hd44780_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","Cols"),
    QT_TRANSLATE_NOOP("App::Property","Rows")
};

Component* Hd44780::construct( QObject* parent, QString type, QString id )
{
    return new Hd44780( parent, type, id );
}

LibraryItem* Hd44780::libraryItem()
{
    return new LibraryItem(
        tr( "Hd44780" ),
        tr( "Outputs" ),
        "hd44780.png",
        "Hd44780",
        Hd44780::construct );
}

Hd44780::Hd44780( QObject* parent, QString type, QString id )
       : Component( parent, type, id )
       , eElement( (id+"-eElement").toStdString() )
       , m_fontImg(":font2.png")
{
    Q_UNUSED( Hd44780_properties );
    
    m_rows = 2;
    m_cols = 16;
    
    int pinY = 8;//8*((33+m_imgHeight)/8);

    m_pinRS = new Pin( 270, QPoint(16, pinY), id+"-PinRS", 0, this );
    m_pinRW = new Pin( 270, QPoint(24, pinY), id+"-PinRW", 0, this );
    m_pinEn = new Pin( 270, QPoint(32, pinY), id+"-PinEn", 0, this );
    m_pinRS->setLabelText( " RS" );
    m_pinRW->setLabelText( " RW" );
    m_pinEn->setLabelText( " En" );
    
    m_dataPin.resize( 8 );
    
    for( int i=0; i<8; i++ )
    {
        m_dataPin[i] = new Pin( 270, QPoint( 40+i*8, pinY), id+"-dataPin"+QString::number(i) , 0, this );
        m_dataPin[i]->setLabelText( " D"+QString::number(i) );
    }
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos( 70,-82, 0);
    setShowId( true );
    
    resetState();
}

Hd44780::~Hd44780()
{
}

void Hd44780::stamp()
{
    eNode* enode = m_pinEn->getEnode();// Register for clk changes callback
    if( enode ) enode->addToChangedFast(this);
}

void Hd44780::resetState()
{
    //qDebug() << "Hd44780::resetState()" ;
    m_lastClock = false;
    m_writeDDRAM = true;
    m_cursPos     = 0;
    m_shiftPos    = 0;
    m_direction   = 1;
    m_shiftDisp   = 0;
    m_dispOn      = 0;
    m_cursorOn    = 0;
    m_cursorBlink = 0;
    m_dataLength  = 8;
    m_lineLength  = 80;
    m_nibble      = 0;
    m_input = 0;
    
    m_DDaddr = 0;
    m_CGaddr = 0;
    
    m_imgWidth  = (m_cols*6-1)*2;
    m_imgHeight = (m_rows*9-1)*2;
    m_area = QRectF( 0, -(m_imgHeight+33), m_imgWidth+20, m_imgHeight+33 );
    setTransformOriginPoint( togrid( m_area.center() ));
    
    clearLcd();
}
void Hd44780::setVChanged()             // Called when clock Pin changes 
{
    if( m_pinEn->getVolt()>2.5 )                      // Clk Pin is High
    {
        m_lastClock = true;
        return; 
    }
    else                                               // Clk Pin is Low
    {
        if( m_lastClock == false ) return;         // Not a Falling edge
        m_lastClock = false;
    }
                                   // We Had  a Falling Edge: Read input
    if( m_dataLength == 8 )                                // 8 bit mode
    {
        m_input = 0;
        
        for( int pin=0; pin<8; pin++ )
            if( m_dataPin[pin]->getVolt()>2.5 )
                m_input += pow( 2, pin );
    }
    else                                                   // 4 bit mode
    {
        if( m_nibble == 0 )                          // Read high nibble
        {
            m_input = 0;
            
            for( int pin=4; pin<8; pin++ )
                if( m_dataPin[pin]->getVolt()>2.5 )
                    m_input += pow( 2, pin );
                    
            m_nibble = 1;
            return;
        }
        else                                          // Read low nibble
        {
            for( int pin=4; pin<8; pin++ )
                if( m_dataPin[pin]->getVolt()>2.5 )
                    m_input += pow( 2, (pin-4) );
                    
            m_nibble = 0;
        }
    }
    
    //Get RS state: data or command
    if( m_pinRS->getVolt()>2.5 ) writeData( m_input );
    else                         proccessCommand( m_input );
}

void Hd44780::writeData( int data )
{
    if( m_writeDDRAM )                                 // Write to DDRAM
    {
        //qDebug() << "Hd44780::writeData: " << data << m_cursPos<<m_DDaddr;
        m_DDram[m_DDaddr] = data;
        m_DDaddr += m_direction;
        
        if( m_DDaddr > 79 ) m_DDaddr = 0;
        if( m_DDaddr < 0 )  m_DDaddr = 79;
        
        if( m_shiftDisp )
        {
            m_shiftPos += m_direction;
            
            int lineEnd = m_lineLength-1;

            if( m_shiftPos>lineEnd ) m_shiftPos = 0;
            if( m_shiftPos<0 )       m_shiftPos = lineEnd;
        }
    }
    else                                               // Write to CGRAM
    {
        m_CGram[m_CGaddr] = data;
        m_CGaddr += 1;
        
        if( m_CGaddr > 63 ) m_CGaddr = 0;
    }
}

void Hd44780::proccessCommand( int command )
{
    //qDebug() << "Hd44780::proccessCommand: " << command;
    if( command<2 )   { clearLcd();               return; } //00000001 //Clear display           //Clears display and returns cursor to the home position (address 0).//1.52 ms
    if( command<4 )   { cursorHome();             return; } //0000001. //Cursor home             //Returns cursor to home position. Also returns display being shifted to the original position. DDRAM content remains unchanged.//1.52 ms
    if( command<8 )   { entryMode( command );     return; } //000001.. //Entry mode set          //Sets cursor move direction (I/D); specifies to shift the display (S). These operations are performed during data read/write.//37 μs
    if( command<16 )  { dispControl( command );   return; } //00001... //Display on/off          //Sets on/off of all display (D), cursor on/off (C), and blink of cursor position character (B).//37 μs
    if( command<32 )  { C_D_Shift( command );     return; } //0001.... //Cursor/display shift    //Sets cursor-move or display-shift (S/C), shift direction (R/L). DDRAM content remains unchanged//37 μs
    if( command<64 )  { functionSet( command );   return; } //001..... //Function set            //Sets interface data length (DL), number of display line (N), and character font (F)//37 μs
    if( command<128 ) { setCGaddr( command-64 );  return; } //01...... //Set CGRAM address       //Sets the CGRAM address. CGRAM data are sent and received after this setting//37 μs
    else              { setDDaddr( command-128 ); return; } //1....... //Set DDRAM address       //Sets the DDRAM address. DDRAM data are sent and received after this setting.//37 μs
}

void Hd44780::functionSet( int data ) 
{
    if( data & 16 ) m_dataLength = 8;    // Data Length
    else            m_dataLength = 4;
    
    if( data & 8 ) m_lineLength = 40;    // Display Lines 
    else           m_lineLength = 80;
    
    // Sets the character font.
    //if( data & 4 ) ;
    //else            ;
    //qDebug()<<m_dataLength<<m_lineLength<<(data & 16);
}
  
void Hd44780::C_D_Shift( int data )
{
    int dir = -1;    // Move Right/Left
    if( data & 4 ) dir = 1;

    if( data & 8 )     // Shift Cursor/Display
    {
        m_shiftPos += dir;
        int lineEnd = m_lineLength-1;

        if( m_shiftPos>lineEnd ) m_shiftPos = 0;
        if( m_shiftPos<0 )       m_shiftPos = lineEnd;
    }
    else           m_cursPos  += dir;
}

void Hd44780::dispControl( int data ) 
{
    
    if( data & 4 ) m_dispOn = 1;                       // Display On/Off
    else           m_dispOn = 0;
    
    if( data & 2 ) m_cursorOn = 1;                      // Cursor On/Off
    else           m_cursorOn = 0;
    
    if( data & 1 ) m_cursorBlink = 1;                    // Cursor Blink
    else           m_cursorBlink = 0;
    //qDebug()<<m_cursorOn;
}

void Hd44780::entryMode( int data )
{
    // Cursor move left/right
    if( data & 2 ) m_direction = 1;
    else           m_direction = -1;
    
    // Shift Display
    if( data & 1 ) m_shiftDisp = 1;
    else           m_shiftDisp = 0;
}

void Hd44780::clearLcd() 
{
    clearDDRAM();
    cursorHome();
}

void Hd44780::cursorHome()
{
    m_DDaddr   = 0;
    m_cursPos  = 0;
    m_shiftPos = 0;
}

void Hd44780::setDDaddr( int addr )
{
    if( (m_lineLength==40) & (addr>63) ) addr -= 24;
    m_DDaddr = addr & 0b01111111;
    
    m_writeDDRAM = true;
    //qDebug() << "Hd44780::setDDaddr: "<< addr << m_DDaddr;
}

void Hd44780::setCGaddr( int addr )
{
    m_CGaddr = addr & 0b00111111;
    
    m_writeDDRAM = false;
    //qDebug() << "set_CGaddr: " << m_CGaddr;
}

void Hd44780::clearDDRAM() 
{
    for(int i=0; i<80; i++) m_DDram[i] = 32;
}

int Hd44780::cols()
{
    return m_cols;
}

void Hd44780::setCols( int cols )
{
    if( cols > 20 ) cols = 20;
    if( cols < 8 ) cols = 8;
    
    m_cols = cols;
    
    resetState();
}

int Hd44780::rows()
{
    return m_rows;
}
void Hd44780::setRows( int rows )
{
    if( rows > 4 ) rows = 4;
    if( rows < 1 ) rows = 1;
    
    m_rows = rows;
    
    resetState();
}

void Hd44780::updateStep()
{
    update();
}

void Hd44780::remove()
{
    if( m_pinRS->isConnected() ) m_pinRS->connector()->remove();
    if( m_pinRW->isConnected() ) m_pinRW->connector()->remove();
    if( m_pinEn->isConnected() ) m_pinEn->connector()->remove();
    
    for( int i=0; i<8; i++ )
    {
        if( m_dataPin[i]->isConnected() ) m_dataPin[i]->connector()->remove();
    }
    
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

ePin* Hd44780::getEpin( QString pinName )
{
    if     ( pinName == "RS" ) return m_pinRS;
    else if( pinName == "RW" ) return m_pinRW;
    else if( pinName == "En" ) return m_pinEn;
    else if( pinName.contains( "D" ) )
    {
        int pin = pinName.remove("D").toInt();
        return m_dataPin[pin];
    }

    return 0l;
}

void Hd44780::showPins( bool show )
{
    m_pinRS->setVisible( show );
    m_pinRW->setVisible( show );
    m_pinEn->setVisible( show );
    
    for( int i=0; i<8; i++ ) m_dataPin[i]->setVisible( show );
}

void Hd44780::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area, 2, 2 );
    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( 4, -(29+m_imgHeight), m_imgWidth+12, m_imgHeight+12, 8, 8 );

    if( m_dispOn == 0 ) return;
    
    for( int row=0; row<m_rows; row++ )
    {
        for( int col=0; col<m_cols; col++ )
        {
            int mem_pos = 0;
            if( row < 2 ) mem_pos += row*40+col;
            else          mem_pos += (row-2)*40+20+col;
            
            int lineEnd = 79;
            int lineStart = 0;
            
            if( m_lineLength == 40 ) 
            {
                if( mem_pos < 40 ) lineEnd   = 39;
                else               lineStart = 40;
            }
            
            mem_pos += m_shiftPos;
            if( mem_pos>lineEnd )   mem_pos -= m_lineLength;
            if( mem_pos<lineStart ) mem_pos += m_lineLength;
            
            //qDebug() << row << col << mem_pos;
            int char_num = m_DDram[mem_pos];
            QImage charact = m_fontImg.copy(char_num*10, 0, 10, 14);
            
            if( char_num < 8 )                        // CGRam Character
            {
                int addr = char_num*8;
                
                for( int y=0; y<14; y+=2 )
                {
                    int data = m_CGram[ addr ];
                    addr++;
                    
                    for( int x=9; x>0; x-=2 )
                    {
                        if( data & 1 )
                        {
                            charact.setPixel(x,   y,   qRgb(0, 0, 0));
                            charact.setPixel(x-1, y,   qRgb(0, 0, 0));
                            charact.setPixel(x,   y+1, qRgb(0, 0, 0));
                            charact.setPixel(x-1, y+1, qRgb(0, 0, 0));
                        }
                        data = data>>1;
                    }
                }
            }
            p->drawImage(10+col*12,-(m_imgHeight+22)+row*18,charact );
            
            if( (mem_pos == m_DDaddr) & m_cursorOn )      // Draw cursor
            {
                if( m_cursorBlink ) m_blinkStep++;
                else                m_blinkStep = 0;
                
                if( m_blinkStep < 20 )//m_cursorBlink
                {
                    charact = m_fontImg.copy(95*10, 0, 10, 14);
                    p->drawImage(10+col*12,-(m_imgHeight+22)+row*18,charact );
                }
                if( m_blinkStep == 40 ) m_blinkStep = 0;
            }
        }
    }
}


#include "moc_hd44780.cpp"


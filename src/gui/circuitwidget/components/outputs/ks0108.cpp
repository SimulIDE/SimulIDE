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

#include "itemlibrary.h"
#include "connector.h"
#include "simulator.h"
#include "ks0108.h"

static const char* Ks0108_properties[] = {
    QT_TRANSLATE_NOOP("App::Property","CS Active Low")
};


Component* Ks0108::construct( QObject* parent, QString type, QString id )
{
    return new Ks0108( parent, type, id );
}

LibraryItem* Ks0108::libraryItem()
{
    return new LibraryItem(
        tr( "Ks0108" ),
        tr( "Outputs" ),
        "ks0108.png",
        "Ks0108",
        Ks0108::construct );
}

Ks0108::Ks0108( QObject* parent, QString type, QString id )
      : Component( parent, type, id )
      , eElement( (id+"-eElement").toStdString() )
      , m_pinRst( 270, QPoint(-56, 56), id+"-PinRst" , 0, this )
      , m_pinCs2( 270, QPoint(-48, 56), id+"-PinCs2" , 0, this )
      , m_pinCs1( 270, QPoint(-40, 56), id+"-PinCs1" , 0, this )
      , m_pinEn ( 270, QPoint( 32, 56), id+"-PinEn"  , 0, this )
      , m_pinRW ( 270, QPoint( 40, 56), id+"-PinRW"  , 0, this )
      , m_pinDC ( 270, QPoint( 48, 56), id+"-PinDC"  , 0, this )
{
    Q_UNUSED( Ks0108_properties );
    
    m_area = QRectF( -74, -52, 148, 100 );
    m_csActLow = false;
    
    m_pinRst.setLabelText( " RST" );
    m_pinCs1.setLabelText( " CS1" );
    m_pinCs2.setLabelText( " CS2" );
    m_pinDC.setLabelText(  " RS" );
    m_pinRW.setLabelText(  " RW" );
    m_pinEn.setLabelText(  " En" );
    
    m_dataPin.resize( 8 );
    m_dataeSource.resize( 8 );
    
    int pinY = 56;
    
    for( int i=0; i<8; i++ )
    {
        QString pinId = id+"-dataPin"+QString::number(i);
        m_dataPin[i] = new Pin( 270, QPoint(-32+(7-i)*8, pinY), pinId , 0, this );
        m_dataPin[i]->setLabelText( " D"+QString::number(i) );

        pinId.append(QString("-eSource"));
        m_dataeSource[i] = new eSource( pinId.toStdString(), m_dataPin[i] );
        m_dataeSource[i]->setVoltHigh( 5 );
        m_dataeSource[i]->setImp( high_imp );
    }
    
    m_pdisplayImg = new QImage( 128, 64, QImage::Format_MonoLSB );
    m_pdisplayImg->setColor( 1, qRgb(0,0,0));
    m_pdisplayImg->setColor( 0, qRgb(200,215,180) );
    
    Simulator::self()->addToUpdateList( this );
    
    setLabelPos( -32,-68, 0);
    setShowId( true );
    
    resetState();
}

Ks0108::~Ks0108(){}

void Ks0108::stamp()
{
    eNode* enode = m_pinEn.getEnode();// Register for Scl changes callback
    if( enode ) enode->addToChangedFast(this); 
    
    enode = m_pinRst.getEnode();       // Register for Rst changes callback
    if( enode ) enode->addToChangedFast(this); 
}

void Ks0108::resetState()
{
    clearDDRAM();
    clearLcd();
    reset() ;
    updateStep();
}

void Ks0108::setVChanged()                 // Called when En Pin changes 
{
    if( m_pinRst.getVolt()<2.5 ) reset();            // Reset Pin is Low
    else                         m_reset = false;
    
    bool Write = ( m_pinRW.getVolt()<2.5 );             // Read or Write
    if( m_Write != Write )               // Set Read or Write Impedances
    {
        m_Write = Write;
        double imped;
        if( Write ) imped = high_imp;               // Data bus as Input
        else        imped = 40;                    // Data bus as Output
        
        for( int i=0; i<8; i++ ) 
        {
            m_dataeSource[i]->setOut( false );
            m_dataeSource[i]->setImp( imped );
        }
    }
    
    bool Scl = (m_pinEn.getVolt()>2.5);
    
    if    ( Scl && !m_lastScl )            // This is a clock Rising Edge
    {
        m_lastScl = true;  
        if( Write ) return;                  // Only Read in Rising Edge
    }
    else if( !Scl && m_lastScl )          // This is a clock Falling edge
    {
        m_lastScl = false;  
        if( !Write ) return;               // Only Write in Falling Edge
    }
    else
    {
        m_lastScl = Scl;
        return;
    }
    m_input = 0;
    if( Write )
    {
        //qDebug()<<"Reading "<<m_input;
        for( int pin=0; pin<8; pin++ )                     // Read input
            if( m_dataPin[pin]->getVolt()>2.5 )
            {
                //qDebug()<<pin<<pow( 2, pin );
                m_input += pow( 2, pin );
            }
            //qDebug()<<"Data =  "<<m_input;
    }
    
    m_Cs1 =  (m_pinCs1.getVolt()>2.5);               // Half 1 selected?
    m_Cs2 =  (m_pinCs2.getVolt()>2.5);               // Half 2 selected?
    if( !m_Cs1 & !m_Cs2 ) m_Cs2 = true;
    
    if( m_csActLow )
    {
        m_Cs1 = !m_Cs1;
        m_Cs2 = !m_Cs2;
    }
    
    if( (m_pinDC.getVolt()>2.5)               )                  // Data
    {
        if( m_reset ) return;            // Can't erite data while Reset
        if( Write ) writeData( m_input );                  // Write Data
        else        ReadData();                             // Read Data
    }
    else                                                      // Command
    {
        if( Write )proccessCommand( m_input );          // Write Command
        else       ReadStatus();                          // Read Status
    }
}

void Ks0108::ReadData()
{
    int data = 0;
    if( m_Cs1 ) data = m_aDispRam[m_addrX1][m_addrY1];
    if( m_Cs2 ) data = m_aDispRam[m_addrX2][m_addrY2+64];
    //qDebug() << "Ks0108::ReadData()" << data;
    for( int i=0; i<8; i++ )
    {
        //qDebug() << "Ks0108::ReadData()" << i<<(data & 1)<<((data & 1)==1);
        m_dataeSource[i]->setOut( ((data & 1)==1) );
        m_dataeSource[i]->stampOutput();
        data >>= 1;
    }
}

void Ks0108::ReadStatus()
{
    for( int i=0; i<8; i++ ) 
    {
        bool out = false;
        
        if     ( i == 4 ) out = m_reset;
        else if( i == 5 ) out = !m_dispOn;
        
        m_dataeSource[i]->setOut( out );
        m_dataeSource[i]->stampOutput();
    }
}

void Ks0108::writeData( int data )
{
    //qDebug() << "Ks0108::writeData "<<data <<m_Cs1<<m_Cs2;
    if( m_Cs1 ) 
    {
        //qDebug() << "Ks0108::writeData 1  "<<m_addrX1 <<m_addrY1<<data;
        m_aDispRam[m_addrX1][m_addrY1]    = data;  // Write Half 1 
    }
    if( m_Cs2 ) 
    {
        //qDebug() << "Ks0108::writeData 2  "<<m_addrX2 <<m_addrY2<<data;
        m_aDispRam[m_addrX2][m_addrY2+64] = data;  // Write Half 2 
    }
    incrementPointer();
}

void Ks0108::proccessCommand( int command )
{
    //qDebug() << "Ks0108::proccessCommand: " << command;
    if( command<62 )  {                           return; }             // Not Valid
    if( command<64 )  { dispOn( command & 1 )   ; return; } //0011111.  // Display On/Off internal status &Ram not affected
    if( command<128 ) { setYaddr( command & 63 ); return; } //01......  // Set Y address
    if( command<184 ) {                           return; }             // Not Valid
    if( command<192 ) { setXaddr( command & 7 );  return; } //10111...  // Set X address     
    else              { startLin( command & 63 ); return; } //11......  // Set Display Start Line
}
void Ks0108::dispOn( int state )
{
    m_dispOn = (state > 0);
}

void Ks0108::setYaddr( int addr )
{
    //qDebug() << "Ks0108::setYaddr "<<addr <<m_Cs1<<m_Cs2;
    if( m_Cs1 ) m_addrY1  = addr ;
    if( m_Cs2 ) m_addrY2  = addr ;
}

void Ks0108::setXaddr( int addr )
{
    //qDebug() << "Ks0108::setXaddr "<<addr <<m_Cs1<<m_Cs2;
    if( m_Cs1 ) m_addrX1  = addr ;
    if( m_Cs2 ) m_addrX2  = addr ;
}

void Ks0108::startLin( int line )
{
    m_startLin = line;
}

void Ks0108::clearLcd() 
{
    m_pdisplayImg->fill(0);
}

void Ks0108::clearDDRAM() 
{
    for(int row=0;row<8;row++) 
        for( int col=0;col<128;col++ ) 
            m_aDispRam[row][col] = 0;
}

void Ks0108::incrementPointer() 
{
    if( m_Cs1 )
    {
        m_addrY1++;
        if( m_addrY1 > 63 )
        {
            m_addrY1 = 0;
            //m_addrX++;
        }
        /*if( m_addrX > 7 ) 
        {
            m_addrX = 0;
        }*/
    }
    if( m_Cs2 )
    {
        m_addrY2++;
        if( m_addrY2 > 63 )
        {
            m_addrY2 = 0;
            //m_addrX++;
        }
        /*if( m_addrX > 7 ) 
        {
            m_addrX = 0;
        }*/
    }
}

void Ks0108::reset() 
{
    m_addrX1  = 0;
    m_addrY1  = 0;
    m_addrX2  = 0;
    m_addrY2  = 0;
    m_startLin = 0;
    m_dispOn = false;
    m_reset = true;
}

void Ks0108::remove()
{
    if( m_pinRst.isConnected()) m_pinRst.connector()->remove();
    if( m_pinCs1.isConnected()) m_pinCs1.connector()->remove();
    if( m_pinCs2.isConnected()) m_pinCs2.connector()->remove();
    if( m_pinDC.isConnected() ) m_pinDC.connector()->remove();
    if( m_pinRW.isConnected() ) m_pinRW.connector()->remove();
    if( m_pinEn.isConnected() ) m_pinEn.connector()->remove();

    for( int i=0; i<8; i++ )
    {
        if( m_dataPin[i]->isConnected() ) m_dataPin[i]->connector()->remove();

        delete m_dataeSource[i];
    }
    
    delete m_pdisplayImg;
    Simulator::self()->remFromUpdateList( this );
    
    Component::remove();
}

void Ks0108::updateStep()
{
    if( !m_dispOn ) m_pdisplayImg->fill(0);               // Display Off
    else
    {
        for(int row=0;row<8;row++) 
        {
            for( int col=0;col<128;col++ ) 
            {
                char abyte = m_aDispRam[row][col];
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

void Ks0108::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget )
{
    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    p->setPen( pen );
    
    p->setBrush( QColor(50, 70, 100) );
    p->drawRoundedRect( m_area,2,2 );
    p->setBrush( QColor(200, 220, 180) );
    p->drawRoundedRect( -70, -48, 140, 76, 8, 8 );
    p->drawImage(-64,-42,*m_pdisplayImg );
}

#include "moc_ks0108.cpp"

/*
  SoftwareI2C.cpp
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  Author:Loovee

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <Arduino.h>

#include "softI2C.h"

void SoftI2C::begin( int Sda, int Scl )
{
    pinSda = Sda;
    pinScl = Scl;

    pinMode(pinScl, OUTPUT);
    pinMode(pinSda, OUTPUT);
    sda_in_out = OUTPUT;
    digitalWrite(pinScl, HIGH);
    digitalWrite(pinSda, HIGH);
}

void SoftI2C::sdaSet(uchar ucDta)
{
    if(sda_in_out != OUTPUT)
    {
        sda_in_out = OUTPUT;
        pinMode(pinSda, OUTPUT);
    }
    digitalWrite(pinSda, ucDta);
}

void SoftI2C::sclSet(uchar ucDta)
{
    digitalWrite(pinScl, ucDta);
}

uchar SoftI2C::getAck(void)
{
    sclSet(LOW); 
    pinMode(pinSda, INPUT);
    sda_in_out = INPUT;
    
    sclSet(HIGH);
    unsigned long timer_t = micros();
    while(1)
    {
        if(!digitalRead(pinSda)) return GETACK;
        
        if(micros() - timer_t > 100)return GETNAK;
    }
}

void SoftI2C::sendStart( void )
{
    sdaSet(LOW);
}

void SoftI2C::sendStop( void )
{
    sclSet(LOW);
    sdaSet(LOW);
    sclSet(HIGH);
    sdaSet(HIGH);   
}

void SoftI2C::sendByte( uchar ucDta )
{
    for( int i=0; i<8; i++ )
    {
        sclSet(LOW);
        sdaSet((ucDta&0x80)!=0);
        ucDta <<= 0;
        sclSet(HIGH);
        sdaSet((ucDta&0x80)!=0);
        ucDta <<= 1;
    }
}

uchar SoftI2C::sendByteAck(uchar ucDta)
{
    sendByte(ucDta);
    return getAck();
}

uchar SoftI2C::beginTransmission(uchar addr)
{
    sendStart();                            // start signal
    uchar ret = sendByteAck(addr<<1);       // send write address and get ack
    //sclSet(LOW);
    return ret;
}

uchar SoftI2C::endTransmission()
{
    sendStop();   
    return 0;
}

uchar SoftI2C::write(uchar dta)
{
    return sendByteAck(dta);
}

uchar SoftI2C::write(uchar len, uchar *dta)
{
    for( int i=0; i<len; i++ )
    {
        if(GETACK != write(dta[i])) return GETNAK;
    }
    return GETACK;
}

uchar SoftI2C::requestFrom(uchar addr, uchar len)
{
    sendStart();                       // start signal
    recv_len = len;
    uchar ret = sendByteAck((addr<<1)+1);       // send write address and get ack
    //sclSet(LOW);
    return ret;
}

uchar SoftI2C::read()
{
    if( !recv_len ) return 0;

    uchar ucRt = 0;

    pinMode(pinSda, INPUT);
    sda_in_out = INPUT;
    
    for(int i=0; i<8; i++)
    {
        unsigned  char  ucBit;
        sclSet(LOW);
        sclSet(HIGH);
        ucBit = digitalRead(pinSda);
        ucRt = (ucRt << 1) + ucBit;
    }
    uchar dta = ucRt;
    recv_len--;

    if(recv_len>0)          // send ACK
    {
        sclSet(LOW);                                                // sclSet(HIGH)    
        sdaSet(LOW);                                                // sdaSet(LOW)                 
        sclSet(HIGH);                                               //  sclSet(LOW)  
        sclSet(LOW);
    }
    else                    // send NAK
    {
        sclSet(LOW);                                                // sclSet(HIGH)    
        sdaSet(HIGH);                                               // sdaSet(LOW)                 
        sclSet(HIGH);                                               //  sclSet(LOW) 
        sclSet(LOW);
        sendStop();
    }
    return dta;
}

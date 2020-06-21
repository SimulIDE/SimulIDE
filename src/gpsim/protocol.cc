/*
   Copyright (C) 2004 T. Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see 
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"

#include "protocol.h"

uint a2i( char b )
{
  if( b>='0'  && b<='9') return b-'0';
  if( b>='A' && b<='F') return b-'A'+10;
  if( b>='a' && b<='f') return b-'a'+10;

  return 0;
}
char i2a(uint i)
{
  i &= 0x0f;
  if(i < 10) return '0' + i;

  return 'A'+ i- 10;
}

uint ascii2uint(char **buffer, int digits)
{
  uint ret = 0;
  char *b = *buffer;

  for(int i=0; i<digits; i++) ret = (ret << 4) + a2i(*b++);

  *buffer = b;

  return ret;
}

static uint ascii2uint64(char *buffer, int digits)
{
  uint ret = 0;
  char *b = buffer;

  for(int i=0; i<digits; i++) ret = (ret << 4) + a2i(*b++);

  return ret;
}

static uint ascii2uint(char *buffer, int digits)
{
  uint long long i = ascii2uint64(buffer,digits);
  return (uint) i;
}

//========================================================================

PacketBuffer::PacketBuffer(uint _size)
  : size(_size)
{
  buffer = new char[_size];
  index = 0;
}

PacketBuffer::~PacketBuffer()
{
  delete [] buffer;
}

void PacketBuffer::puts(const char *s, int len)
{
  if(!s || len <= 0) return;

  uint ulen = len;
  if(ulen > (size - index)) ulen = size-index;

  if(ulen)
  {
    memcpy(&buffer[index],s, ulen);
    index += ulen;
  }
}

void PacketBuffer::advanceIndex(uint amount)
{
  if(index + amount <  size) index += amount;
  else                       index = size-1;
}

void PacketBuffer::terminate()
{
  if(index < size) buffer[index]=0;
}
//========================================================================
Packet::Packet(uint rxsize, uint txsize)
{
  rxBuffer = new PacketBuffer(rxsize);
  txBuffer = new PacketBuffer(txsize);
}

bool Packet::DecodeHeader()
{
  if(*rxBuffer->buffer == '$') 
  {
    rxBuffer->index = 1;
    return true;
  }
  rxBuffer->index = 0;

  return false;
}

bool Packet::DecodeChar(char c)
{
  if(*rxBuffer->getBuffer() == c) 
  {
    rxBuffer->index++;
    return true;
  }
  return false;
}

bool Packet::DecodeUInt32(uint &i)
{
  char *b = rxBuffer->getBuffer();

  if(ascii2uint(&b,2) == eGPSIM_TYPE_UINT32) 
  {
    i = ascii2uint(b,8);
    rxBuffer->index += 2+8;

    return true;
  }
  return false;
}

bool Packet::DecodeUInt64(uint64_t  &i)
{
  char *b = rxBuffer->getBuffer();

  if(ascii2uint(&b,2) == eGPSIM_TYPE_UINT64) 
  {
    i = ascii2uint64(b,16);
    rxBuffer->index += 2+16;

    return true;
  }
  return false;
}

bool Packet::DecodeBool(bool  &b)
{
  char *buff = rxBuffer->getBuffer();

  if(ascii2uint(&buff,2) == eGPSIM_TYPE_BOOLEAN) 
  {
    if(*buff == '0')      b = false;
    else if(*buff == '1') b = true;
    else                  return false;

    rxBuffer->index += 2+1;

    return true;
  }
  return false;
}

bool Packet::DecodeFloat(double  &d)
{
  char *b = rxBuffer->getBuffer();

  if(ascii2uint(&b,2) == eGPSIM_TYPE_FLOAT) 
  {
    double dtry = strtod(b, &b);
    uint len = b - rxBuffer->buffer;
    if( len < rxBuffer->size - rxBuffer->index) 
    {
      rxBuffer->index += len;
      d = dtry;
      return true;
    }
  }
  return false;
}

bool Packet::DecodeObjectType(uint &i)
{
  i = ascii2uint(rxBuffer->getBuffer(),2);
  rxBuffer->index += 2;

  return true;
}

bool Packet::DecodeString(char *retStr, int maxLen)
{
  char *b = rxBuffer->getBuffer();

  if(ascii2uint(&b,2) == eGPSIM_TYPE_STRING) 
  {
    int length = ascii2uint(&b,2);

    maxLen--;   // reserve space for a terminating 0.

    length = (maxLen < length) ? maxLen  : length;

    strncpy(retStr, b, length);
    retStr[length] = 0;

    //*buffer = b + length;

    rxBuffer->index += 2+2+length;
    return true;
  }

  return false;
}

bool Packet::EncodeHeader()
{
  txBuffer->putc('$');
  txBuffer->terminate();

  return true;
}

bool Packet::EncodeUInt32(uint i)
{
  txBuffer->putc(i2a(eGPSIM_TYPE_UINT32 /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_UINT32 ));

  for(int j=7; j>=0; j--) txBuffer->putc ( i2a ( i>> (4*j)));

  return true;
}

bool Packet::EncodeUInt64(uint64_t  i)
{

  txBuffer->putc(i2a(eGPSIM_TYPE_UINT64 /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_UINT64 ));

  for(int j=15; j>=0; j--) txBuffer->putc ( i2a ( i>> (4*j)));

  return true;
}

bool Packet::EncodeObjectType(uint i)
{
  EncodeHeader();

  //txBuffer->putc(i2a(eGPSIM_TYPE_OBJECT /16));
  //txBuffer->putc(i2a(eGPSIM_TYPE_OBJECT ));

  txBuffer->putc ( i2a ( i>> (4*1)));
  txBuffer->putc ( i2a ( i>> (4*0)));

  return true;
}

bool Packet::EncodeBool(bool b)
{
  txBuffer->putc(i2a(eGPSIM_TYPE_BOOLEAN /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_BOOLEAN ));

  if(b) txBuffer->putc('1');
  else  txBuffer->putc('0');

  return true;
}

bool Packet::EncodeFloat(double  d)
{
  txBuffer->putc(i2a(eGPSIM_TYPE_FLOAT /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_FLOAT ));

  char buff[256];

  snprintf(buff,sizeof(buff),"%8E~",d);

  txBuffer->puts(buff,strlen(buff));

  return true;
}

bool Packet::EncodeString(const char *str, int len)
{
  if(!str) return false;

  txBuffer->putc(i2a(eGPSIM_TYPE_STRING /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_STRING ));

  if(len < 0) len = strlen(str);

  txBuffer->putc(i2a(len>>4));
  txBuffer->putc(i2a(len));

  txBuffer->puts(str, len);

  return true;
}

bool Packet::EncodeCustom(const char *str, int len)
{
  if(!str) return false;

  txBuffer->putc(i2a(eGPSIM_TYPE_CUSTOM /16));
  txBuffer->putc(i2a(eGPSIM_TYPE_CUSTOM ));

  txBuffer->putc(i2a(len>>4));
  txBuffer->putc(i2a(len));

  txBuffer->puts(str, len);

  return true;
}

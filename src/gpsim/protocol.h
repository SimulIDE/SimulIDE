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

#ifndef __PROTCOL_H__
#define __PROTCOL_H__

typedef unsigned int uint;

#ifdef putc
#undef putc
#endif

#include <cstdint>

/// gpsim protocol
///
/// gpsim's protocol interface is designed to provide a way for clients 
/// that are not linked with gpsim to interface with gpsim. 

/// Basic types
/// These are the fundamental types the protocol interface
/// supports.

enum eGPSIMObjectTypes
  {
    eGPSIM_TYPE_CHAR = 1,
    eGPSIM_TYPE_STRING,
    eGPSIM_TYPE_UINT32,
    eGPSIM_TYPE_UCHAR,
    eGPSIM_TYPE_BOOLEAN,
    eGPSIM_TYPE_INT32,
    eGPSIM_TYPE_INT64,
    eGPSIM_TYPE_UINT64,
    eGPSIM_TYPE_FLOAT,
    eGPSIM_TYPE_DOUBLE,
    eGPSIM_TYPE_OBJECT,
    eGPSIM_TYPE_CUSTOM,
  };

/// Socket Commands
/// FIXME - document how these are used.
/// 
enum eGPSIMSocketCommands
  {

    GPSIM_CMD_CREATE_NOTIFY_LINK    = 0xE0,
    GPSIM_CMD_CREATE_CALLBACK_LINK  = 0xE1,

    GPSIM_CMD_CREATE_SOCKET_LINK    = 0xF0,
    GPSIM_CMD_REMOVE_SOCKET_LINK    = 0xF1,
    GPSIM_CMD_QUERY_SOCKET_LINK     = 0xF2,
    GPSIM_CMD_WRITE_TO_SOCKET_LINK  = 0xF3,

    GPSIM_CMD_QUERY_SYMBOL          = 0xF4,
    GPSIM_CMD_WRITE_TO_SYMBOL       = 0xF5,

    GPSIM_CMD_RUN                   = 0xF6,
    GPSIM_CMD_RESET                 = 0xF7,

  };


/// PacketBuffer
/// A packet buffer is an area of memory that gpsim and a client
/// use to exchange information. The buffer consists of a sequence
/// encoded GPSIMObjectTypes. Member functions for encoding and 
/// decoding each type.

class PacketBuffer
{
public:
  PacketBuffer(uint _size);
  ~PacketBuffer();

  char * getBuffer()
  {
    return &buffer[index];
  }

  uint getSize()
  {
    return size-index;
  }

  void terminate();

  void putc(char c)
  {
    if(index < size)
      buffer[index++] = c;
  }
  void putAt(int pos, char c)
  {
    if(pos >=0 && pos < (int) size) 
      buffer[pos] = c;
  }

  void puts(const char *, int);

  /// advanceIndex() will move the index pointer forward

  void advanceIndex(uint amount);

  bool bHasData() { return index!=0; }

  //private:
  char          *buffer;
  uint   index;
  uint   size;

};

class Packet
{
public:
  Packet(uint rxsize, uint txsize);

  bool DecodeHeader();
  bool DecodeObjectType(uint &);
  bool DecodeChar(char);
  bool DecodeUInt32(uint &);
  bool DecodeUInt64(uint64_t &);
  bool DecodeString(char *, int);
  bool DecodeBool(bool &);
  bool DecodeFloat(double &);

  bool EncodeHeader();
  bool EncodeUInt32(uint);
  bool EncodeUInt64(uint64_t);
  bool EncodeObjectType(uint);
  bool EncodeString(const char *str, int len=-1);
  bool EncodeCustom(const char *str, int len);
  bool EncodeBool(bool);
  bool EncodeFloat(double);

  char *rxBuff()
  {
    return rxBuffer->getBuffer();
  }
  uint rxSize()
  {
    return rxBuffer->getSize();
  }
  void rxTerminate(int pos)
  {
    rxBuffer->putAt(pos,0);
  }

  void rxAdvance(uint amount)
  {
    rxBuffer->advanceIndex(amount);
  }
  bool brxHasData()
  { 
    return rxBuffer->bHasData(); 
  }

  char *txBuff()
  {
    return txBuffer->buffer;
  }
  uint txBytesBuffered()
  {
    return txBuffer->index;
  }
  void txTerminate()
  {
    txBuffer->terminate();
  }
  void prepare()
  {
    rxBuffer->index = 0;
    txBuffer->index = 0;
  }

private:
  PacketBuffer *rxBuffer;
  PacketBuffer *txBuffer;

};


#endif

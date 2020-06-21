/*
   Copyright (C) 1998 T. Scott Dattalo

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

// T. Scott Dattalo 12bit core routines

/* pic12.c  - pic 12bit core routines	*/
/* version 0.1				*/
/* (c) I.King 1994			*/

#include <iostream>

#include "config.h"
#include "12bit-processors.h"
#include "12bit-instructions.h"

instruction * disasm12 (pic_processor *cpu, uint address, uint inst)
{

  unsigned char topnibble;
  unsigned char midnibble;
  unsigned char lownibble;
  unsigned char bbyte;
  unsigned char bits6and7;

  topnibble = (inst & 0x0f00) >> 8;
  midnibble = (inst & 0x00f0) >> 4;
  lownibble = (inst & 0x000f);
  bbyte	  = (inst & 0x00ff);
  bits6and7 = (unsigned char)((int) (bbyte & 0xc0) >> 6);

  switch(topnibble)
    {
    case 0x00:	
      if (midnibble == 0)
	switch(lownibble)
	  {
	  case 0x00:
	    return(new NOP(cpu,inst,address));
	  case 0x02:
	    return(new OPTION(cpu,inst,address));
	  case 0x03:
	    return(new SLEEP(cpu,inst,address));
	  case 0x04:
	    return(new CLRWDT(cpu,inst,address));
	  default:
	    return(new TRIS(cpu,inst,address));
	  }
      else
	switch(bits6and7)
	  {
	  case 0x00:
	    return(new MOVWF(cpu,inst,address));
	  case 0x01:
	    if(midnibble & 0x02)
	      return(new CLRF(cpu,inst,address));
	    else
	      return(new CLRW(cpu,inst,address));
	  case 0x02:
	    return(new SUBWF(cpu,inst,address));
	  case 0x03:
	    return(new DECF(cpu,inst,address));

	  }

      break;

    case 0x01:	switch(bits6and7)
      {
      case 0x00:
	return(new IORWF(cpu,inst,address));
      case 0x01:
	return(new ANDWF(cpu,inst,address));
      case 0x02:
	return(new XORWF(cpu,inst,address));
      case 0x03:
	return(new ADDWF(cpu,inst,address));
      }
    break;

    case 0x02:	switch(bits6and7)
      {
      case 0x00:
	return(new MOVF(cpu,inst,address));
      case 0x01:
	return(new COMF(cpu,inst,address));
      case 0x02:
	return(new INCF(cpu,inst,address));
      case 0x03:
	return(new DECFSZ(cpu,inst,address));
      }
    break;

    case 0x03:	switch(bits6and7)
      {
      case 0x00:
	return(new RRF(cpu,inst,address));
      case 0x01:
	return(new RLF(cpu,inst,address));
      case 0x02:
	return(new SWAPF(cpu,inst,address));
      case 0x03:
	return(new INCFSZ(cpu,inst,address));
      }

    break;

    case 0x04:
      return(new BCF(cpu,inst,address));
    case 0x05:
      return(new BSF(cpu,inst,address));
    case 0x06:
      return(new BTFSC(cpu,inst,address));
    case 0x07:
      return(new BTFSS(cpu,inst,address));

    case 0x08:
      return(new RETLW(cpu,inst,address));
    case 0x09:
      return(new CALL(cpu,inst,address));
    case 0x0a:
    case 0x0b:
      return(new GOTO(cpu,inst,address));
    case 0x0c:
      return(new MOVLW(cpu,inst,address));
    case 0x0d:
      return(new IORLW(cpu,inst,address));
    case 0x0e:
      return(new ANDLW(cpu,inst,address));
    case 0x0f:
      return(new XORLW(cpu,inst,address));
    }
    // shouldn't get here
	return 0;
}

/* ... The End ... */

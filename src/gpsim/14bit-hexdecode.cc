/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2013 Roy R Rankin

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

// T. Scott Dattalo 14bit core routines

// Portions of this file are from:
//
/* pic14.c  - pic 14bit core routines   */
/* version 0.1                          */
/* (c) I.King 1994                      */

#include <iostream>

#include "config.h"
#include "14bit-processors.h"
#include "14bit-instructions.h"

struct instruction_constructor op_16ext[] = {
  { 0x3f80,  0x3100,  ADDFSR::construct },
  { 0x3f00,  0x3d00,  ADDWFC::construct },
  { 0x3f00,  0x3700,  ASRF::construct },
  { 0x3e00,  0x3200,  BRA::construct },
  { 0x3fff,  0x000b,  BRW::construct },
  { 0x3fff,  0x000a,  CALLW::construct },
  { 0x3ff8,  0x0010,  MOVIW::construct },
  { 0x3f80,  0x3f00,  MOVIW::construct },
  { 0x3fe0,  0x0020,  MOVLB::construct },
  { 0x3f80,  0x3180,  MOVLP::construct },
  { 0x3ff8,  0x0018,  MOVWI::construct },
  { 0x3f80,  0x3f80,  MOVWI::construct },
  { 0x3f00,  0x3500,  LSLF::construct },
  { 0x3f00,  0x3600,  LSRF::construct },
  { 0x3fff,  0x0001,  RESET::construct },
  { 0x3f00,  0x3b00,  SUBWFB::construct },

};
struct instruction_constructor op_16cxx[] = {

  { 0x3f00,  0x3e00,  ADDLW::construct }, 
  { 0x3f00,  0x3f00,  ADDLW::construct }, // Accomdate don't care bit
  { 0x3f00,  0x0700,  ADDWF::construct },
  { 0x3f00,  0x3900,  ANDLW::construct },
  { 0x3f00,  0x0500,  ANDWF::construct },
  { 0x3c00,  0x1000,  BCF::construct },
  { 0x3c00,  0x1400,  BSF::construct },
  { 0x3c00,  0x1800,  BTFSC::construct },
  { 0x3c00,  0x1c00,  BTFSS::construct },
  { 0x3800,  0x2000,  CALL::construct },
  { 0x3f80,  0x0180,  CLRF::construct },
  { 0x3fff,  0x0103,  CLRW::construct },
  { 0x3fff,  0x0064,  CLRWDT::construct },
  { 0x3f00,  0x0900,  COMF::construct },
  { 0x3f00,  0x0300,  DECF::construct },
  { 0x3f00,  0x0b00,  DECFSZ::construct },
  { 0x3800,  0x2800,  GOTO::construct },
  { 0x3f00,  0x0a00,  INCF::construct },
  { 0x3f00,  0x0f00,  INCFSZ::construct },
  { 0x3f00,  0x3800,  IORLW::construct },
  { 0x3f00,  0x0400,  IORWF::construct },
  { 0x3f00,  0x0800,  MOVF::construct },
  { 0x3f00,  0x3000,  MOVLW::construct },
  { 0x3f00,  0x3100,  MOVLW::construct },
  { 0x3f00,  0x3200,  MOVLW::construct },
  { 0x3f00,  0x3300,  MOVLW::construct },
  { 0x3f80,  0x0080,  MOVWF::construct },
  { 0x3fff,  0x0000,  NOP::construct },
  { 0x3fff,  0x0020,  NOP::construct },
  { 0x3fff,  0x0040,  NOP::construct },
  { 0x3fff,  0x0060,  NOP::construct },
  { 0x3fff,  0x0062,  OPTION::construct },
  { 0x3fff,  0x0009,  RETFIE::construct },
  { 0x3f00,  0x3400,  RETLW::construct },
  { 0x3f00,  0x3500,  RETLW::construct },
  { 0x3f00,  0x3600,  RETLW::construct },
  { 0x3f00,  0x3700,  RETLW::construct },
  { 0x3fff,  0x0008,  RETURN::construct },
  { 0x3f00,  0x0d00,  RLF::construct },
  { 0x3f00,  0x0c00,  RRF::construct },
  { 0x3fff,  0x0063,  SLEEP::construct },
  { 0x3f00,  0x3c00,  SUBLW::construct },
  { 0x3f00,  0x3d00,  SUBLW::construct },
  { 0x3f00,  0x0200,  SUBWF::construct },
  { 0x3fff,  0x0065,  TRIS::construct },
  { 0x3fff,  0x0066,  TRIS::construct },
  { 0x3fff,  0x0067,  TRIS::construct },
  { 0x3f00,  0x0e00,  SWAPF::construct },
  { 0x3f00,  0x3a00,  XORLW::construct },
  { 0x3f00,  0x0600,  XORWF::construct },

};


const int NUM_OP_16CXX	= sizeof(op_16cxx) / sizeof(op_16cxx[0]);
const int NUM_OP_16EXT	= sizeof(op_16ext) / sizeof(op_16ext[0]);


instruction * disasm14 (_14bit_processor *cpu, uint addr, uint inst)
{
  instruction *pi;

  pi = 0;

  for(int i =0; i<NUM_OP_16CXX && !pi; i++)
    if((op_16cxx[i].inst_mask & inst) == op_16cxx[i].opcode)
      pi = op_16cxx[i].inst_constructor(cpu, inst, addr);

  if(!pi)
    pi = invalid_instruction::construct(cpu, inst, addr);

  return (pi);
}
// decode for 14bit processors with enhanced instructions 
instruction * disasm14E (_14bit_e_processor *cpu, uint addr, uint inst)
{
  instruction *pi;

  pi = 0;

  for(int i =0; i<NUM_OP_16EXT && !pi; i++)
      if((op_16ext[i].inst_mask & inst) == op_16ext[i].opcode)
        pi = op_16ext[i].inst_constructor(cpu, inst, addr);
  
  for(int i =0; i<NUM_OP_16CXX && !pi; i++)
    if((op_16cxx[i].inst_mask & inst) == op_16cxx[i].opcode)
      pi = op_16cxx[i].inst_constructor(cpu, inst, addr);

  if(!pi)
    pi = invalid_instruction::construct(cpu, inst, addr);

  return (pi);
}



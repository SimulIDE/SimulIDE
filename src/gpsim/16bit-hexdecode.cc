/*
   Copyright (C) 1998,1999 T. Scott Dattalo

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

// T. Scott Dattalo 16bit core routines

#include <iostream>
#include <iomanip>
#include <string>
#include <list>

#include "config.h"
#include "16bit-processors.h"
#include "pic-instructions.h"
#include "12bit-instructions.h"
#include "16bit-instructions.h"

#include "pic-processor.h"
#include "stimuli.h"

/* PIC 16-bit instruction set */

struct instruction_constructor op_18cxx[] = {
  // Extended Instructions
  { 0xfe00,  0xe800,  ADDFSR16::construct }, // ADDFSR & SUBFSR, ADDULNK, SUBULNK
  { 0xffff,  0x0014,  CALLW16::construct },
  { 0xff00,  0xeb00,  MOVSF::construct },  // MOVSF & MOVSS
  { 0xff00,  0xea00,  PUSHL::construct },

  // Normal instructions
  { 0xff00,  0x0f00,  ADDLW16::construct },
  { 0xfc00,  0x2400,  ADDWF16::construct },
  { 0xfc00,  0x2000,  ADDWFC16::construct },
  { 0xff00,  0x0b00,  ANDLW16::construct },
  { 0xfc00,  0x1400,  ANDWF16::construct },
  { 0xff00,  0xe200,  BC::construct },
  { 0xf000,  0x9000,  BCF16::construct },
  { 0xff00,  0xe600,  BN::construct },
  { 0xff00,  0xe300,  BNC::construct },
  { 0xff00,  0xe700,  BNN::construct },
  { 0xff00,  0xe500,  BNOV::construct },
  { 0xff00,  0xe100,  BNZ::construct },
  { 0xff00,  0xe400,  BOV::construct },
  { 0xf800,  0xd000,  BRA16::construct },
  { 0xf000,  0x8000,  BSF16::construct },
  { 0xf000,  0xb000,  BTFSC16::construct },
  { 0xf000,  0xa000,  BTFSS16::construct },
  { 0xf000,  0x7000,  BTG::construct },
  { 0xff00,  0xe000,  BZ::construct },
  { 0xfe00,  0xec00,  CALL16::construct },
  { 0xfe00,  0x6a00,  CLRF16::construct },
  { 0xffff,  0x0004,  CLRWDT::construct },
  { 0xfc00,  0x1c00,  COMF16::construct },
  { 0xfe00,  0x6200,  CPFSEQ::construct },
  { 0xfe00,  0x6400,  CPFSGT::construct },
  { 0xfe00,  0x6000,  CPFSLT::construct },
  { 0xffff,  0x0007,  DAW::construct },
  { 0xfc00,  0x0400,  DECF16::construct },
  { 0xfc00,  0x2c00,  DECFSZ16::construct },
  { 0xfc00,  0x4c00,  DCFSNZ::construct },
  { 0xff00,  0xef00,  GOTO16::construct },
  { 0xfc00,  0x2800,  INCF16::construct },
  { 0xfc00,  0x3c00,  INCFSZ16::construct },
  { 0xfc00,  0x4800,  INFSNZ::construct },
  { 0xff00,  0x0900,  IORLW16::construct },
  { 0xfc00,  0x1000,  IORWF16::construct },
  { 0xffc0,  0xee00,  LFSR::construct },
  { 0xfc00,  0x5000,  MOVF16::construct },
  { 0xf000,  0xc000,  MOVFF::construct },
  { 0xff00,  0x0100,  MOVLB16::construct },
  { 0xff00,  0x0e00,  MOVLW::construct },
  { 0xfe00,  0x6e00,  MOVWF16::construct },
 //RRR { 0xff00,  0x6f00,  MOVWF16::construct },
 //RRR { 0xff00,  0x6e00,  MOVWF16a::construct },
  { 0xff00,  0x0d00,  MULLW::construct },
  { 0xfe00,  0x0200,  MULWF::construct },
  { 0xfe00,  0x6c00,  NEGF::construct },
  { 0xffff,  0x0000,  NOP::construct },
  { 0xf000,  0xf000,  NOP::construct },
  { 0xffff,  0x0006,  POP::construct },
  { 0xffff,  0x0005,  PUSH::construct },
  { 0xf800,  0xd800,  RCALL::construct },
  { 0xffff,  0x00ff,  RESET::construct },
  { 0xfffe,  0x0010,  RETFIE16::construct },
  { 0xff00,  0x0c00,  RETLW::construct },
  { 0xfffe,  0x0012,  RETURN16::construct },
  { 0xfc00,  0x3400,  RLCF::construct },
  { 0xfc00,  0x4400,  RLNCF::construct },
  { 0xfc00,  0x3000,  RRCF::construct },
  { 0xfc00,  0x4000,  RRNCF::construct },
  { 0xfe00,  0x6800,  SETF::construct },
  { 0xffff,  0x0003,  SLEEP16::construct },
  { 0xfc00,  0x5400,  SUBFWB::construct },
  { 0xff00,  0x0800,  SUBLW16::construct },
  { 0xfc00,  0x5c00,  SUBWF16::construct },
  { 0xfc00,  0x5800,  SUBWFB16::construct },
  { 0xfc00,  0x3800,  SWAPF16::construct },
  { 0xfffc,  0x0008,  TBLRD::construct },
  { 0xfffc,  0x000c,  TBLWT::construct },
  { 0xfe00,  0x6600,  TSTFSZ::construct },
  { 0xff00,  0x0a00,  XORLW16::construct },
  { 0xfc00,  0x1800,  XORWF16::construct },
};

struct instruction_constructor op_17cxx[] = {
  { 0xff00,  0xb100,  ADDLW16::construct  },
  { 0xfe00,  0x0e00,  ADDWF16::construct  },
  { 0xfe00,  0x1000,  ADDWFC::construct  },
  { 0xff00,  0xb500,  ANDLW16::construct  },
  { 0xfe00,  0x0a00,  ANDWF16::construct  },
  { 0xf800,  0x8800,  BCF::construct  },
  { 0xf800,  0x8000,  BSF::construct  },
  { 0xf800,  0x9800,  BTFSC::construct  },
  { 0xf800,  0x9000,  BTFSS::construct  },
  { 0xf800,  0x3800,  BTG::construct  },
  { 0xe000,  0xe000,  CALL16::construct  },
  { 0xfe00,  0x2800,  CLRF::construct  },
  { 0xffff,  0x0004,  CLRWDT::construct  },
  { 0xfe00,  0x1200,  COMF16::construct  },
  { 0xff00,  0x3100,  CPFSEQ::construct  },
  { 0xff00,  0x3200,  CPFSGT::construct  },
  { 0xff00,  0x3000,  CPFSLT::construct  },
  { 0xfe00,  0x2e00,  DAW::construct  },
  { 0xfe00,  0x0600,  DECF16::construct  },
  { 0xfe00,  0x1600,  DECFSZ16::construct  },
  { 0xfe00,  0x2600,  DCFSNZ::construct  },
  { 0xe000,  0xc000,  GOTO16::construct  },
  { 0xfe00,  0x1400,  INCF16::construct  },
  { 0xfe00,  0x1e00,  INCFSZ16::construct  },
  { 0xfe00,  0x2400,  INFSNZ::construct  },
  { 0xff00,  0xb300,  IORLW16::construct  },
  { 0xfe00,  0x0800,  IORWF16::construct  },
  { 0xff00,  0xb700,  LCALL16::construct  },
  { 0xe000,  0x6000,  MOVFP::construct  },
  { 0xe000,  0x4000,  MOVPF::construct  },
  { 0xff00,  0xb800,  MOVLB::construct  },
  { 0xfe00,  0xba00,  MOVLR::construct  },
  { 0xff00,  0xb000,  MOVLW::construct  },
  { 0xff00,  0x0100,  MOVWF16::construct  },
  { 0xff00,  0xbc00,  MULLW::construct  },
  { 0xff00,  0x3400,  MULWF::construct  },
  { 0xfe00,  0x2c00,  NEGW::construct  },
  { 0xffff,  0x0000,  NOP::construct  },
  { 0xffff,  0x0005,  RETFIE16::construct  },
  { 0xff00,  0xb600,  RETLW::construct  },
  { 0xffff,  0x0002,  RETURN16::construct  },
  { 0xfe00,  0x1a00,  RLCF::construct  },
  { 0xfe00,  0x2200,  RLNCF::construct  },
  { 0xfe00,  0x1800,  RRCF::construct  },
  { 0xfe00,  0x2000,  RRNCF::construct  },
  { 0xfe00,  0x2a00,  SETF::construct  },
  { 0xffff,  0x0003,  SLEEP16::construct  },
  { 0xff00,  0xb200,  SUBLW16::construct  },
  { 0xfe00,  0x0400,  SUBWF16::construct  },
  { 0xfe00,  0x0200,  SUBWFB::construct  },
  { 0xfe00,  0x1c00,  SWAPF::construct  },
  { 0xfc00,  0xa800,  TBLRD::construct  },
  { 0xfc00,  0xac00,  TBLWT::construct  },
  { 0xfc00,  0xa000,  TLRD::construct  },
  { 0xfc00,  0xa400,  TLWT::construct  },
  { 0xff00,  0x3300,  TSTFSZ::construct  },
  { 0xff00,  0xb400,  XORLW16::construct  },
  { 0xfe00,  0x0c00,  XORWF16::construct  }
};

const int NUM_OP_18CXX	= sizeof(op_18cxx) / sizeof(op_18cxx[0]);
const int NUM_OP_17CXX	= sizeof(op_17cxx) / sizeof(op_17cxx[0]);

instruction * disasm16 (pic_processor *cpu, uint address, uint inst)
{
  instruction *pi;

  cpu16->setCurrentDisasmAddress(address);

  pi = 0;
  for(int i =0; i<NUM_OP_18CXX && !pi; i++)
    if((op_18cxx[i].inst_mask & inst) == op_18cxx[i].opcode)
      pi = op_18cxx[i].inst_constructor(cpu, inst,address);

  if(pi == 0)
    pi = invalid_instruction::construct(cpu, inst, address);

  return (pi);
}

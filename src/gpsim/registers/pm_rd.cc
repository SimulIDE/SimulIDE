/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2003 Mike Durian
                 2006,2017 Roy Rankin
                 2006 David Barnett

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

#include <assert.h>

#include <iostream>
#include <iomanip>
using namespace std;


#include "pic-processor.h"
#include "pm_rd.h"


//------------------------------------------------------------------------
//
// PM-related registers

void PMCON1::put(uint new_value)
{
  new_value &= valid_bits;
  
  bool rd_rise = (bool)(new_value & ~value.get() & RD);
  value.put((value.get() & RD) | new_value);

  if (rd_rise) pm_rd->start_read();
}

uint PMCON1::get()
{
  return(value.get());
}

PMCON1::PMCON1(Processor *pCpu, PM_RD *pRd)
  : sfr_register(pCpu, "pmcon1", "Program Memory Read Write Control"),
    pm_rd(pRd)
{
  valid_bits = PMCON1_VALID_BITS;
}

void PMCON1_RW::put(uint new_value)
{
  uint diff = value.get() ^ new_value;

  new_value |= 0x80;
  value.put(new_value);

  if ((diff & WR) && (new_value & (WR|WREN)) == (WR|WREN))
  {
      if ((pm_rw->get_reg_pmcon2())->is_ready_for_write())
      {
          if (new_value & FREE)                // erase row
                pm_rw->erase_row();
          else if (new_value & LWLO)        // write to latches
              pm_rw->write_latch();
          else
              pm_rw->write_row(); // write latches to memory
      }
      else
          new_value |= WRERR;
  }
  else if (new_value & RD)
    pm_rw->start_read();
}
void PMCON2::put(uint new_value)
{
  if(new_value == value.get()) return;

  value.put(new_value);
    if( (state == WAITING) && (0x55 == new_value))
    {
      state = HAVE_0x55;
    }
  else if ( (state == HAVE_0x55) && (0xaa == new_value))
    {
      state = READY_FOR_WRITE;
    }
  else if ((state == HAVE_0x55) || (state == READY_FOR_WRITE))
    {
      state = WAITING;
    }

}

uint PMDATA::get()
{
  return(value.get());
}

void PMDATA::put(uint new_value)
{
  value.put(new_value);
}

PMDATA::PMDATA(Processor *pCpu, const char *pName)
  : sfr_register(pCpu, pName, "Program Memory Data")
{}


uint PMADR::get()
{
  return(value.get());
}

void PMADR::put(uint new_value)
{
  value.put(new_value);
}


PMADR::PMADR(Processor *pCpu, const char *pName)
  : sfr_register(pCpu, pName, "Program Memory Address")
{}

// ----------------------------------------------------------

PM_RD::PM_RD(pic_processor *pCpu)
  : cpu(pCpu),
    pmcon1(pCpu,this),
    pmdata(pCpu,"pmdatl"),
    pmdath(pCpu,"pmdath"),
    pmadr(pCpu,"pmadr"),
    pmadrh(pCpu,"pmadrh")
{
}

void PM_RD::start_read()
{
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);

  get_cycles().set_break(get_cycles().get() + READ_CYCLES, this);
}

void PM_RD::callback()
{
  // read program memory
  if(pmcon1.value.get() & PMCON1::RD) {
    int opcode = cpu->pma->get_opcode(rd_adr);
    pmdata.value.put(opcode & 0xff);
    pmdath.value.put((opcode>>8) & 0xff);
    pmcon1.value.put(pmcon1.value.get() & (~PMCON1::RD));
  }
}

// ----------------------------------------------------------

PM_RW::PM_RW(pic_processor *pCpu)
  : PM_RD(pCpu),
    pmcon1_rw(pCpu,this),
    pmcon2(pCpu,this), num_latches(16)
{
   write_latches = new uint [num_latches];
   for(int i = 0; i < num_latches; i++)
        write_latches[i] = LATCH_EMPTY;

}
PM_RW::~PM_RW()
{
    delete[] write_latches;
}

void PM_RW::callback()
{
  if(pmcon1_rw.value.get() & PMCON1_RW::RD) 
  {
    pmcon1_rw.value.put(pmcon1_rw.value.get() & (~PMCON1_RW::RD));
    return;
  }
  else if (pmcon1_rw.value.get() & PMCON1_RW::WR)
  {
/*
      int opcode = pmdata.value.get() | (pmdath.value.get() << 8);
      cpu->init_program_memory_at_index(rd_adr, opcode);
*/
      pmcon1_rw.value.put(pmcon1_rw.value.get() & (~PMCON1_RW::WR));
      pmcon2.unarm();
      return;
  }
}
void PM_RW::start_read()
{
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);

  if (pmcon1_rw.value.get() & PMCON1_RW::CFGS)
        rd_adr |= 0x2000;

  int opcode = cpu->get_program_memory_at_address(rd_adr);
  pmdata.value.put(opcode & 0xff);
  pmdath.value.put((opcode>>8) & 0xff);
  get_cycles().set_break(get_cycles().get() + READ_CYCLES, this);
}
void PM_RW::write_row()
{
  int index;
  uint opcode;
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);
  
  if (pmcon1_rw.value.get() & PMCON1_RW::CFGS)
        rd_adr |= 0x2000;

  index = rd_adr & (num_latches - 1);
  write_latches[index] = pmdata.value.get() | (pmdath.value.get() << 8);
  get_cycles().set_break(get_cycles().get() + 2e-3*get_cycles().instruction_cps(), this);
  rd_adr &= ~(num_latches - 1);
  for(index= 0; index < num_latches; index++)
  {
      opcode = cpu->get_program_memory_at_address(rd_adr);
      if (opcode != LATCH_EMPTY)
        fprintf(stderr, "Error write to un-erased program memory address=0x%x\n", rd_adr);
      cpu->init_program_memory_at_index(rd_adr, write_latches[index]);
      write_latches[index] = LATCH_EMPTY;
      rd_adr++;
  }
        
}

void PM_RW::erase_row()
{
  int index;
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);
  
  if (pmcon1_rw.value.get() & PMCON1_RW::CFGS)
        rd_adr |= 0x2000;

  index = rd_adr & (num_latches - 1);
  get_cycles().set_break(get_cycles().get() + 2e-3*get_cycles().instruction_cps(), this);
  rd_adr &= ~(num_latches - 1);
  
  for(index= 0; index < num_latches; index++)
  {
      cpu->init_program_memory_at_index(rd_adr, LATCH_EMPTY);
      write_latches[index] = LATCH_EMPTY;
      rd_adr++;
  }        
}

void PM_RW::write_latch()
{
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);
  
  if (pmcon1_rw.value.get() & PMCON1_RW::CFGS)
        rd_adr |= 0x2000;

  uint index = rd_adr & (num_latches - 1);
  write_latches[index] = pmdata.value.get() | (pmdath.value.get() << 8);
  get_cycles().set_break(get_cycles().get() + READ_CYCLES, this);
}

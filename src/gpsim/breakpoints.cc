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


#include <iostream>
#include <iomanip>

#include "config.h"
#include "pic-processor.h"
#include "breakpoints.h"
#include "14bit-processors.h"

/*extern "C"{
#include "lxt_write.h"
}*/

#define PCPU ((Processor *)cpu)

// Global declaration of THE breakpoint object
Breakpoints &(*dummy_bp)() = get_bp;
Breakpoints bp;

//------------------------------------------------------------------------
// find_free - search the array that holds the break points for a free slot
//
int Breakpoints::find_free()
{

  for(int i=0; i<MAX_BREAKPOINTS; i++) 
  {
    if(break_status[i].type == BREAK_CLEAR)  
    {
      if (i + 1 > m_iMaxAllocated) m_iMaxAllocated = i + 1;
      return i;
    }
  }
  cout << "*** out of breakpoints\n";
  return(MAX_BREAKPOINTS);
}

//------------------------------------------------------------------------
// set_breakpoint - Set a breakpoint of a specific type.
//
int Breakpoints::set_breakpoint(BREAKPOINT_TYPES break_type,
                                Processor *cpu,
                                uint arg1,
                                unsigned arg2,
                                TriggerObject *f1)
{
  breakpoint_number = find_free();
  
  if(breakpoint_number >= MAX_BREAKPOINTS) return breakpoint_number;

  BreakStatus &bs = break_status[breakpoint_number];
  bs.type = break_type;
  bs.cpu  = cpu;
  bs.arg1 = arg1;
  bs.arg2 = arg2;
  bs.bpo  = f1;
  switch (break_type)
    {

    case BREAK_ON_INVALID_FR:
      return(breakpoint_number);
      break;

    case BREAK_ON_CYCLE:
    {
      uint64_t cyc = arg2;
      cyc = (cyc<<32) | arg1;

      // The cycle counter does its own break points.
      if(get_cycles().set_break(cyc, f1, breakpoint_number)) 
      {
        return(breakpoint_number);
      }
      else bs.type = BREAK_CLEAR;
    }
      break;

    case BREAK_ON_STK_OVERFLOW:
      if ((cpu->GetCapabilities() & Processor::eBREAKONSTACKOVER)
        == Processor::eBREAKONSTACKOVER) {
        // pic_processor should not be referenced here
        // Should have a GetStack() virtual function in Processor class.
        // Of course then the Stack class needs to be a virtual class.
        if(((pic_processor *)(cpu))->stack->set_break_on_overflow(1))
          return (breakpoint_number);
      }
      else {
        // Need to add console object
        printf("Stack breaks not available on a %s processor\n", cpu->name().c_str());
      }
      bs.type = BREAK_CLEAR;
      break;

    case BREAK_ON_STK_UNDERFLOW:
      if ((cpu->GetCapabilities() & Processor::eBREAKONSTACKUNDER)
        == Processor::eBREAKONSTACKUNDER) {
        // pic_processor should not be referenced here
        // Should have a GetStack() virtual function in Processor class.
        // Of course then the Stack class needs to be a virtual class.
        if(((pic_processor *)(cpu))->stack->set_break_on_underflow(1))
          return (breakpoint_number);
      }
      else {
        // Need to add console object
        printf("Stack breaks not available on a %s processor\n", cpu->name().c_str());
      }
      bs.type = BREAK_CLEAR;
      break;

    case BREAK_ON_WDT_TIMEOUT:
      if ((cpu->GetCapabilities() & Processor::eBREAKONWATCHDOGTIMER)
        == Processor::eBREAKONWATCHDOGTIMER) {
        // pic_processor should not be referenced here
        // Should have a GetStack() virtual function in Processor class.
        // Of course then the Stack class needs to be a virtual class.
        ((_14bit_processor *)cpu)->wdt.set_breakpoint(BREAK_ON_WDT_TIMEOUT | breakpoint_number);
        return(breakpoint_number);
      }
      else {
        // Need to add console object
        printf("Watch dog timer breaks not available on a %s processor\n", cpu->name().c_str());
      }
    default:   // Not a valid type
      bs.type = BREAK_CLEAR;
      break;
    }
  return(MAX_BREAKPOINTS);
}

int  Breakpoints::set_cycle_break(Processor *cpu, uint64_t future_cycle, TriggerObject *f1)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_CYCLE, cpu,
                         (uint)(future_cycle & 0xffffffff),
                         (uint)(future_cycle>>32),
                         f1));
}

int Breakpoints::set_stk_overflow_break(Processor *cpu)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_STK_OVERFLOW, cpu, 0, 0));
}

int Breakpoints::set_stk_underflow_break(Processor *cpu)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_STK_UNDERFLOW, cpu, 0, 0));
}

int  Breakpoints::set_wdt_break(Processor *cpu)
{
  if ((cpu->GetCapabilities() & Processor::eBREAKONWATCHDOGTIMER)
    == Processor::eBREAKONWATCHDOGTIMER) 
  {
    // Set a wdt break only if one is not already set.
    if(!cpu14->wdt.hasBreak())
      return(set_breakpoint (Breakpoints::BREAK_ON_WDT_TIMEOUT, cpu, 0, 0));
  }
  else 
  {
    // Need to add console object
    printf("Watch dog timer breaks not available on a %s processor\n", cpu->name().c_str());
  }
  return MAX_BREAKPOINTS;
}

int Breakpoints::check_cycle_break(uint bpn)
{
    cout << "cycle break: 0x" << hex << get_cycles().get()
       << dec << " = " << get_cycles().get() << endl;

    halt();
    if( bpn < MAX_BREAKPOINTS)
    {
      if (break_status[bpn].bpo)
          break_status[bpn].bpo->callback();

      clear(bpn);
    }
    return(1);
}

bool Breakpoints::dump(TriggerObject *pTO)
{
  if (!pTO) return false;

  pTO->print();
  return true;
}
bool Breakpoints::dump1(uint bp_num, int dump_type)
{
  return true;
}

void Breakpoints::dump(int dump_type)
{
  bool have_breakpoints = 0;
  if(dump_type != BREAK_ON_CYCLE)  
  {
    for(int i = 0; i<m_iMaxAllocated; i++)
      {
        if(dump1(i, dump_type))
          have_breakpoints = 1;
      }
  }
  if(dump_type == BREAK_DUMP_ALL || dump_type == BREAK_ON_CYCLE)  
  {
    cout << "Internal Cycle counter break points" << endl;
    get_cycles().dump_breakpoints();
    have_breakpoints = 1;
    cout << endl;
  }
  if(!have_breakpoints) cout << "No user breakpoints are set" << endl;
}

void Breakpoints::clear(uint b)
{
  if (!bIsValid(b)) return;

  BreakStatus &bs = break_status[b];

  if(bs.bpo) 
  {
    bs.bpo->clear();
    bs.type = BREAK_CLEAR;
    delete bs.bpo;
    bs.bpo = 0;
    return;
  }
  switch (bs.type) 
  {
      case BREAK_ON_CYCLE:
        bs.type = BREAK_CLEAR;
        //cout << "Cleared cycle breakpoint number " << b << '\n';
        break;

      case BREAK_ON_STK_OVERFLOW:
        bs.type = BREAK_CLEAR;
        if ((bs.cpu->GetCapabilities() & Processor::eSTACK)
            == Processor::eSTACK) {
          if(((pic_processor *)(bs.cpu))->stack->set_break_on_overflow(0))
            cout << "Cleared stack overflow break point.\n";
          else
            cout << "Stack overflow break point is already cleared.\n";
        }
        break;

      case BREAK_ON_STK_UNDERFLOW:
        bs.type = BREAK_CLEAR;
        if ((bs.cpu->GetCapabilities() & Processor::eSTACK)
            == Processor::eSTACK) {
          if(((pic_processor *)(bs.cpu))->stack->set_break_on_underflow(0))
            cout << "Cleared stack underflow break point.\n";
          else
            cout << "Stack underflow break point is already cleared.\n";
        }
        break;

      case BREAK_ON_WDT_TIMEOUT:
        bs.type = BREAK_CLEAR;
        if ((bs.cpu->GetCapabilities() & Processor::eBREAKONWATCHDOGTIMER)
            == Processor::eBREAKONWATCHDOGTIMER) {
          cout << "Cleared wdt timeout breakpoint number " << b << '\n';
          ((_14bit_processor *)bs.cpu)->wdt.set_breakpoint(0);
        }
        break;

      default:
        bs.type = BREAK_CLEAR;
        break;
  }
}

bool Breakpoints::bIsValid(uint b)
{
  return b < MAX_BREAKPOINTS;
}

bool Breakpoints::bIsClear(uint b)
{
  return  bIsValid(b) && break_status[b].type == BREAK_CLEAR;
}

void Breakpoints::set_message(uint b,string &m)
{
  if (bIsValid(b) && break_status[b].type != BREAK_CLEAR && break_status[b].bpo)
    break_status[b].bpo->new_message(m);
}

// Clear all break points that are set for a specific processor
// This only be called when a processor is being removed and not when a user
// wants to clear the break points. Otherwise, internal break points like
// invalid register accesses will get cleared.

void Breakpoints::clear_all(Processor *c)
{
  for(int i=0; i<MAX_BREAKPOINTS; i++)
    if(break_status[i].type != BREAK_CLEAR
       && break_status[i].cpu == c)
      clear(i);
}

void Breakpoints::clear_all_set_by_user(Processor *c)
{
  for(int i=0; i<MAX_BREAKPOINTS; i++)
    {
      if((c == break_status[i].cpu) && (break_status[i].type != BREAK_ON_INVALID_FR))
        clear(i);
    }
}

void Breakpoints::halt()
{
  global_break |= GLOBAL_STOP_RUNNING;
}

Breakpoints::Breakpoints()
{
  m_iMaxAllocated = 0;
  breakpoint_number = 0;

  for(int i=0; i<MAX_BREAKPOINTS; i++)
    break_status[i].type = BREAK_CLEAR;
}


/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2003 Mike Durian

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

#include "intcon.h"
#include "gpsim_classes.h"        // for RESET_TYPE
#include "pir.h"
#include "16bit-registers.h"
#include "16bit-processors.h"
#include "breakpoints.h"


//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//--------------------------------------------------
// member functions for the INTCON base class
//--------------------------------------------------
INTCON::INTCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
    in_interrupt = false;
}

void INTCON::set_T0IF()
{
  put(value.get() | T0IF);
}

void INTCON::set_rbif(bool b)
{
    bool current = (value.get() & RBIF) == RBIF;

    if (b && !current) put(value.get() | RBIF);
    if (!b && current)
    {
      put(value.get() & ~RBIF);
      if (portGReg)                // check if IOC match condition still exists
        portGReg->setIOCif();
    }
}

void INTCON::put(uint new_value)
{
  put_value(new_value);
}

void INTCON::put_value(uint new_value)
{
  uint diff = new_value ^ value.get();

  value.put(new_value);

  // If we are clearing RBIF and we are using simple IOC, reset RBIF if
  // port miss-match still exists 
  if ((diff & RBIF) && !(new_value & RBIF) && portGReg)
        portGReg->setIOCif();

  // Now let's see if there's a pending interrupt
  // The INTCON bits are:
  // GIE | ---- | TOIE | INTE | RBIE | TOIF | INTF | RBIF
  // There are 3 sources for interrupts, TMR0, RB0/INTF
  // and RBIF (RB7:RB4 change). If the corresponding interrupt
  // flag is set AND the corresponding interrupt enable bit
  // is set AND global interrupts (GIE) are enabled, THEN
  // there's an interrupt pending.
  // note: bit6 is not handled here because it is processor
  // dependent (e.g. EEIE for x84 and ADIE for x7x).

  if(!in_interrupt)
  { 
      if ((value.get()>>3)&value.get() & (T0IF | INTF | RBIF)) 
      {
          if (cpu_pic->is_sleeping())
              cpu_pic->exit_sleep();

           cpu_pic->BP_set_interrupt();
      }
      if( (diff & GIE) && (value.get() & GIE)  && check_peripheral_interrupt() )
      {
          peripheral_interrupt(false);
      }
  }
}

void INTCON::peripheral_interrupt ( bool hi_pri )
{
  uint reg_val = value.get();

  if ( hi_pri ) cout << "Dodgy call to 14-bit INTCON::peripheral_interrupt with priority set\n";

  if (reg_val & XXIE)
  {
      if (cpu_pic->is_sleeping()) cpu_pic->exit_sleep();
      if((reg_val & GIE) &&  !in_interrupt) cpu_pic->BP_set_interrupt();
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
INTCON2::INTCON2(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), m_bsRBPU(0)
{}

bool INTCON2::assignBitSink(uint bitPosition, BitSink *pBS)
{
  if (bitPosition == 7)  m_bsRBPU = pBS;
  return true;
}

bool INTCON2::releaseBitSink(uint bitPosition, BitSink *)
{
  if (bitPosition == 7) m_bsRBPU = 0;
  return true;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void INTCON2::put_value(uint new_value)
{
  uint old_value = value.get();
  value.put(new_value);

  if ((old_value ^ new_value) & RBPU  && m_bsRBPU)
    m_bsRBPU->setSink((new_value & RBPU) != 0);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void INTCON2::put(uint new_value)
{
  put_value(new_value);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
INTCON3::INTCON3(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void INTCON3::put_value(uint new_value)
{
  value.put(new_value);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void INTCON3::put(uint new_value)
{
  put_value(new_value);
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
INTCON_14_PIR::INTCON_14_PIR(Processor *pCpu, const char *pName, const char *pDesc)
  : INTCON(pCpu, pName, pDesc),
    pir_set(0), write_mask(0xff)
{}

void INTCON_14_PIR::reset(RESET_TYPE r)
{
  switch (r) 
  {
      case POR_RESET:
        value.put(por_value.data);
        break;

      default:
        if (wdtr_value.initialized()) value.put(wdtr_value.data);
        break;
  }
  in_interrupt = false;
}

void INTCON_14_PIR::put_value(uint new_value)
{
  uint diff = new_value ^ value.get();

  value.put(new_value);
  // Now let's see if there's a pending interrupt
  // The INTCON bits are:
  // GIE | ---- | TOIE | INTE | RBIE | TOIF | INTF | RBIF
  // There are 3 sources for interrupts, TMR0, RB0/INTF
  // and RBIF (RB7:RB4 change). If the corresponding interrupt
  // flag is set AND the corresponding interrupt enable bit
  // is set AND global interrupts (GIE) are enabled, THEN
  // there's an interrupt pending.
  // note: bit6 is not handled here because it is processor
  // dependent (e.g. EEIE for x84 and ADIE for x7x).

  // If we are clearing IOCIF and we are using simple IOC, reset IOCIF if
  // port miss-match still exists 
  if ((diff & IOCIF) && !(new_value & IOCIF) && portGReg)
        portGReg->setIOCif();

  if(value.get() & GIE  &&  !in_interrupt &&
     ( ((value.get()>>3) & value.get() & (T0IF | INTF | IOCIF)) ||
       ( value.get() & PEIE && check_peripheral_interrupt() )))
  {
      cpu_pic->BP_set_interrupt();
  }
}
int INTCON_14_PIR::check_peripheral_interrupt()
{
  assert(pir_set != 0);

  Dprintf((" INTCON::%s\n",__FUNCTION__));
  return (pir_set->interrupt_status());
}
void INTCON_14_PIR::put(uint new_value)
{
  // preserve read only bits, but do not let them be written
  uint read_only = value.get() & ~write_mask;
  Dprintf((" INTCON_14_PIR::%s new_value %02x read_only %02x\n",__FUNCTION__, new_value, read_only));

  put_value((new_value & write_mask) | read_only);
}

void INTCON_14_PIR::aocxf_val(IOCxF *ptr, uint val)
{
    int i;
    int sum = val;
    bool found = false;

    for(i = 0; i < (int)aocxf_list.size(); i++)
    {
        if (aocxf_list[i].ptr_iocxf == ptr)
        {
            found = true;
            aocxf_list[i].val = val;
        }
        sum |= aocxf_list[i].val;
    }
    if (!found)
    {
        aocxf_list.push_back(aocxf());
        aocxf_list[i].ptr_iocxf = ptr;
        aocxf_list[i].val = val;
    }
    set_rbif(sum);
}

void INTCON_14_PIR::set_rbif(bool b)
{
    bool current = (value.get() & IOCIF) == IOCIF;
    
    if( b && !current ) put_value(value.get() | IOCIF);
    if( !b && current ) put_value(value.get() & ~IOCIF);
}

//----------------------------------------------------------------------
// INTCON_16 
//
// intcon for the 16-bit processor cores.
//
//----------------------------------------------------------------------
INTCON_16::INTCON_16(Processor *pCpu, const char *pName, const char *pDesc)
  : INTCON(pCpu, pName, pDesc),
    interrupt_vector(0), rcon(0), intcon2(0), pir_set(0)
{
}

void INTCON_16::peripheral_interrupt ( bool hi_pri )
{
  Dprintf((" INTCON_16::%s\n",__FUNCTION__));
  assert(rcon != 0);

  if(rcon->value.get() & RCON::IPEN)
  {
//      cout << "peripheral interrupt, priority " << hi_pri << "\n";
    if ( hi_pri )
    {
      if( value.get() & GIEH &&  !in_interrupt )
      {
        set_interrupt_vector(INTERRUPT_VECTOR_HI);
        cpu_pic->BP_set_interrupt();
      }
    }
    else
    {
      if( ( value.get() & (GIEH|GIEL) ) == (GIEH|GIEL) &&  !in_interrupt)
      {
        set_interrupt_vector(INTERRUPT_VECTOR_LO);
        cpu_pic->BP_set_interrupt();
      }
    }
  }
  else
  {
    if((value.get() & (GIE | XXIE)) == (GIE | XXIE) &&  !in_interrupt)
      cpu_pic->BP_set_interrupt();
  }
}

int INTCON_16::check_peripheral_interrupt()
{
  assert(pir_set != 0);

  Dprintf((" INTCON_16::%s\n",__FUNCTION__));
  return (pir_set->interrupt_status());     // Not quite right, but...
  // was return 0; but that was blatantly broken
}

//----------------------------------------------------------------------
// void INTCON_16::clear_gies()
//
//  This routine clears the global interrupt enable bit(s). If priority
// interrupts are used (IPEN in RCON is set) then the appropriate gie
// bit (either giel or gieh) is cleared.
//
// This routine is called from 16bit_processor::interrupt().
//
void INTCON_16::clear_gies()
{
  assert(cpu != 0);

  if ( !(rcon->value.get() & RCON::IPEN) ) put(value.get() & ~GIE);
  else if ( isHighPriorityInterrupt() )    put(value.get() & ~GIEH);
  else                                     put(value.get() & ~GIEL);
}

void INTCON_16::set_gies()
{
  assert(rcon != 0);
  assert(intcon2 != 0);
  assert(cpu != 0);

  get();   // Update the current value of intcon
           // (and emit 'register read' trace).

    if(rcon->value.get() & RCON::IPEN) // Interrupt priorities are being used.
    {                             
        if(0 == (value.get() & GIEH)) // GIEH is cleared, so we need to set it
        {
          put(value.get() | GIEH);
          return;
        }
        else
        {
          // GIEH is set. This means high priority interrupts are enabled.
          // So we most probably got here because of an RETFIE instruction
          // after handling a low priority interrupt. We could check to see
          // if GIEL is low before calling put(), but it's not necessary.
          // So we'll just blindly re-enable giel, and continue with the
          // simulation.

          put(value.get() | GIEL);
          return;
        }
    }
    else  // Interrupt priorities are not used, so re-enable GIEH (which is in
    {     // the same bit-position as GIE on the mid-range core).

      put(value.get() | GIEH);
      return;
    }
}

//----------------------------------------------------------------------
// void INTCON_16::put(uint new_value)
//
//  Here's were the 18cxxx interrupt logic is primarily handled. 
//
// inputs: new_value - 
// outputs: none
//
void INTCON_16::put(uint new_value)
{
  put_value(new_value);
}

void INTCON_16::put_value(uint new_value)
{
  value.put(new_value);
  //cout << " INTCON_16::put\n";
  // Now let's see if there's a pending interrupt
  // if IPEN is set in RCON, then interrupt priorities
  // are being used. (In other words, there are two
  // interrupt priorities on the 18cxxx core. If a
  // low priority interrupt is being serviced, it's
  // possible for a high priority interrupt to interject.

    if(rcon->value.get() & RCON::IPEN)
    {
      uint i1;
      int i2;

      // Use interrupt priorities
      // %%%FIXME%%% ***BUG*** - does not attempt to look for peripheral interrupts

      if( 0==(value.get() & GIEH) || in_interrupt) return; // Interrupts are disabled
        
      // First we check the high priorities and then we check the
      // low ones. When ever we detect an interrupt, then the 
      // bp.interrupt flag is set (which will cause the interrupt
      // to be handled at the high level) and additional checks
      // are aborted.

      // If TO, INT, or RB flags are set AND their correspond
      // interrupts are enabled, then the lower three bits of
      // i1 will reflect this. Note that INTF does NOT have an
      // associated priority bit!

      i1 =  ( (value.get()>>3)&value.get()) & (T0IF | INTF | RBIF);
      i2 =  check_peripheral_interrupt();

        if ( ( i1 & ( (intcon2->value.get() & (T0IF | RBIF)) | INTF) )
        || ( i2 & 2 ) )
        {
          set_interrupt_vector(INTERRUPT_VECTOR_HI);
          cpu_pic->BP_set_interrupt();
          return;
        }
        // If we reach here, then there are no high priority
        // interrupts pending. So let's check for the low priority
        // ones.
        if ( ( (i1 & (~intcon2->value.get() & (T0IF | RBIF)))
          || (i2 & 1) )  
        && (value.get() & GIEL) )
        {
          //cout << " selecting low priority vector\n";
          set_interrupt_vector(INTERRUPT_VECTOR_LO);
          cpu_pic->BP_set_interrupt();
          return;
        }
    }
    else
    {                                     // ignore interrupt priorities
        set_interrupt_vector(INTERRUPT_VECTOR_HI);

        if(value.get() & GIE &&  !in_interrupt) 
        {
            if( ( (value.get()>>3)&value.get()) & (T0IF | INTF | RBIF) )
                cpu_pic->BP_set_interrupt();
                
            else if(value.get() & XXIE)
            {
              if(check_peripheral_interrupt())
                cpu_pic->BP_set_interrupt();
            }
        }
    }
}

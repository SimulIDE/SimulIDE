/*
   Copyright (C) 1998 Scott Dattalo

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


#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "config.h"
#include "16bit-registers.h"
#include "16bit-processors.h"
//#include "interface.h"
#include "stimuli.h"

#include "clock_phase.h"

//--------------------------------------------------
// member functions for the FSR class
//--------------------------------------------------
//
FSRL::FSRL(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void  FSRL::put(uint new_value)
{
  value.put(new_value & 0xff);

  iam->update_fsr_value();
}

void  FSRL::put_value(uint new_value)
{
  put(new_value);

  update();
  cpu16->indf->update();
}

FSRH::FSRH(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void  FSRH::put(uint new_value)
{
  value.put(new_value & 0x0f);

  iam->update_fsr_value();
}

void  FSRH::put_value(uint new_value)
{

  put(new_value);

  update();
  cpu16->indf->update();
}

INDF16::INDF16(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void INDF16::put(uint new_value)
{
  iam->fsr_value += iam->fsr_delta;
  iam->fsr_delta = 0;
  iam->put(new_value);
}

void INDF16::put_value(uint new_value)
{
  put(new_value);
  update();
}

uint INDF16::get()
{
  iam->fsr_value += iam->fsr_delta;
  iam->fsr_delta = 0;

  return(iam->get());
}

uint INDF16::get_value()
{
  return(iam->get_value());
}

//------------------------------------------------
// PREINC
PREINC::PREINC(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

uint PREINC::get()
{
  iam->preinc_fsr_value();

  return(iam->get());
}

uint PREINC::get_value()
{
  return(iam->get_value());
}

void PREINC::put(uint new_value)
{
  iam->preinc_fsr_value();
  iam->put(new_value);
}

void PREINC::put_value(uint new_value)
{
  put(new_value);
  update();
}

//------------------------------------------------
// POSTINC
POSTINC::POSTINC(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

uint POSTINC::get()
{
  iam->postinc_fsr_value();
  return(iam->get());
}

uint POSTINC::get_value()
{
  return(iam->get_value());
}

void POSTINC::put(uint new_value)
{
  iam->postinc_fsr_value();
  iam->put(new_value);
}

void POSTINC::put_value(uint new_value)
{
  put(new_value);
  update();
}


//------------------------------------------------
// POSTDEC
POSTDEC::POSTDEC(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}
uint POSTDEC::get()
{
  iam->postdec_fsr_value();
  return(iam->get());
}

uint POSTDEC::get_value()
{
  return(iam->get_value());
}

void POSTDEC::put(uint new_value)
{
  iam->postdec_fsr_value();
  iam->put(new_value);
}

void POSTDEC::put_value(uint new_value)
{
  put(new_value);
  update();
}


//------------------------------------------------
// PLUSW
PLUSW::PLUSW(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

uint PLUSW::get()
{
  int destination = iam->plusw_fsr_value();
  if(destination >= 0)
    return (cpu_pic->registers[destination]->get());
  else
    return 0;
}

uint PLUSW::get_value()
{
  int destination = iam->plusw_fsr_value();
  if(destination >= 0)
    return (cpu_pic->registers[destination]->get_value());
  else
    return 0;
}

void PLUSW::put(uint new_value)
{
  int destination = iam->plusw_fsr_value();
  if(destination >= 0)
    cpu_pic->registers[destination]->put(new_value);
}

void PLUSW::put_value(uint new_value)
{
  int destination = iam->plusw_fsr_value();
  if(destination >= 0)
    cpu_pic->registers[destination]->put_value(new_value);

  update();
  if(destination >= 0)
    cpu_pic->registers[destination]->update();
}

//------------------------------------------------

Indirect_Addressing::Indirect_Addressing(pic_processor *pCpu, const string &n)
  : fsrl(pCpu, (string("fsrl")+n).c_str(), "FSR Low", this),
    fsrh(pCpu, (string("fsrh")+n).c_str(), "FSR High", this),
    indf(pCpu, (string("indf")+n).c_str(), "Indirect Register", this),
    preinc(pCpu, (string("preinc")+n).c_str(), "Pre Increment Indirect", this),
    postinc(pCpu, (string("postinc")+n).c_str(), "Post Increment Indirect", this),
    postdec(pCpu, (string("postdec")+n).c_str(), "Post Decrement Indirect", this),
    plusw(pCpu, (string("plusw")+n).c_str(), "Literal Offset Indirect", this)
{
  /*
  fsrl.iam = this;
  fsrh.iam = this;
  indf.iam = this;
  preinc.iam = this;
  postinc.iam = this;
  postdec.iam = this;
  plusw.iam = this;
  */
  current_cycle = (uint64_t)(-1);   // Not zero! See bug #3311944
  fsr_value = 0;
  fsr_state = 0;
  fsr_delta = 0;
  cpu = pCpu;

}

/*
 * put - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * write data.
 */
void Indirect_Addressing::put(uint new_value)
{
  /*  uint midbits;

  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return;
    }
  */
  if(is_indirect_register(fsr_value))
    return;

  cpu_pic->registers[get_fsr_value()]->put(new_value);

}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
uint Indirect_Addressing::get()
{
  //  uint midbits;

  // See the comment in Indirect_Addressing::put about fsr address checking
  if(is_indirect_register(fsr_value))
    return 0;
  else
    /*
  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return 0;
    }
    */

  return cpu_pic->registers[get_fsr_value()]->get();

}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
uint Indirect_Addressing::get_value()
{
  /*
  uint midbits;

  See the comment in Indirect_Addressing::put about fsr address checking

  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return 0;
    }
  */
  if(is_indirect_register(fsr_value))
    return 0;
  else
    return cpu_pic->registers[get_fsr_value()]->get_value();

}

void Indirect_Addressing::put_fsr(uint new_fsr)
{

  fsrl.put(new_fsr & 0xff);
  fsrh.put((new_fsr>>8) & 0x0f);

}


/*
 * update_fsr_value - This routine is called by the FSRL and FSRH
 * classes. It's purpose is to update the 16-bit (actually 12-bit)
 * address formed by the concatenation of FSRL and FSRH.
 *
 */

void Indirect_Addressing::update_fsr_value()
{

  if(current_cycle != get_cycles().get())
    {
      fsr_value = (fsrh.value.get() << 8) |  fsrl.value.get();
      fsr_delta = 0;
    }
}

/*
 * preinc_fsr_value - This member function pre-increments the current
 * fsr_value. If the preinc access is a read-modify-write instruction
 * (e.g. bcf preinc0,1 ) then the increment operation should occur
 * only once. 
 */

void Indirect_Addressing::preinc_fsr_value()
{

  if(current_cycle != get_cycles().get())
    {
      fsr_value += (fsr_delta+1);
      fsr_delta = 0;
      current_cycle = get_cycles().get();
      put_fsr(fsr_value);
    }

}

void Indirect_Addressing::postinc_fsr_value()
{

  if(current_cycle != get_cycles().get())
    {
      fsr_value += fsr_delta;
      fsr_delta = 1;
      current_cycle = get_cycles().get();
      put_fsr(fsr_value+1);
      
    }
}

void Indirect_Addressing::postdec_fsr_value()
{

  if(current_cycle != get_cycles().get())
    {
      fsr_value += fsr_delta;
      fsr_delta = -1;
      current_cycle = get_cycles().get();
      put_fsr(fsr_value-1);
      
    }

}

int Indirect_Addressing::plusw_fsr_value()
{

  fsr_value += fsr_delta;
  fsr_delta = 0;
  int signExtendedW = cpu_pic->Wreg->value.get() | ((cpu_pic->Wreg->value.get() > 127) ? 0xf00 : 0);
  uint destination = (fsr_value + signExtendedW) & _16BIT_REGISTER_MASK;
  if(is_indirect_register(destination))
    return -1;
  else
    return destination;

}

int Indirect_Addressing::plusk_fsr_value(int k)
{

  fsr_value += fsr_delta;
  fsr_delta = 0;
  uint destination = (fsr_value + k) & _16BIT_REGISTER_MASK;
  if(is_indirect_register(destination))
    return -1;
  else
    return destination;

}

//------------------------------------------------
void Fast_Stack::init(_16bit_processor *new_cpu)
{
  cpu = new_cpu;
}

void Fast_Stack::push()
{
  w = cpu->Wreg->value.get();
  status = cpu->status->value.get();
  bsr = cpu->bsr.value.get();

}

void Fast_Stack::pop()
{
  //cout << "popping fast stack\n";
  cpu->Wreg->put(w);
  cpu->status->put(status);
  cpu->bsr.put(bsr);

}
//--------------------------------------------------
// member functions for the PCL base class
//--------------------------------------------------
PCL16::PCL16(Processor *pCpu, const char *pName, const char *pDesc)
  : PCL(pCpu,pName,pDesc)
{
}


/*
   These get functions return the next PCL value rather than
   the current value.

   The get() will update the PCLATH and PCLATU registers
*/
uint PCL16::get()
{
  cpu_pic->pclath->value.put((cpu_pic->pc->get_value() >> 8) & 0xff);
  cpu16->pclatu.value.put((cpu_pic->pc->get_value() >> 16) & 0xff);
  value.put(cpu_pic->pc->get_value() & 0xff);
  return((value.get()+2) & 0xff);
}

uint PCL16::get_value()
{
  value.put(cpu_pic->pc->get_value() & 0xff);
  return((value.get()+2) & 0xff);

}


//--------------------------------------------------
// Program_Counter16
// The Program_Counter16 is almost identical to Program_Counter.
// The major difference is that the PC counts by 2 in the 16bit core.
Program_Counter16::Program_Counter16(Processor *pCpu)
  : Program_Counter("pc","Program Counter", pCpu)
{
}

//--------------------------------------------------
// computed_goto - update the program counter. Anytime the pcl register is written to
//                 by the source code we'll pass through here.
//
void Program_Counter16::computed_goto(uint new_address)
{
  // Use the new_address and the cached pclath
  // to generate the destination address:
  value = ( (new_address | cpu_pic->get_pclath_branching_modpcl() )>>1);

  if (value >= memory_size) value -= memory_size;

  update_pcl();

  // The instruction modifying the PCL will also increment the program counter.
  // So, pre-compensate the increment with a decrement:
  value--;

  // The computed goto is a 2-cycle operation. The first cycle occurs within 
  // the instruction (i.e. via the ::increment() method). The second cycle occurs
  // here:
  cpu_pic->mExecute2ndHalf->advance();
}

//--------------------------------------------------
// put_value - Change the program counter without affecting the cycle counter
//             (This is what's called if the user changes the pc.)

void Program_Counter16::put_value(uint new_value)
{
  // RP - The new_value passed in is a byte address, but the Program_Counter16
  // class's internal value is a word address
  value = new_value >> 1;
  if (value >= memory_size) value -= memory_size;

  cpu_pic->pcl->value.put(new_value & 0xfe);

// RP - removed these lines as setting the actual PC should not affect the latches
//  cpu_pic->pclath->value.put((new_value >> 8) & 0xff);
//  cpu16->pclatu.value.put((new_value >> 16) & 0xff);
  cpu_pic->pcl->update();
  cpu_pic->pclath->update();
  update();
}

uint Program_Counter16::get_value()
{
  return value << 1;
}

//--------------------------------------------------
// update_pcl - Updates the PCL from within the Program_Counter class.
//   
 
void Program_Counter16::update_pcl()
{
  // For 16 bit devices the PCL will be Program_Counter*2
  cpu_pic->pcl->value.put((value<<1) & 0xff);
}

//------------------------------------------------
// TOSU
TOSU::TOSU(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{}
uint TOSU::get()
{
  value.put((stack->get_tos() >> 16) & 0x1f);

  return(value.get());
}

uint TOSU::get_value()
{

  value.put((stack->get_tos() >> 16) & 0x1f);
  return(value.get());

}

void TOSU::put(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffe0ffff) | ( (new_value & 0x1f) << 16));
}

void TOSU::put_value(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffe0ffff) | ( (new_value & 0x1f) << 16));
  update();
}


//------------------------------------------------
// STKPTR
STKPTR16::STKPTR16(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{}
void STKPTR16::put_value(uint new_value)
{
  stack->pointer = new_value & stack->stack_mask;
  value.put(new_value);
  update();
}

void STKPTR16::put(uint new_value)
{
    put_value(new_value);
}

//--------------------------------------------------
//
Stack16::Stack16(Processor *pCpu) : Stack(pCpu), 
    stkptr(pCpu, "stkptr", "Stack pointer"),
    tosl(pCpu, "tosl", "Top of Stack low byte"),
    tosh(pCpu, "tosh", "Top of Stack high byte"),
    tosu(pCpu, "tosu", "Top of Stack upper byte")
{
  stkptr.stack = this;
  tosl.stack = this;
  tosh.stack = this;
  tosu.stack = this;

}

Stack16::~Stack16()
{

  pic_processor *pCpu = dynamic_cast<pic_processor *>(cpu);
  if (pCpu)
  {
    pCpu->remove_sfr_register(&stkptr);
    pCpu->remove_sfr_register(&tosl);
    pCpu->remove_sfr_register(&tosh);
    pCpu->remove_sfr_register(&tosu);
  }
}


// pop of empty stack sets undeflow and returns 0
uint Stack16::pop() 
{ 
    if(pointer <= 0)
    {
	pointer = 0;
	stack_underflow();
	return(0);
    }
    --pointer;
    uint stkptr_status = stkptr.value.get() & ~stack_mask;
    stkptr.value.put((pointer & stack_mask) | stkptr_status);
    return(contents[pointer & stack_mask] >> 1);
}
// When stack is full last(top) entry is overwritten
bool Stack16::push(uint address) 
{ 
    contents[pointer & stack_mask] = address << 1;
    if(pointer >= (int)stack_mask)
    {
	pointer = stack_mask;
	return stack_overflow();
    }
    pointer++;
    stkptr.value.put((pointer & stack_mask) | (stkptr.value.get() & ~stack_mask));
    return true;
}

void Stack16::reset(RESET_TYPE r)
{
  uint reg_value;

  if (r != POR_RESET && r != BOD_RESET)
	reg_value = stkptr.value.get() & ~stack_mask;
  else
	reg_value = 0;
  pointer = 0;
  stkptr.value.put( reg_value);
}
bool Stack16::stack_underflow()
{
    stkptr.value.put(STKPTR::STKUNF); // don't decrement past 0, signalize STKUNF
    if(STVREN)
    {
 	cpu->reset(STKUNF_RESET);
	return false;
    }
    cout <<"Stack undeflow\n";
    return true;
}
bool Stack16::stack_overflow()
{
    stkptr.value.put( STKPTR::STKOVF | (pointer & stack_mask));
    if(STVREN)
    {
	cpu->reset(STKOVF_RESET);
	return false;
    }
    cout << "Stack overflow\n";
    return true;
}

//--------------------------------------------------
// member functions for the RCON base class
//--------------------------------------------------
RCON::RCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{
}
//--------------------------------------------------
// member functions for the CPUSTA base class
//--------------------------------------------------
CPUSTA::CPUSTA(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{
}
//--------------------------------------------------
// member functions for the T0CON base class
//--------------------------------------------------
T0CON::T0CON(Processor *pCpu, const char *pName, const char *pDesc)
  : OPTION_REG(pCpu,pName,pDesc)
{
  por_value = RegisterValue(0xff,0);
  wdtr_value = RegisterValue(0xff,0);
}

void T0CON::put(uint new_value)
{
  uint old_value = value.get();

  value.put(new_value);

  if (new_value == old_value) return;

  // new_prescale causes issues in 16 bit mode, so save current tmr0l and
  // tmr0h values , call new_prescale (if required), and then restart timer
  // using saved values
  //
  uint initialTmr0value = (cpu16->tmr0l.value.get() & 0xff) |
        (( (value.get() & T08BIT)) ? 0: ((cpu16->tmr0h.value.get() & 0xff)<<8));


  cpu16->option_new_bits_6_7(value.get() & (BIT6 | BIT7));

  // %%%FIX ME%%% - can changing the state of TOSE cause the timer to
  // increment if tmr0 is being clocked by an external clock?
  // 
  if( (value.get() ^ old_value) & (T0CS | T0SE | PSA | PS2 | PS1 | PS0))
    cpu16->tmr0l.new_prescale();

  if(value.get() & TMR0ON) {
      cpu16->tmr0l.start(initialTmr0value);
  } else
      cpu16->tmr0l.stop();
}

//--------------------------------------------------
void T0CON::initialize()
{
//    cpu16->tmr0l.new_prescale();
    cpu16->wdt.set_postscale( (value.get() & PSA) ? (value.get() & ( PS2 | PS1 | PS0 )) : 0);
    cpu16->option_new_bits_6_7(value.get() & (T0CS | BIT6 | BIT7));

}

//--------------------------------------------------

TMR0H::TMR0H(Processor *pCpu, const char *pName, const char *pDesc)
  :sfr_register(pCpu,pName,pDesc)
{
}

//--------------------------------------------------
void TMR0H::put(uint new_value)
{
  value.put(new_value & 0xff);
}

//--------------------------------------------------
void TMR0H::put_value(uint new_value)
{
  value.put(new_value & 0xff);
}

uint TMR0H::get()
{
  return(value.get());
}

uint TMR0H::get_value()
{
  return(value.get());
}

//--------------------------------------------------
// TMR0_16 member functions
//
TMR0_16::TMR0_16(Processor *pCpu, const char *pName, const char *pDesc)
  : TMR0(pCpu,pName,pDesc),
    t0con(0), intcon(0), tmr0h(0), value16(0)
{
}
//--------------------------------------------------
// TMR0_16::get_prescale
//
//  If the prescaler is assigned to the WDT (and not TMR0)
//    then return 0
//  other wise
//    then return the Prescale select bits (plus 1)
//
uint TMR0_16::get_prescale()
{
  if(t0con->value.get() & 0x8)
    return 0;
  else
    return ((t0con->value.get() & 7) + 1);

}

void TMR0_16::set_t0if()
{
  intcon->set_t0if();
  if (m_t1gcon)
  {
      m_t1gcon->T0_gate(true);
      // Spec sheet does not indicate when the overflow signal
      // is cleared, so I am assuming it is just a pulse. RRR
      m_t1gcon->T0_gate(false);
  }
}

bool TMR0_16::get_t0cs()
{
 return (t0con->value.get() & 0x20) != 0;
}

void TMR0_16::initialize()
{
  t0con = &cpu16->t0con;
  intcon = &cpu16->intcon;
  tmr0h  = &cpu16->tmr0h;
}

uint TMR0_16::max_counts()
{

  if(t0con->value.get() & T0CON::T08BIT)
    return 0x100;
  else
    return 0x10000;

}
void TMR0_16::start(int restart_value, int sync)
{
  m_pOptionReg = t0con;
  TMR0::start(restart_value, sync);
}

void TMR0_16::put_value(uint new_value)
{
  value.put(new_value & 0xff);
  value16 = (new_value & 0xff) | (tmr0h ? (tmr0h->get_value()<<8)  : 0);

  if(t0con->value.get() & T0CON::TMR0ON) {
    if(t0con->value.get() & T0CON::T08BIT)
      TMR0::put_value(new_value);
    else
      start(value16);
  } else {
    // TMR0 is not enabled
  }
}


// %%%FIX ME%%% 
void TMR0_16::increment()
{
  if(--prescale_counter == 0)
    {
      prescale_counter = prescale;

      if(t0con->value.get() & T0CON::T08BIT)
	{
	  if(value.get() == 255)
	    {
	      value.put(0);
	      set_t0if();
	    }
	  else
	    value.put(value.get()+1);
	}
      else
	{
	  if(value.get() == 255)
	    {
	      value.put(0);
	      if(tmr0h->value.get() == 255)
		{
		  tmr0h->put(0);
		  set_t0if();
		}
	      else
		tmr0h->value.put(tmr0h->value.get()+1);

	    }
	  else
	    {
	      value.put(value.get()+1);
	    }
	}

    }
  //  cout << value << '\n';
}


uint TMR0_16::get_value()
{
  if(t0con->value.get() & T0CON::TMR0ON) {

    // If TMR0L:H is configured as an 8-bit timer, then treat as an 8-bit timer
    if(t0con->value.get() & T0CON::T08BIT) {
      if (tmr0h)
	tmr0h->put_value( (value16>>8)&0xff);

      return(TMR0::get_value());

    }
    value16 = (int) ((get_cycles().get() - last_cycle)/ prescale);

    value.put(value16 & 0xff);
  }
  return(value.get());
  
}

uint TMR0_16::get()
{
  get_value();

  if(t0con->value.get() & T0CON::T08BIT)
    return value.get();

  // reading the low byte of tmr0 latches in the high byte.
  tmr0h->put_value((value16 >> 8)&0xff);
  return value.get();
}

void TMR0_16::callback()
{

  //cout<<"_TMR0 rollover: " << hex << cycles.value << '\n';
  if((t0con->value.get() & T0CON::TMR0ON) == 0) {
    cout << " tmr0 isn't turned on\n";
    return;
  }

  TMR0::callback();   // Let the parent class handle the lower eight bits

  //Now handle the upper 8 bits:

  if(future_cycle &&
     !(t0con->value.get() & T0CON::T08BIT)) 
    {
      // 16-bit mode
      tmr0h->put_value(0);
    }


}

void TMR0_16::callback_print()
{
  cout << "TMR0_16 " << name() << " CallBack ID " << CallBackID << '\n';
}

void TMR0_16::sleep()
{
    if((state & RUNNING))
    {
        TMR0::stop();
        state = SLEEPING;
    }
}

void TMR0_16::wake()
{
    if ((state & SLEEPING))
    {
        if (! (state & RUNNING))
        {
            state = STOPPED;
            start(value.get(), 0);
        }
        else state &= ~SLEEPING;
    }
}

//--------------------------------------------------
// T3CON
T3CON::T3CON(Processor *pCpu, const char *pName, const char *pDesc)
  : T1CON(pCpu,pName,pDesc),
    ccpr1l(0),ccpr2l(0),tmr1l(0), t1con(0)
{}

void T3CON::put(uint new_value)
{
  int diff = (value.get() ^ new_value);

  if(diff & (T3CCP1 |  T3CCP2)) {
    switch(new_value & (T3CCP1 |  T3CCP2)) {
    case 0:
      ccpr1l->assign_tmr(tmr1l);   // Both CCP modules use TMR1 as their source
      ccpr2l->assign_tmr(tmr1l);
      break;
    case T3CCP1:
      ccpr1l->assign_tmr(tmr1l);   // CCP1 uses TMR1
      ccpr2l->assign_tmr(tmrl);    // CCP2 uses TMR3
      break;
    default:
      ccpr1l->assign_tmr(tmrl);    // Both CCP modules use TMR3 as their source
      ccpr2l->assign_tmr(tmrl);
    } 
  }

  // Let the T1CON class deal with everything else.
  T1CON::put(new_value  & ~(T3CCP1 |  T3CCP2));

}

//--------------------------------------------------
// TMR3_MODULE
//
// 

TMR3_MODULE::TMR3_MODULE()
{

  t3con = 0;
  pir_set = 0;

}

void TMR3_MODULE::initialize(T3CON *t3con_, PIR_SET *pir_set_)
{

  t3con = t3con_;
  pir_set  = pir_set_;

}


//-------------------------------------------------------------------
//
//  Table Reads and Writes
//
// The 18cxxx family provides a peripheral that will allow the program
// memory to read and write to itself. 
//
//-------------------------------------------------------------------

TBL_MODULE::TBL_MODULE(_16bit_processor *pCpu)
  : EEPROM_EXTND(pCpu, 0), cpu(pCpu),
    tablat(pCpu,"tablat"),
    tblptrl(pCpu,"tblptrl"),
    tblptrh(pCpu,"tblptrh"),
    tblptru(pCpu,"tblptru")
{
}

//void TBL_MODULE::initialize(_16bit_processor *new_cpu)
//{
//  cpu = new_cpu;
//}

//-------------------------------------------------------------------
//  void TBL_MODULE::increment()
//
//  This function increments the 24-bit ptr that is formed by the
// concatenation of tabptrl,tabptrh, and tabptru. It is called by
// the TBLRD and TBLWT pic instructions when the auto-increment
// operand is specified (e.g. TBLWT *+ )
//
//
// Inputs:  none
// Outputs: none
//
//-------------------------------------------------------------------
void TBL_MODULE::increment()
{

  if(tblptrl.value.get() >= 0xff) {
    tblptrl.put(0);
    if(tblptrh.value.get() >= 0xff) {
      tblptrh.put(0);
      tblptru.put(tblptru.value.get() + 1);
    } else {
      tblptrh.put(tblptrh.value.get() + 1);
    }
  }
  else
    tblptrl.put(tblptrl.value.get() + 1);


}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::decrement()
{

  if(tblptrl.value.get() == 0) {
    tblptrl.put(0xff);
    if(tblptrh.value.get() == 0) {
      tblptrh.put(0xff);
      tblptru.put(tblptru.value.get() - 1);
    } else {
      tblptrh.put(tblptrh.value.get() - 1);
    }
  }
  else
    tblptrl.put(tblptrl.value.get() - 1);

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::read()
{
  uint tblptr,opcode;

  // tblptr is 12 bit address pointer
  tblptr = 
    ( (tblptru.value.get() & 0xff) << 16 ) |
    ( (tblptrh.value.get() & 0xff) << 8 )  |
    ( (tblptrl.value.get() & 0xff) << 0 );

  // read 16 bits of program memory from even address
  opcode = cpu_pic->pma->get_rom(tblptr & 0xfffffe);

  // return high or low byte depending on lsb of address
  if(tblptr & 1)
    {
      tablat.put((opcode >> 8) & 0xff);
      internal_latch = (internal_latch & 0x00ff) | (opcode & 0xff00);
    }
  else
    {
      tablat.put((opcode >> 0) & 0xff);
      internal_latch = (internal_latch & 0xff00) | (opcode & 0x00ff);
    }

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::write()
{

  uint tblptr;
  uint latch_index;
  uint *pt;

  tblptr = 
    ( (tblptru.value.get() & 0xff) << 16 ) |
    ( (tblptrh.value.get() & 0xff) << 8 )  |
    ( (tblptrl.value.get() & 0xff) << 0 );

  latch_index = (tblptr >> 1) % num_write_latches;
  pt = &write_latches[latch_index];

  if(tblptr & 1)
    {
      *pt = (*pt & 0x00ff) | ((tablat.value.get()<<8) & 0xff00);
    }
  else
    {
      *pt = (*pt & 0xff00) | (tablat.value.get() & 0x00ff);
    }
}

void TBL_MODULE::start_write()
{
  eecon1.value.put( eecon1.value.get()  | eecon1.WRERR);

  if (eecon1.value.get() & (EECON1::EEPGD|EECON1::CFGS))
  {
      int index;
      wr_adr  =
          ( (tblptru.value.get() & 0xff) << 16 ) |
          ( (tblptrh.value.get() & 0xff) << 8 )  |
          ( (tblptrl.value.get() & 0xff) << 0 );
      wr_adr = cpu->map_pm_address2index(wr_adr);
      index = wr_adr % num_write_latches;
      wr_data = write_latches[index];

      eecon2.start_write();

      // stop execution fo 2 ms
      get_cycles().set_break(get_cycles().get() + (uint64_t)(.002*get_cycles().instruction_cps()), this);
      bp.set_pm_write();
      cpu_pic->pm_write();
  }
  else
  {
      get_cycles().set_break(get_cycles().get() + EPROM_WRITE_TIME, this);
      wr_adr = eeadr.value.get() + (eeadrh.value.get() << 8);
      wr_data = eedata.value.get() + (eedatah.value.get() << 8);
      eecon2.start_write();
  }
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
LVDCON::LVDCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName,pDesc),
    valid_bits(0x3f)
{
}


/*******************************************************************
	HLVDCON - High/Low-Voltage Detect Module
*/

HLVD_stimulus::HLVD_stimulus(HLVDCON *_hlvd, const char *cPname):
  stimulus(cPname, 2.5, 1e12), hlvd(_hlvd)
{
}
HLVD_stimulus::~HLVD_stimulus() 
{
}
void HLVD_stimulus::set_nodeVoltage(double v)
{
    nodeVoltage = v;
    hlvd->check_hlvd();
}

HLVDCON::HLVDCON(Processor *pCpu, const char *pName, const char *pDesc) :
	sfr_register(pCpu, pName, pDesc), hlvdin(0), hlvdin_stimulus(0),
	stimulus_active(false),write_mask(0x9f), IntSrc(0)
  {}
HLVDCON::~HLVDCON()
{
    if (IntSrc)
	delete IntSrc;
    if (stimulus_active)
    {
        hlvdin->getPin().snode->detach_stimulus(hlvdin_stimulus);
        stimulus_active = false;
     }
     if (hlvdin_stimulus)
	delete hlvdin_stimulus;
}

void HLVDCON::put(uint new_value)
{
    double tivrst = 20e-6;	// typical time for IVR stable
    uint diff = value.get() ^ new_value;

    if (!diff) return;

    if (diff & HLVDEN) 
    {
        if (new_value & HLVDEN)	// Turning on
	{
	    // wait tivrst before doing anything
	    value.put(new_value & write_mask);
	    get_cycles().set_break(
		get_cycles().get() + tivrst * get_cycles().instruction_cps(),
		this);
	    return;
	}
	else	// Turning off
	{
	    value.put(new_value & write_mask);
	    if (stimulus_active)
	    {
	        hlvdin->getPin().snode->detach_stimulus(hlvdin_stimulus);
	        stimulus_active = false;
            }
	    return;
        }
    }
    value.put((new_value & write_mask) | (value.get() & ~write_mask));
    if (!(value.get() & IRVST))		// Just return if voltage not stable
        return;
    check_hlvd();

}
void HLVDCON::callback()
{
	uint reg = value.get();

	reg |= (BGVST | IRVST);
	value.put(reg);
	check_hlvd();
}

double hldv_volts[] = { 1.84, 2.07, 2.28, 2.44, 2.54, 2.74, 2.87, 3.01,
			3.30, 3.48, 3.69, 3.91, 4.15, 4.41, 4.74};
void HLVDCON::check_hlvd()
{
    uint reg = value.get();

    assert(IntSrc);
    assert(hlvdin);
    if (!(reg & IRVST))
	return;
    if ((reg & HLVDL_MASK) == HLVDL_MASK)	// using HLVDIN pin
    {
	if (!hlvdin_stimulus)
	    hlvdin_stimulus = new HLVD_stimulus(this, "hlvd_stim");

        if (!stimulus_active && hlvdin->getPin().snode)
	{
	    hlvdin->getPin().snode->attach_stimulus(hlvdin_stimulus);
	    stimulus_active = true;
	    hlvdin->getPin().snode->update();
	}
	double voltage = hlvdin->getPin().get_nodeVoltage();
	// High voltage trip ?
	if ((reg & VDIRMAG) && (voltage >= 1.024))
	{
	       IntSrc->Trigger();
	}

	// Low voltage trip ?
	else if (!(reg & VDIRMAG) && (voltage <= 1.024))
	       IntSrc->Trigger();
    }
    else	// Voltage divider on Vdd
    {
	double voltage = hldv_volts[reg & HLVDL_MASK];
	Processor *Cpu = (Processor *)cpu;
	if ((reg & VDIRMAG) && (Cpu->get_Vdd() >= voltage))
	       IntSrc->Trigger();
	else if (!(reg & VDIRMAG) && (Cpu->get_Vdd() <= voltage))
	       IntSrc->Trigger();
    }
}



    

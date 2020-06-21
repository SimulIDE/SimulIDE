/*
   Copyright (C) 1998-2000 Scott Dattalo
   Copyright (C) 2013-2017 Roy R. Rankin

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
#include <assert.h>


#include "config.h"
#include "14bit-processors.h"
#include "14bit-registers.h"
#include "psp.h"     // needed for operator[] on WPU::wpu_gpio (not sure why)
#include "14bit-tmrs.h"

#include <string>
#include "stimuli.h"

//S#include "xref.h"
#define PCLATH_MASK              0x1f

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("0x%06" PRINTF_GINT64_MODIFIER "X %s() ", cycles.get(), __FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

// Debug OSCCON
//#define CDEBUG
#if defined(CDEBUG)
#define CDprintf(arg) {printf("0x%06" PRINTF_GINT64_MODIFIER "X %s() ", cycles.get(), __FUNCTION__); printf arg; }
#else
#define CDprintf(arg) {}
#endif

pic_processor *temp_cpu;
// FIXME file_register::put_value has a useful feature...

//
#if 0
//-----------------------------------------------------------
//  void file_register::put_value(uint new_value)
//
//  put_value is used by the gui to change the contents of
// file registers. We could've let the gui use the normal
// 'put' member function to change the contents, however
// there are instances where 'put' has a cascading affect.
// For example, changing the value of an i/o port's tris
// could cause i/o pins to change states. In these cases,
// we'd like the gui to be notified of all of the cascaded
// changes. So rather than burden the real-time simulation
// with notifying the gui, I decided to create the 'put_value'
// function instead.
//   Since this is a virtual function, derived classes have
// the option to override the default behavior.
//
// inputs:
//   uint new_value - The new value that's to be
//                            written to this register
// returns:
//   nothing
//
//-----------------------------------------------------------

void file_register::put_value(uint new_value)
{

  // go ahead and use the regular put to write the data.
  // note that this is a 'virtual' function. Consequently,
  // all objects derived from a file_register should
  // automagically be correctly updated.

  put(new_value);

  // Even though we just wrote a value to this register,
  // it's possible that the register did not get fully
  // updated (e.g. porta on many pics has only 5 valid
  // pins, so the upper three bits of a write are meaningless)
  // So we should explicitly tell the gui (if it's
  // present) to update its display.

  if(xref)
    {
      xref->update();

      if(cpu && address == cpu_pic->fsr->value)
        {
          if(cpu_pic->indf->xref)
            cpu_pic->indf->xref->update();
        }
    }
}

#endif

//--------------------------------------------------
// member functions for the BORCON class
// currently does not do anything
//--------------------------------------------------
//
BORCON::BORCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{
}

void  BORCON::put(uint new_value)
{
  value.put(new_value & 0x80);
}

void  BORCON::put_value(uint new_value)
{
  put(new_value&0x80);
}

//--------------------------------------------------
// member functions for the BSR class
//--------------------------------------------------
//
BSR::BSR(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc),
    register_page_bits(0)
{
}

void  BSR::put(uint new_value)
{
  value.put(new_value & 0x01f);
  //value.put(new_value & 0x01f);
  if(cpu_pic->base_isa() == _14BIT_E_PROCESSOR_)
      cpu_pic->register_bank = &cpu_pic->registers[ value.get() << 7 ];
  else
      cpu_pic->register_bank = &cpu_pic->registers[ value.get() << 8 ];
}

void  BSR::put_value(uint new_value)
{
  put(new_value);

  update();
  cpu_pic->indf->update();
}

void  IOCxF::put(uint new_value)
{
  uint masked_value = new_value & mValidBits;
  Dprintf((" %s value %x masked %x\n", name().c_str(), new_value, masked_value));

  value.put(masked_value);
  if (intcon)
  {
    ((INTCON_14_PIR *)intcon)->set_rbif(masked_value != 0);
    ((INTCON_14_PIR *)intcon)->aocxf_val(this, masked_value);
  }
}

// Adjust internal RC oscillator frequency as per 12f675/629
// Spec sheet does not give range so assume +/- 12.5% as per 16f88
// The fact that base_freq is not 0. indicates the RC oscillator is being used
// and thus an adjustment should be made.
//
// This will work for any number of adjustment bits in byte but must be left justified
// and 1000000 centre frequency and 11111111 highest frequency
void  OSCCAL::put(uint new_value)
{
  int   adj = new_value & mValidBits;
  float tune;

  value.put(adj);
  if (base_freq > 0.)
  {
    adj  = adj -  0x80;
    // A hook to honour configured frequency - if we're going to change it now
    if(cpu_pic->get_frequency() > base_freq*0.875 && base_freq*1.125 > cpu_pic->get_frequency()) {
      base_freq=cpu_pic->get_frequency();
    }
    tune = (1. + 0.125 * adj / 0x80) * base_freq;
    cpu_pic->set_frequency(tune);
  }
}

void OSCCAL::set_freq(float new_base_freq)
{
  base_freq = new_base_freq;
  put(value.get());
}

void  OSCTUNE::put(uint new_value)
{
  value.put(new_value);
  osccon->set_rc_frequency();
}
// Clock is stable
void OSCCON::callback()
{
    uint new_value = value.get();

    if (future_cycle <= get_cycles().get())
        future_cycle = 0;
    CDprintf(("OSCCON clock_state=%u\n", clock_state));
    switch(clock_state)
    {
    case OST:
      CDprintf(("OSCCON switch clock\n"));
      if (has_iofs_bit)
            new_value &= ~IOFS;
      else
            new_value &= ~(HTS|LTS);
      new_value |= OSTS;
      value.put(new_value);
      clock_state = EXCSTABLE;
      cpu_pic->set_RCfreq_active(false);
      return;

    case LFINTOSC:
      if (has_iofs_bit)
        new_value |= IOFS;
      else
      {
        new_value &= ~HTS;
        new_value |= LTS;
      }
      value.put(new_value);
      CDprintf(("OSCCON HF osccon=0x%x\n", value.get()));
      return;

    case HFINTOSC:
      if (!has_iofs_bit) new_value &= ~LTS;
      new_value |= HTS;
      value.put(new_value);
      CDprintf(("OSCCON HF osccon=0x%x\n", value.get()));
      return;

    case INTOSC:
      new_value |= IOFS;
      value.put(new_value);
      return;

    case EXCSTABLE:
      if (!has_iofs_bit) new_value &= ~LTS;
      new_value &= ~HTS;
      value.put(new_value);
      return;

    default:
      fprintf(stderr, "OSCCON::callback unexpexted clock state %u\n", clock_state);
      return;
    }
}

// Is internal RC clock selected?
bool OSCCON::internal_RC()
{
    uint scs = (value.get() & (SCS0|SCS1)) & write_mask;
    bool ret = false;
    if (scs == 0 && config_irc)
        ret = true;
    else if ((SCS1 & write_mask) && scs == 2)        // using SCS1 and SCS0
        ret = true;
    else if (scs == 1)
        ret = true;

    CDprintf(("OSCCON internal_RC ret %d osccon=0x%x\n", ret, value.get()));
    return ret;
}

void OSCCON::reset(RESET_TYPE r)
{

  switch(r) {
  case POR_RESET:

    value.put(por_value.data);
    por_wake();
    break;

  default:
    // Most registers simply retain their value across WDT resets.
    if (wdtr_value.initialized())
    {
        putRV(wdtr_value);
    }
    break;
  }
}

void OSCCON::sleep()
{
    is_sleeping = true;
}

void OSCCON::wake()
{
    if (!is_sleeping)
        return;
    is_sleeping = false;
    CDprintf(("OSCCON config_ieso %d int RC %d two_speed_clock=%d cpu=%s\n", config_ieso, internal_RC(), (config_xosc  && config_ieso), cpu_pic->name().c_str()));
    por_wake();
}

void OSCCON::por_wake()
{
    bool two_speed_clock = config_xosc  && config_ieso;
    uint new_value = value.get();

    CDprintf(("OSCCON config_xosc=%d config_ieso=%d\n", config_xosc,config_ieso));

    CDprintf(("OSCCON POR two_speed_clock=%d f=%4.1e osccon=0x%x por_value=0x%x\n", two_speed_clock, cpu_pic->get_frequency(), new_value, por_value.data));

    if (future_cycle)
    {
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    // internal RC osc
    if (internal_RC())
    {
        if (has_iofs_bit)
        {
            new_value &= ~IOFS;
            clock_state = INTOSC;
        }
        else if (new_value & (IRCF0 | IRCF1 | IRCF2 ))
        {
            new_value &= ~(HTS|LTS);
            clock_state = HFINTOSC;
        }
        else
        {
            new_value &= ~(HTS|LTS);
            clock_state = LFINTOSC;
        }
        new_value |= OSTS;
        value.put(new_value);
        CDprintf(("OSCCON internal RC clock_state %u osccon %x\n", clock_state, new_value));
//RRR        set_rc_frequency();
        if (future_cycle)
            get_cycles().clear_break(future_cycle);
        future_cycle = get_cycles().get() + irc_por_time();
        get_cycles().set_break(future_cycle, this);
        return;
    }
    if (two_speed_clock)
    {
        if (has_iofs_bit)
            new_value &= ~(IOFS | OSTS);
        else
            new_value &= ~( HTS | LTS | OSTS);
        value.put(new_value);
        set_rc_frequency(true);
        CDprintf(("OSCCON  2 speed, set osccon 0x%x \n", value.get()));
        clock_state = OST;
        future_cycle = 1024 + get_cycles().get();
        get_cycles().set_break(future_cycle, this);
        return;
    }
}
/*
 *
 *
 */
bool OSCCON::set_rc_frequency(bool override)
{
    double base_frequency = 31.e3;
    uint old_clock_state = clock_state;

    uint new_IRCF = (value.get() & ( IRCF0 | IRCF1 | IRCF2 )) >> 4;
    if (!internal_RC() && !override) return false;

    switch (new_IRCF)
    {
    case 0:
        base_frequency = 31.e3;
        break;

    case 1:
        base_frequency = 125e3;
        break;

    case 2:
        base_frequency = 250e3;
        break;

    case 3:
        base_frequency = 500e3;
        break;

    case 4:
        base_frequency = 1e6;
        break;

    case 5:
        base_frequency = 2e6;
        break;

    case 6:
        base_frequency = 4e6;
        break;

    case 7:
        base_frequency = 8e6;
        break;
   }
   if (osctune)
   {
       int tune;
       uint osctune_value = osctune->value.get();
       tune = osctune_value & (OSCTUNE::TUN5-1);
       tune = (OSCTUNE::TUN5 & osctune_value) ? -tune : tune;
       base_frequency *= 1. + 0.125 * tune / 31.;
   }
   cpu_pic->set_RCfreq_active(true);
   cpu_pic->set_frequency_rc(base_frequency);
   clock_state = new_IRCF ? HFINTOSC : LFINTOSC;
   if (old_clock_state != clock_state)
   {
        if ((old_clock_state == LFINTOSC) && clock_state != LFINTOSC)
        {
            if (has_iofs_bit)
               value.put(value.get() & ~(IOFS));
            else
               value.put(value.get() & ~(LTS|HTS));
            if (future_cycle) get_cycles().clear_break(future_cycle);

            future_cycle = get_cycles().get() + irc_lh_time();
            get_cycles().set_break(future_cycle, this);
            CDprintf(("OSCCON future_cycle %" PRINTF_GINT64_MODIFIER "d now %" PRINTF_GINT64_MODIFIER "d\n", future_cycle, get_cycles().get()));
        }
        else
        {
/*RRR
            uint current = value.get();
            if (clock_state == HFINTOSC)
            {
                if (has_iofs_bit) current &= ~LTS;
                current |= HTS;
                value.put(current);
            }
*/
            callback();
        }
   }
   CDprintf(("OSCCON new_ircf %u  %4.1f \n", new_IRCF, cpu_pic->get_frequency()));

   return true;
}

void  OSCCON::put(uint new_value)
{
  uint org_value = value.get();
  new_value = (new_value & write_mask) | (org_value & ~write_mask);
  value.put(new_value);
  uint diff = (new_value ^ org_value);

  value.put(new_value);
  CDprintf(("OSCCON org_value=0x%02x new_value=0x%02x diff=0x%02x state %u\n",
        org_value, new_value, diff, clock_state));
  if (diff == 0) return;

  if(internal_RC())
  {
#ifdef CDEBUG
      uint old_clock_state = clock_state;
#endif

      if ((diff & (IRCF0 | IRCF1 | IRCF2))) // freq change
      {
          set_rc_frequency();
          CDprintf(("OSCCON change of IRCF old_clock %u new_clock %u\n", old_clock_state, clock_state));
      }
        // switching to intrc
      else if (diff & (SCS0 | SCS1)) // still OK if SCS1 is non-writtabe LTS
      {
           set_rc_frequency(true);
          CDprintf(("OSCCON diff 0x%x old_clock_state %u clock_state %u\n", (diff & (SCS0 | SCS1)), old_clock_state, clock_state));
      }
  }
  else  // not Internal RC clock
  {
        clock_state = EXCSTABLE;
           cpu_pic->set_RCfreq_active(false);
        callback();
      CDprintf(("OSCCON not RC osccon=0x%x\n", new_value));
  }
}

// Time required for stable clock after transition between high and low
// irc frequencies
uint64_t OSCCON::irc_lh_time()
{
    uint64_t delay = (get_cycles().instruction_cps() * 1e-6) + 1;
    return delay;
}
// Time required for stable irc clock after POR
uint64_t OSCCON::irc_por_time()
{
    return (uint64_t) 2;
}

uint64_t OSCCON_1::irc_lh_time()
{
    uint64_t delay = get_cycles().instruction_cps() * 4e-3;
    CDprintf(("OSCCON_1 LH irc time 4ms %" PRINTF_GINT64_MODIFIER "d cycles\n", delay));
    return delay;
}
// Time required for stable irc clock after POR (4 ms)
uint64_t OSCCON_1::irc_por_time()
{
    uint64_t delay = get_cycles().instruction_cps() * 4e-3;
    CDprintf(("OSCCON_1 POR irc time 4ms %" PRINTF_GINT64_MODIFIER "d cycles\n", delay));
    return delay;
}

// Clock is stable
void OSCCON_2::callback()
{
    uint add_bits = 0;
    uint val;

    future_cycle = 0;

    if (!oscstat) return;

    val = oscstat->value.get();

    CDprintf(("OSCCON_2 oscstat = 0x%x\n", val));

    if (clock_state & PLL)
        add_bits = OSCSTAT::PLLR;

    switch(clock_state & ~PLL)
    {
    case OST:
        add_bits = OSCSTAT::OSTS;
        cpu_pic->set_RCfreq_active(false);
        break;

    case LFINTOSC:
        add_bits = OSCSTAT::LFIOFR;
        val &= ~(OSCSTAT::HFIOFL | OSCSTAT::HFIOFR  | OSCSTAT::HFIOFS | OSCSTAT::MFIOFR);
        break;

    case MFINTOSC:
        add_bits = OSCSTAT::MFIOFR;
        val &= ~(OSCSTAT::HFIOFL | OSCSTAT::HFIOFR  | OSCSTAT::HFIOFS | OSCSTAT::LFIOFR);
        break;

    case HFINTOSC:
        add_bits = OSCSTAT::HFIOFL | OSCSTAT::HFIOFR  | OSCSTAT::HFIOFS;
        val &= ~(OSCSTAT::MFIOFR | OSCSTAT::LFIOFR);
        break;

    case T1OSC:
        break;
    }
    val |= add_bits;
    oscstat->value.put(val);
}
bool OSCCON_2::set_rc_frequency(bool override)
{
  double base_frequency = 31.25e3;
  uint sys_clock = value.get() & (SCS0 | SCS1);
  bool osccon_pplx4 = value.get() & SPLLEN;
  bool config_pplx4 = cpu_pic->get_pplx4_osc();

  CDprintf(("OSCCON_2 new_IRCF 0x%x\n", (value.get() & ( IRCF0 | IRCF1 | IRCF2 |IRCF3)) >> 3));



  if ((sys_clock == 0) && !config_irc) // Not internal oscillator
  {
        if (!config_xosc ) // always run at full speed
        {

              uint oscstat_reg = (oscstat->value.get() & 0x1f);
            oscstat->value.put(oscstat_reg | OSCSTAT::OSTS);
            clock_state = EC;
        }
        else if (config_ieso) // internal/external switchover
        {
            clock_state = OST;
        }
  }

  if((osccon_pplx4 && !config_pplx4) && sys_clock == 0)
  {
        clock_state |= PLL;
        return true;
  }
  if (!cpu_pic->get_int_osc() && (sys_clock == 0) && !override)
     return false;

  if (sys_clock == 1) // T1OSC
  {
        base_frequency = 32.e3;
        clock_state = T1OSC;
  }
  else if (sys_clock > 1 || config_irc || override)
  {
    uint new_IRCF = (value.get() & ( IRCF0 | IRCF1 | IRCF2 |IRCF3)) >> 3;
    switch (new_IRCF)
    {
    case 0:
    case 1:
        base_frequency = 30.e3;
        clock_state = LFINTOSC;
        break;

    case 2:
        clock_state = MFINTOSC;
        base_frequency = 31.25e3;
        break;

    case 3:
        clock_state = HFINTOSC;
        base_frequency = 31.25e3;
        break;

    case 4:
        clock_state = HFINTOSC;
        base_frequency = 62.5e3;
        break;

    case 5:
        clock_state = HFINTOSC;
        base_frequency = 125e3;
        break;

    case 6:
        clock_state = HFINTOSC;
        base_frequency = 250e3;
        break;

    case 7:
        clock_state = HFINTOSC;
        base_frequency = 500e3;
        break;

    case 8:
        clock_state = HFINTOSC;
        base_frequency = 125e3;
        break;

    case 9:
        clock_state = HFINTOSC;
        base_frequency = 250e3;
        break;

    case 10:
        clock_state = HFINTOSC;
        base_frequency = 500e3;
        break;

    case 11:
        clock_state = HFINTOSC;
        base_frequency = 1e6;
        break;

    case 12:
        clock_state = HFINTOSC;
        base_frequency = 2e6;
        break;

    case 13:
        clock_state = HFINTOSC;
        base_frequency = 4e6;
        break;

    case 14:
        // The treatment for PPL based on Fig 5-1 of P12f1822 ref manual
        if (osccon_pplx4 || config_pplx4)
        {
           clock_state = PLL;
           base_frequency = 32e6;
        }
        else
        {
           clock_state = HFINTOSC;
           base_frequency = 8e6;
        }
        break;

    case 15:
        clock_state = HFINTOSC;
        base_frequency = 16e6;
        break;
    }
   }
   if (osctune)
   {
       int tune;
       uint osctune_value = osctune->value.get();
       tune = osctune_value & (OSCTUNE::TUN5-1);
       tune = (OSCTUNE::TUN5 & osctune_value) ? -tune : tune;
       base_frequency *= 1. + 0.125 * tune / 31.;
   }
   cpu_pic->set_RCfreq_active(true);
   cpu_pic->set_frequency_rc(base_frequency);

   return true;
}

void OSCCON_2::por_wake()
{
    bool two_speed_clock = config_xosc  && config_ieso;

    CDprintf(("OSCCON_2 two_speed_clock=%d f=%4.1e\n", two_speed_clock, cpu_pic->get_frequency()));

    if (future_cycle)
    {
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
        clock_state = UNDEF;
    }
    // internal RC osc
    if (internal_RC())
    {
        CDprintf(("OSCCON_2 internal RC clock_state %u\n", clock_state));
        oscstat->value.put(OSCSTAT::OSTS);
        set_rc_frequency();
        future_cycle = get_cycles().get() + irc_por_time();
        get_cycles().set_break(future_cycle, this);
        return;
    }
    if (two_speed_clock)
    {
        bool config_pplx4 = cpu_pic->get_pplx4_osc();
        oscstat->value.put(0);
        set_rc_frequency(true);
        clock_state = OST;
        if (config_pplx4)
             clock_state |= PLL;
        CDprintf(("OSCCON_2  2 speed, set osccon 0x%x \n", value.get()));
        future_cycle = 1024 + get_cycles().get();
        get_cycles().set_break(future_cycle, this);
        return;
    }
    oscstat->value.put(0);
}
void  OSCCON_2::put_value(uint new_value)
{

    CDprintf(("OSCCON_2 0x%x\n", new_value));
    value.put(new_value);
}
void  OSCCON_2::put(uint new_value)
{

  uint old_value = value.get();
  new_value = (new_value & write_mask);
  uint oscstat_reg = 0;
  uint oscstat_new = 0;

  value.put(new_value);

  if (old_value == new_value) return;

  assert(oscstat);

  oscstat_reg = oscstat->value.get();
  oscstat_new = oscstat_reg;
        if (((new_value & (SCS0 | SCS1))==0) && !cpu_pic->get_int_osc())
                oscstat_new |= OSCSTAT::OSTS;
        else
                oscstat_new &= ~OSCSTAT::OSTS;

  CDprintf(("OSCCON_2 0x%x\n", new_value));


  if (set_rc_frequency())  // using internal RC Oscillator
        set_callback();
}

void OSCCON_2::set_callback()
{
        uint oscstat_reg = oscstat->value.get();;
        uint oscstat_new = oscstat_reg;
        uint64_t settle = 0;

        CDprintf(("OSCCON_2 clock_state 0x%x\n", clock_state));

        switch(clock_state &~ PLL)
        {
        case LFINTOSC:
                oscstat_new &= ~(OSCSTAT::OSTS | OSCSTAT::PLLR | OSCSTAT::T1OSCR);
                settle = get_cycles().get() + 2;
                break;

        case MFINTOSC:
                oscstat_new &= ~(OSCSTAT::OSTS | OSCSTAT::PLLR | OSCSTAT::T1OSCR);
                settle = get_cycles().get(2e-6); // 2us settle time
                break;

        case HFINTOSC:
                oscstat_new &= ~(OSCSTAT::OSTS | OSCSTAT::PLLR | OSCSTAT::T1OSCR);
                settle = get_cycles().get(2e-6); // 2us settle time
                CDprintf(("OSCCON_2 settle %" PRINTF_GINT64_MODIFIER "d\n", settle));
                break;

        case T1OSC:
                settle = get_cycles().get() + 1024/4;
                break;
        }
          if((clock_state & PLL) && (oscstat_reg & OSCSTAT::PLLR) == 0)
                settle = get_cycles().get(2e-3); // 2ms

        if (settle)
        {
                settle += get_cycles().get();
                if (future_cycle > get_cycles().get())
                    get_cycles().clear_break(future_cycle);

                get_cycles().set_break(settle, this);
                future_cycle = settle;
        }
        if(oscstat && (oscstat_new != oscstat_reg))
                oscstat->put(oscstat_new);
}

void OSCCON2::put(uint new_value)
{
  new_value = (new_value & write_mask) | (new_value & ~write_mask);
  value.put(new_value);
  assert(osccon);
  osccon->set_rc_frequency();
}

void OSCCON_HS::callback()
{
    assert(osccon2);
    uint val_osccon2 = osccon2->value.get();
    uint val_osccon = value.get();

    if (future_cycle <= get_cycles().get())
        future_cycle = 0;
    CDprintf(("OSCCON_HS clock_state=%u osccon=0x%x osccon2=0x%x\n", clock_state, val_osccon, val_osccon2));
    switch(clock_state)
    {
   case OST:
        val_osccon &= ~ HFIOFS;
        val_osccon |= OSTS;
        val_osccon2 &= ~(OSCCON2::LFIOFS | OSCCON2::MFIOFS);
        cpu_pic->set_RCfreq_active(false);
        clock_state = EXCSTABLE;
        break;

    case LFINTOSC:
        val_osccon &= ~HFIOFS;
        val_osccon2 &= ~OSCCON2::MFIOFS;
        val_osccon2 |= OSCCON2::LFIOFS;
        break;

    case MFINTOSC:
        val_osccon &= ~HFIOFS;
        val_osccon2 &= ~OSCCON2::LFIOFS;
        val_osccon2 |= OSCCON2::MFIOFS;
        break;

    case HFINTOSC:
        val_osccon |= HFIOFS;
        val_osccon2 &= ~(OSCCON2::LFIOFS|OSCCON2::MFIOFS);
        break;

    case T1OSC:
        break;

    case EXCSTABLE:
        val_osccon &= ~HFIOFS;
        val_osccon |= OSTS;
        val_osccon2 &= ~(OSCCON2::LFIOFS|OSCCON2::MFIOFS);
        break;
    }
    value.put(val_osccon);
    CDprintf(("OSCCON_HS osccon 0x%x val_osccon 0x%x\n", value.get(), val_osccon));
    osccon2->value.put(val_osccon2);
}
bool OSCCON_HS::set_rc_frequency(bool override)
{
  double base_frequency = 31.e3;
  bool config_pplx4 = cpu_pic->get_pplx4_osc();
  bool osccon_pplx4 = (osctune)?osctune->value.get() & OSCTUNE::PLLEN:0;
  bool intsrc            = (osctune) ? osctune->value.get() & OSCTUNE::INTSRC : false;
  bool mfiosel            = (osccon2) ? osccon2->value.get() & OSCCON2::MFIOSEL : false;

  uint old_clock_state = clock_state;


  CDprintf(("OSCCON_HS override=%d int_osc=%d osccon=0x%x\n", override, cpu_pic->get_int_osc(), value.get()));
  if (!cpu_pic->get_int_osc() && !(value.get() & SCS1) && !override)
     return false;


    uint new_IRCF = (value.get() & ( IRCF0 | IRCF1 | IRCF2)) >> 4;
    switch (new_IRCF)
    {
    case 0:
        base_frequency = 31.e3;
        if (mfiosel)
            clock_state = intsrc ? MFINTOSC : LFINTOSC;
        else
            clock_state = intsrc ? HFINTOSC : LFINTOSC;
        break;

    case 1:
        clock_state = mfiosel ? MFINTOSC : HFINTOSC;
        base_frequency = 125e3;
        break;

    case 2:
        clock_state = mfiosel ? MFINTOSC : HFINTOSC;
        base_frequency = 250e3;
        break;

    case 3:
        clock_state = HFINTOSC;
        base_frequency = 1e6;
        break;

    case 4:
        clock_state = HFINTOSC;
        base_frequency = 2e6;
        break;

    case 5:
        clock_state = HFINTOSC;
        base_frequency = 4e6;
        break;

    case 6:
        clock_state = HFINTOSC;
        base_frequency = 8e6;
        break;

    case 7:
        clock_state = HFINTOSC;
        base_frequency = 16e6;
        break;
   }
   if ( (new_IRCF>=minValPLL) && (osccon_pplx4 || config_pplx4) )
       base_frequency *= 4;
   if (osctune)
   {
       int tune;
       uint osctune_value = osctune->value.get();
       tune = osctune_value & (OSCTUNE::TUN5-1);
       tune = (OSCTUNE::TUN5 & osctune_value) ? -tune : tune;
       base_frequency *= 1. + 0.125 * tune / 31.;
   }
   cpu_pic->set_frequency_rc(base_frequency);
   if (cpu_pic->get_int_osc() || (value.get() & SCS1))
   {
       CDprintf(("OSCCON_HS clock_state %u->%u f=%.1e osccon=0x%x\n", old_clock_state, clock_state, base_frequency, value.get()));
       cpu_pic->set_RCfreq_active(true);
       if (old_clock_state != clock_state)
       {
            if ((old_clock_state == LFINTOSC) && clock_state != LFINTOSC)
            {
                    if (future_cycle) get_cycles().clear_break(future_cycle);

                    future_cycle = get_cycles().get() + irc_lh_time();
                get_cycles().set_break(future_cycle, this);
                CDprintf(("OSCCON_HS future_cycle %" PRINTF_GINT64_MODIFIER "d now %" PRINTF_GINT64_MODIFIER "d\n", future_cycle, get_cycles().get()));
            }
            else
                callback();
        }
   }
   return true;
}
// Is internal RC clock selected?
bool OSCCON_HS::internal_RC()
{
    bool ret = false;
    if ((value.get() & SCS1) || config_irc)
        ret = true;

    return ret;
}

void OSCCON_HS::por_wake()
{
    bool two_speed_clock = config_xosc  && config_ieso;
    uint val_osccon2 = osccon2->value.get();
    uint val_osccon = value.get();

    CDprintf(("OSCCON_HS config_xosc=%d config_ieso=%d\n", config_xosc,config_ieso));

    CDprintf(("OSCCON_HS POR two_speed_clock=%d f=%4.1e osccon=0x%x por_value=0x%x\n", two_speed_clock, cpu_pic->get_frequency(), val_osccon, por_value.data));

    if (future_cycle)
    {
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    // internal RC osc
    if (internal_RC())
    {
        CDprintf(("OSCCON_HS internal RC clock_state %u osccon %x osccon2 %x\n", clock_state, val_osccon, val_osccon2));
        set_rc_frequency();
        if (future_cycle)
            get_cycles().clear_break(future_cycle);
        future_cycle = get_cycles().get() + irc_por_time();
        get_cycles().set_break(future_cycle, this);
        return;
    }
    if (two_speed_clock)
    {
        val_osccon &= ~ (HFIOFS | OSTS);
        val_osccon2 &= ~(OSCCON2::LFIOFS | OSCCON2::MFIOFS);
        value.put(val_osccon);
        osccon2->value.put(val_osccon2);
        set_rc_frequency(true);
        cpu_pic->set_RCfreq_active(true);
        CDprintf(("OSCCON_HS 2 speed, set osccon 0x%x \n", value.get()));
        if (future_cycle)
            get_cycles().clear_break(future_cycle);
        clock_state = OST;
        future_cycle = 1024 + get_cycles().get();
        get_cycles().set_break(future_cycle, this);
        return;
    }
}

void  OSCCON_HS2::put(uint new_value)
{
  uint org_value = value.get();
  new_value = (new_value & write_mask) | (org_value & ~write_mask);
  value.put(new_value);
  uint diff = (new_value ^ org_value);

  value.put(new_value);
  CDprintf(("OSCCON org_value=0x%02x new_value=0x%02x diff=0x%02x state %u\n",
        org_value, new_value, diff, clock_state));
  if (diff == 0) return;

  if(internal_RC())
  {
#ifdef CDEBUG
      uint old_clock_state = clock_state;
#endif

      if ((diff & (IRCF0 | IRCF1 | IRCF2))) // freq change
      {
          set_rc_frequency();
          CDprintf(("OSCCON_HS2 change of IRCF old_clock %u new_clock %u\n", old_clock_state, clock_state));
      }
  }
}
bool OSCCON_HS2::set_rc_frequency(bool override)
{
  double base_frequency = 31.e3;

  uint old_clock_state = clock_state;


  CDprintf(("OSCCON_HS2 override=%d int_osc=%d osccon=0x%x\n", override, cpu_pic->get_int_osc(), value.get()));
  if (!cpu_pic->get_int_osc()  && !override)
     return false;


    uint new_IRCF = (value.get() & ( IRCF0 | IRCF1 | IRCF2)) >> 4;
    switch (new_IRCF)
    {
    case 0:
        base_frequency = 31.e3;
        clock_state = LFINTOSC;
        break;

    case 1:
        clock_state = HFINTOSC;
        base_frequency = 250e3;
        break;


    case 2:
        clock_state = HFINTOSC;
        base_frequency = 500e3;
        break;


    case 3:
        clock_state = HFINTOSC;
        base_frequency = 1e6;
        break;

    case 4:
        clock_state = HFINTOSC;
        base_frequency = 2e6;
        break;

    case 5:
        clock_state = HFINTOSC;
        base_frequency = 4e6;
        break;

    case 6:
        clock_state = HFINTOSC;
        base_frequency = 8e6;
        break;

    case 7:
        clock_state = HFINTOSC;
        base_frequency = 16e6;
        break;
   }
   cpu_pic->set_frequency_rc(base_frequency);
   if (cpu_pic->get_int_osc())
   {
       CDprintf(("OSCCON_HS2 clock_state %u->%u f=%.1e osccon=0x%x\n", old_clock_state, clock_state, base_frequency, value.get()));
       cpu_pic->set_RCfreq_active(true);
       if (old_clock_state != clock_state)
       {
            if ((old_clock_state == LFINTOSC) && clock_state != LFINTOSC)
            {
                    if (future_cycle) get_cycles().clear_break(future_cycle);

                    future_cycle = get_cycles().get() + irc_lh_time();
                get_cycles().set_break(future_cycle, this);
                CDprintf(("OSCCON_HS2 future_cycle %" PRINTF_GINT64_MODIFIER "d now %" PRINTF_GINT64_MODIFIER "d\n", future_cycle, get_cycles().get()));
            }
            else
                callback();
        }
   }
   return true;
}
// Is internal RC clock selected?
bool OSCCON_HS2::internal_RC()
{
    return cpu_pic->get_int_osc();
}

void OSCCON_HS2::callback()
{
    uint val_osccon = value.get() & write_mask;

    if (future_cycle <= get_cycles().get())
        future_cycle = 0;
    CDprintf(("OSCCON_HS2 clock_state=%u osccon=0x%x val_osccon=0x%x\n", clock_state, value.get(), val_osccon));
    switch(clock_state)
    {
    case LFINTOSC:
        val_osccon |= LFIOFR;
        break;

    case HFINTOSC:
        val_osccon |= HFIOFS|HFIOFR;
        break;

    }
    value.put(val_osccon);
    CDprintf(("OSCCON_HS2 osccon 0x%x val_osccon 0x%x\n", value.get(), val_osccon));
}
void OSCCON_HS2::por_wake()
{

    CDprintf(("OSCCON_HS2 config_xosc=%d config_ieso=%d\n", config_xosc,config_ieso));

    CDprintf(("OSCCON_HS2 POR  f=%4.1e osccon=0x%x por_value=0x%x\n", cpu_pic->get_frequency(), value.get(), por_value.data));

    if (future_cycle)
    {
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    // internal RC osc
    if (internal_RC())
    {
        CDprintf(("OSCCON_HS2internal RC clock_state %u osccon %x \n", clock_state, val_osccon));
        set_rc_frequency();
        if (future_cycle)
            get_cycles().clear_break(future_cycle);
        future_cycle = get_cycles().get() + irc_por_time();
        get_cycles().set_break(future_cycle, this);
        return;
    }
}
void WDTCON::put(uint new_value)
{
  uint masked_value = new_value & valid_bits;

  value.put(masked_value);

  if (valid_bits > 1)
      cpu_pic->wdt.set_prescale(masked_value >> 1);
  if (cpu_pic->swdten_active())
      cpu_pic->wdt.swdten((masked_value & SWDTEN) == SWDTEN);
}
void WDTCON::reset(RESET_TYPE r)
{
   putRV(por_value);
}
//
//--------------------------------------------------
// member functions for the FSR class
//--------------------------------------------------
//
FSR::FSR(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{}

void  FSR::put(uint new_value)
{
  value.put(new_value);
}

void  FSR::put_value(uint new_value)
{
  put(new_value);

  update();
  cpu_pic->indf->update();
}


uint FSR::get()
{
  return(value.get());
}

uint FSR::get_value()
{
  return(value.get());
}


//
//--------------------------------------------------
// member functions for the FSR_12 class
//--------------------------------------------------
//
FSR_12::FSR_12(Processor *pCpu, const char *pName, uint _rpb, uint _valid_bits)
  : FSR(pCpu, pName, ""),
    valid_bits(_valid_bits),
    register_page_bits(_rpb)
{}

void  FSR_12::put(uint new_value)
{
  value.put(new_value);

  /* The 12-bit core selects the register page using the fsr */
  cpu_pic->register_bank = &cpu_pic->registers[ value.get() & register_page_bits ];
}

void  FSR_12::put_value(uint new_value)
{

  put(new_value);

  update();
  cpu_pic->indf->update();

}


uint FSR_12::get()
{
  uint v = get_value();
  return(v);
}

uint FSR_12::get_value()
{
  // adjust for missing bits
  //cout << "FSR_12:get_value - valid_bits 0x" << hex << valid_bits << endl;
  return ((value.get() & valid_bits) | (~valid_bits & 0xff));

}

//
//--------------------------------------------------
// member functions for the Status_register class
//--------------------------------------------------
//

//--------------------------------------------------

Status_register::Status_register(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
  rcon = NULL;
  break_point = 0;
  break_on_z =0;
  break_on_c =0;
  address = 3;
  rp_mask = RP_MASK;
  write_mask = 0xff & ~STATUS_TO & ~STATUS_PD;
  new_name("status");
}

//--------------------------------------------------
void Status_register::reset(RESET_TYPE r)
{
  switch (r) {

  case POR_RESET:
    putRV(por_value);
    put_TO(1);
    put_PD(1);
    break;

  case WDT_RESET:
    put_TO(0);
    break;

  default:
    break;
  }

}

//--------------------------------------------------
// put

void Status_register::put(uint new_value)
{
  value.put((value.get() & ~write_mask) | (new_value & write_mask));

  if(cpu_pic->base_isa() == _14BIT_PROCESSOR_)
    {
      cpu_pic->register_bank = &cpu_pic->registers[(value.get() & rp_mask) << 2];
    }

}


//--------------------------------------------------
// get
//uint Status_register::get()

//--------------------------------------------------
// put_Z

//void Status_register::put_Z(uint new_z)

//--------------------------------------------------
// get_Z
//uint Status_register::get_Z()
//--------------------------------------------------
// put_C

//void Status_register::put_C(uint new_c)

//--------------------------------------------------
// get_C
//uint Status_register::get_C()

//--------------------------------------------------
// put_Z_C_DC

//--------------------------------------------------
// member functions for the INDF class
//--------------------------------------------------
INDF::INDF(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
  fsr_mask = 0x7f;           // assume a 14bit core
  base_address_mask1 = 0;    //   "          "
  base_address_mask2 = 0xff; //   "          "
}

void INDF::initialize()
{

  switch(cpu_pic->base_isa()) {

  case _12BIT_PROCESSOR_:
    fsr_mask = 0x1f;
    base_address_mask1 = 0x0;
    base_address_mask2 = 0x1f;

    break;

  case _14BIT_PROCESSOR_:
    fsr_mask = 0x7f;
    break;

  case _PIC17_PROCESSOR_:
  case _PIC18_PROCESSOR_:
    cout << "BUG: INDF::"<<__FUNCTION__<<". 16bit core uses a different class for indf.";
    break;
  default:
    cout << " BUG - invalid processor type INDF::initialize\n";
  }


}
void INDF::put(uint new_value)
{
    
  int reg = (cpu_pic->fsr->get_value() + //cpu_pic->fsr->value +
             ((cpu_pic->status->value.get() & base_address_mask1)<<1) ) &  base_address_mask2;

  // if the fsr is 0x00 or 0x80, then it points to the indf
  if(reg & fsr_mask){
    cpu_pic->registers[reg]->put(new_value);

    //(cpu_pic->fsr->value & base_address_mask2) + ((cpu_pic->status->value & base_address_mask1)<<1)
  }

}

void INDF::put_value(uint new_value)
{

  // go ahead and use the regular put to write the data.
  // note that this is a 'virtual' function. Consequently,
  // all objects derived from a file_register should
  // automagically be correctly updated (which isn't
  // necessarily true if we just write new_value on top
  // of the current register value).

  put(new_value);

  update();
  int r = (cpu_pic->fsr->get_value() + //cpu_pic->fsr->value +
           (((cpu_pic->status->value.get() & base_address_mask1)<<1)& base_address_mask2));
  if(r & fsr_mask)
    cpu_pic->registers[r]->update();

}


uint INDF::get()
{
    
  int reg = (cpu_pic->fsr->get_value() +
             ((cpu_pic->status->value.get() & base_address_mask1)<<1) ) &  base_address_mask2;
  if(reg & fsr_mask)
    return(cpu_pic->registers[reg]->get());
  else
    return(0);   // avoid infinite loop if fsr points to the indf
}

uint INDF::get_value()
{
  int reg = (cpu_pic->fsr->get_value() +
               ((cpu_pic->status->value.get() & base_address_mask1)<<1) ) &  base_address_mask2;
  if(reg & fsr_mask)
    return(cpu_pic->registers[reg]->get_value());
  else
    return(0);   // avoid infinite loop if fsr points to the indf
}



//--------------------------------------------------
// member functions for the PCL base class
//--------------------------------------------------
PCL::PCL(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
  por_value = RegisterValue(0,0);
}

// %%% FIX ME %%% breaks are different
void PCL::put(uint new_value)
{
  cpu_pic->pc->computed_goto(new_value);
}

void PCL::put_value(uint new_value)
{

  value.put(new_value & 0xff);
  cpu_pic->pc->put_value( (cpu_pic->pc->get_value() & 0xffffff00) | value.get());

  // The gui (if present) will be updated in the pc->put_value call.
}

uint PCL::get()
{
  return((value.get()+1) & 0xff);
}

uint PCL::get_value()
{
  value.put(cpu_pic->pc->get_value() & 0xff);
  return((value.get()+1) & 0xff);
}
//------------------------------------------------------------
// PCL reset
//
void PCL::reset(RESET_TYPE r)
{
  putRV_notrace(por_value);
}

//--------------------------------------------------
// member functions for the PCLATH base class
//--------------------------------------------------

PCLATH::PCLATH(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
    mValidBits = PCLATH_MASK;
}

void PCLATH::put(uint new_value)
{
  value.put(new_value & mValidBits);
}

void PCLATH::put_value(uint new_value)
{
  cout << "PCLATH::put_value(" << new_value << ")\n";
  value.put(new_value & mValidBits);

  // RP - I cannot think of a single possible reason I'd want to affect the real PC here!
  //  cpu_pic->pc->put_value( (cpu_pic->pc->get_value() & 0xffff00ff) | (value.get()<<8) );

  // The gui (if present) will be updated in the pc->put_value call.
}

uint PCLATH::get()
{
  return(value.get() & mValidBits);
}

//--------------------------------------------------
// member functions for the PCON base class
//--------------------------------------------------
//
PCON::PCON(Processor *pCpu, const char *pName, const char *pDesc,
        uint bitMask)
  : sfr_register(pCpu, pName, pDesc)
{
  valid_bits = bitMask;
}

void PCON::put(uint new_value)
{
  Dprintf((" value %x add %x\n", new_value, new_value&valid_bits));

  value.put(new_value&valid_bits);
}


//------------------------------------------------

Indirect_Addressing14::Indirect_Addressing14(pic_processor *pCpu, const string &n)
  : fsrl(pCpu, (string("fsrl")+n).c_str(), "FSR Low", this),
    fsrh(pCpu, (string("fsrh")+n).c_str(), "FSR High", this),
    indf(pCpu, (string("indf")+n).c_str(), "Indirect Register", this)
{
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
void Indirect_Addressing14::put(uint new_value)
{
    uint fsr_adj = fsr_value + fsr_delta;

    if (fsr_adj < 0x1000)         // Traditional Data Memory
    {
        if(is_indirect_register(fsr_adj))
            return;
        cpu_pic->registers[fsr_adj]->put(new_value);
    }
    else if (fsr_adj >= 0x2000 && fsr_adj < 0x29b0) // Linear GPR region
    {
        uint bank = (fsr_adj & 0xfff) / 0x50;
        uint low_bits = ((fsr_adj & 0xfff) % 0x50) + 0x20;
        Dprintf(("fsr_adj %x bank %x low_bits %x add %x\n", fsr_adj, bank, low_bits, (bank*0x80 + low_bits)));
        cpu_pic->registers[bank * 0x80 + low_bits]->put(new_value);
    }
    else if (fsr_adj >= 0x8000 && fsr_adj <= 0xffff) // program memory
    {
        cout << "WARNING cannot write via FSR/INDF to program memory address 0x"
                <<hex << fsr_adj << endl;
        return;        // Not writable
    }

}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
uint Indirect_Addressing14::get()
{
    uint fsr_adj = fsr_value + fsr_delta;

    if (fsr_adj < 0x1000) // Traditional Data Memory
    {
        if(is_indirect_register(fsr_adj))
            return 0;
        return cpu_pic->registers[fsr_adj]->get();
    }
    else if (fsr_adj >= 0x2000 && fsr_adj < 0x29b0) // Linear GPR region
    {
        uint bank = (fsr_adj & 0xfff) / 0x50;
        uint low_bits = ((fsr_adj & 0xfff) % 0x50) + 0x20;
        return(cpu_pic->registers[bank * 0x80 + low_bits]->get());
    }
    else if (fsr_adj >= 0x8000 && fsr_adj <= 0xffff) // program memory
    {
        uint pm;
        unsigned address = fsr_adj - 0x8000;

          if (address <= cpu_pic->program_memory_size())
        {
          pm = cpu_pic->get_program_memory_at_address(address);
          Dprintf((" address %x max %x value %x\n",address, cpu_pic->program_memory_size(), pm));
            return pm & 0xff;
        }
    }
    return 0;
}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
uint Indirect_Addressing14::get_value()
{
    uint fsr_adj = fsr_value + fsr_delta;
    if (fsr_adj < 0x1000)        // Traditional Data Memory
    {
        if(is_indirect_register(fsr_adj))
            return 0;

        return cpu_pic->registers[fsr_adj]->get_value();
    }
    else if (fsr_adj >= 0x2000 && fsr_adj < 0x29b0) // Linear GPR region
    {
        uint bank = (fsr_adj & 0xfff) / 0x50;
        uint low_bits = ((fsr_adj & 0xfff) % 0x50) + 0x20;

        return(cpu_pic->registers[bank * 0x80 + low_bits]->get_value());
    }
    else if (fsr_adj >= 0x8000 && fsr_adj <= 0xffff) // program memory
    {
        uint pm;
        unsigned address = fsr_adj - 0x8000;

          if (address <= cpu_pic->program_memory_size())
        {
          pm = cpu_pic->get_program_memory_at_address(address);
            return pm & 0xff;
        }
    }
    return 0;
}

void Indirect_Addressing14::put_fsr(uint new_fsr)
{

  fsrl.put(new_fsr & 0xff);
  fsrh.put((new_fsr>>8) & 0xff);

}


/*
 * update_fsr_value - This routine is called by the FSRL and FSRH
 * classes. It's purpose is to update the 16-bit
 * address formed by the concatenation of FSRL and FSRH.
 *
 */

void Indirect_Addressing14::update_fsr_value()
{

  if(current_cycle != get_cycles().get())
    {
      fsr_value = (fsrh.value.get() << 8) |  fsrl.value.get();
      fsr_delta = 0;
    }
}
//--------------------------------------------------
// member functions for the FSR class
//--------------------------------------------------
//
FSRL14::FSRL14(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing14 *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void  FSRL14::put(uint new_value)
{
  value.put(new_value & 0xff);
  iam->fsr_delta = 0;
  iam->update_fsr_value();
}

void  FSRL14::put_value(uint new_value)
{

  value.put(new_value & 0xff);
  iam->fsr_delta = 0;
  iam->update_fsr_value();

  update();
  cpu14->indf->update();

}

FSRH14::FSRH14(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing14 *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void  FSRH14::put(uint new_value)
{
  value.put(new_value & 0xff);

  iam->update_fsr_value();
}

void  FSRH14::put_value(uint new_value)
{

  value.put(new_value & 0xff);
  iam->update_fsr_value();

  update();
  cpu14->indf->update();
}

// INDF14 used by 14bit enhanced indirect addressing
INDF14::INDF14(Processor *pCpu, const char *pName, const char *pDesc, Indirect_Addressing14 *pIAM)
  : sfr_register(pCpu,pName,pDesc),
    iam(pIAM)
{
}

void INDF14::put(uint new_value)
{
  if(iam->fsr_value & 0x8000) // extra cycle for program memory access
      get_cycles().increment();

  iam->put(new_value);
  iam->fsr_delta = 0;
}

void INDF14::put_value(uint new_value)
{
  iam->put(new_value);
  iam->fsr_delta = 0;
  update();
}

uint INDF14::get()
{
  uint ret;

  Dprintf((" get val %x delta %x \n", iam->fsr_value, iam->fsr_delta));

  if(iam->fsr_value & 0x8000) get_cycles().increment();

  ret = iam->get();
  iam->fsr_delta = 0;
  return ret;
}

uint INDF14::get_value()
{
  return(iam->get_value());
}
//--------------------------------------------------
Stack::Stack(Processor *pCpu) : cpu(pCpu)
{

  stack_warnings_flag = 0;   // Do not display over/under flow stack warnings
  break_on_overflow = true;     // Do not break if the stack over flows
  break_on_underflow = 0;    // Do not break if the stack under flows
  stack_mask = 7;            // Assume a 14 bit core.
  pointer = 0;

  for(int i=0; i<31; i++)
    contents[i] = 0;

  STVREN = 0;

}

//
// Stack::push
//
// push the passed address onto the stack by storing it at the current
//

bool Stack::push(uint address)
{
  Dprintf(("pointer=%d address 0x%x\n",pointer,address));
  // Write the address at the current point location. Note that the '& stack_mask'
  // implicitly handles the stack wrap around.


  // If the stack pointer is too big, then the stack has definitely over flowed.
  // However, some pic programs take advantage of this 'feature', so provide a means
  // for them to ignore the warnings.

  if(pointer > (int)stack_mask) {
        stack_overflow();
        return false;
  }
  contents[pointer++ & stack_mask] = address;

  return true;

}
bool Stack::stack_overflow()
{
    Dprintf(("stack_warnings_flag=%d break_on_overflow=%d\n", stack_warnings_flag,break_on_overflow));
    if(stack_warnings_flag || break_on_overflow)
      cout << "stack overflow \n";
    if(break_on_overflow)
      bp.halt();
    return true;
}

//
// Stack::pop
//

uint Stack::pop()
{

  // First decrement the stack pointer.

  if(--pointer < 0)  {
        stack_underflow();
        return 0;
  }


  Dprintf(("pointer=%d address 0x%x\n",pointer,contents[pointer & stack_mask]));

  return(contents[pointer & stack_mask]);
}
bool Stack::stack_underflow()
{
    pointer = 0;
    if(stack_warnings_flag || break_on_underflow)
      cout << "stack underflow ";
    if(break_on_underflow)
      bp.halt();
    return true;
}

//
//  bool Stack::set_break_on_overflow(bool clear_or_set)
//
//  Set or clear the break on overflow flag


bool Stack::set_break_on_overflow(bool clear_or_set)
{
  if(break_on_overflow == clear_or_set)
    return 0;

  break_on_overflow = clear_or_set;

  return 1;

}

//
//  bool Stack::set_break_on_underflow(bool clear_or_set)
//
//  Set or clear the break on underflow flag


bool Stack::set_break_on_underflow(bool clear_or_set)
{
  if(break_on_underflow == clear_or_set)
    return 0;

  break_on_underflow = clear_or_set;

  return 1;

}

// Read value at top of stack
//
uint Stack::get_tos()
{
  if (pointer > 0)
    return (contents[pointer-1]);
  else
    return (0);
}

// Modify value at top of stack;
//
void Stack::put_tos(uint new_tos)
{

  if (pointer > 0)
      contents[pointer-1] = new_tos;

}

// Stack14E for extended 14bit processors
// This stack implementation differs from both the other 14bit
// and 16bit stacks as a dummy empty location is used so a
// stack with 16 slots can hold 16 values. The other implementaion
// of the stack hold n-1 values for an n slot stack.
// This stack also supports stkptr, tosl, and tosh like the 16bit
// (p18) processors
Stack14E::Stack14E(Processor *pCpu) : Stack(pCpu),
    stkptr(pCpu, "stkptr", "Stack pointer"),
    tosl(pCpu, "tosl", "Top of Stack low byte"),
    tosh(pCpu, "tosh", "Top of Stack high byte")
{
  stkptr.stack = this;
  tosl.stack = this;
  tosh.stack = this;


  STVREN = 1;
}

Stack14E::~Stack14E()
{
  pic_processor *pCpu = dynamic_cast<pic_processor *>(cpu);
  if (pCpu)
  {
    pCpu->remove_sfr_register(&stkptr);
    pCpu->remove_sfr_register(&tosl);
    pCpu->remove_sfr_register(&tosh);
  }
}
void Stack14E::reset(RESET_TYPE r)
{
    pointer = NO_ENTRY;
    if (STVREN)
        contents[stack_mask] = 0;
    else
        contents[pointer-1] = contents[stack_mask];

    Dprintf((" pointer 0x%x\n", pointer));
    stkptr.put(pointer-1);
}

bool Stack14E::push(uint address)
{
  Dprintf(("pointer=%d address 0x%x\n",pointer,address));
  // Write the address at the current point location. Note that the '& stack_mask'
  // implicitly handles the stack wrap around.

  if(pointer == NO_ENTRY)
        pointer = 0;

  contents[pointer & stack_mask] = address;

  // If the stack pointer is too big, then the stack has definitely over flowed.
  // However, some pic programs take advantage of this 'feature', so provide a means
  // for them to ignore the warnings.

  if(pointer++ > (int)stack_mask) {
        return stack_overflow();
  }
  stkptr.put(pointer-1);
  return true;

}
uint  Stack14E::pop()
{
    uint ret = 0;

    if (pointer == NO_ENTRY)
    {
        return stack_underflow();
    }
    pointer--;
    ret = contents[pointer];
    if (pointer <= 0)
        pointer = NO_ENTRY;

    stkptr.put(pointer-1);
    return(ret);
}
bool Stack14E::stack_overflow()
{
    cpu14e->pcon.put(cpu14e->pcon.get() | PCON::STKOVF);
    if(STVREN)
    {
        cpu->reset(STKOVF_RESET);
        return false;
    }
    else
    {
        cout << "Stack overflow\n";
    }
    return true;
}
bool Stack14E::stack_underflow()
{
    Dprintf((" cpu %p STVREN %d\n", cpu, STVREN));
    cpu14e->pcon.put(cpu14e->pcon.get() | PCON::STKUNF);
    if(STVREN)
    {
        cpu->reset(STKUNF_RESET);
            return false;
    }
    else
    {
        cout << "Stack underflow\n";
    }
    return true;

}

//------------------------------------------------
// TOSL
TOSL::TOSL(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{}

uint TOSL::get()
{
  value.put(stack->get_tos() & 0xff);

  return(value.get());
}

uint TOSL::get_value()
{
  value.put(stack->get_tos() & 0xff);
  return(value.get());
}

void TOSL::put(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffffff00) | (new_value & 0xff));
  value.put(new_value & 0xff);
}

void TOSL::put_value(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffffff00) | (new_value & 0xff));
  value.put(new_value & 0xff);
  update();
}


//------------------------------------------------
// TOSH
TOSH::TOSH(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{}

uint TOSH::get()
{
  value.put((stack->get_tos() >> 8) & 0xff);

  return(value.get());
}

uint TOSH::get_value()
{
  value.put((stack->get_tos() >> 8) & 0xff);
  return(value.get());
}

void TOSH::put(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffff00ff) | ( (new_value & 0xff) << 8));
  value.put(new_value & 0xff);
}

void TOSH::put_value(uint new_value)
{
  stack->put_tos( (stack->get_tos() & 0xffff00ff) | ( (new_value & 0xff) << 8));
  value.put(new_value & 0xff);

  update();
}

STKPTR::STKPTR(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{}
void STKPTR::put_value(uint new_value)
{
  stack->pointer = (new_value & 0x1f) + 1;
  value.put(new_value);
  update();
}

void STKPTR::put(uint new_value)
{
  put_value(new_value);
}




WREG::WREG(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu,pName,pDesc)
{
}

WREG::~WREG()
{
}

void WPU::put(uint new_value)
{
    uint masked_value = new_value & mValidBits;
    int i;

    value.put(masked_value);
    for(i = 0; i < 8; i++)
    {
        if((1<<i) & mValidBits)
        {
            (&(*wpu_gpio)[i])->getPin().update_pullup((((1<<i) & masked_value) && wpu_pu )? '1' : '0', true);
        }
    }
}
/*
 * enable/disable all WPU  pullups
 */
void WPU::set_wpu_pu(bool pullup_enable)
{
    if (pullup_enable != wpu_pu)
    {
        wpu_pu = pullup_enable;
        put(value.get());        // change pull-ups based on value of WPU and gpio_pu
    }
}

CPSCON0::CPSCON0(Processor *pCpu, const char *pName, const char *pDesc)
    : sfr_register(pCpu, pName, pDesc), m_tmr0(0), chan(0),
        future_cycle(0), cps_stimulus(0)
{
      mValidBits = 0xcf;
      for(int i =0; i < 16; i++)
        pin[i] = 0;
}

CPSCON0::~CPSCON0()
{
    if (cps_stimulus)
        delete cps_stimulus;
}

void CPSCON0::put(uint new_value)
{
    uint masked_value = (new_value & mValidBits) & ~CPSOUT;
    uint diff = masked_value ^ value.get();

    value.put(masked_value);
    if (diff & T0XCS)
    m_tmr0->set_t0xcs(masked_value & T0XCS);
    calculate_freq();
}

#define p_cpu ((Processor *)cpu)

void CPSCON0::calculate_freq()
{

    if (!(value.get() & CPSON)) return;// not active, return
    if (!pin[chan] || !pin[chan]->getPin().snode) return;

    double cap = pin[chan]->getPin().snode->Cth;
    double current = 0;
    double deltat;

    switch((value.get() & (CPSRNG0 | CPSRNG1)) >> 2)
    {
    case 1:
        current = (value.get() & CPSRM) ? 9e-6 : 0.1e-6;
        break;

    case 2:
        current = (value.get() & CPSRM) ? 30e-6 : 1.2e-6;
        break;

    case 3:
        current = (value.get() & CPSRM) ? 100e-6 : 18e-6;
        break;
    };

    if (current < 1e-12)
        return;
    // deltat is the time required to charge the capacitance on the pin
    // from a constant current source for the specified voltage swing.
    // The voltage swing for the internal reference is not specified
    // and it is just a guess that it is Vdd - 2 diode drops.
    //
    // This implimentation does not work if capacitor oscillator
    // runs faster than Fosc/4
    //
    if (value.get() & CPSRM)
    {
        deltat = (FVR_voltage - DAC_voltage)*cap/current;
        if (deltat <= 0.)
        {
            cout << "CPSCON FVR must be greater than DAC for high range to work\n";
            return;
        }
    }
    else
    {
        deltat = (p_cpu->get_Vdd() - 1.2) * cap / current;
    }

    period = (p_cpu->get_frequency() * deltat + 2) / 4;

    if (period <= 0)
    {
        cout << "CPSCON Oscillator > Fosc/4, setting to Fosc/4\n";
        period = 1;
    }
    uint64_t fc = get_cycles().get() + period;

    if (future_cycle > get_cycles().get())
    {
        get_cycles().reassign_break(future_cycle, fc, this);
    }
    else
        get_cycles().set_break(fc, this);

    future_cycle = fc;
}

void CPSCON0::callback_print()
{
    cout <<  name() << " has callback, ID = " << CallBackID << '\n';
}
void CPSCON0::callback()
{
    Dprintf(("now=0x%" PRINTF_GINT64_MODIFIER "x\n",get_cycles().get()));

    if (!(value.get() & CPSON))
        return;

    if (value.get() & CPSOUT) // High to low transition
    {
        value.put(value.get() & ~CPSOUT);
        if (m_tmr0 && (value.get() & T0XCS) &&
            m_tmr0->get_t0se() && m_tmr0->get_t0cs())
        {
                m_tmr0->increment();
        }

    }
    else                        // Low to high transition
    {
        value.put(value.get() | CPSOUT);
        if (m_tmr0 && (value.get() & T0XCS) &&
            !m_tmr0->get_t0se() && m_tmr0->get_t0cs())
        {
                m_tmr0->increment();
        }
        if (m_t1con_g)
            m_t1con_g->t1_cap_increment();
    }


    calculate_freq();
}

void CPSCON0::set_chan(uint _chan)
{
    if (_chan == chan)
        return;

    if (!pin[_chan])
    {
        cout << "CPSCON Channel " << _chan << " reserved\n";
        return;
    }
    if (!pin[_chan]->getPin().snode)
    {
        cout << "CPSCON Channel " << pin[_chan]->getPin().name() << " requires a node attached\n";
        chan = _chan;
        return;
    }
    if (!cps_stimulus)
        cps_stimulus = new CPS_stimulus(this, "cps_stimulus");
    else if (pin[_chan]->getPin().snode)
    {
        (pin[_chan]->getPin().snode)->detach_stimulus(cps_stimulus);
    }

    chan = _chan;
    (pin[_chan]->getPin().snode)->attach_stimulus(cps_stimulus);
    calculate_freq();
}

void CPSCON0::set_DAC_volt(double volt)
{
    DAC_voltage = volt;
    if ((value.get() & (CPSON|CPSRM)) == (CPSON|CPSRM))
        calculate_freq();
}
void CPSCON0::set_FVR_volt(double volt)
{
    FVR_voltage = volt;
    if ((value.get() & (CPSON|CPSRM)) == (CPSON|CPSRM))
        calculate_freq();
}

CPS_stimulus::CPS_stimulus(CPSCON0 * arg, const char *cPname,double _Vth, double _Zth)
  : stimulus(cPname, _Vth, _Zth)
{
    m_cpscon0 = arg;
}

// Thisvis also called when the capacitance chages,
// not just when the voltage changes
void   CPS_stimulus::set_nodeVoltage(double v)
{
        Dprintf(("set_nodeVoltage =%.1f\n", v));
         nodeVoltage = v;
        m_cpscon0->calculate_freq();
}

void CPSCON1::put(uint new_value)
{
    uint masked_value = new_value & mValidBits;

    value.put(masked_value);
    assert(m_cpscon0);
    m_cpscon0->set_chan(masked_value);
}

SRCON0::SRCON0(Processor *pCpu, const char *pName, const char *pDesc, SR_MODULE *_sr_module)
    : sfr_register(pCpu, pName, pDesc),
        m_sr_module(_sr_module)
{
}
void SRCON0::put(uint new_value)
{
    uint diff = new_value ^ value.get();

    if (!diff) return;

    value.put(new_value  & ~(SRPR|SRPS)); // SRPR AND SRPS not saved

    if ((diff & SRPS) && (new_value & SRPS))
        m_sr_module->pulse_set();
    if ((diff & SRPR) && (new_value & SRPR))
        m_sr_module->pulse_reset();

    if (diff & CLKMASK)
        m_sr_module->clock_diff((new_value & CLKMASK) >> CLKSHIFT);

    if (diff & (SRQEN | SRLEN))
        m_sr_module->Qoutput();
    if (diff & (SRNQEN | SRLEN))
        m_sr_module->NQoutput();

    m_sr_module->update();
}

SRCON1::SRCON1(Processor *pCpu, const char *pName, const char *pDesc, SR_MODULE *_sr_module)
    : sfr_register(pCpu, pName, pDesc),
        m_sr_module(_sr_module), mValidBits(0xdd)
{
}

void SRCON1::put(uint new_value)
{
    uint masked_value = new_value & mValidBits;
    uint diff = masked_value ^ value.get();

    value.put(masked_value);

    if (!diff) return;

    if (diff & (SRRCKE | SRSCKE))
    {
        if (!(new_value & (SRRCKE | SRSCKE)))        // all clocks off
            m_sr_module->clock_disable(); // turn off clock
        else
            m_sr_module->clock_enable(); // turn on clock
    }
    m_sr_module->update();
}

class SRinSink : public SignalSink
{
public:
  SRinSink(SR_MODULE *_sr_module)
     : sr_module(_sr_module)
  {}

  virtual void setSinkState(char new3State)
  {
        sr_module->setState(new3State);
  }
  virtual void release() { delete this; }
private:
  SR_MODULE *sr_module;
};

class SRnSource : public PeripheralSignalSource
{

public:

  SRnSource(PinModule *_pin, SR_MODULE *_sr_module, int _index) :
        PeripheralSignalSource(_pin), sr_module(_sr_module),
        index(_index)
  { ;}
  virtual void release() { sr_module->releasePin(index); }
private:
  SR_MODULE *sr_module;
  int       index;
};

enum {
        SRQ = 0,
        SRNQ
};
SR_MODULE::SR_MODULE(Processor *_cpu) :
    srcon0(_cpu, "srcon0", "SR Latch Control 0 Register", this),
    srcon1(_cpu, "srcon1", "SR Latch Control 1 Register", this),
    cpu(_cpu), future_cycle(0), state_set(false), state_reset(false),
    state_Q(false), srclk(0), syncc1out(false), syncc2out(false),
    SRI_pin(0), SRQ_pin(0), SRNQ_pin(0), m_SRinSink(0),
    m_SRQsource(0), m_SRNQsource(0), m_SRQsource_active(false),
    m_SRNQsource_active(false)
{
}
SR_MODULE::~SR_MODULE()
{
    if (m_SRQsource_active)
        SRQ_pin->setSource(0);
    if ( m_SRQsource)
        delete  m_SRQsource;
    if (m_SRNQsource_active)
        SRNQ_pin->setSource(0);
    if ( m_SRNQsource)
        delete  m_SRNQsource;
}

// determine output state of RS flip-flop
// If both state_set and state_reset are true, Q output is 0
// SPR[SP] and clocked inputs maybe set outside the update
// function prior to its call.
void SR_MODULE::update()
{

    if ((srcon1.value.get() & SRCON1::SRSC1E) && syncc1out)
        state_set = true;

    if ((srcon1.value.get() & SRCON1::SRSC2E) && syncc2out)
        state_set = true;

    if ((srcon1.value.get() & SRCON1::SRSPE) && SRI_pin->getPin().getState())
        state_set = true;

    if ((srcon1.value.get() & SRCON1::SRRC1E) && syncc1out)
        state_reset = true;

    if ((srcon1.value.get() & SRCON1::SRRC2E) && syncc2out)
        state_reset = true;

    if ((srcon1.value.get() & SRCON1::SRRPE) && SRI_pin->getPin().getState())
        state_reset = true;
    if (state_set)
        state_Q = true;

    // reset overrides a set
    if (state_reset)
        state_Q = false;


    state_set = state_reset = false;

    if (!(srcon0.value.get() & SRCON0::SRLEN))
        return;

    if ((srcon0.value.get() & SRCON0::SRQEN))
        m_SRQsource->putState(state_Q ? '1' : '0');

    if ((srcon0.value.get() & SRCON0::SRNQEN))
        m_SRNQsource->putState(!state_Q ? '1' : '0');

}

// Stop clock if currently running
void SR_MODULE::clock_disable()
{
    if (future_cycle> get_cycles().get())
    {
        get_cycles().clear_break(this);
        future_cycle = 0;
    }
    future_cycle = 0;
}

// Start clock if not running
// As break works on instruction cycles, clock runs every 1-128
// instructions which is 1 << srclk
//
void SR_MODULE::clock_enable()
{
    if (!future_cycle)
    {
        future_cycle = get_cycles().get() + (1 << srclk);
        get_cycles().set_break(future_cycle, this);
    }
}

// Called for clock rate change
void SR_MODULE::clock_diff(uint _srclk)
{
    srclk = _srclk;

    clock_disable();

    if (srcon1.value.get() & (SRCON1::SRSCKE | SRCON1::SRRCKE))
    {
        clock_enable();
    }
}

void SR_MODULE::callback()
{
    bool active = false;

    if (srcon1.value.get() & (SRCON1::SRSCKE)) //Set clock enabled
    {
        active = true;
        pulse_set();
    }

    if (srcon1.value.get() & (SRCON1::SRRCKE)) //Reset clock enabled
    {
        active = true;
        pulse_reset();
    }
    if (active)
    {
        future_cycle = 0;
        clock_enable();
    }
    update();

}
void SR_MODULE::setPins(PinModule *sri, PinModule *srq, PinModule *srnq)
{

    if(!SRI_pin)
    {
        m_SRinSink = new SRinSink(this);
        sri->addSink(m_SRinSink);
    }
    else if (SRI_pin != sri)
    {
        SRI_pin->removeSink(m_SRinSink);
        sri->addSink(m_SRinSink);
    }
    SRI_pin = sri;
        SRQ_pin = srq;
        SRNQ_pin = srnq;

}

// If pin chnages and we are looking at it, call update
void SR_MODULE::setState(char IOin)
{
    if (srcon1.value.get() & (SRCON1::SRSPE | SRCON1::SRRPE))
        update();
}

void SR_MODULE::syncC1out(bool val)
{
    if (syncc1out != val)
    {
        syncc1out = val;
        if (srcon1.value.get() & (SRCON1::SRSC1E | SRCON1::SRRC1E))
        {
                update();
        }
    }
}
void SR_MODULE::syncC2out(bool val)
{
    if (syncc2out != val)
    {
        syncc2out = val;
        if (srcon1.value.get() & (SRCON1::SRSC2E | SRCON1::SRRC2E))
        {
                update();
        }
    }
}

// Setup or tear down RSQ output pin
// This is only call if SRLEN OR SRQEN has changed
void SR_MODULE::Qoutput()
{
    if ((srcon0.value.get() & (SRCON0::SRLEN | SRCON0::SRQEN)) ==
        (SRCON0::SRLEN | SRCON0::SRQEN))
    {
        if (!m_SRQsource)
            m_SRQsource = new SRnSource(SRQ_pin, this, SRQ);

        SRQ_pin->setSource(m_SRQsource);

        m_SRQsource_active = true;
    }
    else SRQ_pin->setSource(0);

}

// Setup or tear down RSNQ output pin
// This is only call if SRLEN OR SRNQEN has changed
void SR_MODULE::NQoutput()
{
    if ((srcon0.value.get() & (SRCON0::SRLEN | SRCON0::SRNQEN)) ==
        (SRCON0::SRLEN | SRCON0::SRNQEN))
    {
        if (!m_SRNQsource) m_SRNQsource = new SRnSource(SRNQ_pin, this, SRNQ);

        SRNQ_pin->setSource(m_SRNQsource);

        m_SRNQsource_active = true;
    }
    else SRNQ_pin->setSource(0);

}

void SR_MODULE::releasePin(int index)
{
    switch(index)
    {
    case SRQ:
        m_SRQsource_active = false;
        break;

    case SRNQ:
        m_SRNQsource_active = false;
        break;
    }
}

//-------------------------------------------------------------------
LVDCON_14::LVDCON_14(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName,pDesc),
    write_mask(0x17), IntSrc(0)
{
}

double ldv_volts[] = {1.9, 2.0, 2.1, 2.2, 2.3, 4.0, 4.2, 4.5};

void LVDCON_14::check_lvd()
{
    uint reg = value.get();
    if (!(reg & IRVST))
        return;

    double voltage = ldv_volts[reg & (LVDL0|LVDL1|LVDL2)];
    Processor *Cpu = (Processor *)cpu;
    if (Cpu->get_Vdd() <= voltage)
        IntSrc->Trigger();
}
void LVDCON_14::callback()
{
    value.put(value.get() | IRVST);
    check_lvd();
}
void LVDCON_14::put(uint new_value)
{
    new_value &= write_mask;
    uint diff = value.get() ^ new_value;
    if (!diff) return;

    value.put(new_value);

    if ((diff & LVDEN) && (new_value & LVDEN)) // Turning on
    {
     // wait before doing anything
        get_cycles().set_break(
            get_cycles().get() + 50e-6 * get_cycles().instruction_cps(),
            this);
    }
    return;
}


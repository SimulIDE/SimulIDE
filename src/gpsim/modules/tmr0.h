/*
   Copyright (C) 1998,1999,2000 T. Scott Dattalo

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

#ifndef __TMR0_H__
#define __TMR0_H__

#include "ioports.h"

class T1GCON;
class ADCON2_TRIG;
class CLC;

//---------------------------------------------------------
// TMR0 - Timer
class TMR0 : public sfr_register, public TriggerObject, public SignalSink
{
public:
  uint 
    prescale,
    prescale_counter,
    old_option,       // Save option register contents here.
    state;            // Either on or off right now.
  uint64_t
    synchronized_cycle,
    future_cycle;
  int64_t
    last_cycle;   // can be negative ...

  OPTION_REG *m_pOptionReg;


  virtual void callback();

  TMR0(Processor *, const char *pName, const char *pDesc=0);

  virtual void release();

  virtual void put(uint new_value);
  virtual void put_value(uint new_value);
  virtual uint get();
  virtual uint get_value();
  virtual void start(int new_value,int sync=0);
  virtual void stop();
  virtual void increment();   // Used when tmr0 is attached to an external clock
  virtual void new_prescale();
  virtual uint get_prescale();
  virtual uint max_counts() {return 256;};
  virtual bool get_t0cs();
  virtual bool get_t0se();
  virtual void set_t0if();
  virtual void set_t0xcs(bool _t0xcs){t0xcs = _t0xcs;}
  virtual bool get_t0xcs() {return t0xcs;}
  virtual void reset(RESET_TYPE r);
  virtual void callback_print();
  virtual void clear_trigger();

  virtual void set_cpu(Processor *, PortRegister *, uint pin,OPTION_REG *);
  virtual void set_cpu(Processor *new_cpu, PinModule *pin,OPTION_REG *);
  virtual void setSinkState(char);
  virtual void sleep();
  virtual void wake();
  void set_t1gcon(T1GCON *_t1gcon) { m_t1gcon = _t1gcon; }
  void set_adcon2(ADCON2_TRIG *_adcon2) { m_adcon2 = _adcon2; }
  void set_clc(CLC *_clc, int index);

  enum {
    STOPPED = 0,
    RUNNING = 1,
    SLEEPING = 2
  };

protected:
  T1GCON     *m_t1gcon;
  ADCON2_TRIG   *m_adcon2;
  CLC        *m_clc[4];

private:
  bool         m_bLastClockedState;
  bool        t0xcs;            //  clock source is the capacitive sensing oscillator
};

#endif

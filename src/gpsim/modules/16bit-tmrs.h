/*
   Copyright (C) 2000 T. Scott Dattalo

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

#ifndef __16_BIT_TMRS_H__
#define __16_BIT_TMRS_H__

#include "14bit-tmrs.h"

//---------------------------------------------------------
// T5CON - Timer 1,3,5 control register

class T5CON : public T1CON
{
public:

enum
{
  TMRxON  = 1<<0,
  TxRD16  = 1<<1,
  TxSYNC  = 1<<2,
  TxSOSCEN = 1<<3,
  TxCKPS0 = 1<<4,
  TxCKPS1 = 1<<5,
  TMRxCS0  = 1<<6,
  TMRxCS1  = 1<<7,
};


  T5CON(Processor *pCpu, const char *pName, const char *pDesc=0);
  ~T5CON();

  //RRR uint get();

  // For (at least) the 18f family, there's a 4X PLL that effects the
  // the relative timing between gpsim's cycle counter (which is equivalent
  // to the cumulative instruction count) and the external oscillator. In
  // all parts, the clock source for the timer is fosc, the external oscillator.
  // However, for the 18f parts, the instructions execute 4 times faster when
  // the PLL is selected.

  virtual uint get_prescale()
  {
      return(((value.get() &(TxCKPS0 | TxCKPS1)) >> 4) );
  }

  virtual uint get_tmr1cs()
  {
      return((value.get() & (TMRxCS0 | TMRxCS1)) >> 6);
  }
  virtual bool get_tmr1on()
  {
      return(value.get() & TMRxON);
  }
  virtual bool  get_t1oscen()
  {
      return(value.get() & TxSOSCEN);
  }
  virtual bool get_tmr1GE()
  {
      return t1gcon->get_tmr1GE();
  }
  virtual bool get_t1sync()
  {
      return(value.get() & TxSYNC);
  }
  virtual void put(uint new_value);

  virtual bool get_t1GINV() {return true;}

  T1GCON *t1gcon;

};

class CCPTMRS;

class CCPTMRS0 : public sfr_register
{

public:
   
  CCPTMRS0(CCPTMRS *_ccptmrs,  Processor *pCpu, const char *pName, const char *pDesc=0);
  ~CCPTMRS0();

  virtual void put(uint new_value);

  uint bit_mask;
  CCPTMRS *ccptmrs;
};
class CCPTMRS1 : public sfr_register
{
public:
   
  CCPTMRS1(CCPTMRS * _ccptmrs, Processor *pCpu, const char *pName, const char *pDesc=0);
  ~CCPTMRS1();

  virtual void put(uint new_value);

  uint bit_mask;
  CCPTMRS *ccptmrs;
};

class CCPTMRS
{
public:

  enum
  {
    C1TSEL0 = 1<<0,
    C1TSEL1 = 1<<1,
    C2TSEL0 = 1<<3,
    C2TSEL1 = 1<<4,
    C3TSEL0 = 1<<6,
    C3TSEL1 = 1<<7,
    C4TSEL0 = 1<<0,
    C4TSEL1 = 1<<1,
    C5TSEL0 = 1<<2,
    C5TSEL1 = 1<<3,
  };
    CCPTMRS(Processor *pCpu);
    ~CCPTMRS();

    void update0(uint reg_value);
    void update1(uint reg_value);
    //void set_tmr135(TMR5 *t1, TMR5 *t3, TMR5 *t5);
    void set_tmr246(TMR2 *t2, TMR2 *t4, TMR2 *t6);
    void set_ccp(CCPCON *_c1, CCPCON *_c2, CCPCON *_c3, CCPCON *_c4, CCPCON *_c5);
    void change(CCPCON *c, uint old, uint val);

    CCPTMRS0 ccptmrs0;
    CCPTMRS1 ccptmrs1;

    //TMR5 *t1, *t3, *t5;
    TMR2 *t2, *t4, *t6;
    CCPCON *ccp1, *ccp2, *ccp3, *ccp4, *ccp5;
    uint last_value0;
    uint last_value1;

};
#endif // __16_BIT_TMRS_H__

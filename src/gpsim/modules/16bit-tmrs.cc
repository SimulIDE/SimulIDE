/*
   Copyright (C) 2000 T. Scott Dattalo
   Copyright (C) 2015 Roy R. Rankin

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
#include <string>

#include "config.h"
#include "16bit-tmrs.h"
#include "stimuli.h"


//
// 16bit-tmrs.cc
//
// Timer 1&2  modules for the 16bit core pic processors.
//
//--------------------------------------------------
// T5CON
//--------------------------------------------------
T5CON::T5CON(Processor *pCpu, const char *pName, const char *pDesc)
  : T1CON(pCpu, pName, pDesc), t1gcon(nullptr)
{
}

T5CON::~T5CON()
{
}

void T5CON::put(uint new_value)
{
  uint diff = value.get() ^ new_value;
  value.put(new_value);
  if (!tmrl)
    return;
  // First, check the tmr1 clock source bit to see if we are  changing from
  // internal to external (or vice versa) clocks.
  if( diff & (TMRxCS0 | TMRxCS1 | TxSOSCEN))
    tmrl->new_clock_source();

  if( diff & TMRxON)
    tmrl->on_or_off(value.get() & TMRxON);
  else  if( diff & (TxCKPS0 | TxCKPS1 ))
    tmrl->update();

}

CCPTMRS0::CCPTMRS0(CCPTMRS *_ccptmrs, Processor *pCpu, const char *pName, const char *pDesc)
	: sfr_register(pCpu, pName, pDesc), bit_mask(0xdb), ccptmrs(_ccptmrs)
{}
CCPTMRS0::~CCPTMRS0()
{
}

void CCPTMRS0::put(uint reg_value)
{
  uint new_value = reg_value & bit_mask;

  uint diff = value.get() ^ new_value;
  value.put(new_value);

  if (diff) ccptmrs->update0(new_value);
}

CCPTMRS1::CCPTMRS1(CCPTMRS *_ccptmrs, Processor *pCpu, const char *pName, const char *pDesc)
	: sfr_register(pCpu, pName, pDesc), bit_mask(0x0f), ccptmrs(_ccptmrs)
{}
CCPTMRS1::~CCPTMRS1()
{
}

void CCPTMRS1::put(uint reg_value)
{
  uint new_value = reg_value & bit_mask;

  uint diff = value.get() ^ new_value;
  value.put(new_value);

  if (diff) ccptmrs->update1(new_value);
}

CCPTMRS::CCPTMRS(Processor *pCpu)
    : ccptmrs0(this, pCpu, "ccptmrs0", "PWM Timer Selection Control Register 0"),
     ccptmrs1(this, pCpu, "ccptmrs1", "PWM Timer Selection Control Register 1"),
	t2(0), t4(0), t6(0),
	ccp1(0), ccp2(0), ccp3(0), ccp4(0), ccp5(0),
	last_value0(0), last_value1(0)
{
}
CCPTMRS::~CCPTMRS()
{
}

void CCPTMRS::change(CCPCON *ccp, uint old, uint val)
{
    switch (old)
    {
    case 0:
	t2->rm_ccp(ccp);
	break;

    case 1:
	t4->rm_ccp(ccp);
	break;

    case 2:
	t6->rm_ccp(ccp);
	break;
    };
    switch (val)
    {
    case 0:
	t2->add_ccp(ccp);
        ccp->set_tmr2(t2);
	break;

    case 1:
        ccp->set_tmr2(t4);
	t4->add_ccp(ccp);
	break;

    case 2:
        ccp->set_tmr2(t6);
	t6->add_ccp(ccp);
	break;
    };
}
void CCPTMRS::update0(uint reg_value)
{
    uint diff = last_value0 ^ reg_value;
    if (diff & (C1TSEL0 | C1TSEL1))
    {
	change(ccp1, last_value0 & (C1TSEL0 | C1TSEL1), reg_value & (C1TSEL0 | C1TSEL1));
    }
    if (diff & (C2TSEL0 | C2TSEL1))
    {
	change(ccp2, (last_value0 & (C2TSEL0 | C2TSEL1))>>3,
		(reg_value & (C2TSEL0 | C2TSEL1))>>3);
    }
    if (diff & (C3TSEL0 | C3TSEL1))
    {
	change(ccp3, (last_value0 & (C3TSEL0 | C3TSEL1))>>6,
		(reg_value & (C3TSEL0 | C3TSEL1))>>6);
    }
    last_value0 = reg_value;
}
void CCPTMRS::update1(uint reg_value)
{
}
void CCPTMRS::set_ccp(CCPCON *_c1, CCPCON *_c2, CCPCON *_c3, CCPCON *_c4, CCPCON *_c5)
{
	ccp1 = _c1;
	ccp2 = _c2;
	ccp3 = _c3;
	ccp4 = _c4;
	ccp5 = _c5;
}
void CCPTMRS::set_tmr246(TMR2 *_t2, TMR2 *_t4, TMR2 *_t6)
{
    t2 = _t2;
    t4 = _t4;
    t6 = _t6;
}

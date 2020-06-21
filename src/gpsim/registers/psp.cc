/*
   Copyright (C) 2006 Roy R Rankin

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
#include "config.h"
#include "stimuli.h"
#include "psp.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//--------------------------------------------------
//
//--------------------------------------------------


class CS_SignalSink : public SignalSink
{
public:
  CS_SignalSink(PSP *_psp)
    : m_psp(_psp)
  {
    assert(_psp);
  }
  virtual void release(){ delete this;}

  void setSinkState(char new3State)
  {
    m_psp->setCS_State(new3State);
  }
private:
  PSP *m_psp;
};

class RD_SignalSink : public SignalSink
{
public:
  RD_SignalSink(PSP *_psp)
    : m_psp(_psp)
  {
    assert(_psp);
  }
  virtual void release(){ delete this;}

  void setSinkState(char new3State)
  {
    m_psp->setRD_State(new3State);
  }
private:
  PSP *m_psp;
};

class WR_SignalSink : public SignalSink
{
public:
  WR_SignalSink(PSP *_psp)
    : m_psp(_psp)
  {
    assert(_psp);
  }
  virtual void release(){delete this;}

  void setSinkState(char new3State)
  {
    m_psp->setWR_State(new3State);
  }
private:
  PSP *m_psp;
};


/*
 * Some devices use high bits of a TRIS register, but others
 * have a dedicated PSPCON register which is defined here
 */

PSPCON::PSPCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
}
void PSPCON::put(uint new_value)
{
  uint mask = (PSP::OBF | PSP::IBF | 0x0f);
  uint fixed;

  if (! (new_value & PSP::PSPMODE))
    fixed = 0;
  else
    fixed = value.data & mask;

  value.data = (new_value & ~mask) | fixed;
}

void PSPCON::put_value(uint new_value)
{
  value.data = new_value;
}
//
// setup information for PSP module
//
void PSP::initialize( PIR_SET *_pir_set, PicPSP_PortRegister *_port_set,
	PicTrisRegister *_port_tris, sfr_register *_pspcon,
        PinModule *pin_RD, PinModule *pin_WR, PinModule *pin_CS)
{
   pir_set = _pir_set;
   parallel_port = _port_set;
   parallel_port->setPSP(this);
   parallel_tris = _port_tris;
   cntl_tris = _pspcon;
   //
   // The rest of this function allows catching of changes to PSP contol signals
   //
   if (!m_rd_sink) 
   {
      m_rd_sink = new RD_SignalSink(this);
      Not_RD = pin_RD;
      if (Not_RD)
        Not_RD->addSink(m_rd_sink);
   }
   if (!m_cs_sink) 
   {
      m_cs_sink = new CS_SignalSink(this);
      Not_CS = pin_CS;
      if (Not_CS)
        Not_CS->addSink(m_cs_sink);
   }
   if (!m_wr_sink) 
   {
      m_wr_sink = new WR_SignalSink(this);
      Not_WR = pin_WR;
      if (Not_WR)
        Not_WR->addSink(m_wr_sink);
   }

}
//
// process changes on the control pins
//
void PSP::state_control()
{
    if (! pspmode()) return;
    
    if (rd && wr && cs)	// this is an error condition
    {
	cerr << "PSP: Error CS, WR and RD must not all be low\n";
	parallel_tris->put(0xff);
	state = ST_INACTIVE;
	return;
    }
    else if (cs && rd)
    {
	parallel_tris->put(0);
	parallel_port->put_value(put_value);
	cntl_tris->put_value(cntl_tris->get() & ~OBF);
	state = ST_READ;
    }
    else if (cs && wr)
    {
	parallel_tris->put(0xff);
	get_value = parallel_port->get_value();
	state = ST_WRITE;
    }
    else
    {
  	if (state != ST_INACTIVE)
        {
	    pir_set->set_pspif();
	}

	//
	// On first bus write set IBF flag.
 	// if a second bus write occurs prior to read of pic port (portd)
	// IBOV flag is also set.
	//
	if (state == ST_WRITE)
	{
	    uint trise_val = cntl_tris->get();
	    if (trise_val & IBF)
		cntl_tris->put_value(trise_val | IBOV);
	    else
	        cntl_tris->put_value(trise_val | IBF);
	}

	parallel_tris->put(0xff);
	state = ST_INACTIVE;
    }
    return;
}
//
// The next three functions are called when their control pin change state
// The control pins are active low which is converted to active high signals
void PSP::setRD_State(char new3State)
{
	rd = new3State == '0';
	state_control();
}
void PSP::setCS_State(char new3State)
{
	cs = new3State == '0';
	state_control();
}
void PSP::setWR_State(char new3State)
{
	wr = new3State == '0';
	state_control();
}

//
// psp_put is called on write to portd when pspmode is active
// set OBF register bit and save value for next bus read
//
void PSP::psp_put(uint new_value)
{
    cntl_tris->put_value(cntl_tris->get() | OBF);
    put_value = new_value;
}
//
// psp_get is called on read of portd when pspmode is active so
// we can clear the IBF flag
//
uint PSP::psp_get(void)
{
	cntl_tris->put_value(cntl_tris->get() & ~IBF);
	return(get_value);
}

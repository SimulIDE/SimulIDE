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

#ifndef __PSP_H__
#define __PSP_H__

#include "pic-processor.h"
#include "14bit-registers.h"
#include "pic-ioports.h"
#include "pir.h"

class RD_SignalSink;
class CS_SignalSink;
class WR_SignalSink;
class PicPSP_PortRegister;
class PicTrisRegister;

class PSPCON : public sfr_register, public TriggerObject
{
public:
  PSPCON(Processor *pCpu, const char *pName, const char *pDesc);
  virtual void put(uint new_value);
  virtual void put_value(uint new_value);
};

class PSP
{
public:
  void initialize( PIR_SET *pir_set, PicPSP_PortRegister *port_set, 
        PicTrisRegister *port_tris, sfr_register *PSPcon,
        PinModule *pin_RD, PinModule *pin_CS, PinModule *pin_WR);
  void setRD_State(char new3State);
  void setCS_State(char new3State);
  void setWR_State(char new3State);
  void ParallelSetbit(int m_iobit,char cNewSinkState);
  void state_control(void);
  bool pspmode(void) { return((cntl_tris->get() & PSPMODE) == PSPMODE);}
  void psp_put(uint new_value);
  uint psp_get(void);

  PSP() { m_rd_sink=0; m_cs_sink=0; m_wr_sink=0; state=0;}

  enum {
        TRIS_MASK = 7,
        PSPMODE = 1<<4,
        IBOV    = 1<<5,
        OBF     = 1<<6,
        IBF     = 1<<7
        };
protected:

  enum {
        ST_INACTIVE = 0,
        ST_READ,
        ST_WRITE
        };

  uint put_value = 0;
  uint get_value = 0;
  int state = 0;
  
  bool rd = false;
  bool cs = false;
  bool wr = false;
  
  PIR_SET             *pir_set = nullptr;
  PicPSP_PortRegister *parallel_port = nullptr;
  PicTrisRegister     *parallel_tris = nullptr;
  sfr_register        *cntl_tris = nullptr;
  
  PinModule         *Not_RD = nullptr;
  PinModule         *Not_CS = nullptr;
  PinModule         *Not_WR = nullptr;
  
  RD_SignalSink     *m_rd_sink = nullptr;
  CS_SignalSink     *m_cs_sink = nullptr;
  WR_SignalSink     *m_wr_sink = nullptr;
};

#endif        // __PSP_H__

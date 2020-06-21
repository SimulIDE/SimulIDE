/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2003 Mike Durian
                 2006 David Barnett
		 2017 Roy R Rankin

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

#ifndef PM_RD_H
#define PM_RD_H

#include <assert.h>

#include "gpsim_classes.h"
#include "registers.h"
#include "breakpoints.h"

class pic_processor;
class PM_RD;
class PM_RW;

//---------------------------------------------------------
// PMCON1 - PM control register 1
//

class PMCON1 : public sfr_register
{
public:
enum
{
 RD    = (1<<0),
};


  PMCON1(Processor *p, PM_RD *);

  void put(uint new_value);
  uint get();

  inline void set_pm(PM_RD *pm) {pm_rd = pm;}
  inline void set_valid_bits(uint vb) { valid_bits = vb; }
  inline uint get_valid_bits() { return (valid_bits); }
  inline void set_bits(uint b) { valid_bits |= b; }
  inline void clear_bits(uint b) { valid_bits &= ~b; }

  uint valid_bits = 0;
  PM_RD *pm_rd;
};

const uint PMCON1_VALID_BITS = (PMCON1::RD);

class PMCON1_RW : public sfr_register
{
public:
enum
{
 RD    = (1<<0),
 WR    = (1<<1),
 WREN  = (1<<2),
 WRERR = (1<<3),
 FREE  = (1<<4),
 LWLO  = (1<<5),
 CFGS  = (1<<6)
};


  PMCON1_RW(Processor *pCpu, PM_RW *pRW)
    : sfr_register(pCpu, "pmcon1", "Program Memory Read Write Control 1"),
    pm_rw(pRW) {}


  void put(uint new_value);

  inline void set_pm(PM_RW *pm) {pm_rw = pm;}
  inline void set_valid_bits(uint vb) { valid_bits = vb; }
  inline uint get_valid_bits() { return (valid_bits); }
  inline void set_bits(uint b) { valid_bits |= b; }
  inline void clear_bits(uint b) { valid_bits &= ~b; }

  uint valid_bits;
  PM_RW *pm_rw;
};

class PMCON2 : public sfr_register
{
public:

enum STATES
{
  WAITING = 0,
  HAVE_0x55,
  READY_FOR_WRITE,
};



  PMCON2(Processor *pCpu, PM_RW *pRW)
    : sfr_register(pCpu, "pmcon2", "Program Memory Read Write Control 2"),
    pm_rw(pRW), lock1(false), state(WAITING)  {;}


  void put(uint new_value);
  PM_RW *pm_rw;

  inline bool is_ready_for_write() {return(state == READY_FOR_WRITE);}
  inline void unarm() { state = WAITING; }


  bool  lock1;
  enum STATES state;

};
//
// PMDATA - PM data register
//

class PMDATA : public sfr_register
{
public:

  PMDATA(Processor *p, const char *pName);

  void put(uint new_value);
  uint get();

};

//
// PMADR - PM address register
//

class PMADR : public sfr_register
{
public:

  PMADR(Processor *p, const char *pName);

  void put(uint new_value);
  uint get();
};


//------------------------------------------------------------------------

// For storing callback and cpu ptr and grouping PM regs
class PM_RD :  public TriggerObject
{
public:
  static const uint READ_CYCLES = 2;

  PM_RD(pic_processor *p);
  //virtual void set_cpu(pic_processor *p) { cpu = p; }

  virtual void callback();
  virtual void start_read();  

  inline virtual PMCON1 *get_reg_pmcon1() { return (&pmcon1); }
  inline virtual PMDATA *get_reg_pmdata() { return (&pmdata); }
  inline virtual PMDATA *get_reg_pmdath() { return (&pmdath); }
  inline virtual PMADR *get_reg_pmadr() { return (&pmadr); }
  inline virtual PMADR *get_reg_pmadrh() { return (&pmadrh); }

  //protected:
  pic_processor *cpu;

  PMCON1 pmcon1;
  PMDATA pmdata;
  PMDATA pmdath;
  PMADR  pmadr;
  PMADR  pmadrh;

  uint rd_adr;          // latched adr

};

//------------------------------------------------------------------------
class PM_RW : public PM_RD
{
public:
  inline virtual PMCON1_RW *get_reg_pmcon1_rw() { return (&pmcon1_rw); }
  inline virtual PMCON2 *get_reg_pmcon2() { return (&pmcon2); }
  PMCON1_RW pmcon1_rw;
  PMCON2  pmcon2;
  void set_write_enable() { write_enable = true;}
  PM_RW(pic_processor *pCpu);
  ~PM_RW();
  virtual void callback();
  virtual void start_read();  
  virtual void write_latch();  // Place data in write latch
  virtual void write_row();  // Write latches to program memory
  virtual void erase_row();

#define LATCH_EMPTY 0x3fff
  bool write_enable;
  int  num_latches;
  uint *write_latches;

};

#endif /* PM_RD_H */

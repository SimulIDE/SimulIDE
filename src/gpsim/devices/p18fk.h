/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2010,2015 Roy R Rankin


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

#ifndef __P18FK_H__
#define __P18FK_H__

#include "16bit-processors.h"
#include "eeprom.h"
#include "psp.h"
#include "pir.h"
#include "comparator.h"
#include "spp.h"
#include "ctmu.h"

#ifdef IESO
#undef IESO
#endif
#define IESO  (1<<7)

class PicPortRegister;
class PicTrisRegister;
class PicLatchRegister;
class ADCON0_V2;
class ADCON1_V2;
class ADCON2_V2;


class P18F14K22 : public _16bit_processor
{
 public:

  ADCON0_V2	adcon0;
  ADCON1_2B	adcon1;
  ADCON2_V2	adcon2;
  FVRCON_V2	vrefcon0;
  DACCON0_V2	vrefcon1;
  DACCON1	vrefcon2;
  ECCPAS        eccp1as;
  PWM1CON       pwm1con;
  OSCTUNE       osctune;

  ComparatorModule2 comparator;
  ANSEL_2A	ansela;
  ANSEL_2A	anselb;
  IOC		slrcon;  // using IOC for it's mask this register is a NOP in gpsim
//  sfr_register	slrcon;  // this register is a NOP in gpsim
  CCPTMRS	ccptmrs;
  PSTRCON	pstrcon;
  SR_MODULE	sr_module;
  SSP1_MODULE   ssp1;
  OSCCON2	osccon2;

  WPU		*wpua;
  WPU		*wpub;
  IOC		*ioca;
  IOC		*iocb;
  virtual bool HasCCP2(void) { return false; }
  virtual bool MovedReg() { return false;}
  virtual OSCCON * getOSCCON(void);

  virtual PROCESSOR_TYPE isa(){return _P18F14K22_;}
  P18F14K22(const char *_name=0, const char *desc=0);
  ~P18F14K22();
  static Processor *construct(const char *name);

  virtual PROCESSOR_TYPE base_isa(){return _PIC18_PROCESSOR_;};
  virtual uint access_gprs() { return 0x60; };
  virtual uint program_memory_size() const { return 0x4000; };
  virtual uint last_actual_register () const { return 0x01FF;};
  virtual uint eeprom_memory_size() const { return 256; };
  virtual void create_iopin_map();
  virtual void create_sfr_map();
  virtual void osc_mode(uint value);
  virtual void set_config3h(int64_t x);
  virtual string string_config3h(int64_t x)
	{return string("fix string_config3h");}
  virtual uint get_device_id() { return (0x20 << 8)|(0x3 <<5); }

  void create();

  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as the 18Fxxx devices use an EEPROM with PIR
   assert(0);
  }
  virtual void set_eeprom_pir(EEPROM_PIR *ep) { eeprom = ep; }
  virtual EEPROM_PIR *get_eeprom() { return ((EEPROM_PIR *)eeprom); }
};



class P18F26K22 : public _16bit_processor
{
 public:

  PicPortRegister  *m_porte;
  PicTrisRegister  *m_trise;
  PicLatchRegister *m_late;


  ADCON0_V2	adcon0;
  ADCON1_2B	adcon1;
  ADCON2_V2	adcon2;
  FVRCON_V2	vrefcon0;
  DACCON0_V2	vrefcon1;
  DACCON1	vrefcon2;
  ECCPAS        eccp1as;
  ECCPAS        eccp2as;
  ECCPAS        eccp3as;
  PWM1CON       pwm1con;
  PWM1CON       pwm2con;
  PWM1CON       pwm3con;
  OSCTUNE      osctune;

  T1GCON	t1gcon;
  T5CON		*t3con2;
  T1GCON 	t3gcon;
  TMRL		tmr5l;
  TMRH		tmr5h;
  T5CON		*t5con;
  T1GCON	t5gcon;
  T2CON        t4con;
  PR2          pr4;
  TMR2         tmr4;
  T2CON        t6con;
  PR2          pr6;
  TMR2         tmr6;
  PIR3v3       pir3;
  PIE          pie3;
  PIR4v1       pir4;
  PIE          pie4;
  PIR5v1       pir5;
  PIE          pie5;
  sfr_register ipr3;
  sfr_register ipr4;
  sfr_register ipr5;
  CCPCON       ccp3con;
  CCPRL        ccpr3l;
  CCPRH        ccpr3h;
  CCPCON       ccp4con;
  CCPRL        ccpr4l;
  CCPRH        ccpr4h;
  CCPCON       ccp5con;
  CCPRL        ccpr5l;
  CCPRH        ccpr5h;
  USART_MODULE         usart2;
  ComparatorModule2 comparator;
  sfr_register pmd0;
  sfr_register pmd1;
  sfr_register pmd2;
  ANSEL_2B	ansela;
  ANSEL_2B	anselb;
  ANSEL_2B	anselc;
  IOC		slrcon;  // using IOC for it's mask this register is a NOP in gpsim
  CCPTMRS	ccptmrs;
  PSTRCON	pstr1con;
  PSTRCON	pstr2con;
  PSTRCON	pstr3con;
  SR_MODULE	sr_module;
  SSP1_MODULE   ssp1;
  SSP1_MODULE   ssp2;
  CTMU		ctmu;
  HLVDCON	hlvdcon;
  OSCCON2	osccon2;

  WPU		*wpub;
  IOC		*iocb;
  virtual bool HasCCP2(void) { return false; }; // at wrong address
  virtual bool MovedReg() { return true;}
  virtual OSCCON * getOSCCON(void)	{ return new OSCCON_HS(this, "osccon", "OSC Control"); }

  virtual PROCESSOR_TYPE isa(){return _P18F26K22_;};
  P18F26K22(const char *_name=0, const char *desc=0);
  ~P18F26K22();
  static Processor *construct(const char *name);

  virtual PROCESSOR_TYPE base_isa(){return _PIC18_PROCESSOR_;};
  virtual uint access_gprs() { return 0x60; };
  virtual uint program_memory_size() const { return 0x8000; };
  virtual uint last_actual_register () const { return 0x0F37;};
  virtual uint eeprom_memory_size() const { return 1024; };
  virtual void create_iopin_map();
  virtual void create_sfr_map();
  virtual void osc_mode(uint value);
  virtual void set_config3h(int64_t x);
  virtual string string_config3h(int64_t x)
	{return string("fix string_config3h");}
  virtual uint get_device_id() { return (0x54 << 8)|(0x3 <<5); }

  void create();
  virtual void update_vdd();

  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as the 18Fxxx devices use an EEPROM with PIR
   assert(0);
  }
  virtual void set_eeprom_pir(EEPROM_PIR *ep) { eeprom = ep; }
  virtual EEPROM_PIR *get_eeprom() { return ((EEPROM_PIR *)eeprom); }
};



#endif

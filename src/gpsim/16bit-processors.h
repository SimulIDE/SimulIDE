/*
   Copyright (C) 1998 T. Scott Dattalo

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



#ifndef __16_BIT_PROCESSORS_H__
#define __16_BIT_PROCESSORS_H__

#include "pic-processor.h"
#include "pic-ioports.h"
#include "intcon.h"
#include "16bit-registers.h"
#include "16bit-tmrs.h"
#include "pir.h"
#include "uart.h"
#include "a2dconverter.h"
#include "a2d_v2.h"
//#include "value.h"

// forward references

extern instruction *disasm16 (pic_processor *cpu, uint address, uint inst);

class ConfigMemory;


//------------------------------------------------------------------------
//
//    pic_processor
//        |
//        \__ _16bit_processor
//
// Base class for the 16bit PIC processors
//
class _16bit_processor : public pic_processor
{

public:

  static const uint CONFIG1L = 0x300000;
  static const uint CONFIG1H = 0x300001;
  static const uint CONFIG2L = 0x300002;
  static const uint CONFIG2H = 0x300003;
  static const uint CONFIG3L = 0x300004;
  static const uint CONFIG3H = 0x300005;
  static const uint CONFIG4L = 0x300006;
  static const uint CONFIG4H = 0x300007;
  static const uint CONFIG5L = 0x300008;
  static const uint CONFIG5H = 0x300009;
  static const uint CONFIG6L = 0x30000A;
  static const uint CONFIG6H = 0x30000B;
  static const uint CONFIG7L = 0x30000C;
  static const uint CONFIG7H = 0x30000D;

  // The early 18xxx parts all contain ports A,B,C
  PicPortRegister  *m_porta;
  PicTrisRegister  *m_trisa;
  PicLatchRegister *m_lata;

  PicPortBRegister *m_portb;
  PicTrisRegister  *m_trisb;
  PicLatchRegister *m_latb;

  PicPortRegister  *m_portc;
  PicTrisRegister  *m_trisc;
  PicLatchRegister *m_latc;


  sfr_register adresl;
  sfr_register adresh;
  INTCON_16    intcon;
  INTCON2      intcon2;
  INTCON3      intcon3;
  BSR          bsr;
  TMR0_16      tmr0l;
  TMR0H        tmr0h;
  T0CON        t0con;
  RCON         rcon;
  PIR1v2       pir1;
  sfr_register ipr1;
  sfr_register ipr2;
  T1CON        *t1con;
  PIE          pie1;
  PIR2v2       *pir2;
  PIE          pie2;
  T2CON        t2con;
  PR2          pr2;
  TMR2         tmr2;
  TMRL         tmr1l;
  TMRH         tmr1h;
  CCPCON       ccp1con;
  CCPRL        ccpr1l;
  CCPRH        ccpr1h;
  CCPCON       ccp2con;
  CCPRL        ccpr2l;
  CCPRH        ccpr2h;
  TMRL         tmr3l;
  TMRH         tmr3h;
  T3CON        *t3con;
  PIR_SET_2    pir_set_def;

  OSCCON      *osccon;
  LVDCON       lvdcon;
  WDTCON       wdtcon;

  sfr_register prodh,prodl;

  sfr_register pclatu;

  Fast_Stack   fast_stack;
  Indirect_Addressing  ind0;
  Indirect_Addressing  ind1;
  Indirect_Addressing  ind2;
  USART_MODULE         usart;

  TBL_MODULE           tbl;
  TMR2_MODULE          tmr2_module;
  TMR3_MODULE          tmr3_module;
  SSP_MODULE           ssp;

  // Some configuration stuff for stripping down where needed
  virtual bool HasPortC(void) { return true; };
  virtual bool HasCCP2(void) { return true; };
  virtual bool MovedReg() { return false;}
  virtual bool T3HasCCP() { return true;}

  virtual OSCCON * getOSCCON(void)	{ return new OSCCON(this, "osccon", "OSC Control"); }

  void interrupt();
  virtual void create();// {return;};
  virtual PROCESSOR_TYPE isa(){return _PIC17_PROCESSOR_;};
  virtual PROCESSOR_TYPE base_isa(){return _PIC17_PROCESSOR_;};
  virtual uint access_gprs() { return 0x80; };
  virtual instruction * disasm (uint address, uint inst)
    {
      return disasm16(this, address, inst);
    }
  virtual void create_sfr_map();
  virtual void delete_sfr_map();
  virtual void create_config_memory();

  // Return the portion of pclath that is used during branching instructions
  virtual uint get_pclath_branching_jump()
  {
    return ((pclatu.value.get()<<16) | ((pclath->value.get() & 0xf8)<<8));
  }

  // Return the portion of pclath that is used during modify PCL instructions
  virtual uint get_pclath_branching_modpcl()
  {
    return ((pclatu.value.get()<<16) | ((pclath->value.get() & 0xff)<<8));
  }

  virtual void option_new_bits_6_7(uint);

  // Declare a set of functions that will allow the base class to
  // get information about the derived classes. NOTE, the values returned here
  // will cause errors if they are used (in some cases)
  // -- the derived classes must define their parameters appropriately.

  virtual uint register_memory_size () const { return 0x1000;};
  virtual uint last_actual_register () const { return 0x0F7F;};
  virtual void set_out_of_range_pm(uint address, uint value);

  virtual void create_iopin_map();

  virtual int  map_pm_address2index(int address) const {return address/2;};
  virtual int  map_pm_index2address(int index) const {return index*2;};
  virtual uint get_program_memory_at_address(uint address);
  virtual uint get_config_word(uint address);
  virtual uint get_device_id() { return 0;}
  virtual bool set_config_word(uint address, uint cfg_word);
  virtual uint configMemorySize() { return CONFIG7H-CONFIG1L+1; }
  virtual uint IdentMemorySize() const { return 4; }    // four words default (18F)
  virtual void enter_sleep();
  virtual void exit_sleep();
  virtual void osc_mode(uint );
  virtual void set_extended_instruction(bool);
  virtual bool extended_instruction() {return extended_instruction_flag;}

  static pic_processor *construct();
  _16bit_processor(const char *_name=0, const char *desc=0);
  virtual ~_16bit_processor();

  uint getCurrentDisasmAddress() { return m_current_disasm_address;}
  uint getCurrentDisasmIndex()   { return m_current_disasm_address/2;}
  void setCurrentDisasmAddress(uint a) { m_current_disasm_address =a; }
  virtual void init_pir2(PIR *pir2, uint bitMask);

protected:
  uint m_current_disasm_address;  // Used only when .hex/.cod files are loaded

  uint idloc[4];    ///< ID locations - not all 16-bit CPUs have 8 bytes
  bool	extended_instruction_flag;	// Instruction set extension and Indexed Addressing 

  uint last_register;
};

class _16bit_compat_adc : public _16bit_processor 
{
    public:

      ADCON0       *adcon0;
      ADCON1       *adcon1;

      _16bit_compat_adc(const char *_name=0, const char *desc=0);
      ~_16bit_compat_adc();

      virtual void create();
      virtual void create_sfr_map();
      virtual void a2d_compat();
};

class _16bit_v2_adc : public _16bit_processor 
{
    public:

      ADCON0_V2       *adcon0;
      ADCON1_V2       *adcon1;
      ADCON2_V2       *adcon2;

      _16bit_v2_adc(const char *_name=0, const char *desc=0);
      ~_16bit_v2_adc();
      virtual void create(int nChannels);
};

#define cpu16 ( (_16bit_processor *)cpu)

#define FOSC0   (1<<0)
#define FOSC1   (1<<1)
#define FOSC2   (1<<2)
// FOSC3 may not be used
#define FOSC3   (1<<3)
#define PLLCFG  (1<<4)
#define OSCEN   (1<<5)
//RRR#define IESO    (1<<7)

//------------------------------------------------------------------------
// Config1H - default 3 bits FOSC

class Config1H : public ConfigWord 
{
    #define CONFIG1H_default (OSCEN | FOSC2 | FOSC1 | FOSC0)
    public:
      Config1H(_16bit_processor *pCpu, uint addr)
        : ConfigWord("CONFIG1H", CONFIG1H_default, "Oscillator configuration", pCpu, addr)
      {
        set(CONFIG1H_default);
      }

      virtual void set(int64_t v)
      {
        Integer::set(v);

        if (m_pCpu)
        {
        //RRRm_pCpu->osc_mode(v & ( FOSC2 | FOSC1 | FOSC0));
        m_pCpu->osc_mode(v);
        }
      }

      virtual string toString();
};

//------------------------------------------------------------------------
// Config1H -  4 bits FOSC

class Config1H_4bits : public ConfigWord 
{
    public:
      Config1H_4bits(_16bit_processor *pCpu, uint addr, uint def_val)
        : ConfigWord("CONFIG1H", def_val, "Oscillator configuration", pCpu, addr)
      {
        set(def_val);
      }

      virtual void set(int64_t v)
      {
        Integer::set(v);

        if (m_pCpu)
        {
        //m_pCpu->osc_mode(v & ( FOSC3 | FOSC2 | FOSC1 | FOSC0));
        m_pCpu->osc_mode(v);
        }
      }

      virtual string toString();
};

class Config3H : public ConfigWord 
{
    public:
      Config3H(_16bit_processor *pCpu, uint addr, uint def_val)
        : ConfigWord("CONFIG3H", def_val, "Configuration Register 3 High", pCpu, addr)
      {
            set(def_val);
      }
      
      virtual void set(int64_t v)
      {
        Integer::set(v);
        if (m_pCpu) m_pCpu->set_config3h(v);
      }

      virtual string toString()
      {
           int64_t i64;
           get(i64);

            if (m_pCpu) return(m_pCpu->string_config3h(i64));
            else        return string ("m_PCpu not defined");
      }
};

#endif

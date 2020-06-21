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

#ifndef __P12X_H__
#define __P12X_H__

#include "12bit-processors.h"
#include "pic-ioports.h"
#include "a2dconverter.h"


class P12_I2C_EE;
class P12bitBase;

class P12_OSCCON : public  sfr_register
{
public:
  enum {
	FOSC4 = 1 << 0
  };

  P12_OSCCON(Processor *pCpu, const char *pName, const char *pDesc)
    : sfr_register(pCpu,pName,pDesc), m_CPU(0)
  {
  }

    void put(uint new_value);
    void set_cpu(P12bitBase *pCPU) { m_CPU = pCPU;}
private:
  P12bitBase *m_CPU;
};


class GPIO : public PicPortRegister
{
public:
  GPIO(P12bitBase *pCpu, const char *pName, const char *pDesc,
       uint numIopins, 
       uint enableMask,
       uint resetMask  = (1 << 3),
       uint wakeupMask = 0x0b,
       uint configMaskMCLRE = (1<<4));
  void setbit(uint bit_number, char new_value);
  void setPullUp ( bool bNewPU , bool mclr);

private:
  P12bitBase *m_CPU;
  bool m_bPU;
  uint m_resetMask;
  uint m_wakeupMask;
  uint m_configMaskMCLRE;
};

//--------------------------------------------------------
/*
 *         IN_SignalControl is used to set a pin as input
 *                 regardless of the setting to the TRIS register
 */
class IN_SignalControl : public SignalControl
{
public:
  IN_SignalControl(){ }
  ~IN_SignalControl(){}
  char getState() { return '1'; }
  void release() { }
};

//--------------------------------------------------------
/*
 *         OUT_SignalControl is used to set a pin as input
 *                 regardless of the setting to the TRIS register
 */
class OUT_SignalControl : public SignalControl
{
public:
  OUT_SignalControl(){}
  ~OUT_SignalControl(){}
  char getState() { return '0'; }
  void release() { }
};

//--------------------------------------------------------
/*
 *         OUT_DriveControl is used to override output
 *                 regardless of the setting to the GPIO register
 */
class OUT_DriveControl : public SignalControl
{
public:
  OUT_DriveControl(){}
  ~OUT_DriveControl(){}
  char getState() { return '1'; }
  void release() { }
};

class P12bitBase : public  _12bit_processor
{
public:
  GPIO            *m_gpio;
  PicTrisRegister *m_tris;
  P12_OSCCON 	  osccal;  
  

  virtual PROCESSOR_TYPE isa(){return _P12C508_;};


  virtual void enter_sleep();
  virtual void create_sfr_map();
  virtual void dump_registers();
  virtual void tris_instruction(uint tris_register);
  virtual void reset(RESET_TYPE r);

  P12bitBase(const char *_name=0, const char *desc=0);
  virtual ~P12bitBase();
  static Processor *construct(const char *name);
  virtual void create_iopin_map();
  virtual void create_config_memory();

  virtual uint fsr_valid_bits()
  {
    return 0x1f;  // Assume only 32 register addresses 
  }

  virtual uint fsr_register_page_bits()
  {
    return 0;     // Assume only one register page.
  }


  virtual void option_new_bits_6_7(uint);

  IN_SignalControl *m_IN_SignalControl;
  OUT_SignalControl *m_OUT_SignalControl;
  OUT_DriveControl *m_OUT_DriveControl;

  virtual void updateGP2Source();
  virtual void freqCalibration();
  virtual void setConfigWord(uint val, uint diff);


    uint configWord;

// bits of Configuration word
    enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    WDTEN  = 1<<2,
    CP     = 1<<3,
    MCLRE  = 1<<4
  };


};

class P12C508 : public  P12bitBase
{
public:

  P12C508(const char *_name=0, const char *desc=0);
  virtual ~P12C508();
  static Processor *construct(const char *name);
  virtual void create();
  virtual uint program_memory_size() const { return 0x200; }

};

class P12F508 : public P12C508
{
public:

  P12F508(const char *_name=0, const char *desc=0);
  virtual ~P12F508();
  static Processor *construct(const char *name);
  virtual PROCESSOR_TYPE isa(){return _P12F508_;};
};

// A 12c509 is like a 12c508
class P12C509 : public P12C508
{
  public:

  virtual PROCESSOR_TYPE isa(){return _P12C509_;};

  virtual uint program_memory_size() const { return 0x400; };

  virtual void create_sfr_map();

  virtual uint fsr_valid_bits()
    {
      return 0x3f;  // 64 registers in all (some are actually aliased)
    }

  virtual uint fsr_register_page_bits()
    {
      return 0x20;  // 509 has 2 register banks
    }

  P12C509(const char *_name=0, const char *desc=0);
  ~P12C509();
  static Processor *construct(const char *name);
  virtual void create();


};

class P12F509 : public P12C509
{
public:
  P12F509(const char *_name=0, const char *desc=0);
  virtual ~P12F509();
  static Processor *construct(const char *name);
  virtual PROCESSOR_TYPE isa(){return _P12F509_;}
};

// 12F510 - like a '509, but has an A2D and a comparator.
class P12F510 : public P12F509
{
public:
  P12F510(const char *_name=0, const char *desc=0);
  virtual ~P12F510();
  static Processor *construct(const char *name);
  virtual PROCESSOR_TYPE isa(){return _P12F510_;}
};

// A 12CE518 is like a 12c508
class P12CE518 : public P12C508
{
    public:

      virtual PROCESSOR_TYPE isa(){return _P12CE518_;};
      virtual void tris_instruction(uint tris_register);

      P12CE518(const char *_name=0, const char *desc=0);
      ~P12CE518();
      static Processor *construct(const char *name);
      virtual void create();
      virtual void create_iopin_map();
      virtual void freqCalibration();
      
    private:
      P12_I2C_EE *m_eeprom;
      Stimulus_Node *scl;
      Stimulus_Node	*sda;
      IO_bi_directional_pu *io_scl;
      IO_open_collector *io_sda;
  
};


// A 12ce519 is like a 12ce518
class P12CE519 : public P12CE518
{
  public:

  virtual PROCESSOR_TYPE isa(){return _P12CE519_;};

  virtual uint program_memory_size() const { return 0x400; };

  virtual void create_sfr_map();

  virtual uint fsr_valid_bits()
    {
      return 0x3f;  // 64 registers in all (some are actually aliased)
    }

  virtual uint fsr_register_page_bits()
    {
      return 0x20;  // 519 has 2 register banks
    }

  P12CE519(const char *_name=0, const char *desc=0);
  ~P12CE519();
  static Processor *construct(const char *name);
  virtual void create();


};



//  10F200
class P10F200 : public P12bitBase
{
public:

  virtual PROCESSOR_TYPE isa(){return _P10F200_;};
  virtual uint program_memory_size() const { return 0x100; };

  P10F200(const char *_name=0, const char *desc=0);
  virtual ~P10F200();

  static Processor *construct(const char *name);
  virtual void create();
  virtual void create_iopin_map();
  // GP2 can be driven by either FOSC/4, TMR 0, or the GP I/O driver
  virtual void updateGP2Source();
  virtual void freqCalibration();
  // WDT causes reset on sleep
  virtual bool exit_wdt_sleep() { return false; } 

};


// A 10F202 is like a 10f200
class P10F202 : public P10F200
{
public:

  virtual PROCESSOR_TYPE isa(){return _P10F202_;};
  virtual uint program_memory_size() const { return 0x200; };

  P10F202(const char *_name=0, const char *desc=0);
  ~P10F202();
  static Processor *construct(const char *name);
  virtual void create();

};

class CMCON0;
// A 10F204 is like a 10f200
class P10F204 : public P10F200
{
public:

  virtual PROCESSOR_TYPE isa(){return _P10F204_;};

  P10F204(const char *_name=0, const char *desc=0);
  ~P10F204();

  static Processor *construct(const char *name);
  virtual void create();
  // GP2 can be driven by either FOSC/4, COUT, TMR 0, or the GP I/O driver
  virtual void updateGP2Source();
protected:
  CMCON0 *m_cmcon0;
};

// A 10F220 is based on 10f200
class P10F220 : public P10F200
{
public:

  ADCON0_10 adcon0;
  ADCON1 adcon1;
  sfr_register  adres;

  virtual PROCESSOR_TYPE isa(){return _P10F220_;};

  P10F220(const char *_name=0, const char *desc=0);
  ~P10F220();
  static Processor *construct(const char *name);
  virtual void create();
  virtual void enter_sleep();
  virtual void exit_sleep();
  virtual void setConfigWord(uint val, uint diff);

// Bits of configuration word
    enum {
    IOSCFS  = 1<<0,
    NOT_MCPU  = 1<<1,
   };
protected:
};

// A 10F220 is like a 10f220
class P10F222 : public P10F220
{
public:

  virtual PROCESSOR_TYPE isa(){return _P10F222_;};

  P10F222(const char *_name=0, const char *desc=0);
  ~P10F222();
  virtual uint program_memory_size() const { return 0x200; };
  static Processor *construct(const char *name);
  virtual void create();
  // GP2 can be driven by either FOSC/4, TMR 0, or the GP I/O driver
  //virtual void updateGP2Source();
protected:
};

class P16F505 : public P12bitBase
{
public:
    enum {
        FOSC0  = 1<<0,
        FOSC1  = 1<<1,
        FOSC2  = 1<<2,
        WDTEN  = 1<<3,
        CP     = 1<<4,
        MCLRE  = 1<<5
    };

    P16F505(const char *_name=0, const char *desc=0);
    virtual ~P16F505();

    static Processor *construct(const char *name);
    virtual PROCESSOR_TYPE isa() { return _P16F505_; };

    virtual void create();

    virtual void create_iopin_map();
    virtual void create_sfr_map();
    virtual void create_config_memory();
    virtual void tris_instruction(uint tris_register);
    virtual void setConfigWord(uint val, uint diff);
    virtual void updateGP2Source();
    virtual void option_new_bits_6_7(uint bits);
    virtual void reset(RESET_TYPE r);
    virtual void dump_registers();

    virtual uint program_memory_size() const { return 0x400; }
    virtual uint fsr_valid_bits() { return 0x7f; }
    virtual uint fsr_register_page_bits() { return 0x60; }

    GPIO            *m_portb;
    GPIO            *m_portc;
    PicTrisRegister *m_trisb;
    PicTrisRegister *m_trisc;
};

#endif //  __P12X_H__

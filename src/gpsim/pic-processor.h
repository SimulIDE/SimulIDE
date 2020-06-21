/*
   Copyright (C) 1998-2000 T. Scott Dattalo
   Copyright (C) 2013      Roy R. Rankin

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

#ifndef __PIC_PROCESSORS_H__
#define __PIC_PROCESSORS_H__
//#include <glib.h>
#include <stdio.h>
#include <iostream>

#include "gpsim_classes.h"
#include "processor.h"
#include "pic-registers.h"
#include "14bit-registers.h"
#include "trigger.h"

class EEPROM;
class instruction;
class Register;
class sfr_register;
class pic_register;
class ConfigMemory;

enum PROCESSOR_TYPE
{
  _PIC_PROCESSOR_,
  _14BIT_PROCESSOR_,
  _14BIT_E_PROCESSOR_, // 14bit enhanced processor
  _12BIT_PROCESSOR_,
  _PIC17_PROCESSOR_,
  _PIC18_PROCESSOR_,
  _P10F200_,
  _P10F202_,
  _P10F204_,
  _P10F206_,
  _P10F220_,
  _P10F222_,
  _P10F320_,
  _P10LF320_,
  _P10F322_,
  _P10LF322_,
  _P12C508_,
  _P12C509_,
  _P12F508_,
  _P12F509_,
  _P12F510_,
  _P12F629_,
  _P12F675_,
  _P12F683_,
  _P12F1822_,
  _P12LF1822_,
  _P12F1840_,
  _P12LF1840_,
  _P16C84_,
  _P16CR83_,
  _P16CR84_,
  _P12CE518_,
  _P12CE519_,
  _P16F83_,
  _P16F84_,
  _P16C71_,
  _P16C712_,
  _P16C716_,
  _P16C54_,
  _P16C55_,
  _P16C56_,
  _P16C61_,
  _P16C62_,
  _P16C62A_,
  _P16CR62_,
  _P16F505_,
  _P16F627_,
  _P16F628_,
  _P16F630_,
  _P16F631_,
  _P16F648_,
  _P16F676_,
  _P16F677_,
  _P16F684_,
  _P16F685_,
  _P16F687_,
  _P16F689_,
  _P16F690_,
  _P16C63_,
  _P16C64_,
  _P16C64A_,
  _P16CR64_,
  _P16C65_,
  _P16C65A_,
  _P16C72_,
  _P16C73_,
  _P16C74_,
  _P16F73_,
  _P16F74_,
  _P16F716_,
  _P16F87_,
  _P16F88_,
  _P16F818_,
  _P16F819_,
  _P16F871_,
  _P16F873_,
  _P16F873A_,
  _P16F874_,
  _P16F874A_,
  _P16F876_,
  _P16F876A_,
  _P16F877_,
  _P16F877A_,
  _P16F882_,
  _P16F883_,
  _P16F884_,
  _P16F886_,
  _P16F887_,
  _P16F913_,
  _P16F914_,
  _P16F916_,
  _P16F917_,
  _P16F1788_,
  _P16F1503_,
  _P16LF1503_,
  _P16LF1788_,
  _P16F1789_,
  _P16F1823_,
  _P16LF1823_,
  _P16F1825_,
  _P16LF1825_,
  _P17C7xx_,
  _P17C75x_,
  _P17C752_,
  _P17C756_,
  _P17C756A_,
  _P17C762_,
  _P17C766_,
  _P18Cxx2_,
  _P18C2x2_,
  _P18C242_,
  _P18F242_,
  _P18F248_,
  _P18F258_,
  _P18F448_,
  _P18F458_,
  _P18C252_,
  _P18F252_,
  _P18C442_,
  _P18C452_,
  _P18F442_,
  _P18F452_,
  _P18F1220_,
  _P18F1320_,
  _P18F14K22_,
  _P18F2221_,
  _P18F2321_,
  _P18F2420_,
  _P18F2455_,
  _P18F2520_,
  _P18F2525_,
  _P18F2550_,
  _P18F26K22_,
  _P18F2620_,
  _P18F4221_,
  _P18F4321_,
  _P18F4420_,
  _P18F4455_,
  _P18F4520_,
  _P18F4550_,
  _P18F4620_,
  _P18F6520_,
};

// Configuration modes.  DELETE THIS...
//  The configuration mode bits are the config word bits remapped.
//  The remapping removes processor dependent bit definitions.
class ConfigMode 
{
    public:

      enum {
        CM_FOSC0 = 1<<0,    // FOSC0 and  FOSC1 together define the PIC clock
        CM_FOSC1 = 1<<1,    // All PICs todate have these two bits, but the
                            // ones with internal oscillators use them differently
        CM_WDTE =  1<<2,    // Watch dog timer enable
        CM_CP0 =   1<<3,    // Code Protection
        CM_CP1 =   1<<4,
        CM_PWRTE = 1<<5,    // Power on/Reset timer enable
        CM_BODEN = 1<<6,    // Brown out detection enable
        CM_CPD =   1<<7,
        CM_MCLRE = 1<<8,    // MCLR enable

        CM_FOSC1x = 1<<31,   // Hack for internal oscillators
      };

      int config_mode;
      int valid_bits;

      ConfigMode() {
        config_mode = 0xffff;
        valid_bits = CM_FOSC0 | CM_FOSC1 | CM_WDTE;
      };

      virtual ~ConfigMode(){}

      virtual void set_config_mode(int new_value) { config_mode = new_value & valid_bits;};
      virtual void set_valid_bits(int new_value) { valid_bits = new_value;};
      void set_fosc0(){config_mode |= CM_FOSC0;};
      void clear_fosc0(){config_mode &= ~CM_FOSC0;};
      bool get_fosc0(){return (config_mode & CM_FOSC0);};
      void set_fosc1(){config_mode |= CM_FOSC1;};
      void clear_fosc1(){config_mode &= ~CM_FOSC1;};
      bool get_fosc1(){return (0 != (config_mode & CM_FOSC1));};
      bool get_fosc1x(){return (0 != (config_mode & CM_FOSC1x));};
      void set_fosc01(int v)
      {
        config_mode = (config_mode & ~(CM_FOSC0 | CM_FOSC1)) |
          (v & (CM_FOSC0 | CM_FOSC1));
      }

      void set_cp0()  {config_mode |= CM_CP0;  valid_bits |= CM_CP0;};
      void clear_cp0(){config_mode &= ~CM_CP0; valid_bits |= CM_CP0;};
      bool get_cp0()  {return (0 != (config_mode & CM_CP0));};
      void set_cp1()  {config_mode |= CM_CP1;  valid_bits |= CM_CP1;};
      void clear_cp1(){config_mode &= ~CM_CP1; valid_bits |= CM_CP1;};
      bool get_cp1()  {return (0 != (config_mode & CM_CP1));};

      void enable_wdt()  {config_mode |= CM_WDTE;};
      void disable_wdt() {config_mode &= ~CM_WDTE;};
      void set_wdte(bool b) { config_mode = b ? (config_mode | CM_WDTE) : (config_mode & ~CM_WDTE); }
      bool get_wdt()     {return (0 != (config_mode & CM_WDTE));};

      void set_mclre(bool b) { config_mode = b ? (config_mode | CM_MCLRE) : (config_mode & ~CM_MCLRE); }
      bool get_mclre()     {return (0 != (config_mode & CM_MCLRE));};

      void enable_pwrte()   {config_mode |= CM_PWRTE;  valid_bits |= CM_PWRTE;};
      void disable_pwrte()  {config_mode &= ~CM_PWRTE; valid_bits |= CM_PWRTE;};
      void set_pwrte(bool b) { config_mode = b ? (config_mode | CM_PWRTE) : (config_mode & ~CM_PWRTE); }
      bool get_pwrte()      {return (0 != (config_mode & CM_PWRTE));};
      bool is_valid_pwrte() {return (0 != (valid_bits & CM_PWRTE));};

      virtual void print();
};


//---------------------------------------------------------
// Watch Dog Timer
//

class WDT : public TriggerObject, public gpsimObject
{
    public:
      WDT(pic_processor *, double _timeout);
      void put(uint new_value);
      virtual void initialize(bool enable, bool _use_t0_prescale = true);
      virtual void swdten(bool enable);
      void set_timeout(double);
      virtual void set_prescale(uint);
      virtual void set_postscale(uint);
      virtual void reset(RESET_TYPE r);
      void clear();
      virtual void callback();
      virtual void update();
      virtual void callback_print();
      void set_breakpoint(uint bpn);
      bool hasBreak() { return breakpoint != 0;}

    protected:
      pic_processor *cpu;           // The cpu to which this wdt belongs.

      uint breakpoint,
           prescale,
           postscale;
           
      uint64_t future_cycle;

      double timeout;   // When no prescaler is assigned
      bool   wdte;
      bool   warned;
      bool   cfgw_enable;  // Enabled from Configureation word
      bool   use_t0_prescale;
};


/*==================================================================
 *
 * Here are the base class declarations for the pic processors
 */

/*
 * Define a base class processor for the pic processor family
 *
 * All pic processors are derived from this class.
 */
class PicTrisRegister;
class PicLatchRegister;
class IO_SignalControl;

class pic_processor : public Processor
{
    public:
      uint config_word;      // as read from hex or cod file
      ConfigMode   *config_modes;    // processor dependent configuration bits.

      uint pll_factor;       // 2^pll_factor is the speed boost the PLL adds
                                     // to the instruction execution rate.
      WDT          wdt;
      INDF         *indf;
      FSR          *fsr;
      Stack        *stack;

      Status_register *status;
      
      WREG         *Wreg;          // Used when W is a normal register
      PCL          *pcl;
      PCLATH       *pclath;
      PCHelper     *m_PCHelper;
      TMR0         tmr0;
      int          num_of_gprs;

      EEPROM* eeprom;       // set to NULL for PIC's that don't have a data EEPROM

      void add_sfr_register(Register *reg, uint addr,
                            RegisterValue por_value=RegisterValue(0,0),
                            const char *new_name=0,
                            bool warn_dup = true);
                            
      void add_sfr_registerR(sfr_register *reg, uint addr,
                            RegisterValue por_value=RegisterValue(0,0),
                            const char *new_name=0,
                            bool warn_dup = true);
                            
      void delete_sfr_register(Register *pReg);
      void remove_sfr_register(Register *pReg);

      void init_program_memory(uint memory_size);
      void build_program_memory(int *memory,int minaddr, int maxaddr);

      virtual instruction * disasm( uint address,uint inst)=0;
      virtual void create_config_memory() = 0;
      virtual void tris_instruction(uint tris_register) {return;};

      virtual void finish();

      void sleep();
      virtual void enter_sleep();
      virtual void exit_sleep();
      virtual bool exit_wdt_sleep() { return true; } // WDT wakes sleep
      virtual bool swdten_active() { return true; } // WDTCON can enable WDT
      bool is_sleeping();
      
      virtual void step(uint steps,bool refresh=true);
      virtual void step_over(bool refresh=true);
      virtual void step_cycle();

      virtual void step_one(bool refresh=true) 
      {
        if (pc->value < program_memory_size()) program_memory[pc->value]->execute();
        else
        {
            cout << "Program counter not valid " << hex << pc->value << endl;
            get_bp().halt();
        }
      }
      virtual void save_state(); // Take a snap shot of the internal state.

      virtual void interrupt() { return; }
      //// TEMPORARY - consolidate the various bp.set_interrupt() calls to one function:
      void BP_set_interrupt();
      void pm_write();

      virtual ConfigMemory * getConfigMemory(){ return m_configMemory;}
      virtual bool set_config_word(uint address, uint cfg_word);
      virtual uint get_config_word(uint address);
      virtual int get_config_index(uint address);
      virtual uint config_word_address() const {return 0x2007;};
      virtual ConfigMode *create_ConfigMode() { return new ConfigMode; };
      
      virtual void reset(RESET_TYPE r);

      virtual void create();

      virtual PROCESSOR_TYPE isa(){ return _PIC_PROCESSOR_; };
      virtual PROCESSOR_TYPE base_isa(){ return _PIC_PROCESSOR_; };
      virtual uint access_gprs() { return 0; };
      virtual uint bugs() { return 0; };    // default is no errata

      /* The program_counter class calls these two functions to get the upper bits of the PC
       * for branching (e.g. goto) or modify PCL instructions (e.g. addwf pcl,f) */
      virtual uint get_pclath_branching_jump()=0;
      virtual uint get_pclath_branching_modpcl()=0;

      virtual void option_new_bits_6_7(uint)=0;
      virtual void put_option_reg(uint) {}

      virtual void set_eeprom(EEPROM *e);
      virtual EEPROM *get_eeprom() { return (eeprom); }
      virtual void createMCLRPin(int pkgPinNumber);
      virtual void assignMCLRPin(int pkgPinNumber);
      virtual void unassignMCLRPin();
      virtual void osc_mode(uint );
      virtual void set_config3h(int64_t x){;}
      virtual string string_config3h(int64_t x){return string("fix string_config3h");}

      // Activity States reflect what the processor is currently doing
      // (The breakpoint class formally implemented this functionality).
      enum eProcessorActivityStates {
        ePAActive,      // Normal state
        ePAIdle,        // Processor is held in reset
        ePASleeping,    // Processor is sleeping
        ePAInterrupt,   // do we need this?
        ePAPMWrite      // Processor is busy performing a program memory write
      };
      eProcessorActivityStates getActivityState() { return m_ActivityState; }

      pic_processor(const char *_name=0, const char *desc=0);
      virtual ~pic_processor();

      void set_osc_pin_Number(uint i, uint val, PinModule *pm)
            {if (i < 4){osc_pin_Number[i] = val; m_osc_Monitor[i] = pm;}}
      unsigned char get_osc_pin_Number(uint i)
            {return (i<4)?osc_pin_Number[i]:253;}
      PinModule * get_osc_PinMonitor(uint i)
            { return (i<4)?m_osc_Monitor[i]:0; }

      void set_clk_pin(uint pkg_Pin_Number,
                           PinModule *PinMod,
                           const char * name,
                           bool in,
                    PicPortRegister *m_port = 0,
                    PicTrisRegister *m_tris = 0,
                    PicLatchRegister *m_lat = 0
                                      );
      void clr_clk_pin(uint pkg_Pin_Number, PinModule *PinMod,
                    PicPortRegister *m_port = 0,
                    PicTrisRegister *m_tris = 0,
                    PicLatchRegister *m_lat = 0
                     );

      virtual void set_int_osc(bool val){ internal_osc = val;}
      virtual bool get_int_osc(){ return internal_osc; }
      virtual void set_pplx4_osc(bool val){ PPLx4 = val;}
      virtual bool get_pplx4_osc(){ return PPLx4; }

      virtual void Wput(uint);
      virtual uint Wget();

    protected:
      ConfigMemory *m_configMemory;
      eProcessorActivityStates m_ActivityState;
      
      // Most midrange PIC's have a dedicated MCLR pin.
      // For the ones that don't, m_MCLR will be null.
      IOPIN *m_MCLR;
      IOPIN *m_MCLR_Save;
      int   m_MCLR_pin;
      PinMonitor *m_MCLRMonitor;
      string m_mclr_pin_name;
      
      unsigned char osc_pin_Number[4];
      PinModule *m_osc_Monitor[4];
      bool internal_osc;        // internal RC oscilator enabled on Config Word
      bool PPLx4;                // 4x PPL enabled on Config Word
      PeripheralSignalSource *clksource;
      SignalControl *clkcontrol;
      uint64_t sleep_time;
      ClockPhase *save_pNextPhase;
      ClockPhase *save_CurrentPhase;
};

#define cpu_pic ( (pic_processor *)cpu)

// Bit field of known silicon bugs
#define BUG_NONE        0
#define BUG_DAW         0x00000001


//------------------------------------------------------------------------
// Base Class for configuration memory
//
// The configuration memory is only a tiny portion of the overall processor
// program memory space (only 1-word on the mid range devices). So, explicit
// attributes are created for each memory configuration word. Since the meaning
// of configuration memory varies from processor to processor, it is up to
// each process to derive from this class.

class ConfigWord : public Integer
{
    public:
      ConfigWord(const char *_name, uint default_val, const char *desc,
                 pic_processor *pCpu, uint addr, bool EEw=true);
                 
      virtual void get(char *buffer, int buf_size);
      virtual void get(int64_t &i);
      uint ConfigWordAdd() { return m_addr; }
      bool isEEWritable() { return EEWritable;}

    protected:
      pic_processor *m_pCpu;
      uint m_addr;
      bool EEWritable;
};

class ConfigMemory
{
    public:
      ConfigMemory(pic_processor *pCpu, uint nWords);
      ~ConfigMemory();
      
      int addConfigWord(uint addr, ConfigWord *);
      ConfigWord *getConfigWord(uint addr);
      int getnConfigWords() { return m_nConfigWords; }

    protected:
      pic_processor *m_pCpu;
      ConfigWord **m_ConfigWords;
      uint m_nConfigWords;
};
#endif

/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2009,2013 Roy R. Rankin


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

#include "config.h"
#include "14bit-processors.h"
#include "pic-ioports.h"
#include "pic-registers.h"
#include <string>
#include "stimuli.h"
#include "packages.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("0x%06X %s() ",cycles.get(),__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif
//========================================================================
// Generic Configuration word for the midrange family.

class Generic14bitConfigWord : public ConfigWord
{
public:
  Generic14bitConfigWord(_14bit_processor *pCpu)
    : ConfigWord("CONFIG", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
    assert(pCpu);
    pCpu->wdt.initialize(true);
  }

  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    WDTEN  = 1<<2,
    PWRTEN = 1<<3
  };

  virtual void set(int64_t v)
  {
    int64_t oldV = getVal();

    Integer::set(v);
    if (m_pCpu) {

      int64_t diff = oldV ^ v;

      if (diff & WDTEN)
        m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);

      m_pCpu->config_modes->set_fosc01(v & (FOSC0 | FOSC1));
      m_pCpu->config_modes->set_wdte((v&WDTEN)==WDTEN);
      m_pCpu->config_modes->set_pwrte((v&PWRTEN)==PWRTEN);

    }

  }

  virtual string toString()
  {
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];

    snprintf(buff,sizeof(buff),
             "$%3x\n"
             " FOSC=%d - Clk source = %s\n"
             " WDTEN=%d - WDT is %s\n"
             " PWRTEN=%d - Power up timer is %s\n",
             i,
             i & (FOSC0 | FOSC1),
             ((i & FOSC0) ? ((i & FOSC1) ? "EXTRC":"XT") :((i & FOSC1) ? "INTRC":"LP")),
             ((i & WDTEN) ? 1 : 0), ((i & WDTEN) ? "enabled" : "disabled"),
             ((i & PWRTEN) ? 1 : 0), ((i & PWRTEN) ? "disabled" : "enabled"));

    return string(buff);
  }

};


//-------------------------------------------------------------------
_14bit_processor::_14bit_processor(const char *_name, const char *_desc)
  : pic_processor(_name,_desc), intcon(0),
    two_speed_clock(false), config_clock_mode(0),
    m_cpu_temp(0),
    has_SSP(false)

{
  pc = new Program_Counter("pc", "Program Counter", this);
  option_reg = new OPTION_REG(this,"option_reg");
  stack = new Stack(this);
}

_14bit_processor::~_14bit_processor()
{
  unassignMCLRPin();
  delete_sfr_register(fsr);
  delete_sfr_register(option_reg);
  delete pc; pc=0;
}

//-------------------------------------------------------------------
//
//
//    create
//
//  The purpose of this member function is to 'create' those things
// that are unique to the 14-bit core processors.

void _14bit_processor :: create ()
{
  pic_processor::create();
  fsr = new FSR(this, "fsr", "File Select Register for indirect addressing");
}

//-------------------------------------------------------------------
void _14bit_processor::interrupt ()
{
  //bp.clear_interrupt();

  intcon->in_interrupt = true;
  bp.clear_interrupt();
  stack->push(pc->value);

  pc->interrupt(INTERRUPT_VECTOR);
}

//-------------------------------------------------------------------
void _14bit_processor::save_state()
{
  pic_processor::save_state();
}

//-------------------------------------------------------------------
void _14bit_processor::option_new_bits_6_7(uint bits)
{
  cout << "14bit, option bits 6 and/or 7 changed\n";
}
//-------------------------------------------------------------------
void _14bit_processor::put_option_reg(uint val)
{
  option_reg->put(val);
}


//------------------------------------------------------------------
// Fetch the rom contents at a particular address.
uint _14bit_processor::get_program_memory_at_address(uint address)
{
  uint uIndex = map_pm_address2index(address);


  if (uIndex < program_memory_size())
    return  program_memory[uIndex] ? program_memory[uIndex]->get_opcode() : 0xffffffff;

  if (address >= 0x2000 && address < 0x2006)
  {
      return get_user_ids(address - 0x2000);
  }

  if (uIndex == 0x2006)
      return get_device_id(); 

  return get_config_word(address);
}

//-------------------------------------------------------------------
void _14bit_processor::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new Generic14bitConfigWord(this));
}

//-------------------------------------------------------------------

bool _14bit_processor::set_config_word(uint address,uint cfg_word)
{

  if((address == config_word_address()) && config_modes) {

    config_word = cfg_word;
    oscillator_select(cfg_word, false);

    if (m_configMemory && m_configMemory->getConfigWord(0))
      m_configMemory->getConfigWord(0)->set((int)cfg_word);

    return true;
  }

  return false;

}

// The working version of oscillator_select should be called at a higher level
// where the IO pins are defined
//
void _14bit_processor::oscillator_select(uint mode, bool not_clkout)
{
//    printf("Error _14bit_processor::oscillator_select called\n");
}
//-------------------------------------------------------------------
void _14bit_processor::enter_sleep()
{
    tmr0.sleep();
    pic_processor::enter_sleep();
}

 //-------------------------------------------------------------------
void _14bit_processor::exit_sleep()
{
  if (m_ActivityState == ePASleeping)
  {
    tmr0.wake();
    pic_processor::exit_sleep();
  }

}

//-------------------------------------------------------------------
Pic14Bit::Pic14Bit(const char *_name, const char *_desc)
  : _14bit_processor(_name,_desc),
    intcon_reg(this,"intcon","Interrupt Control")
{
  m_porta = new PicPortRegister(this,"porta","", 8,0x1f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);

  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);

  m_portb = new PicPortBRegister(this,"portb","",&intcon_reg,8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);
}

//-------------------------------------------------------------------
Pic14Bit::~Pic14Bit()
{
  unassignMCLRPin();
  remove_sfr_register(&tmr0);
  remove_sfr_register(&intcon_reg);

  delete_sfr_register(m_portb);
  delete_sfr_register(m_trisb);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
}

void Pic14Bit::create_sfr_map()
{
  add_sfr_register(indf,    0x00);
  alias_file_registers(0x00,0x00,0x80);

  add_sfr_register(&tmr0,   0x01);
  add_sfr_register(option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_trisa, 0x85, RegisterValue(0x3f,0));

  add_sfr_register(m_portb, 0x06);
  add_sfr_register(m_trisb, 0x86, RegisterValue(0xff,0));

  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));
  //add_sfr_register(pclath,  0x8a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));
  //add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  alias_file_registers(0x0a,0x0b,0x80);

  intcon = &intcon_reg;


}
//-------------------------------------------------------------------
void Pic14Bit::option_new_bits_6_7(uint bits)
{
  //1 ((PORTB *)portb)->rbpu_intedg_update(bits);
  m_portb->setRBPU( (bits & (1<<7)) == (1<<7));
  m_portb->setIntEdge((bits & (1<<6)) == (1<<6));
}


_14bit_e_processor::_14bit_e_processor(const char *_name, const char *_desc)
  : _14bit_processor(_name,_desc),
    mclr_pin(4),
    intcon_reg(this,"intcon","Interrupt Control"),
    option_reg(this,"option_reg","Option Register"),
    bsr(this, "bsr", "Bank Select Register"),
    pcon(this, "pcon", "Power Control Register", 0xcf),
    wdtcon(this, "wdtcon", "WDT Control", 0x3f),
    ind0(this,string("0")),
    ind1(this,string("1")),
    status_shad(this, "status_shad", "Status shadow register"),
    wreg_shad(this, "wreg_shad", "wreg shadow register"),
    bsr_shad(this, "bsr_shad", "bsr shadow register"),
    pclath_shad(this, "pclath_shad", "pclath shadow register"),
    fsr0l_shad(this, "fsr0l_shad", "fsr0l shadow register"),
    fsr0h_shad(this, "fsr0h_shad", "fsr0h shadow register"),
    fsr1l_shad(this, "fsr1l_shad", "fsr1l shadow register"),
    fsr1h_shad(this, "fsr1h_shad", "fsr1h shadow register")
{
  delete stack;
  stack = new Stack14E(this);
  stack->stack_mask = 0xf; // ehanced has stack 16 high
  intcon = &intcon_reg;

};

_14bit_e_processor::~_14bit_e_processor()
{

    remove_sfr_register(&ind0.indf);
    remove_sfr_register(&ind1.indf);

    remove_sfr_register(&ind0.fsrl);
    remove_sfr_register(&ind0.fsrh);
    remove_sfr_register(&ind1.fsrl);
    remove_sfr_register(&ind1.fsrh);
    remove_sfr_register(&bsr);
    remove_sfr_register(&intcon_reg);

    remove_sfr_register(&pcon);
    remove_sfr_register(&wdtcon);


  // These are copies taken at an interrupt
    remove_sfr_register(&status_shad);
    remove_sfr_register(&wreg_shad);
    remove_sfr_register(&bsr_shad);
    remove_sfr_register(&pclath_shad);
    remove_sfr_register(&fsr0l_shad);
    remove_sfr_register(&fsr0h_shad);
    remove_sfr_register(&fsr1l_shad);
    remove_sfr_register(&fsr1h_shad);

  Stack14E *stack14E = static_cast<Stack14E *>(stack);
    remove_sfr_register(&stack14E->stkptr);
    remove_sfr_register(&stack14E->tosl);
    remove_sfr_register(&stack14E->tosh);
}

void _14bit_e_processor::create_sfr_map()
{
  int bank;

  add_sfr_register(&ind0.indf,  0x00, RegisterValue(0,0), "indf0");
  add_sfr_register(&ind1.indf,  0x01, RegisterValue(0,0), "indf1");

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(&ind0.fsrl,  0x04, RegisterValue(0,0), "fsr0l");
  add_sfr_registerR(&ind0.fsrh,  0x05, RegisterValue(0,0), "fsr0h");
  add_sfr_register(&ind1.fsrl,  0x06, RegisterValue(0,0), "fsr1l");
  add_sfr_registerR(&ind1.fsrh,  0x07, RegisterValue(0,0), "fsr1h");
  add_sfr_register(&bsr,     0x08);
  add_sfr_register(Wreg,     0x09);
  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));
  add_sfr_registerR(&intcon_reg, 0x0b, RegisterValue(0,0));

  add_sfr_register(&pcon,   0x96, RegisterValue(0x0c,0),"pcon");
  wdt.set_postscale(0);
  wdt.set_timeout(1./32000.);
  add_sfr_registerR(&wdtcon,   0x97, RegisterValue(0x16,0),"wdtcon");


  // These are copies taken at an interrupt
  add_sfr_register(&status_shad,  0xfe4);
  add_sfr_register(&wreg_shad,    0xfe5);
  add_sfr_register(&bsr_shad,     0xfe6);
  add_sfr_register(&pclath_shad,  0xfe7);
  add_sfr_register(&fsr0l_shad,   0xfe8);
  add_sfr_register(&fsr0h_shad,   0xfe9);
  add_sfr_register(&fsr1l_shad,   0xfea);
  add_sfr_register(&fsr1h_shad,   0xfeb);

  Stack14E *stack14E = static_cast<Stack14E *>(stack);
  add_sfr_register(&stack14E->stkptr,  0xfed,RegisterValue(0,0),"stkptr");
  add_sfr_register(&stack14E->tosl,    0xfee,RegisterValue(0,0),"tosl");
  add_sfr_register(&stack14E->tosh,    0xfef,RegisterValue(0,0),"tosh");

  for (bank = 1; bank < 32; bank++)
  {
      alias_file_registers(0x00,0x0b,bank*0x80); // Duplicate core registers
      alias_file_registers(0x70,0x7f,bank*0x80); // Duplicate shadow ram
  }
  stack->stack_mask = 15; // enhanced has stack 16 high

}
//-------------------------------------------------------------------
//	Similar to pic_processoer version except sets PCON flags
//
void _14bit_e_processor::reset (RESET_TYPE r)
{


  switch(r)
  {
  case POR_RESET:
    pcon.put(0x0d);
    break;

  case SOFT_RESET:
    pcon.put(pcon.get() & ~PCON::RI);
    break;

  case MCLR_RESET:
    cout << "Reset due to MCLR\n";
    pcon.put(pcon.get() & ~PCON::RMCLR);
    break;

  case STKOVF_RESET:
    pcon.put(pcon.get() | PCON::STKOVF);
    break;

  case STKUNF_RESET:
    pcon.put(pcon.get() | PCON::STKUNF);
    break;

  default:
    break;
  };
  pic_processor::reset(r);
  return;
}

//-------------------------------------------------------------------
// The enhanced processors save a number of registers into
// their shadow registers on interrupt
//
void _14bit_e_processor::interrupt ()
{

  bp.clear_interrupt();

  if (bp.have_sleep()) {
      bp.clear_sleep();
      stack->push(pc->value+1);
  } else {
      stack->push(pc->value);
  }
  status_shad.value = status->value;
  wreg_shad.value = Wreg->value;
  bsr_shad.value = bsr.value;
  pclath_shad.value = pclath->value;
  fsr0l_shad.value = ind0.fsrl.value;
  fsr0h_shad.value = ind0.fsrh.value;
  fsr1l_shad.value = ind1.fsrl.value;
  fsr1h_shad.value = ind1.fsrh.value;

  intcon->in_interrupt = true;
  pc->interrupt(INTERRUPT_VECTOR);

}

//-------------------------------------------------------------------
void _14bit_e_processor::enter_sleep()
{
    tmr0.sleep();
    if (wdt_flag == 2)		// WDT is suspended during sleep
	wdt.initialize(false);
    pic_processor::enter_sleep();
}

 //-------------------------------------------------------------------
bool _14bit_e_processor::exit_wdt_sleep()
{
    return true;
}
 //-------------------------------------------------------------------
void _14bit_e_processor::exit_sleep()
{
  if (m_ActivityState == ePASleeping)
  {
    tmr0.wake();
    if (wdt_flag == 2)
	wdt.initialize(true);
    pic_processor::exit_sleep();
  }

}

//========================================================================
//
// Configuration Memory word 1for the enhanced 14 bit processors

class Config_E : public ConfigWord
{
public:
  Config_E(_14bit_e_processor *pCpu, const char *name, uint address, bool EEw=false)
    : ConfigWord(name, 0x3fff, "Configuration Word", pCpu, address, EEw)
  {
    if (m_pCpu)
    {
	m_pCpu->set_config_word(address, 0x3fff);
    }
  }
};
void _14bit_e_processor::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,9);
  m_configMemory->addConfigWord(0,new Config_E(this, "UserID1", 0x8000, true));
  m_configMemory->addConfigWord(1,new Config_E(this, "UserID2", 0x8001, true));
  m_configMemory->addConfigWord(2,new Config_E(this, "UserID3", 0x8002, true));
  m_configMemory->addConfigWord(3,new Config_E(this, "UserID4", 0x8003, true));
  m_configMemory->addConfigWord(6,new Config_E(this, "DeviceID", 0x8006));
  m_configMemory->addConfigWord(7,new Config_E(this, "ConfigW1", 0x8007));
  m_configMemory->addConfigWord(8,new Config_E(this, "ConfigW2", 0x8008));

};

bool _14bit_e_processor::set_config_word(uint address,uint cfg_word)
{
  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    FOSC2  = 1<<2,
    WDTEN0 = 1<<3,
    WDTEN1 = 1<<4,
    PWRTEN = 1<<5,
    MCLRE  = 1<<6,
    CP     = 1<<7,
    CPD    = 1<<8,
    BOREN0 = 1<<9,
    BOREN1 = 1<<10,
    NOT_CLKOUTEN = 1<<11,
//    IESO   = 1<<12,
    // Config word 2
    WRT0   = 1<<0,
    WRT1   = 1<<1,
    PLLEN  = 1<<8,
    STVREN = 1<<9,


  };

    Dprintf((" add %x word %x\n", address, cfg_word));

    if(address == 0x8007) // Config Word 1
    {
	wdt_flag = (cfg_word & (WDTEN0|WDTEN1)) >> 3;
        Dprintf((" cfg_word %x MCLRE %x\n", cfg_word, cfg_word & MCLRE));
        if ((cfg_word & MCLRE) == MCLRE)
            assignMCLRPin(mclr_pin);
        else
            unassignMCLRPin();

        wdt.initialize(wdt_flag & 2);
        oscillator_select(cfg_word, (cfg_word & NOT_CLKOUTEN) != NOT_CLKOUTEN);
    }
    else if (address == 0x8008)
    {
	// stack over/under reset flag
	stack->STVREN = ((cfg_word & STVREN) == STVREN);
       Dprintf((" STVREN %x flag %d\n", cfg_word&STVREN, stack->STVREN));
	// Program memory write protect (eeprom)
	program_memory_wp(cfg_word & (WRT1|WRT0));
	set_pplx4_osc(cfg_word & PLLEN);

    }
    return(pic_processor::set_config_word(address, cfg_word));
}


// The working version of oscillator_select should be called at a higher level
// where the IO pins are defined
//
void _14bit_e_processor::oscillator_select(uint mode, bool not_clkout)
{
    printf("Error _14bit_e_processor::oscillator_select called\n");
}

// The working version of program_memory_wp should be called at a higher level
// where the eeprom is defined
//
void _14bit_e_processor::program_memory_wp(uint mode)
{
    printf("Error _14bit_e_processor::program_memory_wp called\n");
}

// This function routes Wreg put requests through registers (if possible)
// for breaking and logging
void _14bit_e_processor::Wput(uint value)
{
    if(Wreg->address)
	registers[Wreg->address]->put(value);
    else
	Wreg->put(value);
}

// This function routes Wreg get requests through registers (if possible)
// for breaking and logging
uint _14bit_e_processor::Wget()
{
    if(Wreg->address)
	return registers[Wreg->address]->get();
    else
	return Wreg->get();
}


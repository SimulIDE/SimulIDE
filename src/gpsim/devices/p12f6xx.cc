/*
   Copyright (C) 2009 Roy R. Rankin

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


//
// p12f6xx
//
//  This file supports:
//    PIC12F629
//    PIC12F675
//    PIC12F683
//
//Note: unlike most other 12F processors these have 14bit instructions

#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"
#include "stimuli.h"
#include "eeprom.h"
#include "p12f6xx.h"
#include "pic-ioports.h"
#include "packages.h"


//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//========================================================================
//
// Configuration Memory for the 12F6XX devices.

class Config12F6 : public ConfigWord
{
public:
  Config12F6(pic_processor  *pCpu)
    : ConfigWord("CONFIG12F6", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
    Dprintf(("Config12F6::Config12F6 %p\n", m_pCpu));
    if (m_pCpu) 
    {
        m_pCpu->set_config_word(0x2007, 0x3fff);
    }
  }

};

// Does not match any of 3 versions in pir.h, pir.cc
// If required by any other porcessors should be moved there
//
class PIR1v12f : public PIR
{
public:

  enum {
        TMR1IF         = 1 << 0,
        TMR2IF  = 1 << 1,        //For 12F683
        CMIF        = 1 << 3,
        CLC1IF  = 1 << 3,        // For 10F32x
        NCO1IF  = 1 << 4,        // For 10F32x
        ADIF        = 1 << 6,
        EEIF        = 1 << 7
  };

//------------------------------------------------------------------------

PIR1v12f(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = TMR1IF | CMIF | ADIF | EEIF;
  writable_bits = TMR1IF | CMIF | ADIF | EEIF;
}

  virtual void set_tmr1if()
  {
    put(get() | TMR1IF);
  }
  virtual void set_tmr2if()
  {
    put(get() | TMR2IF);
  }

  virtual void set_cmif()
  {
    value.put(value.get() | CMIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }

  virtual void set_clc1if()
  {
    value.put(value.get() | CLC1IF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }

  virtual void set_nco1if()
  {
    uint pievalue = value.get();
    if (!(pievalue & NCO1IF))
    {
        value.put(pievalue | NCO1IF);
    }
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }

  virtual void set_c1if()
  {
        set_cmif();
  }

  virtual void set_eeif()
  {
    value.put(value.get() | EEIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }

 virtual void set_adif()
  {
    value.put(value.get() | ADIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
};

//========================================================================

void P12F629::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new Config12F6(this));

};

class MCLRPinMonitor;

bool P12F629::set_config_word(uint address,uint cfg_word)
{
  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    FOSC2  = 1<<2,
    WDTEN  = 1<<3,
    PWRTEN = 1<<4,
    MCLRE  = 1<<5,
    BOREN  = 1<<6,
    CP     = 1<<7,
    CPD    = 1<<8,

    BG0    = 1<<12,
    BG1    = 1<<13
  };


    if(address == config_word_address())
    {
        if ((cfg_word & MCLRE) == MCLRE)
            assignMCLRPin(4);        // package pin 4
        else
            unassignMCLRPin();
     
         wdt.initialize((cfg_word & WDTEN) == WDTEN);
        if ((cfg_word & (FOSC2 | FOSC1 )) == 0x04) // internal RC OSC
          osccal.set_freq(4e6);
       

        return(_14bit_processor::set_config_word(address, cfg_word));
        
    }
    return false;
}

P12F629::P12F629(const char *_name, const char *desc)
  : _14bit_processor(_name,desc), 
    intcon_reg(this,"intcon","Interrupt Control"),
    comparator(this),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    t1con(this, "t1con", "TMR1 Control"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    pcon(this, "pcon", "pcon"),
    osccal(this, "osccal", "Oscillator Calibration Register", 0xfc)

{
  m_ioc = new IOC(this, "ioc", "Interrupt-On-Change GPIO Register");
  m_gpio = new PicPortGRegister(this,"gpio","", &intcon_reg, m_ioc,8,0x3f);
  m_trisio = new PicTrisRegister(this,"trisio","", m_gpio, false);

  m_wpu = new WPU(this, "wpu", "Weak Pull-up Register", m_gpio, 0x37);

  pir1 = new PIR1v12f(this,"pir1","Peripheral Interrupt Register",&intcon_reg, &pie1);


  tmr0.set_cpu(this, m_gpio, 4, option_reg);
  tmr0.start(0);

  if(config_modes)
    config_modes->valid_bits = ConfigMode::CM_FOSC0 | ConfigMode::CM_FOSC1 | 
      ConfigMode::CM_FOSC1x | ConfigMode::CM_WDTE | ConfigMode::CM_PWRTE;

}

P12F629::~P12F629()
{

  delete_file_registers(0x20, ram_top);
  remove_sfr_register(&tmr0);
  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);
  remove_sfr_register(&pcon);
  remove_sfr_register(&t1con);
  remove_sfr_register(&intcon_reg);
  remove_sfr_register(&pie1);
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(&osccal);

  delete_sfr_register(m_gpio);
  delete_sfr_register(m_trisio);
  delete_sfr_register(m_wpu);
  delete_sfr_register(m_ioc);
  delete_sfr_register(pir1);
    delete e;
}
Processor * P12F629::construct(const char *name)
{

  P12F629 *p = new P12F629(name);

  p->create(0x5f, 128);
  p->create_invalid_registers ();

  return p;

}

void P12F629::create_sfr_map()
{
  pir_set_def.set_pir1(pir1);

  add_sfr_register(indf,    0x00);
  alias_file_registers(0x00,0x00,0x80);

  add_sfr_register(&tmr0,   0x01, RegisterValue(0xff,0));
  add_sfr_register(option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

  add_sfr_register(&pcon,   0x8e, RegisterValue(0,0),"pcon");

  add_sfr_register(&t1con,  0x10, RegisterValue(0,0));

  add_sfr_register(m_gpio, 0x05);
  add_sfr_register(m_trisio, 0x85, RegisterValue(0x3f,0));


  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));
  alias_file_registers(0x0a,0x0b,0x80);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  add_sfr_register(pir1,   0x0c, RegisterValue(0,0),"pir1");

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));
  
  tmr1h.tmrl  = &tmr1l;
  t1con.tmrl  = &tmr1l;

  tmr1l.setIOpin(&(*m_gpio)[5]);
  tmr1l.setGatepin(&(*m_gpio)[4]);

  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));
  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.setPir(pir1);

  // Link the comparator and voltage ref to porta
  comparator.initialize(get_pir_set(), NULL, 
        &(*m_gpio)[0], &(*m_gpio)[1], 
        NULL, NULL,
        &(*m_gpio)[2], NULL);

  comparator.cmcon.set_configuration(1, 0, AN0, AN1, AN0, AN1, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN0, AN1, AN0, AN1, OUT0);
  comparator.cmcon.set_configuration(1, 2, AN0, AN1, AN0, AN1, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN1, VREF, AN1, VREF, OUT0);
  comparator.cmcon.set_configuration(1, 4, AN1, VREF, AN1, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, AN1, VREF, AN0, VREF, OUT0);
  comparator.cmcon.set_configuration(1, 6, AN1, VREF, AN0, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 0, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 1, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 2, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 3, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 4, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 6, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0x19, RegisterValue(0,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0x99, RegisterValue(0,0),"cvrcon");

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x9a);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x9b);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x9c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x9d);
  add_sfr_register(m_wpu, 0x95, RegisterValue(0x37,0),"wpu");
  add_sfr_register(m_ioc, 0x96, RegisterValue(0,0),"ioc");
  add_sfr_register(&osccal, 0x90, RegisterValue(0x80,0));


}

//-------------------------------------------------------------------
void P12F629::set_out_of_range_pm(uint address, uint value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
      get_eeprom()->change_rom(address - 0x2100, value);
}

void P12F629::create_iopin_map()
{
  package = new Package(8);
  if(!package) return;

  // Now Create the package and place the I/O pins

  package->assign_pin( 7, m_gpio->addPin(new IO_bi_directional_pu("gpio0"),0));
  package->assign_pin( 6, m_gpio->addPin(new IO_bi_directional_pu("gpio1"),1));
  package->assign_pin( 5, m_gpio->addPin(new IO_bi_directional_pu("gpio2"),2));
  package->assign_pin( 4, m_gpio->addPin(new IOPIN("gpio3"),3));
  package->assign_pin( 3, m_gpio->addPin(new IO_bi_directional_pu("gpio4"),4));
  package->assign_pin( 2, m_gpio->addPin(new IO_bi_directional_pu("gpio5"),5));

  package->assign_pin( 1, 0);
  package->assign_pin( 8, 0);
}

void  P12F629::create(int _ram_top, int eeprom_size)
{
  ram_top = _ram_top;
  create_iopin_map();

  _14bit_processor::create();

  e = new EEPROM_PIR(this, pir1);
  e->initialize(eeprom_size);
  e->set_intcon(&intcon_reg);
  set_eeprom(e);

  add_file_registers(0x20, ram_top, 0x80);
  P12F629::create_sfr_map();
}

//-------------------------------------------------------------------
void P12F629::enter_sleep()
{
        tmr1l.sleep();
        _14bit_processor::enter_sleep();
}

//-------------------------------------------------------------------
void P12F629::exit_sleep()
{
        tmr1l.wake();
        _14bit_processor::exit_sleep();
}

//-------------------------------------------------------------------
void P12F629::option_new_bits_6_7(uint bits)
{
        Dprintf(("P12F629::option_new_bits_6_7 bits=%x\n", bits));
  m_gpio->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6); 
  m_wpu->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7); 
}
//========================================================================
//
// Pic 16F675
//

Processor * P12F675::construct(const char *name)
{
  P12F675 *p = new P12F675(name);

  p->create(0x5f, 128);
  p->create_invalid_registers ();

  return p;
}

P12F675::P12F675(const char *_name, const char *desc)
  : P12F629(_name,desc),
    ansel(this,"ansel", "Analog Select"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low")
{
}

P12F675::~P12F675()
{
  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&ansel);
}
void  P12F675::create(int ram_top, int eeprom_size)
{
  P12F629::create(ram_top, eeprom_size);
  create_sfr_map();
}


void P12F675::create_sfr_map()
{
  //
  //  adcon1 is not a register on the 12f675, but it is used internally
  //  to perform the ADC conversions
  //
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));

  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&ansel, 0x9f, RegisterValue(0x0f,0));


  ansel.setAdcon1(&adcon1);
  ansel.setAdcon0(&adcon0);
  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(3);
  adcon0.setChannel_shift(2);

  adcon1.setNumberOfChannels(4);

  adcon1.setIOPin(0, &(*m_gpio)[0]);
  adcon1.setIOPin(1, &(*m_gpio)[1]);
  adcon1.setIOPin(2, &(*m_gpio)[2]);
  adcon1.setIOPin(3, &(*m_gpio)[4]);

  adcon1.setVrefHiConfiguration(2, 1);

/* Channel Configuration done dynamiclly based on ansel */

   adcon1.setValidCfgBits(ADCON1::VCFG0 | ADCON1::VCFG1 , 4);


}
//========================================================================
//
// Pic 16F683
//

Processor * P12F683::construct(const char *name)
{
  P12F683 *p = new P12F683(name);

  p->create(0x7f, 256);
  p->create_invalid_registers ();

  return p;
}

P12F683::P12F683(const char *_name, const char *desc)
  : P12F675(_name,desc),
   t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    wdtcon(this, "wdtcon", "WDT Control", 0x1f),
    osccon(0),
    osctune(this, "osctune", "OSC Tune")

{
    internal_osc = false;
    pir1->valid_bits |= PIR1v12f::TMR2IF;
    pir1->writable_bits |= PIR1v12f::TMR2IF;
}

P12F683::~P12F683()
{
  delete_file_registers(0x20, 0x7f);
  delete_file_registers(0xa0, 0xbf);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);

  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(osccon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&comparator.cmcon1);
}

void  P12F683::create(int _ram_top, int eeprom_size)
{

  P12F629::create(0, eeprom_size);
  osccon = new OSCCON(this, "osccon", "OSC Control");
  add_file_registers(0x20, 0x6f, 0);
  add_file_registers(0xa0, 0xbf, 0);
  add_file_registers(0x70, 0x7f, 0x80);

  create_sfr_map();
}


void P12F683::create_sfr_map()
{
  P12F675::create_sfr_map();

  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));

  add_sfr_register(&ccpr1l,  0x13, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,  0x14, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x15, RegisterValue(0,0));
  add_sfr_register(&wdtcon, 0x18, RegisterValue(0x08,0),"wdtcon");
  add_sfr_register(osccon, 0x8f, RegisterValue(0,0),"osccon");
  remove_sfr_register(&osccal);
  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");

  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);


  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &ccp1con );
  pr2.tmr2    = &tmr2;


  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v1::CCP1IF, &tmr2);
  ccp1con.setIOpin(&((*m_gpio)[2]));
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  comparator.cmcon.new_name("cmcon0");
  comparator.cmcon.set_tmrl(&tmr1l);
  comparator.cmcon1.set_tmrl(&tmr1l);
  add_sfr_register(&comparator.cmcon1, 0x1a, RegisterValue(2,0),"cmcon1");
  wdt.set_timeout(1./31000.);

}
P10F32X::P10F32X(const char *_name, const char *desc)
  : _14bit_processor(_name,desc), 
    intcon_reg(this,"intcon","Interrupt Control"),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    tmr2(this, "tmr2", "TMR2 Register"),
    pr2(this, "pr2", "Timer2 Period Register"),
    pcon(this, "pcon", "pcon"),
    ansela(this,"ansela", "Analog Select"),
    fvrcon(this, "fvrcon", "Voltage reference control register", 0xf3, 0x00),
    borcon(this, "borcon", "Brown-out reset control register"),
    wdtcon(this, "wdtcon", "WDT Control", 0x3f),
    adcon0(this,"adcon", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"), // virtual register
    adres(this,"adres", "A2D Result Low"),
    pwm1con(this, "pwm1con", "PWM CONTROL REGISTER 1", 1),
    pwm1dcl(this, "pwm1dcl", "PWM DUTY CYCLE LOW BITS"),
    pwm1dch(this, "pwm1dch", "PWM DUTY CYCLE HIGH BITS"),
    pwm2con(this, "pwm2con", "PWM CONTROL REGISTER 2", 2),
    pwm2dcl(this, "pwm2dcl", "PWM DUTY CYCLE LOW BITS"),
    pwm2dch(this, "pwm2dch", "PWM DUTY CYCLE HIGH BITS"),
    pm_rw(this), cwg(this), nco(this), clcdata(this),
    clc1(this, 0, &clcdata),
    vregcon(this, "vregcon", "Voltage Regulator Control Register")

{
  m_iocaf = new IOCxF(this, "iocaf", "Interrupt-On-Change flag Register", 0x0f);
  m_iocap = new IOC(this, "iocap", "Interrupt-On-Change positive edge", 0x0f);
  m_iocan = new IOC(this, "iocan", "Interrupt-On-Change negative edge", 0x0f);
  m_porta = new PicPortIOCRegister(this,"porta","", &intcon_reg, m_iocap, m_iocan, m_iocaf, 8,0x0f);

  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false, 0x07);
  m_lata  = new PicLatchRegister(this,"lata","",m_porta, 0x07);
  m_wpu = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x0f);

  pir1 = new PIR1v12f(this,"pir1","Peripheral Interrupt Register",&intcon_reg, &pie1);
  pir1->valid_bits |= PIR1v12f::TMR2IF|PIR1v12f::NCO1IF;
  pir1->valid_bits &= ~(PIR1v12f::EEIF|PIR1v12f::TMR1IF);
  pir1->writable_bits = pir1->valid_bits;
  m_cpu_temp = new CPU_Temp("cpu_temperature", 30., "CPU die temperature");
  osccon = new OSCCON_HS2(this, "osccon", "Oscillator Control Register");
  tmr0.set_cpu(this, m_porta, 3, option_reg);
  tmr0.start(0);
}

P10F32X::~P10F32X()
{

  delete_file_registers(0x40, ram_top);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);
  remove_sfr_register(&pcon);
  remove_sfr_register(&intcon_reg);
  remove_sfr_register(&pie1);
  remove_sfr_register(&ansela);
  remove_sfr_register(&fvrcon);
  remove_sfr_register(&tmr0);
  remove_sfr_register(&borcon);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
  remove_sfr_register(&pwm1dcl);
  remove_sfr_register(&pwm1dch);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&pwm2dcl);
  remove_sfr_register(&pwm2dch);
  remove_sfr_register(&pwm2con);
  remove_sfr_register(pm_rw.get_reg_pmadr());
  remove_sfr_register(pm_rw.get_reg_pmadrh());
  remove_sfr_register(pm_rw.get_reg_pmdata());
  remove_sfr_register(pm_rw.get_reg_pmdath());
  remove_sfr_register(pm_rw.get_reg_pmcon1_rw());
  remove_sfr_register(pm_rw.get_reg_pmcon2());
  remove_sfr_register(&nco.nco1accl);
  remove_sfr_register(&nco.nco1acch);
  remove_sfr_register(&nco.nco1accu);
  remove_sfr_register(&nco.nco1incl);
  remove_sfr_register(&nco.nco1inch);
  remove_sfr_register(&nco.nco1con);
  remove_sfr_register(&nco.nco1clk);
  remove_sfr_register(&clc1.clcxcon);
  remove_sfr_register(&clc1.clcxpol);
  remove_sfr_register(&clc1.clcxsel0);
  remove_sfr_register(&clc1.clcxsel1);
  remove_sfr_register(&clc1.clcxgls0);
  remove_sfr_register(&clc1.clcxgls1);
  remove_sfr_register(&clc1.clcxgls2);
  remove_sfr_register(&clc1.clcxgls3);
  remove_sfr_register(&cwg.cwg1con0);
  remove_sfr_register(&cwg.cwg1con1);
  remove_sfr_register(&cwg.cwg1con2);
  remove_sfr_register(&cwg.cwg1dbr);
  remove_sfr_register(&cwg.cwg1dbf);
  remove_sfr_register(&vregcon);



  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_lata);
  delete_sfr_register(m_wpu);
  delete_sfr_register(m_iocaf);
  delete_sfr_register(m_iocap);
  delete_sfr_register(m_iocan);
  delete_sfr_register(pir1);
  delete_sfr_register(osccon);
  delete m_cpu_temp;
}

void  P10F32X::create()
{

  ram_top = 0x7f;
  P10F32X::create_iopin_map();

  _14bit_processor::create();

  status->write_mask &= ~0xe0; // IRP RP0 RP1 read only
  add_file_registers(0x40, ram_top, 0x00);
  P10F32X::create_sfr_map();
}

void P10F32X::option_new_bits_6_7(uint bits)
{
  Dprintf(("P10F32X::option_new_bits_6_7 bits=%x\n", bits));
  m_porta->setIntEdge ( (bits & OPTION_REG::BIT6));
  m_wpu->set_wpu_pu ( !(bits & OPTION_REG::BIT7));
}

void P10F32X::create_sfr_map()
{
  pir_set_def.set_pir1(pir1);

  add_sfr_register(indf,    0x00);

  add_sfr_register(&tmr0,   0x01, RegisterValue(0xff,0));
  add_sfr_register(option_reg,  0x0e, RegisterValue(0xff,0));

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  add_sfr_register(m_porta,  0x05, RegisterValue(0x0,0));
  add_sfr_register(m_trisa,  0x06, RegisterValue(0x0f,0));
  add_sfr_register(m_lata,  0x07, RegisterValue(0x00,0));
  add_sfr_registerR(&ansela, 0x08, RegisterValue(0x07,0));
  add_sfr_registerR(m_wpu, 0x09, RegisterValue(0x0f,0),"wpu");

  add_sfr_registerR(pclath,  0x0a, RegisterValue(0,0));

  add_sfr_registerR(&intcon_reg, 0x0b, RegisterValue(0,0));

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  add_sfr_register(pir1,   0x0c, RegisterValue(0,0),"pir1");

  add_sfr_registerR(&pie1,   0x0d, RegisterValue(0,0));
  add_sfr_register(&pcon,   0x0f, RegisterValue(0,0),"pcon");
  add_sfr_registerR(osccon,   0x10, RegisterValue(0x60,0));
  add_sfr_registerR(&tmr2,     0x11, RegisterValue(0,0));
  add_sfr_registerR(&pr2,      0x12, RegisterValue(0xff,0));
  add_sfr_registerR(&t2con,    0x13, RegisterValue(0,0));
  add_sfr_register(&pwm1dcl,  0x14, RegisterValue(0,0));
  add_sfr_register(&pwm1dch,  0x15, RegisterValue(0,0));
  add_sfr_registerR(&pwm1con,  0x16, RegisterValue(0,0));
  add_sfr_register(&pwm2dcl,  0x17, RegisterValue(0,0));
  add_sfr_register(&pwm2dch,  0x18, RegisterValue(0,0));
  add_sfr_registerR(&pwm2con,  0x19, RegisterValue(0,0));
  add_sfr_registerR(m_iocap,   0x1a, RegisterValue(0,0));
  add_sfr_registerR(m_iocan,   0x1b, RegisterValue(0,0));
  add_sfr_registerR(m_iocaf,   0x1c, RegisterValue(0,0));
  add_sfr_registerR(&fvrcon,   0x1d, RegisterValue(0,0));
  add_sfr_register(&adres,    0x1e, RegisterValue(0,0));
  add_sfr_registerR(&adcon0,   0x1f, RegisterValue(0,0));
  add_sfr_registerR(pm_rw.get_reg_pmadr(),  0x20 );
  add_sfr_registerR(pm_rw.get_reg_pmadrh(), 0x21 );
  add_sfr_register(pm_rw.get_reg_pmdata(), 0x22 );
  add_sfr_register(pm_rw.get_reg_pmdath(), 0x23 );
  add_sfr_registerR(pm_rw.get_reg_pmcon1_rw(), 0x24 );
  add_sfr_registerR(pm_rw.get_reg_pmcon2(), 0x25 );
  add_sfr_registerR(&nco.nco1accl, 0x27, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1acch, 0x28, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1accu, 0x29, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1incl, 0x2a, RegisterValue(1,0));
  add_sfr_registerR(&nco.nco1inch, 0x2b, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1con,  0x2d, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1clk,  0x2e, RegisterValue(0,0));

  add_sfr_registerR(&wdtcon,   0x30, RegisterValue(0x16,0));
  add_sfr_registerR(&clc1.clcxcon, 0x31, RegisterValue(0,0), "clc1con");
  add_sfr_register(&clc1.clcxpol, 0x32, RegisterValue(0,0), "clc1pol");
  add_sfr_register(&clc1.clcxsel0, 0x33, RegisterValue(0,0), "clc1sel0");
  add_sfr_register(&clc1.clcxsel1, 0x34, RegisterValue(0,0), "clc1sel1");
  add_sfr_register(&clc1.clcxgls0, 0x35, RegisterValue(0,0), "clc1gls0");
  add_sfr_register(&clc1.clcxgls1, 0x36, RegisterValue(0,0), "clc1gls1");
  add_sfr_register(&clc1.clcxgls2, 0x37, RegisterValue(0,0), "clc1gls2");
  add_sfr_register(&clc1.clcxgls3, 0x38, RegisterValue(0,0), "clc1gls3");
  add_sfr_registerR(&cwg.cwg1con0, 0x39, RegisterValue(0,0)); 
  add_sfr_register(&cwg.cwg1con1, 0x3a); 
  add_sfr_register(&cwg.cwg1con2, 0x3b); 
  add_sfr_register(&cwg.cwg1dbr, 0x3c); 
  add_sfr_register(&cwg.cwg1dbf, 0x3d); 
  add_sfr_registerR(&vregcon, 0x3f, RegisterValue(1,0));
  add_sfr_register(&borcon,   0x3f, RegisterValue(0x80,0));
  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.setPir(pir1);
  ansela.setValidBits(0x07);
  ansela.setAdcon1(&adcon1);
  ansela.config(7, 0);
  adcon1.setNumberOfChannels(8);
  adcon0.setAdres(&adres);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(8);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(7);
  adcon0.setChannel_shift(2);

  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);

  fvrcon.set_adcon1(&adcon1);
  fvrcon.set_VTemp_AD_chan(6);
  fvrcon.set_FVRAD_AD_chan(7);
  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &pwm1con );
  tmr2.add_ccp ( &pwm2con );
  pr2.tmr2    = &tmr2;

  pwm1con.set_pwmdc(&pwm1dcl, &pwm1dch);
  pwm1con.setIOPin1(&(*m_porta)[0]);
  pwm1con.set_tmr2(&tmr2);
  pwm1con.set_cwg(&cwg);
  pwm1con.set_clc(&clc1, 0);
  pwm2con.set_pwmdc(&pwm2dcl, &pwm2dch);
  pwm2con.setIOPin1(&(*m_porta)[1]);
  pwm2con.set_tmr2(&tmr2);
  pwm2con.set_cwg(&cwg);
  pwm2con.set_clc(&clc1, 0);

  cwg.set_IOpins(&(*m_porta)[0], &(*m_porta)[1], &(*m_porta)[2]);

  clc1.p_nco = &nco;
  clc1.set_clcPins(&(*m_porta)[0], &(*m_porta)[2], &(*m_porta)[1]);
  tmr0.set_clc(&clc1, 0);
  clc1.setInterruptSource(new InterruptSource(pir1, (1<<3)));
  nco.setIOpins(&(*m_porta)[1], &(*m_porta)[2]);
  nco.pir = pir1;

}
void P10F32X::create_iopin_map()
{

  package = new Package(8);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional_pu("ra0"),0));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional_pu("ra1"),1));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional_pu("ra2"),2));
  package->assign_pin( 8, m_porta->addPin(new IO_bi_directional_pu("ra3"),3));

  package->assign_pin( 1, 0);
  package->assign_pin( 2, 0);
  package->assign_pin( 6, 0);
  package->assign_pin( 7, 0);


}
void P10F32X::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new Config12F6(this));

};

class MCLRPinMonitor;

bool P10F32X::set_config_word(uint address,uint cfg_word)
{
  enum {
    FOSC  = 1<<0,
    BOREN  = 1<<1,
    BOREN1  = 1<<2,
    WDTEN0  = 1<<3,
    WDTEN1  = 1<<4,
    MCLRE  = 1<<6,
    CP     = 1<<7,
    LVP    = 1<<8,

  };


    Dprintf(("P10F32X::set_config_word address 0x%x cfg=0x%x\n", address, cfg_word));
    if(address == config_word_address())
    {
        if ((cfg_word & MCLRE)) assignMCLRPin(8);    // package pin 8
        else                    unassignMCLRPin();
     
        wdt_flag = (cfg_word & (WDTEN0|WDTEN1)) >> 3;
        wdt.set_timeout(1./31000.);
        wdt.initialize(wdt_flag & 2, false);

        if (cfg_word & FOSC) // EC on CLKIN
        {
            set_int_osc(false);
        }
        else                     // INTRC
        {
            set_int_osc(true);
            osccon->set_rc_frequency();
        }
        return(_14bit_processor::set_config_word(address, cfg_word));        
    }
    return false;
}

void P10F32X::enter_sleep()
{
    tmr0.sleep();
    if (wdt_flag == 2)          // WDT is suspended during sleep
        wdt.initialize(false);
    pic_processor::enter_sleep();
}

void P10F32X::exit_sleep()
{
  if (m_ActivityState == ePASleeping)
  {
    tmr0.wake();
    if (wdt_flag == 2)
        wdt.initialize(true);
    pic_processor::exit_sleep();
  }
}



P10F320::P10F320(const char *_name, const char *desc)
  : P10F32X(_name,desc) 
{
}
P10F320::~P10F320()
{
}
Processor * P10F320::construct(const char *name)
{
  P10F320 *p = new P10F320(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

Processor * P10LF320::construct(const char *name)
{
  P10LF320 *p = new P10LF320(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}
P10F322::P10F322(const char *_name, const char *desc)
  : P10F32X(_name,desc) 
{
}

P10F322::~P10F322()
{
}

Processor * P10F322::construct(const char *name)
{
  P10F322 *p = new P10F322(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

Processor * P10LF322::construct(const char *name)
{
  P10LF322 *p = new P10LF322(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

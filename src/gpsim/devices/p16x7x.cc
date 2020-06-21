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


//
// p16x7x
//
//  This file supports:
//    P16C71
//    P16C712
//    P16C716
//    P16F716
//    P16C72
//    P16C73
//    P16C74


#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"
#include "packages.h"
#include "p16x7x.h"
#include "pic-ioports.h"
#include "stimuli.h"
#include "pm_rd.h"

//#define DEBUG_AD

//------------------------------------------------------
class P16C71::PIR_16C71 : public PIR_SET
{
public:
  PIR_16C71(ADCON0 *adcon0)
    : m_adcon0(adcon0) { }

  virtual int interrupt_status() { return m_adcon0->getADIF(); }
  
private:
  ADCON0 *m_adcon0;
};

//------------------------------------------------------------------------
//
P16C71::P16C71(const char *_name, const char *desc)
  : P16X8X(_name, desc),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")

{
  m_pir = new PIR_16C71(&adcon0);
}

P16C71::~P16C71()
{
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
  delete m_pir;
}

void P16C71::create_sfr_map()
{
  add_sfr_register(&adcon0, 0x08, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x88, RegisterValue(0,0));

  add_sfr_register(&adres,  0x09, RegisterValue(0,0));

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1,0);
  adcon1.setNumberOfChannels(4);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setChannelConfiguration(0, 0x0f);
  adcon1.setChannelConfiguration(1, 0x0f);
  adcon1.setChannelConfiguration(2, 0x03);
  adcon1.setChannelConfiguration(3, 0x00);
  adcon1.setVrefHiConfiguration(1, 3);

  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(8);

  intcon = &intcon_reg;

  intcon_reg.set_pir_set(m_pir);
}


void P16C71::create()
{
  ram_top = 0x2f;
  P16X8X::create_iopin_map();
   _14bit_processor::create();

  set_eeprom(0);

  add_file_registers(0x0c, ram_top, 0x80);
  Pic14Bit::create_sfr_map();

  create_sfr_map();

}

Processor * P16C71::construct(const char *name)
{
  P16C71 *p = new P16C71(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16x71x::P16x71x(const char *_name, const char *desc)
  : _14bit_processor(_name, desc),
    intcon_reg(this,"intcon","Interrupt Control"),
    t1con(this, "t1con", "TMR1 Control"),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    pcon(this, "pcon", "pcon"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")
{
  pir1 = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg, &pie1);

  m_porta = new PicPortRegister(this,"porta","", 8,0x1f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);

  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);

  m_ioc = new IOC(this, "iocen", "Interrupt-On-Change negative edge", 0xf0);
  m_portb = new PicPortGRegister(this,"portb","",&intcon_reg,m_ioc,8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);
  m_portb->intf_bit = 0;

  m_ioc->put_value(0xf0);
}

P16x71x::~P16x71x()
{
  unassignMCLRPin();
  remove_sfr_register(&tmr0);
  remove_sfr_register(&intcon_reg);

  delete_sfr_register(m_portb);
  delete_sfr_register(m_trisb);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);

  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);

  remove_sfr_register(&pcon);

  remove_sfr_register(&t1con);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);

  remove_sfr_register(&pie1);
  delete_sfr_register(pir1);

  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);


  delete_file_registers(0x20,0x7f);
  delete_file_registers(0xa0,0xbf);

}

//-------------------------------------------------------------------
void P16x71x::option_new_bits_6_7(uint bits)
{
  //1 ((PORTB *)portb)->rbpu_intedg_update(bits);
  m_portb->setRBPU( (bits & (1<<7)) == (1<<7));
  m_portb->setIntEdge((bits & (1<<6)) == (1<<6));
}


void P16x71x::create_sfr_map()
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
  alias_file_registers(0x0a,0x0b,0x80);

  intcon = &intcon_reg;

  m_porta->setEnableMask(0x1f);
  m_porta->setTris(m_trisa);

  // The 16c62,c64 have general purpose registers
  // at addresses 20-7f and a0-bf
  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xbf, 0);


  add_sfr_register(pir1,   0x0c, RegisterValue(0,0),"pir1");
  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));

  add_sfr_register(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

  add_sfr_register(&pcon,   0x8e, RegisterValue(0,0),"pcon");

  add_sfr_register(&t1con,  0x10, RegisterValue(0,0));
  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));

  // get_pir_set()->set_pir1(get_pir1());
  pir_set_def.set_pir1(pir1);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  // Maybe there's a better place for this, but let's go ahead and link all
  // of the registers together (there's probably a better way too) :

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));

  tmr1h.tmrl  = &tmr1l;

  t1con.tmrl  = &tmr1l;

  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp(&ccp1con);
  pr2.tmr2    = &tmr2;


  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
    pir1->valid_bits = pir1->writable_bits = 0x47;
  }
  pie1.setPir(pir1);

  /* The A/D section is similar to 16x71, but not equal */
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));

  add_sfr_register(&adres,  0x1e, RegisterValue(0,0));

  //1adcon0.analog_port = porta;
  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setChannel_Mask(3);
  adcon0.setA2DBits(8);
  intcon = &intcon_reg;

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1| ADCON1::PCFG2,0);
  adcon1.setNumberOfChannels(4);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setChannelConfiguration(0, 0x0f);
  adcon1.setChannelConfiguration(1, 0x0f);
  adcon1.setChannelConfiguration(2, 0x0f);
  adcon1.setChannelConfiguration(3, 0x0f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);

  add_sfr_register(&ccpr1l,  0x15, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,  0x16, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x17, RegisterValue(0,0));

  ccp1con.setIOpin(&(*m_portb)[3], 0, 0, 0);
  ccp1con.setBitMask(0x3f);
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;



}

void P16x71x::create_iopin_map()
{

  package = new Package(18);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin(17, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin(18, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 1, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 3, m_porta->addPin(new IO_open_collector("porta4"),4));

  createMCLRPin(4);
  package->assign_pin( 5, 0);
  package->assign_pin( 6, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin( 7, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin( 8, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin( 9, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));
  package->assign_pin(14, 0);
  package->assign_pin(15, 0);
  package->assign_pin(16, 0);

}

//--------------------------------------
void P16C712::create_sfr_map()
{
  /* Extra timers and Capture/Compare are like in 16x63 => 16X6X code */
  //P16X6X_processor::create_sfr_map();
  P16x71x::create_sfr_map();
  ccp1con.setIOpin(&(*m_portb)[3], 0, 0, 0);
  ccp1con.setBitMask(0x3f);
  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, 0);
  add_sfr_register(&trisccp, 0x87, RegisterValue(0xff,0));
  add_sfr_register(&dataccp, 0x07, RegisterValue(0x00,0));
}

void P16C712::create()
{
  P16x71x::create_iopin_map(); /* 14 bits 18 pins connections */
  _14bit_processor::create();
  create_sfr_map();

  //1ccp1con.iopin = portb->pins[2];
}

Processor * P16C712::construct(const char *name)
{
  P16C712 *p = new P16C712(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16C712::P16C712(const char *_name, const char *desc)
  : P16x71x(_name, desc),
    trisccp(this, "trisccp", "TRISCCP Register"),
    dataccp(this, "dataccp", "DATACCP Register")
{
}

P16C712::~P16C712()
{
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
  remove_sfr_register(&trisccp);
  remove_sfr_register(&dataccp);
}

//--------------------------------------

Processor * P16C716::construct(const char *name)
{
  P16C716 *p = new P16C716(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16C716::P16C716(const char *_name, const char *desc)
  : P16C712(_name, desc)
{
}

//--------------------------------------

Processor * P16F716::construct(const char *name)
{
  P16F716 *p = new P16F716(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

void P16F716::create_sfr_map()
{
  P16x71x::create_sfr_map();
  add_sfr_register(&pwm1con, 0x18, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0x19, RegisterValue(0,0));

  eccpas.setIOpin(&(*m_portb)[4], 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  
  // portb3 already set
  ccp1con.setIOpin(0, &(*m_portb)[5], &(*m_portb)[6], &(*m_portb)[7]);
  ccp1con.setBitMask(0xff);
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);

}

void P16F716::create()
{
  P16x71x::create_iopin_map(); /* 14 bits 18 pins connections */
  _14bit_processor::create();
  create_sfr_map();
}

P16F716::P16F716(const char *_name, const char *desc)
  : P16C712(_name, desc),
    eccpas(this, "eccpas", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register")
{
}

P16F716::~P16F716()
{
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&eccpas);
}

//--------------------------------------

void P16C72::create_sfr_map()
{
  // Parent classes just set PIR version 1
  pir_set_2_def.set_pir1(pir1_2_reg);
  pir_set_2_def.set_pir2(pir2_2_reg);

  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));

  add_sfr_register(&adres,  0x1e, RegisterValue(0,0));

  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setPir(pir1_2_reg);
  adcon0.setChannel_Mask(7); // even though there are only 5 inputs...
  adcon0.setA2DBits(8);

  intcon = &intcon_reg;

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1| ADCON1::PCFG2, 0);
  adcon1.setNumberOfChannels(5);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[5]);
  adcon1.setChannelConfiguration(0, 0x1f);
  adcon1.setChannelConfiguration(1, 0x1f);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);

  // Link the A/D converter to the Capture Compare Module
  ccp2con.setADCON(&adcon0);
}

void P16C72::create()
{
  P16C62::create();
  P16C72::create_sfr_map();
}

Processor * P16C72::construct(const char *name)
{
  P16C72 *p = new P16C72(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}


P16C72::P16C72(const char *_name, const char *desc)
  : P16C62(_name, desc),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);

  delete pir1;
  delete pir2;
  pir1 = pir1_2_reg;
  pir2 = pir2_2_reg;
}

P16C72::~P16C72()
{
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
}

//--------------------------------------

void P16C73::create_sfr_map()
{
  // Parent classes just set PIR version 1
  pir_set_2_def.set_pir1(pir1_2_reg);
  pir_set_2_def.set_pir2(pir2_2_reg);

  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));

  add_sfr_register(&adres,  0x1e, RegisterValue(0,0));

  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setPir(pir1_2_reg);
  adcon0.setChannel_Mask(7); // even though there are only 5 inputs...
  adcon0.setA2DBits(8);

  intcon = &intcon_reg;

  //1adcon1.analog_port = porta;
  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1| ADCON1::PCFG2, 0);

  adcon1.setNumberOfChannels(5);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[5]);
  adcon1.setChannelConfiguration(0, 0x1f);
  adcon1.setChannelConfiguration(1, 0x1f);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);

  // Link the A/D converter to the Capture Compare Module
  ccp2con.setADCON(&adcon0);
}

void P16C73::create()
{
  P16C63::create();
  P16C73::create_sfr_map();
}

Processor * P16C73::construct(const char *name)
{
  P16C73 *p = new P16C73(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}


P16C73::P16C73(const char *_name, const char *desc)
  : P16C63(_name, desc),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);

  delete pir1;
  pir1 = pir1_2_reg;
  delete pir2;
  pir2 = pir2_2_reg;
}

P16C73::~P16C73()
{
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
}

//------------------------------------------------------------

void P16F73::create_sfr_map()
{
  add_sfr_register(pm_rd.get_reg_pmadr(),  0x10d);
  add_sfr_register(pm_rd.get_reg_pmadrh(), 0x10f);
  add_sfr_register(pm_rd.get_reg_pmdata(), 0x10c);
  add_sfr_register(pm_rd.get_reg_pmdath(), 0x10e);
  add_sfr_register(pm_rd.get_reg_pmcon1(), 0x18c);

  alias_file_registers(0x80,0x80,0x80);
  alias_file_registers(0x01,0x01,0x100);
  alias_file_registers(0x82,0x84,0x80);
  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x8a,0x8b,0x80);
  alias_file_registers(0x100,0x100,0x80);
  alias_file_registers(0x81,0x81,0x100);
  alias_file_registers(0x102,0x104,0x80);
  alias_file_registers(0x86,0x86,0x100);
  alias_file_registers(0x10a,0x10b,0x80);

  alias_file_registers(0x20,0x7f,0x100);
  alias_file_registers(0xa0,0xff,0x100);
}

void P16F73::create()
{
  P16C73::create();

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F73::create_sfr_map();
}

Processor * P16F73::construct(const char *name)
{
  P16F73 *p = new P16F73(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}


P16F73::P16F73(const char *_name, const char *desc)
  : P16C73(_name, desc),
    pm_rd(this)
{
}

P16F73::~P16F73()
{
  remove_sfr_register(pm_rd.get_reg_pmadr());
  remove_sfr_register(pm_rd.get_reg_pmadrh());
  remove_sfr_register(pm_rd.get_reg_pmdata());
  remove_sfr_register(pm_rd.get_reg_pmdath());
  remove_sfr_register(pm_rd.get_reg_pmcon1());
}

//------------------------------------------------------------
//
//           16C74
//

void P16C74::create_sfr_map()
{
  // Parent classes just set PIR version 1
  pir_set_2_def.set_pir1(pir1_2_reg);
  pir_set_2_def.set_pir2(pir2_2_reg);

  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));

  add_sfr_register(&adres,  0x1e, RegisterValue(0,0));

  //1adcon0.analog_port = porta;
  //1adcon0.analog_port2 = porte;

  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setPir(pir1_2_reg);
  adcon0.setChannel_Mask(7);
  adcon0.setA2DBits(8);

  intcon = &intcon_reg;

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 | ADCON1::PCFG2, 0);
  adcon1.setNumberOfChannels(8);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[5]);
  adcon1.setIOPin(5, &(*m_porte)[0]);
  adcon1.setIOPin(6, &(*m_porte)[1]);
  adcon1.setIOPin(7, &(*m_porte)[2]);

  adcon1.setChannelConfiguration(0, 0xff);
  adcon1.setChannelConfiguration(1, 0xff);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);

  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);

  // Link the A/D converter to the Capture Compare Module
  ccp2con.setADCON(&adcon0);
}

void P16C74::create()
{
  P16C65::create();
  P16C74::create_sfr_map();
}

Processor * P16C74::construct(const char *name)
{
  P16C74 *p = new P16C74(name);;

  p->create();
  p->create_invalid_registers ();

  return p;
}


P16C74::P16C74(const char *_name, const char *desc)
  : P16C65(_name, desc) ,
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);

  delete pir1;
  delete pir2;
  pir1 = pir1_2_reg;
  pir2 = pir2_2_reg;
}

P16C74::~P16C74()
{
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
}
//------------------------------------------------------------

void P16F74::create_sfr_map()
{
  add_sfr_register(pm_rd.get_reg_pmadr(),  0x10d);
  add_sfr_register(pm_rd.get_reg_pmadrh(), 0x10f);
  add_sfr_register(pm_rd.get_reg_pmdata(), 0x10c);
  add_sfr_register(pm_rd.get_reg_pmdath(), 0x10e);
  add_sfr_register(pm_rd.get_reg_pmcon1(), 0x18c);

  alias_file_registers(0x80,0x80,0x80);
  alias_file_registers(0x01,0x01,0x100);
  alias_file_registers(0x82,0x84,0x80);
  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x8a,0x8b,0x80);
  alias_file_registers(0x100,0x100,0x80);
  alias_file_registers(0x81,0x81,0x100);
  alias_file_registers(0x102,0x104,0x80);
  alias_file_registers(0x86,0x86,0x100);
  alias_file_registers(0x10a,0x10b,0x80);

  alias_file_registers(0x20,0x7f,0x100);
  alias_file_registers(0xa0,0xff,0x100);
}

void P16F74::create()
{
  P16C74::create();

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F74::create_sfr_map();
}

Processor * P16F74::construct(const char *name)
{
  P16F74 *p = new P16F74(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F74::P16F74(const char *_name, const char *desc)
  : P16C74(_name, desc),
    pm_rd(this)
{
}

P16F74::~P16F74()
{
  remove_sfr_register(pm_rd.get_reg_pmadr());
  remove_sfr_register(pm_rd.get_reg_pmadrh());
  remove_sfr_register(pm_rd.get_reg_pmdata());
  remove_sfr_register(pm_rd.get_reg_pmdath());
  remove_sfr_register(pm_rd.get_reg_pmcon1());
}

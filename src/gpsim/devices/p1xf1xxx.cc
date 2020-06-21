/*
   Copyright (C) 2013,2014,2017 Roy R. Rankin

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
/****************************************************************
*                                                               *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com    *
*                                                               *
*****************************************************************/

//
// p1xf1xxx
//
//  This file supports:
//    PIC12[L]F1822
//    PIC12[L]F1840
//    PIC16[L]F1823
//    PIC16[L]F1825
//
//Note: All these  processors have extended 14bit instructions

#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"
#include "stimuli.h"
#include "eeprom.h"
#include "p1xf1xxx.h"
#include "pic-ioports.h"
#include "packages.h"
#include "apfcon.h"
#include "pir.h"

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


P12F1822::P12F1822(const char *_name, const char *desc)
  : _14bit_e_processor(_name,desc),
    comparator(this),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    pie2(this,"PIE2", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    t1con_g(this, "t1con", "TMR1 Control Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    fvrcon(this, "fvrcon", "Voltage reference control register", 0xbf, 0x40),
    borcon(this, "borcon", "Brown-out reset control register"),
    ansela(this, "ansela", "Analog Select"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low"),
    osccon(0),
    osctune(this, "osctune", "Oscillator Tunning Register"),
    oscstat(this, "oscstat", "Oscillator Status Register"),
    wdtcon(this, "wdtcon", "Watch dog timer control", 0x3f),
    usart(this),
    ssp(this),
    apfcon(this, "apfcon", "Alternate Pin Function Control Register", 0xef),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register"),
    ccp1as(this, "ccp1as", "CCP1 Auto-Shutdown Control Register"),
    pstr1con(this, "pstr1con", "Pulse Sterring Control Register"),
    cpscon0(this, "cpscon0", " Capacitive Sensing Control Register 0"),
    cpscon1(this, "cpscon1", " Capacitive Sensing Control Register 1"),
    sr_module(this), dsm_module(this)
    
{
  m_iocaf = new IOCxF(this, "iocaf", "Interrupt-On-Change flag Register", 0x3f);
  m_iocap = new IOC(this, "iocap", "Interrupt-On-Change positive edge", 0x3f);
  m_iocan = new IOC(this, "iocan", "Interrupt-On-Change negative edge", 0x3f);
  m_porta = new PicPortIOCRegister(this,"porta","", intcon, m_iocap, m_iocan, m_iocaf, 8,0x3f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false, 0x37);
  m_lata  = new PicLatchRegister(this,"lata","",m_porta, 0x37);
  m_daccon0 = new DACCON0(this, "daccon0", "DAC Voltage reference register 0", 0xec, 32);
  m_daccon1 = new DACCON1(this, "daccon1", "DAC Voltage reference register 1", 0x1f, m_daccon0);
  m_cpu_temp = new CPU_Temp("cpu_temperature", 30., "CPU die temperature");

  tmr0.set_cpu(this, m_porta, 4, &option_reg);
  tmr0.start(0);
  tmr0.set_t1gcon(&t1con_g.t1gcon);
  cpscon1.m_cpscon0 = &cpscon0;
  cpscon0.m_tmr0 = &tmr0;
  cpscon0.m_t1con_g = &t1con_g;

  ((INTCON_14_PIR *)intcon)->write_mask = 0xfe;

  m_wpua = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x3f);

  pir1 = new PIR1v1822(this,"pir1","Peripheral Interrupt Register",intcon, &pie1);
  pir2 = new PIR2v1822(this,"pir2","Peripheral Interrupt Register",intcon, &pie2);

  comparator.cmxcon0[0] = new CMxCON0(this, "cm1con0", " Comparator C1 Control Register 0", 0, &comparator);
  comparator.cmxcon1[0] = new CMxCON1(this, "cm1con1", " Comparator C1 Control Register 1", 0, &comparator);
  comparator.cmout = new CMOUT(this, "cmout", "Comparator Output Register");
}

P12F1822::~P12F1822()
{
    unassignMCLRPin();
    delete_file_registers(0x20, 0x7f);
    delete_file_registers(0xa0, 0xbf);

    delete_sfr_register(m_iocap);
    delete_sfr_register(m_iocan);
    delete_sfr_register(m_iocaf);
    delete_sfr_register(m_daccon0);
    delete_sfr_register(m_daccon1);
    delete_sfr_register(m_trisa);
    delete_sfr_register(m_porta);
    delete_sfr_register(m_lata);

    delete_sfr_register(m_wpua);
    remove_sfr_register(&tmr0);

    remove_sfr_register(&tmr1l);
    remove_sfr_register(&tmr1h);
    remove_sfr_register(&t1con_g);
    remove_sfr_register(&t1con_g.t1gcon);

    remove_sfr_register(&tmr2);
    remove_sfr_register(&pr2);
    remove_sfr_register(&t2con);
    remove_sfr_register(&cpscon0);
    remove_sfr_register(&cpscon1);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(ssp.sspmsk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&ccpr1l);
    remove_sfr_register(&ccpr1h);
    remove_sfr_register(&ccp1con);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&ccp1as);
    remove_sfr_register(&pstr1con);
    remove_sfr_register(&pie1);
    remove_sfr_register(&pie2);
    remove_sfr_register(&adresl);
    remove_sfr_register(&adresh);
    remove_sfr_register(&adcon0);
    remove_sfr_register(&adcon1);
    remove_sfr_register(&borcon);
    remove_sfr_register(&fvrcon);
    remove_sfr_register(&sr_module.srcon0);
    remove_sfr_register(&sr_module.srcon1);
    remove_sfr_register(&apfcon );
    remove_sfr_register(&ansela);
    remove_sfr_register(get_eeprom()->get_reg_eeadr());
    remove_sfr_register(get_eeprom()->get_reg_eeadrh());
    remove_sfr_register(get_eeprom()->get_reg_eedata());
    remove_sfr_register(get_eeprom()->get_reg_eedatah());
    remove_sfr_register(get_eeprom()->get_reg_eecon1());
    remove_sfr_register(get_eeprom()->get_reg_eecon2());
    remove_sfr_register(&usart.spbrg);
    remove_sfr_register(&usart.spbrgh);
    remove_sfr_register(&usart.rcsta);
    remove_sfr_register(&usart.txsta);
    remove_sfr_register(&usart.baudcon);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(ssp.sspmsk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&ccpr1l);
    remove_sfr_register(&ccpr1h);
    remove_sfr_register(&ccp1con);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&ccp1as);
    remove_sfr_register(&pstr1con);
    remove_sfr_register(&osctune);
    remove_sfr_register(&option_reg);
    remove_sfr_register(osccon);
    remove_sfr_register(&oscstat);

    remove_sfr_register(comparator.cmxcon0[0]);
    remove_sfr_register(comparator.cmxcon1[0]);
    remove_sfr_register(comparator.cmout);
    delete_sfr_register(usart.rcreg);
    delete_sfr_register(usart.txreg);
    delete_sfr_register(pir1);
    delete_sfr_register(pir2);
    remove_sfr_register(&dsm_module.mdcon);
    remove_sfr_register(&dsm_module.mdsrc);
    remove_sfr_register(&dsm_module.mdcarl);
    remove_sfr_register(&dsm_module.mdcarh);
    delete e;
    delete m_cpu_temp;
    delete osccon;
}

Processor * P12F1822::construct(const char *name)
{
  P12F1822 *p = new P12F1822(name);

  p->create(0x7f, 256, 0x2700);
  p->create_invalid_registers ();

  return p;
}

void P12F1822::create_sfr_map()
{
  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);


  //add_sfr_register(indf,    0x00);
  add_file_registers(0xa0, 0xbf, 0x00);
  add_sfr_register(m_porta, 0x0c);
  add_sfr_registerR(pir1,    0x11, RegisterValue(0,0),"pir1");
  add_sfr_registerR(pir2,    0x12, RegisterValue(0,0),"pir2");
  add_sfr_register(&tmr0,   0x15);

  add_sfr_register(&tmr1l,  0x16, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x17, RegisterValue(0,0),"tmr1h");
  add_sfr_register(&t1con_g,  0x18, RegisterValue(0,0));
  add_sfr_register(&t1con_g.t1gcon, 0x19, RegisterValue(0,0));

  add_sfr_register(&tmr2,   0x1a, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x1b, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x1c, RegisterValue(0,0));
  add_sfr_register(&cpscon0,  0x1e, RegisterValue(0,0), "cpscon0");
  add_sfr_register(&cpscon1,  0x1f, RegisterValue(0,0));


  add_sfr_register(m_trisa, 0x8c, RegisterValue(0x3f,0));

  pcon.valid_bits = 0xcf;
  add_sfr_register(&option_reg, 0x95, RegisterValue(0xff,0));
  add_sfr_register(&osctune,    0x98, RegisterValue(0,0));
  add_sfr_register(osccon,     0x99, RegisterValue(0x38,0));
  add_sfr_register(&oscstat,    0x9a, RegisterValue(0,0));

  intcon_reg.set_pir_set(get_pir_set());


  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con_g;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));

  tmr1h.tmrl  = &tmr1l;
  t1con_g.tmrl  = &tmr1l;
  t1con_g.t1gcon.set_tmrl(&tmr1l);
  t1con_g.t1gcon.setInterruptSource(new InterruptSource(pir1, PIR1v1822::TMR1IF));



  tmr1l.setIOpin(&(*m_porta)[5]);
  t1con_g.t1gcon.setGatepin(&(*m_porta)[3]);

  add_sfr_register(&pie1,   0x91, RegisterValue(0,0));
  add_sfr_register(&pie2,   0x92, RegisterValue(0,0));
  add_sfr_register(&adresl, 0x9b);
  add_sfr_register(&adresh, 0x9c);
  add_sfr_register(&adcon0, 0x9d, RegisterValue(0x00,0));
  add_sfr_register(&adcon1, 0x9e, RegisterValue(0x00,0));


  usart.initialize(pir1,
        &(*m_porta)[0], // TX pin
        &(*m_porta)[1], // RX pin
        new _TXREG(this,"txreg", "USART Transmit Register", &usart),
        new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  usart.set_eusart(true);

  add_sfr_register(m_lata,    0x10c);
  add_sfr_register(comparator.cmxcon0[0], 0x111, RegisterValue(0x04,0));
  add_sfr_register(comparator.cmxcon1[0], 0x112, RegisterValue(0x00,0));
  add_sfr_register(comparator.cmout,      0x115, RegisterValue(0x00,0));
  add_sfr_register(&borcon,   0x116, RegisterValue(0x80,0));
  add_sfr_register(&fvrcon,   0x117, RegisterValue(0x00,0));
  add_sfr_register(m_daccon0, 0x118, RegisterValue(0x00,0));
  add_sfr_register(m_daccon1, 0x119, RegisterValue(0x00,0));
  add_sfr_register(&sr_module.srcon0, 0x11a, RegisterValue(0x00,0));
  add_sfr_register(&sr_module.srcon1, 0x11b, RegisterValue(0x00,0));
  add_sfr_register(&apfcon ,  0x11d, RegisterValue(0x00,0));
  add_sfr_register(&ansela,   0x18c, RegisterValue(0x17,0));
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x191);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x192);
  get_eeprom()->get_reg_eedata()->new_name("eedatl");
  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x193);
  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x194);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x195, RegisterValue(0x00,0));
  
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x196);
  add_sfr_register(usart.rcreg,    0x199, RegisterValue(0,0),"rcreg");
  add_sfr_register(usart.txreg,    0x19a, RegisterValue(0,0),"txreg");
  add_sfr_register(&usart.spbrg,   0x19b, RegisterValue(0,0),"spbrgl");
  add_sfr_register(&usart.spbrgh,  0x19c, RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.rcsta,   0x19d, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta,   0x19e, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.baudcon, 0x19f,RegisterValue(0x40,0),"baudcon");

  add_sfr_register(m_wpua,       0x20c, RegisterValue(0x3f,0),"wpua");
  add_sfr_register(&ssp.sspbuf,  0x211, RegisterValue(0,0),"ssp1buf");
  add_sfr_register(&ssp.sspadd,  0x212, RegisterValue(0,0),"ssp1add");
  add_sfr_register(ssp.sspmsk, 0x213, RegisterValue(0xff,0),"ssp1msk");
  add_sfr_register(&ssp.sspstat, 0x214, RegisterValue(0,0),"ssp1stat");
  add_sfr_register(&ssp.sspcon,  0x215, RegisterValue(0,0),"ssp1con");
  add_sfr_register(&ssp.sspcon2, 0x216, RegisterValue(0,0),"ssp1con2");
  add_sfr_register(&ssp.ssp1con3, 0x217, RegisterValue(0,0),"ssp1con3");
  add_sfr_register(&ccpr1l,      0x291, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,      0x292, RegisterValue(0,0));
  add_sfr_register(&ccp1con,     0x293, RegisterValue(0,0));
  add_sfr_register(&pwm1con,     0x294, RegisterValue(0,0));
  add_sfr_register(&ccp1as,      0x295, RegisterValue(0,0));
  add_sfr_register(&pstr1con,    0x296, RegisterValue(1,0));

  add_sfr_register(m_iocap, 0x391, RegisterValue(0,0),"iocap");
  add_sfr_register(m_iocan, 0x392, RegisterValue(0,0),"iocan");
  add_sfr_register(m_iocaf, 0x393, RegisterValue(0,0),"iocaf");
  m_iocaf->set_intcon(intcon);
  add_sfr_register(&dsm_module.mdcon, 0x39c, RegisterValue(0x20,0));
  add_sfr_register(&dsm_module.mdsrc, 0x39d, RegisterValue(0x00,0));
  add_sfr_register(&dsm_module.mdcarl, 0x39e, RegisterValue(0x00,0));
  add_sfr_register(&dsm_module.mdcarh, 0x39f, RegisterValue(0x00,0));

  tmr2.ssp_module[0] = &ssp;

    ssp.initialize(
        get_pir_set(),    // PIR
        &(*m_porta)[1],   // SCK
        &(*m_porta)[3],   // SS
        &(*m_porta)[0],   // SDO
        &(*m_porta)[2],   // SDI
          m_trisa,        // i2c tris port
        SSP_TYPE_MSSP1
    );
    apfcon.set_pins(0, &ccp1con, CCPCON::CCP_PIN, &(*m_porta)[2], &(*m_porta)[5]); //CCP1/P1A
    apfcon.set_pins(1, &ccp1con, CCPCON::PxB_PIN, &(*m_porta)[0], &(*m_porta)[4]); //P1B
    apfcon.set_pins(2, &usart, USART_MODULE::TX_PIN, &(*m_porta)[0], &(*m_porta)[4]); //USART TX Pin
    apfcon.set_pins(3, &t1con_g.t1gcon, 0, &(*m_porta)[4], &(*m_porta)[3]); //tmr1 gate
    apfcon.set_pins(5, &ssp, SSP1_MODULE::SS_PIN, &(*m_porta)[3], &(*m_porta)[0]); //SSP SS
    apfcon.set_pins(6, &ssp, SSP1_MODULE::SDO_PIN, &(*m_porta)[0], &(*m_porta)[4]); //SSP SDO
    apfcon.set_pins(7, &usart, USART_MODULE::RX_PIN, &(*m_porta)[1], &(*m_porta)[5]); //USART RX Pin
    
    if (pir1) 
    {
        pir1->set_intcon(intcon);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    pie2.setPir(pir2);
    t2con.tmr2 = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
//  tmr2.add_ccp ( &ccp2con );
    pr2.tmr2    = &tmr2;

    ccp1as.setIOpin(0, 0, &(*m_porta)[2]);
    ccp1as.link_registers(&pwm1con, &ccp1con);

    ccp1con.setIOpin(&(*m_porta)[2], &(*m_porta)[0]);
    ccp1con.pstrcon = &pstr1con;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v1822::CCP1IF, &tmr2, &ccp1as);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;


    ansela.config(0x17, 0);
    ansela.setValidBits(0x17);
    ansela.setAdcon1(&adcon1);

    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(intcon);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0x1f);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);

    adcon1.setAdcon0(&adcon0);
    adcon1.setNumberOfChannels(32); // not all channels are used
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);
    adcon1.setValidBits(0xf3);
    adcon1.setVrefHiConfiguration(0, 1);
    adcon1.set_FVR_chan(0x1f);

    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2]);
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_porta)[4]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[0]);
    comparator.cmxcon0[0]->setBitMask(0xf7);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, (1<<5)));
    comparator.cmxcon1[0]->setBitMask(0xf1);
    comparator.assign_pir_set(get_pir_set());
    comparator.assign_t1gcon(&t1con_g.t1gcon);
    comparator.assign_sr_module(&sr_module);
    fvrcon.set_adcon1(&adcon1);
    fvrcon.set_cpscon0(&cpscon0);
    fvrcon.set_daccon0(m_daccon0);
    fvrcon.set_cmModule(&comparator);
    fvrcon.set_VTemp_AD_chan(0x1d);
    fvrcon.set_FVRAD_AD_chan(0x1f);

    m_daccon0->set_adcon1(&adcon1);
    m_daccon0->set_cpscon0(&cpscon0);
    m_daccon0->set_cmModule(&comparator);
    m_daccon0->set_FVRCDA_AD_chan(0x1e);
    m_daccon0->setDACOUT(&(*m_porta)[0]);

    cpscon0.set_pin(0, &(*m_porta)[0]);
    cpscon0.set_pin(1, &(*m_porta)[1]);
    cpscon0.set_pin(2, &(*m_porta)[2]);
    cpscon0.set_pin(3, &(*m_porta)[4]);


    sr_module.setPins(&(*m_porta)[1], &(*m_porta)[2], &(*m_porta)[5]);

    osccon->set_osctune(&osctune);
    osccon->set_oscstat(&oscstat);
    osctune.set_osccon((OSCCON *)osccon);
    osccon->write_mask = 0xfb;
    dsm_module.usart_mod = &usart;
}

//-------------------------------------------------------------------
void P12F1822::set_out_of_range_pm(uint address, uint value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
      get_eeprom()->change_rom(address - 0x2100, value);
}

void P12F1822::create_iopin_map()
{

  package = new Package(8);
  if(!package)
    return;

  // Now Create the package and place the I/O pins
  package->assign_pin(7, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  package->assign_pin(6, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(5, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  package->assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));

  package->assign_pin( 1, 0);        // Vdd
  package->assign_pin( 8, 0);        // Vss
}

void  P12F1822::create(int ram_top, int eeprom_size, int dev_id)
{
  create_iopin_map();

  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);

  osccon = new OSCCON_2(this, "osccon", "Oscillator Control Register");

  pic_processor::create();

  e->initialize(eeprom_size, 16, 16, 0x8000);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0xff);

  add_file_registers(0x20, ram_top, 0x00);
  _14bit_e_processor::create_sfr_map();
  create_sfr_map();
  dsm_module.setOUTpin(&(*m_porta)[0]);
  dsm_module.setMINpin(&(*m_porta)[1]);
  dsm_module.setCIN1pin(&(*m_porta)[2]);
  dsm_module.setCIN2pin(&(*m_porta)[4]);
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(dev_id);
}

void P12F1822::enter_sleep()
{
    tmr1l.sleep();
    osccon->sleep();
    _14bit_e_processor::enter_sleep();
}

void P12F1822::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
        tmr1l.wake();
        osccon->wake();
        _14bit_e_processor::exit_sleep();
    }
}

void P12F1822::option_new_bits_6_7(uint bits)
{
        Dprintf(("P12F1822::option_new_bits_6_7 bits=%x\n", bits));
    m_porta->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
    m_wpua->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
}

void P12F1822::oscillator_select(uint cfg_word1, bool clkout)
{
    uint mask = 0x1f;

    uint fosc = cfg_word1 & (FOSC0|FOSC1|FOSC2);

    osccon->set_config_irc(fosc == 4);
    osccon->set_config_xosc(fosc < 3);
    osccon->set_config_ieso(cfg_word1 & IESO);
    set_int_osc(false);
    switch(fosc)
    {
    case 0:        //LP oscillator: low power crystal
    case 1:        //XT oscillator: Crystal/resonator
    case 2:        //HS oscillator: High-speed crystal/resonator
        mask = 0x0f;
        break;

    case 3:        //EXTRC oscillator External RC circuit connected to CLKIN pin
        mask = 0x1f;
        if(clkout) mask = 0x0f;
        break;

    case 4:        //INTOSC oscillator: I/O function on CLKIN pin
        set_int_osc(true);
        mask = 0x3f;
        if(clkout) mask = 0x2f;
        break;

    case 5:        //ECL: External Clock, Low-Power mode (0-0.5 MHz): on CLKIN pin
        mask = 0x1f;
        if(clkout) mask = 0x0f;
        break;

    case 6:        //ECM: External Clock, Medium-Power mode (0.5-4 MHz): on CLKIN pin
        mask = 0x1f;
        if(clkout) mask = 0x0f;
        break;

    case 7:        //ECH: External Clock, High-Power mode (4-32 MHz): on CLKIN pin
        mask = 0x1f;
        if(clkout) mask = 0x0f;
        break;
    };
    ansela.setValidBits(0x17 & mask);
    m_porta->setEnableMask(mask);
}

void P12F1822::program_memory_wp(uint mode)
{
        switch(mode)
        {
        case 3:        // no write protect
            get_eeprom()->set_prog_wp(0x0);
            break;

        case 2: // write protect 0000-01ff
            get_eeprom()->set_prog_wp(0x0200);
            break;

        case 1: // write protect 0000-03ff
            get_eeprom()->set_prog_wp(0x0400);
            break;

        case 0: // write protect 0000-07ff
            get_eeprom()->set_prog_wp(0x0800);
            break;

        default:
            printf("%s unexpected mode %u\n", __FUNCTION__, mode);
            break;
        }

}
//========================================================================


P12LF1822::P12LF1822(const char *_name, const char *desc)
  : P12F1822(_name,desc)
{}

P12LF1822::~P12LF1822()
{}

Processor * P12LF1822::construct(const char *name)
{
  P12LF1822 *p = new P12LF1822(name);

  p->create(0x7f, 256, 0x2800);
  p->create_invalid_registers ();

  return p;
}

void  P12LF1822::create(int ram_top, int eeprom_size, int dev_id)
{
  P12F1822::create(ram_top, eeprom_size, dev_id);
}

//========================================================================
Processor * P12F1840::construct(const char *name)
{
  P12F1840 *p = new P12F1840(name);

  p->create(0x7f, 256, 0x1b80);
  p->create_invalid_registers ();

  return p;
}

P12F1840::P12F1840(const char *_name, const char *desc) :
        P12F1822(_name, desc)
{
}

P12F1840::~P12F1840()
{
  delete_file_registers(0xc0, 0xef, 0x00);
  delete_file_registers(0x120, 0x16f, 0x00);
  delete_sfr_register(vrefcon);
}

void  P12F1840::create(int ram_top, int eeprom_size, int dev_id)
{
  P12F1822::create(ram_top, eeprom_size, 0x1b80);
  add_file_registers(0xc0, 0xef, 0x00);
  add_file_registers(0x120, 0x16f, 0x00);

  vrefcon = new sfr_register(this, "vrefcon",
                "Voltage Regulator Control Register");
  add_sfr_register(vrefcon, 0x197, RegisterValue(0x01,0));
}

//========================================================================
Processor * P12LF1840::construct(const char *name)
{
  P12LF1840 *p = new P12LF1840(name);

  p->create(0x7f, 256, 0x1b80);
  p->create_invalid_registers ();

  return p;
}

P12LF1840::P12LF1840(const char *_name, const char *desc) :
        P12F1840(_name, desc)
{
}

P12LF1840::~P12LF1840()
{
}

void  P12LF1840::create(int ram_top, int eeprom_size, int dev_id)
{
  P12F1840::create(ram_top, eeprom_size, 0x1bc0);
}

//========================================================================

P16F1823::P16F1823(const char *_name, const char *desc)
  : P12F1822(_name,desc),
    anselc(this, "anselc", "Analog Select port c")
{

  m_portc = new PicPortBRegister(this,"portc","", intcon, 8,0x3f);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false, 0x3f);
  m_latc  = new PicLatchRegister(this,"latc","",m_portc, 0x3f);
  m_wpuc = new WPU(this, "wpuc", "Weak Pull-up Register", m_portc, 0x3f);
  comparator.cmxcon0[1] = new CMxCON0(this, "cm2con0", " Comparator C2 Control Register 0", 1, &comparator);
  comparator.cmxcon1[1] = new CMxCON1(this, "cm2con1", " Comparator C2 Control Register 1", 1, &comparator);
  cpscon1.mValidBits = 0x0f;
  pir2->valid_bits |= PIR2v1822::C2IF;
  pir2->writable_bits |= PIR2v1822::C2IF;
}

P16F1823::~P16F1823()
{
    delete_sfr_register(m_portc);
    delete_sfr_register(m_trisc);
    delete_sfr_register(m_latc);
    remove_sfr_register(comparator.cmxcon0[1]);
    remove_sfr_register(comparator.cmxcon1[1]);
    delete_sfr_register(m_wpuc);
    remove_sfr_register(&anselc);
}

void P16F1823::create_iopin_map()
{
  package = new Package(14);
  if(!package) return;

  // Now Create the package and place the I/O pins
  package->assign_pin(13, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  package->assign_pin(12, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(11, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  package->assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));

  package->assign_pin(10, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));
  package->assign_pin(9, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
  package->assign_pin(8, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
  package->assign_pin(7, m_portc->addPin(new IO_bi_directional_pu("portc3"),3));
  package->assign_pin(6, m_portc->addPin(new IO_bi_directional_pu("portc4"),4));
  package->assign_pin(5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));

  package->assign_pin( 1, 0);        // Vdd
  package->assign_pin( 14, 0);        // Vss
}

Processor * P16F1823::construct(const char *name)
{
  P16F1823 *p = new P16F1823(name);

  p->create(0x7f, 256, 0x2720);
  p->create_invalid_registers ();

  return p;
}

void  P16F1823::create(int ram_top, int eeprom_size, int dev_id)
{
  create_iopin_map();
  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);

  osccon = new OSCCON_2(this, "osccon", "Oscillator Control Register");

  pic_processor::create();

  e->initialize(eeprom_size, 16, 16, 0x8000);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0xff);

  add_file_registers(0x20, ram_top, 0x00);
  _14bit_e_processor::create_sfr_map();
  P12F1822::create_sfr_map();
  create_sfr_map();
  dsm_module.setOUTpin(&(*m_portc)[4]);
  dsm_module.setMINpin(&(*m_portc)[3]);
  dsm_module.setCIN1pin(&(*m_portc)[2]);
  dsm_module.setCIN2pin(&(*m_portc)[5]);
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(dev_id);
}

void P16F1823::create_sfr_map()
{
    add_sfr_register(m_portc, 0x0e);
    add_sfr_register(m_trisc, 0x8e, RegisterValue(0x3f,0));
    add_sfr_register(m_latc, 0x10e);
    add_sfr_register(comparator.cmxcon0[1], 0x113, RegisterValue(0x04,0));
    add_sfr_register(comparator.cmxcon1[1], 0x114, RegisterValue(0x00,0));
    add_sfr_register(&anselc, 0x18e, RegisterValue(0x0f,0));
    add_sfr_register(m_wpuc, 0x20e, RegisterValue(0x3f,0),"wpuc");

    anselc.config(0x0f, 4);
    anselc.setValidBits(0x0f);
    anselc.setAdcon1(&adcon1);
    ansela.setAnsel(&anselc);
    anselc.setAnsel(&ansela);
    adcon1.setIOPin(4, &(*m_portc)[0]);
    adcon1.setIOPin(5, &(*m_portc)[1]);
    adcon1.setIOPin(6, &(*m_portc)[2]);
    adcon1.setIOPin(7, &(*m_portc)[3]);

    ssp.set_sckPin(&(*m_portc)[0]);
    ssp.set_sdiPin(&(*m_portc)[1]);
    ssp.set_sdoPin(&(*m_portc)[2]);
    ssp.set_ssPin(&(*m_portc)[3]);
    ssp.set_tris(m_trisc);

    // Pin values for default APFCON
    usart.set_TXpin(&(*m_portc)[4]); // TX pin
    usart.set_RXpin(&(*m_portc)[5]);  // RX pin

    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    apfcon.set_ValidBits(0xec);
    // pins 0,1 not used for p16f1823
    apfcon.set_pins(2, &usart, USART_MODULE::TX_PIN, &(*m_portc)[4], &(*m_porta)[0]); //USART TX Pin
    // pin 3 defined in p12f1822
    apfcon.set_pins(5, &ssp, SSP1_MODULE::SS_PIN, &(*m_portc)[3], &(*m_porta)[3]); //SSP SS
    apfcon.set_pins(6, &ssp, SSP1_MODULE::SDO_PIN, &(*m_portc)[2], &(*m_porta)[4]); //SSP SDO
    apfcon.set_pins(7, &usart, USART_MODULE::RX_PIN, &(*m_portc)[5], &(*m_porta)[1]); //USART RX Pin
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1],
                                          &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[1]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1],
                                          &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[1]->set_INpinPos(&(*m_portc)[0]);
    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2]);
    comparator.cmxcon1[1]->set_OUTpin(&(*m_portc)[4]);
    comparator.cmxcon0[0]->setBitMask(0xf7);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, (1<<5)));
    comparator.cmxcon0[1]->setBitMask(0xf7);
    comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, (1<<6)));
    comparator.cmxcon1[0]->setBitMask(0xf3);
    comparator.cmxcon1[1]->setBitMask(0xf3);


    cpscon0.set_pin(4, &(*m_portc)[0]);
    cpscon0.set_pin(5, &(*m_portc)[1]);
    cpscon0.set_pin(6, &(*m_portc)[2]);
    cpscon0.set_pin(7, &(*m_portc)[3]);
    sr_module.srcon1.set_ValidBits(0xff);
    sr_module.setPins(&(*m_porta)[1], &(*m_porta)[2], &(*m_portc)[4]);
}
//========================================================================


P16LF1823::P16LF1823(const char *_name, const char *desc)
  : P16F1823(_name,desc)
{
}
P16LF1823::~P16LF1823()
{
}

void  P16LF1823::create(int ram_top, int eeprom_size, int dev_id)
{
  P16F1823::create(ram_top, eeprom_size, dev_id);
}

Processor * P16LF1823::construct(const char *name)
{
  P16LF1823 *p = new P16LF1823(name);

  p->create(0x7f, 256, 0x2820);
  p->create_invalid_registers ();

  return p;
}

//========================================================================
Processor * P16F1825::construct(const char *name)
{
  P16F1825 *p = new P16F1825(name);

  p->create( 0x7f, 256, 0x2760 );
  p->create_invalid_registers ();

  return p;
}
P16F1825::P16F1825(const char *_name, const char *desc)
        : P16F1823(_name, desc)
        , pie3(     this,"pie3", "Peripheral Interrupt Enable")
        , t4con(    this, "t4con", "TMR4 Control")
        , pr4(      this, "pr4", "TMR4 Period Register")
        , tmr4(     this, "tmr4", "TMR4 Register")
        , t6con(    this, "t6con", "TMR6 Control")
        , pr6(      this, "pr6", "TMR6 Period Register")
        , tmr6(     this, "tmr6", "TMR6 Register")
        , ccp2con(  this, "ccp2con", "Capture Compare Control")
        , ccpr2l(   this, "ccpr2l", "Capture Compare 2 Low")
        , ccpr2h(   this, "ccpr2h", "Capture Compare 2 High")
        , pwm2con(  this, "pwm2con", "Enhanced PWM Control Register")
        , ccp2as(   this, "ccp2as", "CCP2 Auto-Shutdown Control Register")
        , pstr2con( this, "pstr2con", "Pulse Sterring Control Register")
        , ccp3con(  this, "ccp3con", "Capture Compare Control")
        , ccpr3l(   this, "ccpr3l", "Capture Compare 3 Low")
        , ccpr3h(   this, "ccpr3h", "Capture Compare 3 High")
        , ccp4con(  this, "ccp4con", "Capture Compare Control")
        , ccpr4l(   this, "ccpr4l", "Capture Compare 4 Low")
        , ccpr4h(   this, "ccpr4h", "Capture Compare 4 High")
        , ccptmrs(  this, "ccptmrs", "PWM Timer Selection Control Register")
        , apfcon0(  this, "apfcon0", "Alternate Pin Function Control Register 0", 0xec)
        , apfcon1(  this, "apfcon1", "Alternate Pin Function Control Register 1", 0x0f)
        , inlvla(this, "inlvla", "PORTA Input Level Control Register")
        , inlvlc(this, "inlvlc", "PORTC Input Level Control Register")
{
    pir3 = new PIR( this, "pir3", "Peripheral Interrupt Register", intcon, &pie3, 0x3a);
}

P16F1825::~P16F1825()
{
    delete_file_registers(0xc0, 0xef);
    delete_file_registers(0x120, 0x16f);
    delete_file_registers(0x1a0, 0x1ef);
    delete_file_registers(0x220, 0x26f);
    delete_file_registers(0x2a0, 0x2ef);
    delete_file_registers(0x320, 0x32f);
    delete_file_registers(0x420, 0x46f);
    delete_file_registers(0x4a0, 0x4ef);
    delete_file_registers(0x520, 0x56f);
    delete_file_registers(0x5a0, 0x5ef);
    delete_sfr_register(pir3);
    remove_sfr_register(&pie3);
    remove_sfr_register(&ccpr2l);
    remove_sfr_register(&ccpr2h);
    remove_sfr_register(&ccp2con);
    remove_sfr_register(&pwm2con);
    remove_sfr_register(&ccp2as);
    remove_sfr_register(&pstr2con);
    remove_sfr_register(&ccptmrs);
    remove_sfr_register(&ccpr3l);
    remove_sfr_register(&ccpr3h);
    remove_sfr_register(&ccp3con);
    remove_sfr_register(&ccpr4l);
    remove_sfr_register(&ccpr4h);
    remove_sfr_register(&ccp4con);
    remove_sfr_register(&apfcon1);
    remove_sfr_register(&inlvla);
    remove_sfr_register(&inlvlc);
    remove_sfr_register(&tmr4);
    remove_sfr_register(&pr4);
    remove_sfr_register(&t4con);
    remove_sfr_register(&tmr6);
    remove_sfr_register(&pr6);
    remove_sfr_register(&t6con);
}

void  P16F1825::create(int ram_top, int eeprom_size, int dev_id)
{
    P16F1823::create( ram_top, eeprom_size, dev_id );
    pir_set_2_def.set_pir3(pir3);
    pie3.setPir(pir3);
    add_file_registers(0xc0, 0xef, 0x00);
    add_file_registers(0x120, 0x16f, 0x00);
    add_file_registers(0x1a0, 0x1ef, 0x00);
    add_file_registers(0x220, 0x26f, 0x00);
    add_file_registers(0x2a0, 0x2ef, 0x00);
    add_file_registers(0x320, 0x32f, 0x00);
    add_file_registers(0x420, 0x46f, 0x00);
    add_file_registers(0x4a0, 0x4ef, 0x00);
    add_file_registers(0x520, 0x56f, 0x00);
    add_file_registers(0x5a0, 0x5ef, 0x00);
    add_sfr_register(pir3,         0x013, RegisterValue(0,0));
    add_sfr_register(&pie3,        0x093, RegisterValue(0,0));
    add_sfr_register(&apfcon1,     0x11e, RegisterValue(0,0));
    add_sfr_register(&ccpr2l,      0x298, RegisterValue(0,0));
    add_sfr_register(&ccpr2h,      0x299, RegisterValue(0,0));
    add_sfr_registerR(&ccp2con,    0x29a, RegisterValue(0,0));
    add_sfr_register(&pwm2con,     0x29b, RegisterValue(0,0));
    add_sfr_register(&ccp2as,      0x29c, RegisterValue(0,0));
    add_sfr_register(&pstr2con,    0x29d, RegisterValue(1,0));
    ccptmrs.set_tmr246(&tmr2, &tmr4, &tmr6);
    ccptmrs.set_ccp(&ccp1con, &ccp2con, &ccp3con, &ccp4con);
    add_sfr_registerR(&ccptmrs,    0x29e, RegisterValue(0,0));

    tmr2.add_ccp ( &ccp2con );
    add_sfr_register(&ccpr3l,      0x311, RegisterValue(0,0));
    add_sfr_register(&ccpr3h,      0x312, RegisterValue(0,0));
    add_sfr_registerR(&ccp3con,    0x313, RegisterValue(0,0));
    add_sfr_register(&ccpr4l,      0x318, RegisterValue(0,0));
    add_sfr_register(&ccpr4h,      0x319, RegisterValue(0,0));
    add_sfr_registerR(&ccp4con,    0x31a, RegisterValue(0,0));
    
    add_sfr_register(&inlvla,      0x38c, RegisterValue(0,0));
    add_sfr_register(&inlvlc,      0x38e, RegisterValue(0,0));

    add_sfr_register(&tmr4,        0x415, RegisterValue(0,0));
    add_sfr_register(&pr4,         0x416, RegisterValue(0,0));
    add_sfr_register(&t4con,       0x417, RegisterValue(0,0));
    add_sfr_register(&tmr6,        0x41c, RegisterValue(0,0));
    add_sfr_register(&pr6,         0x41d, RegisterValue(0,0));
    add_sfr_register(&t6con,       0x41e, RegisterValue(0,0));

    ccp1con.setBitMask(0xff);
    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    ccp2as.setIOpin(0, 0, &(*m_porta)[2]);
    ccp2as.link_registers(&pwm2con, &ccp2con);

    ccp2con.setBitMask(0xff);
    ccp2con.setIOpin(&(*m_portc)[3], &(*m_portc)[2]);
    ccp2con.pstrcon = &pstr2con;
    ccp2con.pwm1con = &pwm2con;
    ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v1822::CCP2IF, &tmr2, &ccp2as);
    ccpr2l.ccprh  = &ccpr2h;
    ccpr2l.tmrl   = &tmr1l;
    ccpr2h.ccprl  = &ccpr2l;
    ccp3con.setCrosslinks(&ccpr3l, pir3, (1<<4), 0, 0);
    ccp3con.setIOpin(&(*m_porta)[2]);
    ccpr3l.ccprh  = &ccpr3h;
    ccpr3l.tmrl   = &tmr1l;
    ccpr3h.ccprl  = &ccpr3l;
    ccp4con.setCrosslinks(&ccpr4l, pir3, (1<<5), 0, 0);
    ccp4con.setIOpin(&(*m_portc)[1]);
    ccpr4l.ccprh  = &ccpr4h;
    ccpr4l.tmrl   = &tmr1l;
    ccpr4h.ccprl  = &ccpr4l;
    t4con.tmr2 = &tmr4;

    tmr4.setInterruptSource(new InterruptSource(pir3, 1<<1));
    tmr4.pir_set = get_pir_set();
    tmr4.pr2    = &pr4;
    tmr4.t2con  = &t4con;


    t6con.tmr2 = &tmr6;
    tmr6.setInterruptSource(new InterruptSource(pir3, 1<<3));
    tmr6.pr2    = &pr6;
    tmr6.t2con  = &t6con;

    pr2.tmr2    = &tmr2;
    pr4.tmr2    = &tmr4;
    pr6.tmr2    = &tmr6;

    apfcon0.set_pins(2, &usart, USART_MODULE::TX_PIN, &(*m_portc)[4], &(*m_porta)[0]); //USART TX Pin
    apfcon0.set_pins(3, &t1con_g.t1gcon, 0, &(*m_porta)[4], &(*m_porta)[3]); //tmr1 gate
    apfcon0.set_pins(5, &ssp, SSP1_MODULE::SS_PIN, &(*m_portc)[3], &(*m_porta)[3]); //SSP SS
    apfcon0.set_pins(6, &ssp, SSP1_MODULE::SDO_PIN, &(*m_portc)[2], &(*m_porta)[4]); //SSP SDO
    apfcon0.set_pins(7, &usart, USART_MODULE::RX_PIN, &(*m_portc)[5], &(*m_porta)[1]); //USART RX Pin
    apfcon1.set_pins(0, &ccp2con, CCPCON::CCP_PIN, &(*m_portc)[3], &(*m_porta)[5]); //CCP2/P2A
    apfcon1.set_pins(1, &ccp2con, CCPCON::PxB_PIN, &(*m_portc)[2], &(*m_porta)[4]); //P2B
    apfcon1.set_pins(2, &ccp1con, CCPCON::PxC_PIN, &(*m_portc)[3], &(*m_portc)[1]); //P1C
    apfcon1.set_pins(3, &ccp1con, CCPCON::PxD_PIN, &(*m_portc)[2], &(*m_portc)[0]); //P1D
}

//========================================================================
Processor * P16LF1825::construct(const char *name)
{
  P16LF1825 *p = new P16LF1825(name);

  p->create(0x3FF, 256, 0x2860);
  p->create_invalid_registers ();

  return p;
}
P16LF1825::P16LF1825(const char *_name, const char *desc)
         : P16F1825(_name, desc)
{
}

P16LF1825::~P16LF1825()
{
}

void  P16LF1825::create(int ram_top, int eeprom_size, int dev_id)
{
  P16F1825::create(ram_top, eeprom_size, dev_id);
}

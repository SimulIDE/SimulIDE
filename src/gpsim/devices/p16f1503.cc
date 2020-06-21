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


// this  processors have extended 14bit instructions

#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"

#include "stimuli.h"
#include "eeprom.h"
#include "p16f1503.h"
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


P16F1503::P16F1503(const char *_name, const char *desc)
  : _14bit_e_processor(_name,desc),
    comparator(this),
    pie1(this,"pie1", "Peripheral Interrupt Enable"),
    pie2(this,"pie2", "Peripheral Interrupt Enable"),
    pie3(this,"pie3", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    t1con_g(this, "t1con", "TMR1 Control Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    fvrcon(this, "fvrcon", "Voltage reference control register", 0xbf, 0x40),
    borcon(this, "borcon", "Brown-out reset control register"),
    ansela(this, "ansela", "Analog Select port a"),
    anselc(this, "anselc", "Analog Select port c"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adcon2(this,"adcon2", "A2D Control 2"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low"),
    osccon(0),
    osctune(this, "osctune", "Oscillator Tunning Register"),
    oscstat(this, "oscstat", "Oscillator Status Register"),
    wdtcon(this, "wdtcon", "Watch dog timer control", 0x3f),
    ssp(this),
    apfcon1(this, "apfcon", "Alternate Pin Function Control Register", 0x3b),
    pwm1con(this, "pwm1con", "PWM 1 Control Register", 0),
    pwm1dcl(this, "pwm1dcl", "PWM 1 DUTY CYCLE LOW BITS"),
    pwm1dch(this, "pwm1dch", "PWM 1 DUTY CYCLE HIGH BITS"),
    pwm2con(this, "pwm2con", "PWM 2 Control Register", 1),
    pwm2dcl(this, "pwm2dcl", "PWM 2 DUTY CYCLE LOW BITS"),
    pwm2dch(this, "pwm2dch", "PWM 2 DUTY CYCLE HIGH BITS"),
    pwm3con(this, "pwm3con", "PWM 3 Control Register", 2),
    pwm3dcl(this, "pwm3dcl", "PWM 3 DUTY CYCLE LOW BITS"),
    pwm3dch(this, "pwm3dch", "PWM 3 DUTY CYCLE HIGH BITS"),
    pwm4con(this, "pwm4con", "PWM 4 Control Register", 3),
    pwm4dcl(this, "pwm4dcl", "PWM 4 DUTY CYCLE LOW BITS"),
    pwm4dch(this, "pwm4dch", "PWM 4 DUTY CYCLE HIGH BITS"),
    cwg(this), nco(this), 
    clcdata(this, "clcdata", "CLC Data Output"), 
    clc1(this, 0, &clcdata), clc2(this, 1, &clcdata),
        frc(600000., CLC::FRC_IN), 
        lfintosc(32000., CLC::LFINTOSC),  // 32kHz is within tolerance or 31kHz 
        hfintosc(16e6, CLC::HFINTOSC),
        vregcon(this, "vregcon", "Voltage Regulator Control Register")
{
  m_portc= new PicPortBRegister(this,"portc","", intcon, 8,0x3f);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false, 0x3f);
  m_latc  = new PicLatchRegister(this,"latc","",m_portc, 0x3f);


  m_iocaf = new IOCxF(this, "iocaf", "Interrupt-On-Change flag Register", 0x3f);
  m_iocap = new IOC(this, "iocap", "Interrupt-On-Change positive edge", 0x3f);
  m_iocan = new IOC(this, "iocan", "Interrupt-On-Change negative edge", 0x3f);
  m_porta= new PicPortIOCRegister(this,"porta","", intcon, m_iocap, m_iocan, m_iocaf, 8,0x3f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false, 0x37);
  m_lata  = new PicLatchRegister(this,"lata","",m_porta, 0x37);
  m_wpua = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x3f);
  m_daccon0 = new DACCON0(this, "daccon0", "DAC1 8bit Voltage reference register 0", 0xb4, 32);
  m_daccon1 = new DACCON1(this, "daccon1", "DAC1 8bit Voltage reference register 1", 0xff, m_daccon0);
  m_cpu_temp = new CPU_Temp("cpu_temperature", 30., "CPU die temperature");


  tmr0.set_cpu(this, m_porta, 4, &option_reg);
  tmr0.start(0);
  tmr0.set_t1gcon(&t1con_g.t1gcon);
  set_mclr_pin(4);

  ((INTCON_14_PIR *)intcon)->write_mask = 0xfe;


  pir1 = new PIR1v1822(this,"pir1","Peripheral Interrupt Register",intcon, &pie1);
  pir2 = new PIR2v1822(this,"pir2","Peripheral Interrupt Register",intcon, &pie2);
  pir3 = new PIR3v178x(this,"pir3","Peripheral Interrupt Register",intcon, &pie3);

  pir1->valid_bits = pir1->writable_bits = 0xcb;
  pir2->valid_bits = pir2->writable_bits = 0x6c;
  pir3->valid_bits = pir3->writable_bits = 0x03;

  comparator.cmxcon0[0] = new CMxCON0(this, "cm1con0", " Comparator C1 Control Register 0", 0, &comparator);
  comparator.cmxcon1[0] = new CMxCON1(this, "cm1con1", " Comparator C1 Control Register 1", 0, &comparator);
  comparator.cmout = new CMOUT(this, "cmout", "Comparator Output Register");
  comparator.cmxcon0[1] = new CMxCON0(this, "cm2con0", " Comparator C2 Control Register 0", 1, &comparator);
  comparator.cmxcon1[1] = new CMxCON1(this, "cm2con1", " Comparator C2 Control Register 1", 1, &comparator);
}

P16F1503::~P16F1503()
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
    delete_sfr_register(m_portc);
    delete_sfr_register(m_trisc);
    delete_sfr_register(m_latc);

    remove_sfr_register(&clcdata);
    remove_sfr_register(&clc1.clcxcon);
    remove_sfr_register(&clc1.clcxpol);
    remove_sfr_register(&clc1.clcxsel0);
    remove_sfr_register(&clc1.clcxsel1);
    remove_sfr_register(&clc1.clcxgls0);
    remove_sfr_register(&clc1.clcxgls1);
    remove_sfr_register(&clc1.clcxgls2);
    remove_sfr_register(&clc1.clcxgls3);
    remove_sfr_register(&clc2.clcxcon);
    remove_sfr_register(&clc2.clcxpol);
    remove_sfr_register(&clc2.clcxsel0);
    remove_sfr_register(&clc2.clcxsel1);
    remove_sfr_register(&clc2.clcxgls0);
    remove_sfr_register(&clc2.clcxgls1);
    remove_sfr_register(&clc2.clcxgls2);
    remove_sfr_register(&clc2.clcxgls3);
    remove_sfr_register(&tmr0);

    remove_sfr_register(&tmr1l);
    remove_sfr_register(&tmr1h);
    remove_sfr_register(&t1con_g);
    remove_sfr_register(&t1con_g.t1gcon);

    remove_sfr_register(&tmr2);
    remove_sfr_register(&pr2);
    remove_sfr_register(&t2con);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(ssp.sspmsk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&pwm1dcl);
    remove_sfr_register(&pwm1dch);
    remove_sfr_register(&pwm2con);
    remove_sfr_register(&pwm2dcl);
    remove_sfr_register(&pwm2dch);
    remove_sfr_register(&pwm3con);
    remove_sfr_register(&pwm3dcl);
    remove_sfr_register(&pwm3dch);
    remove_sfr_register(&pwm4con);
    remove_sfr_register(&pwm4dcl);
    remove_sfr_register(&pwm4dch);
// RRR   remove_sfr_register(&pstr1con);
    remove_sfr_register(&pie1);
    remove_sfr_register(&pie2);
    remove_sfr_register(&pie3);
    remove_sfr_register(&adresl);
    remove_sfr_register(&adresh);
    remove_sfr_register(&adcon0);
    remove_sfr_register(&adcon1);
    remove_sfr_register(&adcon2);
    remove_sfr_register(&borcon);
    remove_sfr_register(&fvrcon);
    remove_sfr_register(&apfcon1);
    remove_sfr_register(&ansela);
    remove_sfr_register(&anselc);
    remove_sfr_register(&vregcon);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(ssp.sspmsk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&nco.nco1accl);
    remove_sfr_register(&nco.nco1acch);
    remove_sfr_register(&nco.nco1accu);
    remove_sfr_register(&nco.nco1incl);
    remove_sfr_register(&nco.nco1inch);
    remove_sfr_register(&nco.nco1con);
    remove_sfr_register(&nco.nco1clk);
    remove_sfr_register(&cwg.cwg1con0);
    remove_sfr_register(&cwg.cwg1con1);
    remove_sfr_register(&cwg.cwg1con2);
    remove_sfr_register(&cwg.cwg1dbr);
    remove_sfr_register(&cwg.cwg1dbf);


//RRR    remove_sfr_register(&pstr1con);
    remove_sfr_register(&option_reg);
    remove_sfr_register(osccon);
    remove_sfr_register(&oscstat);

    remove_sfr_register(comparator.cmxcon0[0]);
    remove_sfr_register(comparator.cmxcon1[0]);
    remove_sfr_register(comparator.cmout);
    remove_sfr_register(comparator.cmxcon0[1]);
    remove_sfr_register(comparator.cmxcon1[1]);
    delete_sfr_register(pir1);
    delete_sfr_register(pir2);
    delete_sfr_register(pir3);
    delete e;
    delete m_cpu_temp;
}

void P16F1503::create_iopin_map()
{
  package = new Package(14);
  if(!package)
    return;

  //createMCLRPin(1);
  // Now Create the package and place the I/O pins
  package->assign_pin(1, 0);        //Vdd
  package->assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
  package->assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  package->assign_pin(5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
  package->assign_pin(6, m_portc->addPin(new IO_bi_directional_pu("portc4"),4));
  package->assign_pin(7, m_portc->addPin(new IO_bi_directional_pu("portc3"),3));

  package->assign_pin(8, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
  package->assign_pin(9, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
  package->assign_pin(10, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));

  package->assign_pin(11, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(12, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(13, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  package->assign_pin(14, 0);        // Vss
}

void P16F1503::create_sfr_map()
{
    pir_set_2_def.set_pir1(pir1);
    pir_set_2_def.set_pir2(pir2);
    pir_set_2_def.set_pir3(pir3);


    add_file_registers(0x20, 0x7f, 0x00);
    add_file_registers(0xa0, 0xbf, 0x00);

    add_sfr_register(m_porta, 0x0c);
    add_sfr_register(m_portc, 0x0e);
    add_sfr_registerR(pir1,    0x11, RegisterValue(0,0),"pir1");
    add_sfr_registerR(pir2,    0x12, RegisterValue(0,0),"pir2");
    add_sfr_registerR(pir3,    0x13, RegisterValue(0,0),"pir3");
    add_sfr_register(&tmr0,   0x15);

    add_sfr_register(&tmr1l,  0x16, RegisterValue(0,0),"tmr1l");
    add_sfr_register(&tmr1h,  0x17, RegisterValue(0,0),"tmr1h");
    add_sfr_register(&t1con_g,  0x18, RegisterValue(0,0));
    add_sfr_register(&t1con_g.t1gcon, 0x19, RegisterValue(0,0));

    add_sfr_registerR(&tmr2,   0x1a, RegisterValue(0,0));
    add_sfr_registerR(&pr2,    0x1b, RegisterValue(0,0));
    add_sfr_registerR(&t2con,  0x1c, RegisterValue(0,0));


    add_sfr_register(m_trisa, 0x8c, RegisterValue(0x3f,0));
    add_sfr_register(m_trisc, 0x8e, RegisterValue(0x3f,0));

    pcon.valid_bits = 0xcf;
    add_sfr_register(&option_reg, 0x95, RegisterValue(0xff,0));
    add_sfr_registerR(osccon,     0x99, RegisterValue(0x38,0));
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

    add_sfr_registerR(&pie1,   0x91, RegisterValue(0,0));
    add_sfr_registerR(&pie2,   0x92, RegisterValue(0,0));
    add_sfr_registerR(&pie3,   0x93, RegisterValue(0,0));
    add_sfr_register(&adresl, 0x9b);
    add_sfr_register(&adresh, 0x9c);
    add_sfr_registerR(&adcon0, 0x9d, RegisterValue(0x00,0));
    add_sfr_registerR(&adcon1, 0x9e, RegisterValue(0x00,0));
    add_sfr_registerR(&adcon2, 0x9f, RegisterValue(0x00,0));

    add_sfr_register(m_lata,    0x10c);
    add_sfr_register(m_latc, 0x10e);
    add_sfr_registerR(comparator.cmxcon0[0], 0x111, RegisterValue(0x04,0));
    add_sfr_registerR(comparator.cmxcon1[0], 0x112, RegisterValue(0x00,0));
    add_sfr_registerR(comparator.cmxcon0[1], 0x113, RegisterValue(0x04,0));
    add_sfr_registerR(comparator.cmxcon1[1], 0x114, RegisterValue(0x00,0));
    add_sfr_registerR(comparator.cmout,      0x115, RegisterValue(0x00,0));
    add_sfr_register(&borcon,   0x116, RegisterValue(0x80,0));
    add_sfr_register(&fvrcon,   0x117, RegisterValue(0x00,0));
    add_sfr_registerR(m_daccon0, 0x118, RegisterValue(0x00,0));
    add_sfr_registerR(m_daccon1, 0x119, RegisterValue(0x00,0));
    add_sfr_registerR(&apfcon1 ,  0x11d, RegisterValue(0x00,0));
    add_sfr_registerR(&ansela,   0x18c, RegisterValue(0x17,0));
    add_sfr_registerR(&anselc,   0x18e, RegisterValue(0x0f,0));
    get_eeprom()->get_reg_eedata()->new_name("pmdatl");
    get_eeprom()->get_reg_eedatah()->new_name("pmdath");
    add_sfr_registerR(get_eeprom()->get_reg_eeadr(), 0x191, RegisterValue(0,0), "pmadrl");
    add_sfr_registerR(get_eeprom()->get_reg_eeadrh(), 0x192, RegisterValue(0,0), "pmadrh");
    add_sfr_register(get_eeprom()->get_reg_eedata(),  0x193);
    add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x194);
    get_eeprom()->get_reg_eecon1()->set_always_on(1<<7);
    add_sfr_registerR(get_eeprom()->get_reg_eecon1(),  0x195, RegisterValue(0x80,0), "pmcon1");
    add_sfr_registerR(get_eeprom()->get_reg_eecon2(),  0x196, RegisterValue(0,0), "pmcon2");
    add_sfr_registerR(&vregcon, 0x197, RegisterValue(1,0));

    add_sfr_register(m_wpua,     0x20c, RegisterValue(0xff,0),"wpua");
  
    add_sfr_registerR(&ssp.sspbuf,  0x211, RegisterValue(0,0),"ssp1buf");
    add_sfr_registerR(&ssp.sspadd,  0x212, RegisterValue(0,0),"ssp1add");
    add_sfr_registerR(ssp.sspmsk, 0x213, RegisterValue(0xff,0),"ssp1msk");
    add_sfr_registerR(&ssp.sspstat, 0x214, RegisterValue(0,0),"ssp1stat");
    add_sfr_registerR(&ssp.sspcon,  0x215, RegisterValue(0,0),"ssp1con");
    add_sfr_registerR(&ssp.sspcon2, 0x216, RegisterValue(0,0),"ssp1con2");
    add_sfr_registerR(&ssp.ssp1con3, 0x217, RegisterValue(0,0),"ssp1con3");

//  add_sfr_register(&pstr1con,    0x296, RegisterValue(1,0));

  add_sfr_registerR(m_iocap, 0x391, RegisterValue(0,0),"iocap");
  add_sfr_registerR(m_iocan, 0x392, RegisterValue(0,0),"iocan");
  add_sfr_registerR(m_iocaf, 0x393, RegisterValue(0,0),"iocaf");
  m_iocaf->set_intcon(intcon);

  add_sfr_registerR(&nco.nco1accl, 0x498, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1acch, 0x499, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1accu, 0x49a, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1incl, 0x49b, RegisterValue(1,0));
  add_sfr_registerR(&nco.nco1inch, 0x49c, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1con,  0x49e, RegisterValue(0,0));
  add_sfr_registerR(&nco.nco1clk,  0x49f, RegisterValue(0,0));

  nco.setIOpins(&(*m_porta)[5], &(*m_portc)[1]);
  nco.m_NCOif = new InterruptSource(pir2, 4);
  nco.set_clc(&clc1, 0);
  nco.set_clc(&clc2, 1);
  nco.set_cwg(&cwg);


  add_sfr_registerR(&pwm1dcl,  0x611, RegisterValue(0,0));
  add_sfr_register(&pwm1dch,  0x612, RegisterValue(0,0));
  add_sfr_registerR(&pwm1con,  0x613, RegisterValue(0,0));
  add_sfr_registerR(&pwm2dcl,  0x614, RegisterValue(0,0));
  add_sfr_register(&pwm2dch,  0x615, RegisterValue(0,0));
  add_sfr_registerR(&pwm2con,  0x616, RegisterValue(0,0));
  add_sfr_registerR(&pwm3dcl,  0x617, RegisterValue(0,0));
  add_sfr_register(&pwm3dch,  0x618, RegisterValue(0,0));
  add_sfr_registerR(&pwm3con,  0x619, RegisterValue(0,0));
  add_sfr_registerR(&pwm4dcl,  0x61a, RegisterValue(0,0));
  add_sfr_register(&pwm4dch,  0x61b, RegisterValue(0,0));
  add_sfr_registerR(&pwm4con,  0x61c, RegisterValue(0,0));

  add_sfr_registerR(&cwg.cwg1dbr, 0x691);
  add_sfr_register(&cwg.cwg1dbf, 0x692);
  add_sfr_registerR(&cwg.cwg1con0, 0x693, RegisterValue(0,0));
  add_sfr_registerR(&cwg.cwg1con1, 0x694);
  add_sfr_registerR(&cwg.cwg1con2, 0x695);

  add_sfr_registerR(&clcdata, 0xf0f, RegisterValue(0,0));
  add_sfr_registerR(&clc1.clcxcon, 0xf10, RegisterValue(0,0), "clc1con");
  add_sfr_register(&clc1.clcxpol, 0xf11, RegisterValue(0,0), "clc1pol");
  add_sfr_register(&clc1.clcxsel0, 0xf12, RegisterValue(0,0), "clc1sel0");
  add_sfr_register(&clc1.clcxsel1, 0xf13, RegisterValue(0,0), "clc1sel1");
  add_sfr_register(&clc1.clcxgls0, 0xf14, RegisterValue(0,0), "clc1gls0");
  add_sfr_register(&clc1.clcxgls1, 0xf15, RegisterValue(0,0), "clc1gls1");
  add_sfr_register(&clc1.clcxgls2, 0xf16, RegisterValue(0,0), "clc1gls2");
  add_sfr_register(&clc1.clcxgls3, 0xf17, RegisterValue(0,0), "clc1gls3");
  add_sfr_registerR(&clc2.clcxcon, 0xf18, RegisterValue(0,0), "clc2con");
  add_sfr_register(&clc2.clcxpol, 0xf19, RegisterValue(0,0), "clc2pol");
  add_sfr_register(&clc2.clcxsel0, 0xf1a, RegisterValue(0,0), "clc2sel0");
  add_sfr_register(&clc2.clcxsel1, 0xf1b, RegisterValue(0,0), "clc2sel1");
  add_sfr_register(&clc2.clcxgls0, 0xf1c, RegisterValue(0,0), "clc2gls0");
  add_sfr_register(&clc2.clcxgls1, 0xf1d, RegisterValue(0,0), "clc2gls1");
  add_sfr_register(&clc2.clcxgls2, 0xf1e, RegisterValue(0,0), "clc2gls2");
  add_sfr_register(&clc2.clcxgls3, 0xf1f, RegisterValue(0,0), "clc2gls3");

  clc1.frc = &frc;
  clc2.frc = &frc;
  clc1.lfintosc = &lfintosc;
  clc2.lfintosc = &lfintosc;
  clc1.hfintosc = &hfintosc;
  clc2.hfintosc = &hfintosc;
  clc1.p_nco = &nco;
  clcdata.set_clc(&clc1, &clc2);
  frc.set_clc(&clc1, &clc2);
  lfintosc.set_clc(&clc1, &clc2);
  hfintosc.set_clc(&clc1, &clc2);
  tmr0.set_clc(&clc1, 0);
  tmr0.set_clc(&clc2, 1);
  t1con_g.tmrl->m_clc[0] = tmr2.m_clc[0] = &clc1;
  t1con_g.tmrl->m_clc[1] = tmr2.m_clc[1] = &clc2;
  comparator.m_clc[0] = &clc1;
  comparator.m_clc[1] = &clc2;

  clc1.set_clcPins(&(*m_porta)[3], &(*m_portc)[4], &(*m_porta)[2]);
  clc2.set_clcPins(&(*m_portc)[3], &(*m_porta)[5], &(*m_portc)[0]);
  clc1.setInterruptSource(new InterruptSource(pir3, 1));
  clc2.setInterruptSource(new InterruptSource(pir3, 2));

  tmr2.ssp_module[0] = &ssp;

    ssp.initialize(
        get_pir_set(),    // PIR
        &(*m_portc)[0],   // SCK
        &(*m_portc)[3],   // SS
        &(*m_portc)[2],   // SDO
        &(*m_portc)[1],    // SDI
          m_trisc,          // i2c tris port
        SSP_TYPE_MSSP1
    );
    apfcon1.set_ValidBits(0x3b);
    apfcon1.set_pins(0, &nco, NCO::NCOout_PIN, &(*m_portc)[1], &(*m_porta)[4]); //NCO
    apfcon1.set_pins(1, &clc1, CLC::CLCout_PIN, &(*m_porta)[2], &(*m_portc)[5]); //CLC
    apfcon1.set_pins(3, &t1con_g.t1gcon, 0, &(*m_porta)[4], &(*m_porta)[3]); //tmr1 gate
    apfcon1.set_pins(4, &ssp, SSP1_MODULE::SS_PIN, &(*m_portc)[3], &(*m_porta)[3]); //SSP SS
    apfcon1.set_pins(5, &ssp, SSP1_MODULE::SDO_PIN, &(*m_portc)[2], &(*m_porta)[4]); //SSP SDO
    
    if (pir1) 
    {
        pir1->set_intcon(intcon);
        pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    pie2.setPir(pir2);
    pie3.setPir(pir3);
    t2con.tmr2 = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &pwm1con );
    tmr2.add_ccp ( &pwm2con );
    tmr2.add_ccp ( &pwm3con );
    tmr2.add_ccp ( &pwm4con );

    pr2.tmr2    = &tmr2;

    pwm1con.set_pwmdc(&pwm1dcl, &pwm1dch);
    pwm1con.setIOPin1(&(*m_portc)[5]);
    pwm1con.set_tmr2(&tmr2);
    pwm1con.set_cwg(&cwg);
    pwm1con.set_clc(&clc1, 0);
    pwm1con.set_clc(&clc2, 1);
    pwm2con.set_pwmdc(&pwm2dcl, &pwm2dch);
    pwm2con.setIOPin1(&(*m_portc)[3]);
    pwm2con.set_tmr2(&tmr2);
    pwm2con.set_cwg(&cwg);
    pwm2con.set_clc(&clc1, 0);
    pwm2con.set_clc(&clc2, 1);
    pwm3con.set_pwmdc(&pwm3dcl, &pwm3dch);
    pwm3con.setIOPin1(&(*m_porta)[2]);
    pwm3con.set_tmr2(&tmr2);
    pwm3con.set_cwg(&cwg);
    pwm3con.set_clc(&clc1, 0);
    pwm3con.set_clc(&clc2, 1);
    pwm4con.set_pwmdc(&pwm4dcl, &pwm4dch);
    pwm4con.setIOPin1(&(*m_portc)[1]);
    pwm4con.set_tmr2(&tmr2);
    pwm4con.set_cwg(&cwg);
    pwm4con.set_clc(&clc1, 0);
    pwm4con.set_clc(&clc2, 1);
  
    cwg.set_IOpins(&(*m_portc)[5], &(*m_portc)[4], &(*m_porta)[2]);
  
    ansela.config(0x17, 0);
    ansela.setValidBits(0x17);
    ansela.setAdcon1(&adcon1);
  
    anselc.config(0x0f, 4);
    anselc.setValidBits(0x0f);
    anselc.setAdcon1(&adcon1);
    ansela.setAnsel(&anselc);
    anselc.setAnsel(&ansela);
  
    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    //  adcon0.setAdcon2(&adcon2);
    adcon0.setIntcon(intcon);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0x1f);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);
    adcon2.setAdcon0(&adcon0);
  
    tmr0.set_adcon2(&adcon2);
  
    adcon1.setAdcon0(&adcon0);
    adcon1.setNumberOfChannels(32); // not all channels are used
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);
    adcon1.setIOPin(4, &(*m_portc)[0]);
    adcon1.setIOPin(5, &(*m_portc)[1]);
    adcon1.setIOPin(6, &(*m_portc)[2]);
    adcon1.setIOPin(7, &(*m_portc)[3]);
    adcon1.setValidBits(0xf7);
    adcon1.setVrefHiConfiguration(0, 0);
  //RRR    adcon1.setVrefLoConfiguration(0, 2);
    adcon1.set_FVR_chan(0x1f);
  
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[0], &(*m_portc)[1],  &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[1]->set_INpinNeg(&(*m_porta)[0], &(*m_portc)[1],  &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[0]);
    comparator.cmxcon1[1]->set_INpinPos(&(*m_portc)[0]);
  
    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2]);
    comparator.cmxcon1[1]->set_OUTpin(&(*m_portc)[4]);
    comparator.cmxcon0[0]->setBitMask(0xbf);
    comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, (1<<5)));
    comparator.cmxcon0[1]->setBitMask(0xbf);
    comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, (1<<6)));
    comparator.cmxcon1[0]->setBitMask(0xff);
    comparator.cmxcon1[1]->setBitMask(0xff);

    comparator.assign_pir_set(get_pir_set());
    comparator.assign_t1gcon(&t1con_g.t1gcon);
    fvrcon.set_adcon1(&adcon1);
    fvrcon.set_daccon0(m_daccon0);
    fvrcon.set_cmModule(&comparator);
    fvrcon.set_VTemp_AD_chan(0x1d);
    fvrcon.set_FVRAD_AD_chan(0x1f);

    m_daccon0->set_adcon1(&adcon1);
    m_daccon0->set_cmModule(&comparator);
    m_daccon0->set_FVRCDA_AD_chan(0x1e);
    m_daccon0->setDACOUT(&(*m_porta)[0], &(*m_porta)[2]);



    osccon->set_osctune(&osctune);
    osccon->set_oscstat(&oscstat);
    osctune.set_osccon((OSCCON *)osccon);
    osccon->write_mask = 0xfb;
}

void P16F1503::set_out_of_range_pm(uint address, uint value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
      get_eeprom()->change_rom(address - 0x2100, value);
}

void  P16F1503::create(int ram_top, int dev_id)
{

  create_iopin_map();

  osccon = new OSCCON_2(this, "osccon", "Oscillator Control Register");

  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);
  e->initialize(0, 16, 16, 0x8000, true);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0x7f);


  pic_processor::create();

  P16F1503::create_sfr_map();
  _14bit_e_processor::create_sfr_map();
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(dev_id);
}

void P16F1503::enter_sleep()
{
    if (wdt_flag == 2)          // WDT is suspended during sleep
        wdt.initialize(false);
    else if (get_pir_set()->interrupt_status() )
    {
        pc->increment();
          return;
    }

    tmr1l.sleep();
    osccon->sleep();
    tmr0.sleep();
    nco.sleep(true);
    pic_processor::enter_sleep();
}

void P16F1503::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
        tmr1l.wake();
        osccon->wake();
        nco.sleep(false);
        _14bit_e_processor::exit_sleep();
    }
}

void P16F1503::option_new_bits_6_7(uint bits)
{
        Dprintf(("P16F1503::option_new_bits_6_7 bits=%x\n", bits));
    m_porta->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
    m_wpua->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
}

void P16F1503::oscillator_select(uint cfg_word1, bool clkout)
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

void P16F1503::program_memory_wp(uint mode)
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

Processor * P16F1503::construct(const char *name)
{
  P16F1503 *p = new P16F1503(name);

  p->create(2048, 0x2ce0);
  p->create_invalid_registers ();

  return p;
}

//========================================================================

P16LF1503::P16LF1503(const char *_name, const char *desc)
  : P16F1503(_name,desc)
{
}

Processor * P16LF1503::construct(const char *name)
{
  P16LF1503 *p = new P16LF1503(name);

  p->create(2048, 0x2da0);
  p->create_invalid_registers ();

  return p;
}

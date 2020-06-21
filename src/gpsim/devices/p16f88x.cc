/*
 *
   Copyright (C) 2010,2015 Roy R. Rankin

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
// p16f88x
//
//  This file supports:
//    PIC16F882
//    PIC16F883
//    PIC16F884
//    PIC16F885
//    PIC16F886
//    PIC16F887
//

#include <stdio.h>
#include <iostream>
#include <string>

//#include "config.h"

#include "stimuli.h"

#include "p16f88x.h"
#include "pic-ioports.h"
#include "packages.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__, __FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//========================================================================
//
// Configuration Memory for the 16F8X devices.

class Config188x : public ConfigWord
{
public:
  Config188x(P16F88x *pCpu)
    : ConfigWord("CONFIG188x", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
  }
  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    FOSC2  = 1<<2,
    WDTEN  = 1<<3,
    PWRTEN = 1<<4,
    MCLRE  = 1<<5,

    BOREN  = 1<<8,
    BOREN1  = 1<<9,
    LVP    = 1<<12,

    CPD    = 1<<8,
    WRT0   = 1<<9,
    WRT1   = 1<<10,
    NOT_DEBUG  = 1<<11,
  };

  virtual void set(int64_t v)
  {
    Integer::set(v);
    Dprintf(("Config188x set %x\n", (int)v));
    if (m_pCpu) 
    {
        m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);
    }
  }
};


//========================================================================

P16F88x::P16F88x(const char *_name, const char *desc)
  : _14bit_processor(_name,desc),
    intcon_reg(this,"intcon","Interrupt Control"),
    t1con(this, "t1con", "TMR1 Control"),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    pie2(this,"PIE2", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    ccp2con(this, "ccp2con", "Capture Compare Control"),
    ccpr2l(this, "ccpr2l", "Capture Compare 2 Low"),
    ccpr2h(this, "ccpr2h", "Capture Compare 2 High"),
    pcon(this, "pcon", "pcon"),
    ssp(this),
    osccon(0),
    osctune(this, "osctune", "OSC Tune"),
    wdtcon(this, "wdtcon", "WDT Control", 1),
    usart(this),
    comparator(this),
    vrcon(this, "vrcon", "Voltage Reference Control Register"),
    srcon(this, "srcon", "SR Latch Control Resgister"),
    ansel(this,"ansel", "Analog Select"),
    anselh(this,"anselh", "Analog Select high"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    eccpas(this, "eccpas", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register"),
    pstrcon(this, "pstrcon", "Pulse Sterring Control Register"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low")
{

  m_porta = new PicPortRegister(this,"porta","", 8,0x1f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);
  m_ioc = new IOC(this, "iocb", "Interrupt-On-Change B Register");
  m_portb = new PicPortGRegister(this,"portb","", &intcon_reg, m_ioc,8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);
  m_portc = new PicPortRegister(this,"portc","",8,0xff);
  m_trisc = new PicTrisRegister(this,"trisc","",m_portc, false);
  m_porte = new PicPortRegister(this,"porte","",8,0x0f);
  m_trise =  new PicPSP_TrisRegister(this,"trise","",m_porte, false);

  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v3(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);
  pir1 = pir1_2_reg;
  pir2 = pir2_2_reg;
  m_wpu = new WPU(this, "wpub", "Weak Pull-up Register", m_portb, 0xff);

  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);
  comparator.cmxcon0[0] = new CMxCON0_V2(this, "cm1con0", 
        " Comparator C1 Control Register 0", 0, &comparator);
  comparator.cmxcon0[1] = new CMxCON0_V2(this, "cm2con0", 
        " Comparator C2 Control Register 0", 1, &comparator);
  comparator.cmxcon1[0] = new CM2CON1_V3(this, "cm2con1", 
        " Comparator C1 Control Register 1", 0, &comparator);
  comparator.cmxcon1[1] = comparator.cmxcon1[0];
}

P16F88x::~P16F88x()
{
  unassignMCLRPin();
  delete_file_registers(0x20, 0x7f);
  delete_file_registers(0xa0, 0xbf);

  remove_sfr_register(&tmr0);
  remove_sfr_register(&intcon_reg);
  remove_sfr_register(&pie2);
  remove_sfr_register(&pie1);
  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);
  remove_sfr_register(&pcon);
  remove_sfr_register(&t1con);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  delete get_eeprom();

  remove_sfr_register(&intcon_reg);
  remove_sfr_register(osccon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  remove_sfr_register(&usart.spbrgh);
  remove_sfr_register(&usart.baudcon);
  remove_sfr_register(&vrcon);
  remove_sfr_register(&srcon);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(&ccpr2l);
  remove_sfr_register(&ccpr2h);
  remove_sfr_register(&ccp2con);
  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);
  remove_sfr_register(&ansel);
  remove_sfr_register(&anselh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&ccpr2l);
  remove_sfr_register(&ccpr2h);
  remove_sfr_register(&ccp2con);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&pstrcon);
  remove_sfr_register(&eccpas);
  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(&ssp.sspbuf);
  remove_sfr_register(&ssp.sspcon);
  remove_sfr_register(&ssp.sspadd);
  remove_sfr_register(&ssp.sspstat);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
  remove_sfr_register(comparator.cmxcon0[0]);
  remove_sfr_register(comparator.cmxcon0[1]);
  remove_sfr_register(comparator.cmxcon1[1]);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_portb);
  delete_sfr_register(m_trisb);
  delete_sfr_register(m_porte);
  delete_sfr_register(m_trise);
  delete_sfr_register(m_portc);
  delete_sfr_register(m_trisc);

  delete_sfr_register(pir1);
  delete_sfr_register(pir2);
  delete_sfr_register(m_wpu);
  delete_sfr_register(m_ioc);
}

void P16F88x::create_iopin_map()
{
  fprintf(stderr, "%s should be defined at a higer level\n", __FUNCTION__);
}

void P16F88x::create_sfr_map()
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

  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));
  //alias_file_registers(0x0a,0x0b,0x80); //Already donw

  intcon = &intcon_reg;

  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);
 
  add_sfr_register(m_porte, 0x09);
  add_sfr_register(m_trise, 0x89, RegisterValue(0xff,0));
  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc, 0x87, RegisterValue(0xff,0));

  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xbf, 0);

  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  add_sfr_register(get_pir2(),   0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));
                                                                                
  pir_set_2_def.set_pir2(pir2);
                                                                                
  pie2.setPir(get_pir2());
  alias_file_registers(0x00,0x04,0x100);
  alias_file_registers(0x80,0x84,0x100);
  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x86,0x86,0x100);

  add_sfr_register(pir1,   0x0c, RegisterValue(0,0),"pir1");
  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));

  add_sfr_register(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

  add_sfr_register(&pcon,   0x8e, RegisterValue(0,0),"pcon");

  add_sfr_register(&t1con,  0x10, RegisterValue(0,0));
  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));

  get_eeprom()->get_reg_eedata()->new_name("eedat");
  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x10f);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  alias_file_registers(0x0a,0x0b,0x080);
  alias_file_registers(0x0a,0x0b,0x100);
  alias_file_registers(0x0a,0x0b,0x180);

  intcon_reg.set_pir_set(get_pir_set());

  add_sfr_register(osccon, 0x8f, RegisterValue(0x60,0),"osccon");
  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");

  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);

  usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(&usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.baudcon,  0x187,RegisterValue(0x40,0),"baudctl");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
  usart.set_eusart(true);
  comparator.assign_tmr1l(&tmr1l);
  comparator.cmxcon1[1]->set_vrcon(&vrcon);

  add_sfr_register(comparator.cmxcon0[0], 0x107, RegisterValue(0,0), "cm1con0");
  add_sfr_register(comparator.cmxcon0[1], 0x108, RegisterValue(0,0), "cm2con0");
  add_sfr_register(comparator.cmxcon1[1], 0x109, RegisterValue(2,0), "cm2con1");
  add_sfr_register(&vrcon, 0x97, RegisterValue(0,0),"vrcon");
  add_sfr_register(&srcon, 0x185, RegisterValue(0,0),"srcon");
  add_sfr_register(&wdtcon, 0x105, RegisterValue(0x08,0),"wdtcon");
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));
  add_sfr_register(&ansel, 0x188, RegisterValue(0xff,0));
  add_sfr_register(&anselh, 0x189, RegisterValue(0xff,0));
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));
  add_sfr_register(m_wpu, 0x95, RegisterValue(0xff,0));
  add_sfr_register(m_ioc, 0x96, RegisterValue(0,0));
                                                                                
  ansel.setAdcon1(&adcon1);
  ansel.setAnselh(&anselh);
  anselh.setAdcon1(&adcon1);
  anselh.setAnsel(&ansel);
  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(0xf);
  adcon0.setChannel_shift(2);
  adcon0.setGo(1);
                               
  adcon1.setValidBits(0xb0);
  adcon1.setNumberOfChannels(14);
  adcon1.setValidCfgBits(ADCON1::VCFG0 | ADCON1::VCFG1 , 4);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[4]);
  adcon1.setIOPin(8, &(*m_portb)[2]);
  adcon1.setIOPin(9, &(*m_portb)[3]);
  adcon1.setIOPin(10, &(*m_portb)[1]);
  adcon1.setIOPin(11, &(*m_portb)[4]);
  adcon1.setIOPin(12, &(*m_portb)[0]);
  adcon1.setIOPin(13, &(*m_portb)[5]);

  // set a2d modes where an3 is Vref+ 
  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);

  // set a2d modes where an2 is Vref-
  adcon1.setVrefLoConfiguration(2, 2);
  adcon1.setVrefLoConfiguration(3, 2);

  vrcon.setValidBits(0xff); // All bits settable

  add_sfr_register(&ccpr1l, 0x15, RegisterValue(0,0));
  add_sfr_register(&ccpr1h, 0x16, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x17, RegisterValue(0,0));
  add_sfr_register(&ccpr2l, 0x1b, RegisterValue(0,0));
  add_sfr_register(&ccpr2h, 0x1c, RegisterValue(0,0));
  add_sfr_register(&ccp2con, 0x1d, RegisterValue(0,0));
  add_sfr_register(&pwm1con, 0x9b, RegisterValue(0,0));
  add_sfr_register(&pstrcon, 0x9d, RegisterValue(1,0));
  add_sfr_register(&eccpas, 0x9c, RegisterValue(0,0));
  eccpas.setIOpin(0, 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  ssp.sspmsk = new _SSPMSK(this, "ssp1msk");
  add_sfr_register(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
  add_sfr_register(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
  add_sfr_register(&ssp.sspcon2,  0x91, RegisterValue(0,0),"sspcon2");
  add_sfr_register(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
  add_sfr_register(ssp.sspmsk,  0x93, RegisterValue(0xff,0), "sspmsk", false);
  add_sfr_register(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
  tmr2.ssp_module[0] = &ssp;

  ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],    // SDI
                m_trisc,          // i2c tris port
                SSP_TYPE_SSP
        );
  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  tmr1h.tmrl  = &tmr1l;

  t1con.tmrl  = &tmr1l;

  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &ccp1con );
  tmr2.add_ccp ( &ccp2con );
  pr2.tmr2    = &tmr2;

  tmr1l.setIOpin(&(*m_portc)[0]);
  ccp1con.setBitMask(0xff);
  ccp1con.pstrcon = &pstrcon;
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  ccp2con.setIOpin(&(*m_portc)[1]);
  ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v3::CCP2IF, &tmr2);
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.setPir(pir1);

  comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[4], &(*m_porta)[5]);
  comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[0], &(*m_porta)[1], 
                                        &(*m_portb)[3],&(*m_portb)[1]);
  comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[3], &(*m_porta)[2]);
  comparator.cmxcon1[0]->setBitMask(0x33);
  comparator.cmxcon0[0]->setBitMask(0xb7);
  comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, PIR2v2::C1IF));
  comparator.cmxcon0[1]->setBitMask(0xb7);
  comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, PIR2v2::C2IF));
}

void P16F88x::option_new_bits_6_7(uint bits)
{
        Dprintf(("P18F88x::option_new_bits_6_7 bits=%x\n", bits));
        m_portb->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
        m_wpu->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
}

void P16F88x::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

bool P16F88x::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<4,
    CFG_MCLRE = 1<<5,
    CFG_CCPMX = 1<<12
  };
  // Let the base class do most of the work:
  if (address == 0x2007)
  {
    pic_processor::set_config_word(address, cfg_word);

    uint valid_pins = m_porta->getEnableMask();

    set_int_osc(false);
    // Careful these bits not adjacent
    switch(cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2)) {

    case 0:  // LP oscillator: low power crystal is on RA6 and RA7
    case 1:     // XT oscillator: crystal/resonator is on RA6 and RA7
    case 2:     // HS oscillator: crystal/resonator is on RA6 and RA7
        break;

    case 0x13:  // ER oscillator: RA6 is CLKOUT, resistor (?) on RA7 
        break;

    case 3:     // EC:  RA6 is an I/O, RA7 is a CLKIN
    case 0x12:  // ER oscillator: RA6 is an I/O, RA7 is a CLKIN
        valid_pins =  (valid_pins & 0x7f)|0x40;
        break;

    case 0x10:  // INTRC: Internal Oscillator, RA6 and RA7 are I/O's
        set_int_osc(true);
        valid_pins |= 0xc0;
        break;

    case 0x11:  // INTRC: Internal Oscillator, RA7 is an I/O, RA6 is CLKOUT
        set_int_osc(true);
        valid_pins = (valid_pins & 0xbf)|0x80;
        break;

    }
    // If the /MCLRE bit is set then RE3 is the MCLR pin, otherwise it's 
    // a general purpose I/O pin.

    if ((cfg_word & CFG_MCLRE)) 
    {
        assignMCLRPin(1);
    }
    else
    {
        unassignMCLRPin();
    }

    if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
    {
        m_porta->setEnableMask(valid_pins);
        m_porta->setTris(m_trisa);
    }
    return true;
  }
  else if (address == 0x2008 )
  {
    //cout << "p16f88x 0x" << hex << address << " config word2 0x" << cfg_word << '\n';
  }
  return false;
}

void P16F88x::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,2);
  m_configMemory->addConfigWord(0,new Config188x(this));
  m_configMemory->addConfigWord(1,new ConfigWord("CONFIG2", 0,"Configuration Word",this,0x2008));
  wdt.initialize(true); // default WDT enabled
  wdt.set_timeout(0.000035);
  set_config_word(0x2007, 0x3fff);

}

void  P16F88x::create(int eesize)
{
  create_iopin_map();

  _14bit_processor::create();
  osccon = new OSCCON(this, "osccon", "OSC Control");

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eesize);
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F88x::create_sfr_map();
}

//========================================================================
//
Processor * P16F882::construct(const char *name)
{
  P16F882 *p = new P16F882(name);

  p->P16F88x::create(128);
  p->P16F882::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F882::P16F882(const char *_name, const char *desc)
  : P16F88x(_name,desc)
{
  m_porta->setEnableMask(0xff);
}

void P16F882::create_iopin_map(void)
{
  package = new Package(28);
  if(!package) return;

  // Now Create the package and place the I/O pins
  package->assign_pin(1, m_porte->addPin(new IO_bi_directional("porte3"),3));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));
  package->assign_pin(8, 0);
  package->assign_pin( 9, m_porta->addPin(new IO_bi_directional("porta7"),7));
  package->assign_pin( 10, m_porta->addPin(new IO_bi_directional("porta6"),6));
  package->assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));
  package->assign_pin(19, 0);
  package->assign_pin(20, 0);
  package->assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

}

void P16F882::create_sfr_map()
{
  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F883 
//
Processor * P16F883::construct(const char *name)
{
  P16F883 *p = new P16F883(name);

  p->P16F88x::create(256);
  p->P16F883::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F883::P16F883(const char *_name, const char *desc)
  : P16F882(_name,desc)
{
  m_porta->setEnableMask(0xff);
}

P16F883::~P16F883()
{
  delete_file_registers(0xc0,0xef);
  delete_file_registers(0x120,0x16f);
}

void P16F883::create_sfr_map()
{
  add_file_registers(0xc0,0xef,0);
  add_file_registers(0x120,0x16f,0);
  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F886 
//

Processor * P16F886::construct(const char *name)
{
  P16F886 *p = new P16F886(name);

  p->P16F88x::create(256);
  p->P16F886::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F886::P16F886(const char *_name, const char *desc)
  : P16F882(_name,desc)
{
  m_porta->setEnableMask(0xff);
}

P16F886::~P16F886()
{
  delete_file_registers(0xc0,0xef);
  delete_file_registers(0x120,0x16f);
  delete_file_registers(0x190,0x1ef);
}

void P16F886::create_sfr_map()
{
  add_file_registers(0xc0,0xef,0);
  add_file_registers(0x120,0x16f,0);
  add_file_registers(0x190,0x1ef,0);
  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);
}
//========================================================================
//
// Pic 16F887 
//

Processor * P16F887::construct(const char *name)
{
  P16F887 *p = new P16F887(name);

  p->P16F88x::create(256);
  p->P16F887::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F887::P16F887(const char *_name, const char *desc)
  : P16F884(_name,desc)
{
}

P16F887::~P16F887()
{
  delete_file_registers(0x110,0x11f);
  delete_file_registers(0x190,0x1ef);
}

void P16F887::create_sfr_map()
{
  add_file_registers(0xc0,0xef,0);
  add_file_registers(0x110,0x16f,0);
  //add_file_registers(0x110,0x11f,0);
  add_file_registers(0x190,0x1ef,0);

  add_sfr_register(m_portd, 0x08);
  add_sfr_register(m_trisd, 0x88, RegisterValue(0xff,0));

  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portd)[5], &(*m_portd)[6], &(*m_portd)[7]);
  adcon1.setIOPin(5, &(*m_porte)[0]);
  adcon1.setIOPin(6, &(*m_porte)[1]);
  adcon1.setIOPin(7, &(*m_porte)[2]);
}

//========================================================================
//
Processor * P16F884::construct(const char *name)
{
  P16F884 *p = new P16F884(name);

  p->P16F88x::create(256);
  p->P16F884::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F884::P16F884(const char *_name, const char *desc)
  : P16F88x(_name,desc)
{
  m_porta->setEnableMask(0xff);

  // trisa5 is an input only pin
  m_trisa->setEnableMask(0xdf);

  m_portd = new PicPSP_PortRegister(this,"portd","",8,0xff);
  m_trisd = new PicTrisRegister(this,"trisd","",(PicPortRegister *)m_portd, false);
}

P16F884::~P16F884()
{
  delete_file_registers(0xc0,0xef);
  delete_file_registers(0x120,0x16f);

  delete_sfr_register(m_portd);
  delete_sfr_register(m_trisd);
}

//------------------------------------------------------------------------
//
void P16F884::create_iopin_map(void)
{
  package = new Package(40);
  if(!package) return;

  // Now Create the package and place the I/O pins
  package->assign_pin(1, m_porte->addPin(new IO_bi_directional("porte3"),3));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));
  package->assign_pin( 8, m_porte->addPin(new IO_bi_directional("porte0"),0));
  package->assign_pin( 9, m_porte->addPin(new IO_bi_directional("porte1"),1));
  package->assign_pin(10, m_porte->addPin(new IO_bi_directional("porte2"),2));
  package->assign_pin(11, 0);
  package->assign_pin(12, 0);
  package->assign_pin( 13, m_porta->addPin(new IO_bi_directional("porta7"),7));
  package->assign_pin( 14, m_porta->addPin(new IO_bi_directional("porta6"),6));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(23, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(24, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(25, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(26, m_portc->addPin(new IO_bi_directional("portc7"),7));
  package->assign_pin(19, m_portd->addPin(new IO_bi_directional("portd0"),0));
  package->assign_pin(20, m_portd->addPin(new IO_bi_directional("portd1"),1));
  package->assign_pin(21, m_portd->addPin(new IO_bi_directional("portd2"),2));
  package->assign_pin(22, m_portd->addPin(new IO_bi_directional("portd3"),3));
  package->assign_pin(27, m_portd->addPin(new IO_bi_directional("portd4"),4));
  package->assign_pin(28, m_portd->addPin(new IO_bi_directional("portd5"),5));
  package->assign_pin(29, m_portd->addPin(new IO_bi_directional("portd6"),6));
  package->assign_pin(30, m_portd->addPin(new IO_bi_directional("portd7"),7));
  package->assign_pin(31, 0);
  package->assign_pin(32, 0);
  package->assign_pin(33, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(34, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(35, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(36, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(37, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(38, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(39, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(40, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));
}

void P16F884::create_sfr_map()
{
  add_file_registers(0xc0,0xef,0);
  add_file_registers(0x120,0x16f,0);

  add_sfr_register(m_portd, 0x08);
  add_sfr_register(m_trisd, 0x88, RegisterValue(0xff,0));

  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portd)[5], &(*m_portd)[6], &(*m_portd)[7]);
  adcon1.setIOPin(5, &(*m_porte)[0]);
  adcon1.setIOPin(6, &(*m_porte)[1]);
  adcon1.setIOPin(7, &(*m_porte)[2]);
}
//------------------------------------------------------------------------
//
//

class ConfigF631 : public ConfigWord
{
public:
  ConfigF631(P16F631 *pCpu)
    : ConfigWord("CONFIG", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
    Dprintf(("ConfigF631::ConfigF631 %p\n", m_pCpu));
  }

    enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    FOSC2  = 1<<2,
    WDTEN  = 1<<3,
    PWRTEN = 1<<4,
    MCLRE =  1<<5,
    BODEN =  1<<6,
    CP =     1<<7,
    CPD =    1<<8
  };

 string toString()
  {
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[356];

    const char *OSCdesc[8] = {
      "LP oscillator",
      "XT oscillator",
      "HS oscillator",
      "EC oscillator w/ OSC2 configured as I/O",
      "INTOSC oscillator: I/O on RA4 pin, I/O on RA5",
      "INTOSC oscillator: CLKOUT on RA4 pin, I/O on RA5",
      "RC oscillator: I/O on RA4 pin, RC on RA5",
      "RC oscillator: CLKOUT on RA4 pin, RC on RA5"
    };
    snprintf(buff,sizeof(buff),
        " $%04x\n"
        " FOSC=%d - Clk source = %s\n"
        " WDTEN=%d - WDT is %s\n"
        " PWRTEN=%d - Power up timer is %s\n"
        " MCLRE=%d - RA3 Pin %s\n"
        " BODEN=%d -  Brown-out Detect %s\n"
        " CP=%d - Code Protection %s\n"
        " CPD=%d -  Data Code Protection %s\n",
             i,
             i&(FOSC0|FOSC1|FOSC2), OSCdesc[i&(FOSC0|FOSC1|FOSC2)],
             ((i&WDTE) ? 1 : 0), ((i&WDTE) ? "enabled" : "disabled"),
             ((i&PWRTEN) ? 1 : 0), ((i&PWRTEN) ? "disabled" : "enabled"),
             ((i&MCLRE) ? 1 : 0), ((i&MCLRE) ? "MCLR" : "Input"),
             ((i&BODEN) ? 1 : 0), ((i&BODEN) ? "enabled" : "disabled"),
             ((i&CP) ? 1 : 0), ((i&CP) ? "disabled" : "enabled"),
             ((i&CPD) ? 1 : 0), ((i&CPD) ? "disabled" : "enabled")
    );
    return string(buff);
  }
};
P16F631::P16F631(const char *_name, const char *desc)
   : _14bit_processor(_name, desc),
    t1con(this, "t1con", "TMR1 Control"),
    pie1(this,"pie1", "Peripheral Interrupt Enable"),
    pie2(this,"pie2", "Peripheral Interrupt Enable"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    osctune(this, "osctune", "OSC Tune"),
    pcon(this, "pcon", "pcon"),
    wdtcon(this, "wdtcon", "WDT Control", 0x1f),
    osccon(0),
    vrcon(this, "vrcon", "Voltage Reference Control Register"),
    srcon(this, "srcon", "SR Latch Control Resgister"),
    ansel(this,"ansel", "Analog Select"),
    comparator(this),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),

     intcon_reg(this,"intcon","Interrupt Control")
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir1 = pir1_2_reg;
  pir2_3_reg = new PIR2v3(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);
  pir2 = pir2_3_reg;

  m_ioca = new IOC(this, "ioca", "Interrupt-On-Change GPIO Register");
  m_iocb = new IOC(this, "iocb", "Interrupt-On-Change GPIO Register");

  m_porta = new PicPortGRegister(this,"porta","",&intcon_reg, m_ioca, 8,0x3f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false, 0x37);

  m_portb = new PicPortGRegister(this,"portb","",&intcon_reg, m_iocb, 8,0xf0);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);

  m_wpua = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x37);
  m_wpub = new WPU(this, "wpub", "Weak Pull-up Register", m_portb, 0xf0);
  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);

  m_portc = new PicPortRegister(this,"portc","",8,0xff);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false);

  comparator.cmxcon0[0] = new CMxCON0_V2(this, "cm1con0", 
        " Comparator C1 Control Register 0", 0, &comparator);
  comparator.cmxcon0[1] = new CMxCON0_V2(this, "cm2con0", 
        " Comparator C2 Control Register 0", 1, &comparator);
  comparator.cmxcon1[0] = new CM2CON1_V4(this, "cm2con1", 
        " Comparator C1 Control Register 1", 0, &comparator);
  comparator.cmxcon1[1] = comparator.cmxcon1[0];
}

P16F631::~P16F631()
{
  unassignMCLRPin();
  delete_file_registers(0x40, 0x7f);
  remove_sfr_register(comparator.cmxcon0[0]);
  remove_sfr_register(comparator.cmxcon0[1]);
  remove_sfr_register(comparator.cmxcon1[1]);

  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(&tmr0);
  remove_sfr_register(&vrcon);
  remove_sfr_register(&ansel);
  remove_sfr_register(&srcon);
  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);
  remove_sfr_register(&t1con);
  remove_sfr_register(&pcon);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(osccon);
  remove_sfr_register(&pie1);
  remove_sfr_register(&pie2);
  remove_sfr_register(&intcon_reg);
  remove_sfr_register(&osctune);
  delete_sfr_register(pir2);
  delete_sfr_register(m_portc);
  delete_sfr_register(m_trisc);

  delete_sfr_register(m_portb);
  delete_sfr_register(m_trisb);
  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_ioca);
  delete_sfr_register(m_iocb);
  delete_sfr_register(m_wpua);
  delete_sfr_register(m_wpub);
  delete_sfr_register(pir1_2_reg);
  delete e;
}
void P16F631::create_iopin_map(void)
{
  package = new Package(20);
  if(!package) return;

  package->assign_pin(1, 0);        // Vdd
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin( 4, m_porta->addPin(new IOPIN("porta3"),3));
  package->assign_pin( 5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
  package->assign_pin( 6, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin( 7, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin( 8, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin( 9, m_portc->addPin(new IO_bi_directional("portc7"),7));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional("portb7"),7));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(14, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));
  package->assign_pin(17, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(18, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(19, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));

  package->assign_pin(20, 0); //VSS

  tmr1l.setIOpin(&(*m_portc)[0]);
}
Processor * P16F631::construct(const char *name)
{
  P16F631 *p = new P16F631(name);

  p->create(128);
  p->create_invalid_registers ();

  return p;
}

void P16F631::create(int eesize)
{
   create_iopin_map();

   _14bit_processor::create();
   osccon = new OSCCON(this, "osccon", "OSC Control");

   e = new EEPROM_WIDE(this,pir2);
   e->initialize(eesize);
   e->set_intcon(&intcon_reg);
   set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

   P16F631::create_sfr_map();
}

//-------------------------------------------------------------------
void P16F631::create_sfr_map()
{
  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);

  add_file_registers(0x40, 0x7f, 0);
  alias_file_registers(0x70, 0x7f, 0x80);
  alias_file_registers(0x70, 0x7f, 0x100);
  alias_file_registers(0x70, 0x7f, 0x180);

  add_sfr_register(indf,    0x00);
  alias_file_registers(0x00,0x00,0x80);
  alias_file_registers(0x00,0x00,0x100);
  alias_file_registers(0x00,0x00,0x180);

  add_sfr_register(&tmr0,   0x01);
  alias_file_registers(0x01,0x01,0x100);
  add_sfr_register(option_reg,  0x81, RegisterValue(0xff,0));
  alias_file_registers(0x81,0x81,0x100);

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);
  alias_file_registers(0x02,0x04,0x100);
  alias_file_registers(0x02,0x04,0x180);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_trisa, 0x85, RegisterValue(0x3f,0));

  add_sfr_register(m_portb, 0x06);
  add_sfr_register(m_trisb, 0x86, RegisterValue(0xf0,0));

  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc, 0x87, RegisterValue(0xff,0));
  alias_file_registers(0x05,0x07,0x100);
  alias_file_registers(0x85,0x87,0x100);

  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x00b, RegisterValue(0,0));

  alias_file_registers(0x0a,0x0b,0x80);
  alias_file_registers(0x0a,0x0b,0x100);
  alias_file_registers(0x0a,0x0b,0x180);
  add_sfr_register(pir1, 0x0c, RegisterValue(0,0));
  add_sfr_register(pir2, 0x0d, RegisterValue(0,0));
  add_sfr_register(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
  add_sfr_register(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
  add_sfr_register(&t1con, 0x10, RegisterValue(0,0));
  add_sfr_register(&pcon, 0x8e, RegisterValue(0,0));
  add_sfr_register(&wdtcon, 0x97, RegisterValue(0x08,0));
  add_sfr_register(osccon, 0x8f, RegisterValue(0x60,0));

  add_sfr_register(&vrcon, 0x118, RegisterValue(0,0),"vrcon");
  add_sfr_register(comparator.cmxcon0[0], 0x119, RegisterValue(0,0), "cm1con0");
  add_sfr_register(comparator.cmxcon0[1], 0x11a, RegisterValue(0,0), "cm2con0");
  add_sfr_register(comparator.cmxcon1[1], 0x11b, RegisterValue(2,0), "cm2con1");
  comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2], &(*m_portc)[4]);
  comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1], 
                                &(*m_portc)[2], &(*m_portc)[3]);
  comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[0], &(*m_portc)[0]);
  comparator.cmxcon1[0]->setBitMask(0x03);
  comparator.cmxcon0[0]->setBitMask(0xb7);
  comparator.cmxcon0[0]->setIntSrc(new InterruptSource(pir2, PIR2v2::C1IF));
  comparator.cmxcon0[1]->setBitMask(0xb7);
  comparator.cmxcon0[1]->setIntSrc(new InterruptSource(pir2, PIR2v2::C2IF));
  comparator.cmxcon1[0]->set_vrcon(&vrcon);
  comparator.cmxcon1[1] = comparator.cmxcon1[0];
  comparator.assign_tmr1l(&tmr1l);

  add_sfr_register(&ansel, 0x11e, RegisterValue(0xff,0));
  add_sfr_register(&srcon, 0x19e, RegisterValue(0,0),"srcon");

  ansel.setAdcon1(&adcon1);
  ansel.setValidBits(0xff);

  adcon1.setNumberOfChannels(12);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(4, &(*m_portc)[0]);

  adcon1.setIOPin(5, &(*m_portc)[1]);
  adcon1.setIOPin(6, &(*m_portc)[2]);
  adcon1.setIOPin(7, &(*m_portc)[3]);
  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  // FIXME -- can't delete this new'd item
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v3::TMR1IF));
  tmr1h.tmrl  = &tmr1l;
  t1con.tmrl  = &tmr1l;

  tmr1l.setIOpin(&(*m_porta)[5]);
  tmr1l.setGatepin(&(*m_porta)[4]);

  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));
  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.setPir(pir1);
  pie2.setPir(pir2);

  get_eeprom()->get_reg_eedata()->new_name("eedat");
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);
  add_sfr_register(m_wpua, 0x95, RegisterValue(0x37,0),"wpua");
  add_sfr_register(m_wpub, 0x115, RegisterValue(0xf0,0),"wpub");
  add_sfr_register(m_ioca, 0x96, RegisterValue(0,0),"ioca");
  add_sfr_register(m_iocb, 0x116, RegisterValue(0,0),"iocb");
  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");

  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);
}
//-------------------------------------------------------------------
void P16F631::option_new_bits_6_7(uint bits)
{
  m_wpua->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
  m_wpub->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
  m_porta->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}
//-------------------------------------------------------------------
void P16F631::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new ConfigF631(this));
  wdt.initialize(true); // default WDT enabled
  wdt.set_timeout(0.000035);
  set_config_word(0x2007, 0x3fff);

};

//-------------------------------------------------------------------
bool P16F631::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<2,
    CFG_WDTE  = 1<<3,
    CFG_MCLRE = 1<<5,
    CFG_IESO  = 1<<10,
  };

   if(address == config_word_address())
    {
       uint valid_pins = m_porta->getEnableMask();

        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE)
        {
            assignMCLRPin(4);  
        }
        else
        {
            unassignMCLRPin();
        }

        wdt.initialize((cfg_word & CFG_WDTE) == CFG_WDTE);

       set_int_osc(false);

        // AnalogReq is used so ADC does not change clock names
        // set_config_word is first called with default and then
        // often called a second time. the following call is to
        // reset porta so next call to AnalogReq sill set the pin name
        //
        (&(*m_porta)[4])->AnalogReq((Register *)this, false, "porta4");
        valid_pins |= 0x20;

        uint fosc = cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2);
        if (osccon)
        {
            osccon->set_config_xosc(fosc < 3);
            osccon->set_config_irc(fosc == 4 || fosc == 5);
            osccon->set_config_ieso(cfg_word & CFG_IESO);
        }

               switch(fosc)
        {
               case 0:  // LP oscillator: low power crystal is on RA4 and RA5
               case 1:     // XT oscillator: crystal/resonator is on RA4 and RA5
               case 2:     // HS oscillator: crystal/resonator is on RA4 and RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "OSC2");

            valid_pins &= 0xcf;
            break;

        case 3:        // EC I/O on RA4 pin, CLKIN on RA5
            valid_pins &= 0xef;
            break;
            
        case 5: // INTOSC CLKOUT on RA4 pin
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");
        case 4: // INTOSC
             set_int_osc(true);
            osccon->set_rc_frequency();
            break;

        case 6: //RC oscillator: I/O on RA4 pin, RC on RA5
            valid_pins &= 0xdf;
            break;

        case 7: // RC oscillator: CLKOUT on RA4 pin, RC on RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");

            valid_pins &= 0xdf;
            break;
        };

        if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
            {
            m_porta->setEnableMask(valid_pins);
            m_trisa->setEnableMask(valid_pins & 0xf7);
            }
        return(true);
    }
    return false;
}

//========================================================================
//
// Pic 16F684
//
P16F684::P16F684(const char *_name, const char *desc)
   : _14bit_processor(_name, desc),
    comparator(this),
    t1con(this, "t1con", "TMR1 Control"),
    t2con(this, "t2con", "TMR2 Control"),
    pie1(this,"pie1", "Peripheral Interrupt Enable"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    osctune(this, "osctune", "OSC Tune"),
    pcon(this, "pcon", "pcon"),
    wdtcon(this, "wdtcon", "WDT Control", 0x1f),
    osccon(0),
    ansel(this,"ansel", "Analog Select"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    eccpas(this, "eccpas", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register"),
    pstrcon(this, "pstrcon", "Pulse Sterring Control Register"),
    intcon_reg(this,"intcon","Interrupt Control")
{
  pir1_3_reg = new PIR1v3(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir1 = pir1_3_reg;
  pir1->valid_bits = pir1->writable_bits = 0xff;

  m_ioca = new IOC(this, "ioca", "Interrupt-On-Change GPIO Register");

  m_porta = new PicPortGRegister(this,"porta","",&intcon_reg, m_ioca, 8,0x3f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);

  m_wpua = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x37);
  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);

  m_portc = new PicPortRegister(this,"portc","",8,0x3f);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false);
}

P16F684::~P16F684()
{
  unassignMCLRPin();

  delete_file_registers(0x20, 0x7f);
  delete_file_registers(0xa0, 0xbf);

  remove_sfr_register(&tmr0);
  remove_sfr_register(&intcon_reg);
  remove_sfr_register(pir1);
  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);
  remove_sfr_register(&t1con);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&eccpas);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.cmcon1);
  remove_sfr_register(&adresh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&pie1);
  remove_sfr_register(&pcon);
  remove_sfr_register(osccon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&ansel);
  remove_sfr_register(&pr2);
  remove_sfr_register(&comparator.vrcon);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(&adresl);
  remove_sfr_register(&adcon1);

  delete_sfr_register(m_portc);
  delete_sfr_register(m_trisc);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_ioca);
  delete_sfr_register(m_wpua);
  delete_sfr_register(pir1_3_reg);
  delete e;
}
void P16F684::create_iopin_map(void)
{
  package = new Package(14);
  if(!package) return;

  package->assign_pin(1, 0);        // Vdd

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin( 4, m_porta->addPin(new IOPIN("porta3"),3));
  package->assign_pin( 5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));
  package->assign_pin( 6, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin( 7, m_portc->addPin(new IO_bi_directional("portc3"),3));

  package->assign_pin( 8, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin( 9, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(10, m_portc->addPin(new IO_bi_directional("portc0"),0));

  package->assign_pin(11, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(12, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(13, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));

  package->assign_pin(14, 0); //VSS

  tmr1l.setIOpin(&(*m_portc)[0]);
}
Processor * P16F684::construct(const char *name)
{
  P16F684 *p = new P16F684(name);

  p->create(256);
  p->create_invalid_registers ();

  return p;
}

void P16F684::create(int eesize)
{
   create_iopin_map();

   _14bit_processor::create();

   osccon = new OSCCON(this, "osccon", "OSC Control");

   e = new EEPROM_WIDE(this,pir1);
   e->initialize(eesize);
   e->set_intcon(&intcon_reg);
   set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

   P16F684::create_sfr_map();
}

//-------------------------------------------------------------------
void P16F684::create_sfr_map()
{

  pir_set_def.set_pir1(pir1);

  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xbf, 0);
  alias_file_registers(0x70, 0x7f, 0x80);

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

  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc, 0x87, RegisterValue(0xff,0));

  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x00b, RegisterValue(0,0));

  alias_file_registers(0x0a,0x0b,0x80);
  add_sfr_register(pir1, 0x0c, RegisterValue(0,0));
  add_sfr_register(&tmr1l, 0x0e, RegisterValue(0,0), "tmr1l");
  add_sfr_register(&tmr1h, 0x0f, RegisterValue(0,0), "tmr1h");
  add_sfr_register(&t1con, 0x10, RegisterValue(0,0));
  add_sfr_register(&tmr2, 0x11, RegisterValue(0,0));
  add_sfr_register(&t2con, 0x12, RegisterValue(0,0));
  add_sfr_register(&ccpr1l, 0x13, RegisterValue(0,0));
  add_sfr_register(&ccpr1h, 0x14, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x15, RegisterValue(0,0));
  add_sfr_register(&pwm1con, 0x16, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0x17, RegisterValue(0,0));
  add_sfr_register(&wdtcon, 0x18, RegisterValue(0x08,0));
  add_sfr_register(&comparator.cmcon, 0x19, RegisterValue(0,0), "cmcon0");
  add_sfr_register(&comparator.cmcon1, 0x1a, RegisterValue(0,0), "cmcon1");
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));

  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));
  add_sfr_register(&pcon, 0x8e, RegisterValue(0,0));
  add_sfr_register(osccon, 0x8f, RegisterValue(0x60,0));

  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");
  add_sfr_register(&ansel, 0x91, RegisterValue(0xff,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));
  add_sfr_register(m_wpua, 0x95, RegisterValue(0x37,0),"wpua");
  add_sfr_register(m_ioca, 0x96, RegisterValue(0,0),"ioca");
  add_sfr_register(&comparator.vrcon, 0x99, RegisterValue(0,0),"vrcon");
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x9a);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x9b);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x9c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x9d);
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));

  ansel.setAdcon1(&adcon1);
  ansel.setValidBits(0xff);

  // Link the comparator and voltage ref to porta
  comparator.initialize(&pir_set_def, NULL, 
        &(*m_porta)[0], &(*m_porta)[1],          // AN0 AN1
        0, 0,
        &(*m_porta)[2], &(*m_portc)[4]);        //OUT0 OUT1

  comparator.cmcon.setINpin(2, &(*m_portc)[0], "an4"); //AN4
  comparator.cmcon.setINpin(3, &(*m_portc)[1], "an5"); //AN5

  comparator.cmcon.set_tmrl(&tmr1l);
  comparator.cmcon1.set_tmrl(&tmr1l);

  comparator.cmcon.set_configuration(1, 0, AN0, AN1, AN0, AN1, ZERO);
  comparator.cmcon.set_configuration(2, 0, AN2, AN3, AN2, AN3, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN1, AN2, AN0, AN2, NO_OUT);
  comparator.cmcon.set_configuration(2, 1, AN3, AN2, AN3, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 2, AN1, VREF, AN0, VREF, NO_OUT);
  comparator.cmcon.set_configuration(2, 2, AN3, VREF, AN2, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(2, 3, AN3, AN2, AN3, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 4, AN1, AN0, AN1, AN0, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN3, AN2, AN3, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 5, AN3, AN2, AN3, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 6, AN1, AN2, AN1, AN2, OUT0);
  comparator.cmcon.set_configuration(2, 6, AN3, AN2, AN3, AN2, OUT1);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.vrcon.setValidBits(0xaf); 

  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(7);
  adcon0.setChannel_shift(2);

  adcon1.setAdcon0(&adcon0);        // VCFG0, VCFG1 in adcon0
  adcon1.setNumberOfChannels(8);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[4]);
  adcon1.setIOPin(4, &(*m_portc)[0]);
  adcon1.setIOPin(5, &(*m_portc)[1]);
  adcon1.setIOPin(6, &(*m_portc)[2]);
  adcon1.setIOPin(7, &(*m_portc)[3]);
  adcon1.setVrefHiConfiguration(2, 1);
  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  // FIXME -- can't delete this new'd item
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v3::TMR1IF));
  tmr1h.tmrl  = &tmr1l;
  t1con.tmrl  = &tmr1l;

  tmr1l.setIOpin(&(*m_porta)[5]);
  tmr1l.setGatepin(&(*m_porta)[4]);

  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.setPir(pir1);

  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &ccp1con );
  pr2.tmr2    = &tmr2;

  eccpas.setIOpin(0, 0, &(*m_portc)[5]);
  eccpas.link_registers(&pwm1con, &ccp1con);

  ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
  ccp1con.setBitMask(0xff);
  ccp1con.pstrcon = &pstrcon;
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);

}
//-------------------------------------------------------------------
void P16F684::option_new_bits_6_7(uint bits)
{
  m_wpua->set_wpu_pu( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7);
  m_porta->setIntEdge((bits & OPTION_REG::BIT6) == OPTION_REG::BIT6);
}
//-------------------------------------------------------------------
void P16F684::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new ConfigF631((P16F631*)this));
  wdt.initialize(true); // default WDT enabled
  wdt.set_timeout(0.000035);
  set_config_word(0x2007, 0x3fff);

};

//-------------------------------------------------------------------
bool P16F684::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<2,
    CFG_WDTE  = 1<<3,
    CFG_MCLRE = 1<<5,
    CFG_IESO  = 1<<11,
  };

   if(address == config_word_address())
    {
               config_clock_mode = (cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2)); 
        if (osccon)
        {
            osccon->set_config_xosc(config_clock_mode < 3);
            osccon->set_config_irc(config_clock_mode == 4 || config_clock_mode == 5);
            osccon->set_config_ieso(cfg_word & CFG_IESO);
        }
       uint valid_pins = m_porta->getEnableMask();

        if ((cfg_word & CFG_MCLRE) == CFG_MCLRE)
        {
            assignMCLRPin(4);  
        }
        else
        {
            unassignMCLRPin();
        }

        wdt.initialize((cfg_word & CFG_WDTE) == CFG_WDTE);

       set_int_osc(false);

        // AnalogReq is used so ADC does not change clock names
        // set_config_word is first called with default and then
        // often called a second time. the following call is to
        // reset porta so next call to AnalogReq sill set the pin name
        //
        (&(*m_porta)[4])->AnalogReq((Register *)this, false, "porta4");
        valid_pins |= 0x20;
               switch(config_clock_mode) 
        {

               case 0:  // LP oscillator: low power crystal is on RA4 and RA5
               case 1:     // XT oscillator: crystal/resonator is on RA4 and RA5
               case 2:     // HS oscillator: crystal/resonator is on RA4 and RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "OSC2");
            valid_pins &= 0xcf;
            break;

        case 3:        // EC I/O on RA4 pin, CLKIN on RA5
            valid_pins &= 0xef;
            break;

            
        case 5: // INTOSC CLKOUT on RA4 pin
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");
        case 4: // INTOSC
             set_int_osc(true);
            osccon->set_rc_frequency();
            break;

        case 6: //RC oscillator: I/O on RA4 pin, RC on RA5
            valid_pins &= 0xdf;
            break;

        case 7: // RC oscillator: CLKOUT on RA4 pin, RC on RA5
            (&(*m_porta)[4])->AnalogReq((Register *)this, true, "CLKOUT");
            valid_pins &= 0xdf;
            break;
        };

        if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
            {
            m_porta->setEnableMask(valid_pins);
            m_trisa->setEnableMask(valid_pins);
            }
        return(true);
    }
    return false;
}
//========================================================================
//
// Pic 16F677 
//

Processor * P16F677::construct(const char *name)
{
  P16F677 *p = new P16F677(name);

  p->create(256);
  p->set_hasSSP();
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F677::P16F677(const char *_name, const char *desc)
  : P16F631(_name,desc),
    ssp(this),
    anselh(this,"anselh", "Analog Select high"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low")
{
}

P16F677::~P16F677()
{
  delete_file_registers(0x20,0x3f);
  delete_file_registers(0xa0,0xbf);

  remove_sfr_register(&anselh);

  if (hasSSP()) {
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(&ssp.sspstat);
  }
  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  delete m_cvref;
  delete m_v06ref;
}

void P16F677::create_sfr_map()
{

  ansel.setAdcon1(&adcon1);
  ansel.setAnselh(&anselh);
  anselh.setAdcon1(&adcon1);
  anselh.setAnsel(&ansel);
  anselh.setValidBits(0x0f);
  ansel.setValidBits(0xff);
  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(0xf);
  adcon0.setChannel_shift(2);
  adcon0.setGo(1);
  adcon0.setValidBits(0xff);
                               
  adcon1.setValidBits(0xb0);
  adcon1.setAdcon0(&adcon0);
  adcon1.setNumberOfChannels(14);
  adcon1.setValidCfgBits(ADCON1::VCFG0 , 6);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[4]);

  adcon1.setIOPin(8, &(*m_portc)[6]);
  adcon1.setIOPin(9, &(*m_portc)[7]);
  adcon1.setIOPin(10, &(*m_portb)[4]);
  adcon1.setIOPin(11, &(*m_portb)[5]);
  adcon1.setVoltRef(12, 0.0);
  adcon1.setVoltRef(13, 0.0);
  
  m_cvref  = new a2d_stimulus(&adcon1, 12, "a2d_cvref");
  m_v06ref = new a2d_stimulus(&adcon1, 13, "a2d_v06ref");
  ((Processor*)this)->CVREF->attach_stimulus(m_cvref);
  ((Processor*)this)->V06REF->attach_stimulus(m_v06ref);

  // set a2d modes where an1 is Vref+ 
  adcon1.setVrefHiConfiguration(2, 1);

  add_sfr_register(&anselh, 0x11f, RegisterValue(0x0f,0));
  add_file_registers(0x20,0x3f,0);
  add_file_registers(0xa0,0xbf,0);
//  ccp1con.setIOpin(&(*m_portc)[2], &(*m_portb)[2], &(*m_portb)[1], &(*m_portb)[4]);


  if (hasSSP()) {
    add_sfr_register(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
    add_sfr_register(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
    add_sfr_register(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
    add_sfr_register(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");

    ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portb)[6],   // SCK
                &(*m_portc)[6],   // SS
                &(*m_portc)[7],   // SDO
                &(*m_portb)[4],    // SDI
                m_trisb,          // i2c tris port
                SSP_TYPE_SSP
        );
  }
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));
}
//========================================================================
//
// Pic 16F685 
//

Processor * P16F685::construct(const char *name)
{
  P16F685 *p = new P16F685(name);

  p->create(256);
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F685::P16F685(const char *_name, const char *desc)
  : P16F677(_name,desc),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    pcon(this, "pcon", "pcon"),
    eccpas(this, "eccpas", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register"),
    pstrcon(this, "pstrcon", "Pulse Sterring Control Register")
{
  set_hasSSP();
}

P16F685::~P16F685()
{
  delete_file_registers(0xc0,0xef);
  delete_file_registers(0x120,0x16f);
  remove_sfr_register(&pstrcon);
  remove_sfr_register(&tmr2);
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);
  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&eccpas);
}

void P16F685::create_sfr_map()
{
  P16F677::create_sfr_map();

  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x10e );
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x10f);

  // Enable program memory reads and writes.
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);


  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));
  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &ccp1con );
  pr2.tmr2    = &tmr2;

  eccpas.setIOpin(0, 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  add_sfr_register(&pstrcon, 0x19d, RegisterValue(1,0));

  ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
  ccp1con.setBitMask(0xff);
  ccp1con.pstrcon = &pstrcon;
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  add_sfr_register(&ccpr1l, 0x15, RegisterValue(0,0));
  add_sfr_register(&ccpr1h, 0x16, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x17, RegisterValue(0,0));

  add_sfr_register(&pwm1con, 0x1c, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0x1d, RegisterValue(0,0));
//  add_file_registers(0x20,0x3f,0);
//  add_file_registers(0xa0,0xef,0);
  add_file_registers(0xc0,0xef,0);
  add_file_registers(0x120,0x16f,0);


}
//========================================================================
//
// Pic 16F687 
//

Processor * P16F687::construct(const char *name)
{
  P16F687 *p = new P16F687(name);

  p->create(256);
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F687::P16F687(const char *_name, const char *desc)
  : P16F677(_name,desc),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    pcon(this, "pcon", "pcon"),
    usart(this)
{
  set_hasSSP();
}

P16F687::~P16F687()
{
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  remove_sfr_register(&usart.spbrgh);
  remove_sfr_register(&usart.baudcon);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
}

void P16F687::create_sfr_map()
{
  P16F677::create_sfr_map();

  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x10f);

//  add_file_registers(0x20,0x3f,0);
//  add_file_registers(0xa0,0xbf,0);

  usart.initialize(pir1,&(*m_portb)[7], &(*m_portb)[5],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(&usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.baudcon,  0x9b,RegisterValue(0x40,0),"baudctl");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
  usart.set_eusart(true);

}
//========================================================================
//
// Pic 16F689 
//

Processor * P16F689::construct(const char *name)
{
  P16F689 *p = new P16F689(name);

  p->create(256);
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F689::P16F689(const char *_name, const char *desc)
  : P16F687(_name,desc)
{
  set_hasSSP();
}

//========================================================================
//
Processor * P16F690::construct(const char *name)
{
  P16F690 *p = new P16F690(name);

  p->create(256);
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F690::P16F690(const char *_name, const char *desc)
  : P16F685(_name,desc),
    ccp2con(this, "ccp2con", "Capture Compare Control"),
    ccpr2l(this, "ccpr2l", "Capture Compare 2 Low"),
    ccpr2h(this, "ccpr2h", "Capture Compare 2 High"),
    usart(this)
{
  set_hasSSP();
}

P16F690::~P16F690()
{
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  remove_sfr_register(&usart.spbrgh);
  remove_sfr_register(&usart.baudcon);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
}

void P16F690::create_sfr_map()
{
  P16F685::create_sfr_map();

  tmr2.ssp_module[0] = &ssp;
  eccpas.setIOpin(0, 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);

  usart.initialize(pir1,&(*m_portb)[7], &(*m_portb)[5],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(&usart.spbrgh, 0x9a, RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.baudcon,  0x9b,RegisterValue(0x40,0),"baudctl");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
  usart.set_eusart(true);

 // add_sfr_register(&pstrcon, 0x19d, RegisterValue(1,0));
}

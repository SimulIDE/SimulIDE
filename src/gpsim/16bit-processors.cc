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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>

#include "config.h"
#include "packages.h"
#include "16bit-processors.h"
#include "stimuli.h"
#include "eeprom.h"


//------------------------------------------------------------------------
// Configuration bits
//
// The 16bit-core PIC devices contain configuration memory starting at
// address 0x300000.
//
string Config1H::toString()
{
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];

    const char *OSCdesc[8] = {
      "LP oscillator",
      "XT oscillator",
      "HS oscillator",
      "RC oscillator",
      "EC oscillator w/ OSC2 configured as divide-by-4 clock output",
      "EC oscillator w/ OSC2 configured as RA6",
      "HS oscillator with PLL enabled/Clock frequency = (4 x FOSC)",
      "RC oscillator w/ OSC2 configured as RA6"
    };
    snprintf(buff,sizeof(buff),
             "$%04x\n"
             " FOSC=%d - Clk source = %s\n"
             " OSCEN=%d - Oscillator switching is %s\n",
             i,
             i & (FOSC0 | FOSC1 | FOSC2), OSCdesc[i & (FOSC0 | FOSC1 | FOSC2)],
             ((i & OSCEN) ? 1 : 0), ((i & OSCEN) ? "disabled" : "enabled"));

    return string(buff);
}

string Config1H_4bits::toString()
{
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];

    const char *OSCdesc[] = {
      "LP oscillator",
      "XT oscillator",
      "HS oscillator",
      "RC oscillator",
      "EC oscillator w/ OSC2 configured as divide-by-4 clock output",
      "EC oscillator w/ OSC2 configured as RA6",
      "HS oscillator with PLL enabled/Clock frequency = (4 x FOSC)",
      "RC oscillator w/ OSC2 configured as RA6",
      "Internal oscillator block, port function on RA6 and RA7",
      "Internal oscillator block, CLKO function on RA6, port function on RA7",
      "External RC oscillator, CLKO function on RA6",
      "External RC oscillator, CLKO function on RA6",
      "External RC oscillator, CLKO function on RA6",
      "External RC oscillator, CLKO function on RA6",
      "External RC oscillator, CLKO function on RA6",
      "External RC oscillator, CLKO function on RA6"
    };
    snprintf(buff,sizeof(buff),
             "$%04x\n"
             " FOSC=%d - Clk source = %s\n"
             " OSCEN=%d - Oscillator switching is %s\n",
             i,
             i & (FOSC0 | FOSC1 | FOSC2 | FOSC3), OSCdesc[i & (FOSC0 | FOSC1 | FOSC2 | FOSC3)],
             ((i & OSCEN) ? 1 : 0), ((i & OSCEN) ? "disabled" : "enabled"));

    return string(buff);
}

//------------------------------------------------------------------------
// Config2H - default
//  The default Config2H register controls the 18F series WDT.
class Config2H : public ConfigWord
{
    #define WDTEN   (1<<0)
    #define WDTPS0  (1<<1)
    #define WDTPS1  (1<<2)
    #define WDTPS2  (1<<3)

    #define CONFIG2H_default (WDTEN | WDTPS0 | WDTPS1 | WDTPS2)
    public:
      Config2H(_16bit_processor *pCpu, uint addr)
        : ConfigWord("CONFIG2H", CONFIG2H_default, "WatchDog configuration", pCpu, addr)
      {
            set(CONFIG2H_default);
      }
      virtual void set(int64_t v)
      {
        Integer::set(v);
        if (m_pCpu)
        {
          m_pCpu->wdt.set_postscale((v & (WDTPS0|WDTPS1|WDTPS2)) >> 1);
          m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);
        }
      }

      virtual string toString()
      {
        int64_t i64;
        get(i64);
        int i = i64 &0xfff;

        char buff[256];

        snprintf(buff,sizeof(buff),
                 "$%04x\n"
                 " WDTEN=%d - WDT is %s, prescale=1:%d\n",
                 i,
                 ((i & WDTEN) ? 1 : 0), ((i & WDTEN) ? "enabled" : "disabled"),
                 1 << (i & (WDTPS0 | WDTPS1 | WDTPS2)>>1));

        return string(buff);
      }
};

//------------------------------------------------------------------------
// Config4L - default
//  The default Config4L register controls the 18F series WDT.
class Config4L : public ConfigWord
{
    #define STKVREN  (1<<0)
    #define LVP        (1<<2)
    #define BBSIZ0        (1<<4)
    #define BBSIZ1        (1<<5)
    #define XINST        (1<<6)
    #define _DEBUG        (1<<7)

    #define CONFIG4L_default (STKVREN | LVP | _DEBUG)
    public:
      Config4L(_16bit_processor *pCpu, uint addr)
        : ConfigWord("CONFIG4L", CONFIG4L_default, "Config word 4L", pCpu, addr)
      {
            set(CONFIG4L_default);
      }
    #define Cpu16 ((_16bit_processor *)m_pCpu)
      virtual void set(int64_t v)
      {
        Integer::set(v);
        if (m_pCpu)
        {
            Cpu16->set_extended_instruction((v & XINST) == XINST);
                if(m_pCpu->stack)
                m_pCpu->stack->STVREN = ((v & STKVREN) == STKVREN);
        }

      }

      virtual string toString()
      {
        int64_t i64;
        get(i64);
        int i = i64 &0xfff;

        char buff[256];

        snprintf(buff,sizeof(buff),
                 "$%04x\n"
                 " STVREN=%d - BBSIZE=%x XINST=%d\n",
                 i,
                 ((i & STKVREN) ? 1 : 0), (i & (BBSIZ1 | BBSIZ0)) >> 4,
                 ((i & XINST) ? 1 : 0));

        return string(buff);
      }
};


#define PWRTEN  1<<0
#define BOREN   1<<1
#define BORV0   1<<2
#define BORV1   1<<2

#define CCP2MX  1<<0

//-------------------------------------------------------------------
_16bit_processor::_16bit_processor(const char *_name, const char *desc)
  : pic_processor(_name,desc),
/*
    adcon0(this, "adcon0", "A2D control register"),
    adcon1(this, "adcon1", "A2D control register"),
*/
    adresl(this, "adresl", "A2D result low"),
    adresh(this, "adresh", "A2D result high"),
    intcon(this, "intcon", "Interrupt control"),
    intcon2(this, "intcon2", "Interrupt control"),
    intcon3(this, "intcon3", "Interrupt control"),
    bsr(this, "bsr", "Bank Select Register"),
    tmr0l(this, "tmr0l", "TMR0 Low"),
    tmr0h(this, "tmr0h", "TMR0 High"),
    t0con(this, "t0con", "TMR0 Control"),
    rcon(this, "rcon", "Reset Control"),
    pir1(this,"pir1","Peripheral Interrupt Register",0,0),
    ipr1(this, "ipr1", "Interrupt Priorities"),
    ipr2(this, "ipr2", "Interrupt Priorities"),
    pie1(this, "pie1", "Peripheral Interrupt Enable"),
    pie2(this, "pie2", "Peripheral Interrupt Enable"),
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
    tmr3l(this, "tmr3l", "TMR3 Low"),
    tmr3h(this, "tmr3h", "TMR3 High"),

    osccon(0),
    lvdcon(this, "lvdcon", "LVD Control"),
    wdtcon(this, "wdtcon", "WDT Control", 1),
    prodh(this, "prodh", "Product High"),
    prodl(this, "prodl", "Product Low"),
    pclatu(this, "pclatu", "Program Counter Latch upper byte"),

    ind0(this,string("0")),
    ind1(this,string("1")),
    ind2(this,string("2")),
    usart(this),

    tbl(this),
    ssp(this)
{

  set_osc_pin_Number(0, 253, NULL);
  set_osc_pin_Number(1, 253, NULL);
  package = 0;
  pll_factor = 0;

  pc = new Program_Counter16(this);

  m_porta = new PicPortRegister(this,"porta","",8,0xff);
  m_porta->setEnableMask(0x7f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);
  m_trisa->setEnableMask(0x7f);
  m_lata  = new PicLatchRegister(this,"lata","", m_porta);
  m_lata->setEnableMask(0x7f);

  m_portb = new PicPortBRegister(this,"portb","", &intcon, 8,0xff,
        &intcon2, &intcon3);
  m_portb->assignRBPUSink(7,&intcon2);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);
  m_latb  = new PicLatchRegister(this,"latb","", m_portb);

  m_portc = new PicPortRegister(this,"portc","",8,0xff);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false);
  m_latc  = new PicLatchRegister(this,"latc","", m_portc);
  pir2 = new  PIR2v2(this,"pir2","Peripheral Interrupt Register",0,0);

  t1con = new T1CON(this, "t1con", "TMR1 Control");
  t3con = new T3CON(this, "t3con", "TMR3 Control");
  //tmr0l.set_cpu(this, m_porta, 4, option_reg);
  //tmr0l.start(0);
  m_porta->addSink(&tmr0l,4);

  stack = new Stack16(this);
  internal_osc = false;
}

//-------------------------------------------------------------------
_16bit_processor::~_16bit_processor()
{
    delete_sfr_map();
}

//-------------------------------------------------------------------
pic_processor *_16bit_processor::construct()
{
    cout << "creating 16bit processor construct\n";

  _16bit_processor *p = new _16bit_processor;

  p->create();
  p->create_invalid_registers();
  p->name_str = "generic 16bit processor";

  return p;
}

void _16bit_processor :: delete_sfr_map()
{
  unassignMCLRPin();
  delete_file_registers(0x0, last_register);
  remove_sfr_register(&pie1);
  remove_sfr_register(&pir1);
  remove_sfr_register(&ipr1);
  remove_sfr_register(&pie2);
  delete_sfr_register(pir2);
  remove_sfr_register(&ipr2);
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
  delete_sfr_register(t3con);
  remove_sfr_register(&tmr3l);
  remove_sfr_register(&tmr3h);

  if ( HasCCP2() )
  {
    remove_sfr_register(&ccp2con);
    remove_sfr_register(&ccpr2l);
    remove_sfr_register(&ccpr2h);
  }
  remove_sfr_register(&ccp1con);
  remove_sfr_register(&ccpr1l);
  remove_sfr_register(&ccpr1h);

  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);

  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(&ssp.sspcon);
  remove_sfr_register(&ssp.sspstat);
  remove_sfr_register(&ssp.sspadd);
  remove_sfr_register(&ssp.sspbuf);

  if (!MovedReg())
  {
  remove_sfr_register(&t2con);
  remove_sfr_register(&pr2);
  remove_sfr_register(&tmr2);
  }

  delete_sfr_register(t1con);
  delete_sfr_register(osccon);
  remove_sfr_register(&tmr1l);
  remove_sfr_register(&tmr1h);

  remove_sfr_register(&rcon);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(&lvdcon);
  remove_sfr_register(&t0con);
  remove_sfr_register(&tmr0l);
  remove_sfr_register(&tmr0h);
  remove_sfr_register(&ind2.fsrl);
  remove_sfr_register(&ind2.fsrh);
  remove_sfr_register(&ind2.plusw);
  remove_sfr_register(&ind2.preinc);
  remove_sfr_register(&ind2.postdec);
  remove_sfr_register(&ind2.postinc);
  remove_sfr_register(&ind2.postinc);
  remove_sfr_register(&ind2.indf);

  remove_sfr_register(&bsr);

  remove_sfr_register(&ind1.fsrl);
  remove_sfr_register(&ind1.fsrh);
  remove_sfr_register(&ind1.plusw);
  remove_sfr_register(&ind1.preinc);
  remove_sfr_register(&ind1.postdec);
  remove_sfr_register(&ind1.postinc);
  remove_sfr_register(&ind1.indf);


  remove_sfr_register(&ind0.fsrl);
  remove_sfr_register(&ind0.fsrh);
  remove_sfr_register(&ind0.plusw);
  remove_sfr_register(&ind0.preinc);
  remove_sfr_register(&ind0.postdec);
  remove_sfr_register(&ind0.postinc);
  remove_sfr_register(&ind0.indf);

  remove_sfr_register(&intcon3);
  remove_sfr_register(&intcon2);
  remove_sfr_register(&intcon);

  remove_sfr_register(&prodl);
  remove_sfr_register(&prodh);

  remove_sfr_register(&tbl.tablat);
  remove_sfr_register(&tbl.tblptrl);
  remove_sfr_register(&tbl.tblptrh);
  remove_sfr_register(&tbl.tblptru);
  remove_sfr_register(&pclatu);

  Stack16 *stack16 = static_cast<Stack16 *>(stack);
  remove_sfr_register(&stack16->stkptr);
  remove_sfr_register(&stack16->tosl);
  remove_sfr_register(&stack16->tosh);
  remove_sfr_register(&stack16->tosu);

  EEPROM *e = get_eeprom();

  if (e) {
    remove_sfr_register(e->get_reg_eedata());
    remove_sfr_register(e->get_reg_eeadr());
    if ( e->get_reg_eeadrh() )
        remove_sfr_register(e->get_reg_eeadrh());
    remove_sfr_register(e->get_reg_eecon1());
    remove_sfr_register(e->get_reg_eecon2());
  }
  delete_sfr_register(m_porta);
  delete_sfr_register(m_lata);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_portb);
  delete_sfr_register(m_latb);
  delete_sfr_register(m_trisb);
  if ( HasPortC() )
  {
      delete_sfr_register(m_portc);
      delete_sfr_register(m_latc);
      delete_sfr_register(m_trisc);
  }
  delete pc;
}

//-------------------------------------------------------------------
void _16bit_processor :: create_sfr_map()
{
  last_register = last_actual_register();
  add_file_registers(0x0, last_register, 0);

  RegisterValue porv(0,0);
  RegisterValue porv2(0,0);

  add_sfr_register(m_porta,       0xf80,porv);
  add_sfr_register(m_portb,       0xf81,porv);
  if ( HasPortC() )
    add_sfr_register(m_portc,       0xf82,porv);

  add_sfr_register(m_lata,        0xf89,porv);
  add_sfr_register(m_latb,        0xf8a,porv);
  if ( HasPortC() )
    add_sfr_register(m_latc,        0xf8b,porv);

  add_sfr_register(m_trisa,       0xf92,RegisterValue(0x7f,0));
  add_sfr_register(m_trisb,       0xf93,RegisterValue(0xff,0));
  if ( HasPortC() )
    add_sfr_register(m_trisc,       0xf94,RegisterValue(0xff,0));

  add_sfr_register(&pie1,          0xf9d,porv,"pie1");
  add_sfr_register(&pir1,          0xf9e,porv,"pir1");
  add_sfr_register(&ipr1,          0xf9f,porv,"ipr1");

  add_sfr_register(&pie2,          0xfa0,porv,"pie2");
  add_sfr_register(&ipr2,          0xfa2,porv,"ipr2");


  if ( HasPortC() )
      usart.initialize(&pir1,&(*m_portc)[6], &(*m_portc)[7],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart),
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));
  else
      usart.initialize(&pir1,0, 0,
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart),
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta,    0xfab,porv,"rcsta");
  add_sfr_register(&usart.txsta,    0xfac,RegisterValue(0x02,0),"txsta");
  add_sfr_register(usart.txreg,     0xfad,porv,"txreg");
  add_sfr_register(usart.rcreg,     0xfae,porv,"rcreg");
  add_sfr_register(&usart.spbrg,    0xfaf,porv,"spbrg");

  add_sfr_register(t3con,          0xfb1,porv);
  add_sfr_register(&tmr3l,          0xfb2,porv,"tmr3l");
  add_sfr_register(&tmr3h,          0xfb3,porv,"tmr3h");

  if ( HasCCP2() )
  {
    add_sfr_register(&ccp2con,          0xfba,porv,"ccp2con");
    add_sfr_register(&ccpr2l,          0xfbb,porv,"ccpr2l");
    add_sfr_register(&ccpr2h,          0xfbc,porv,"ccpr2h");
  }
  add_sfr_register(&ccp1con,          0xfbd,porv,"ccp1con");
  add_sfr_register(&ccpr1l,          0xfbe,porv,"ccpr1l");
  add_sfr_register(&ccpr1h,          0xfbf,porv,"ccpr1h");

  add_sfr_register(&adresl,          0xfc3,porv,"adresl");
  add_sfr_register(&adresh,          0xfc4,porv,"adresh");

  add_sfr_register(&ssp.sspcon2,  0xfc5,porv,"sspcon2");
  add_sfr_register(&ssp.sspcon,   0xfc6,porv,"sspcon1");
  add_sfr_register(&ssp.sspstat,  0xfc7,porv,"sspstat");
  add_sfr_register(&ssp.sspadd,   0xfc8,porv,"sspadd");
  add_sfr_register(&ssp.sspbuf,   0xfc9,porv,"sspbuf");
  if (!MovedReg())
  {
    add_sfr_register(&t2con,          0xfca,porv,"t2con");
    add_sfr_register(&pr2,          0xfcb,RegisterValue(0xff,0),"pr2");
    add_sfr_register(&tmr2,          0xfcc,porv,"tmr2");
  }

  add_sfr_register(t1con,          0xfcd,porv,"t1con");
  add_sfr_register(&tmr1l,          0xfce,porv,"tmr1l");
  add_sfr_register(&tmr1h,          0xfcf,porv,"tmr1h");

  add_sfr_register(&rcon,          0xfd0,RegisterValue(0x1c,0),"rcon");
  add_sfr_register(&wdtcon,          0xfd1,porv,"wdtcon");
  add_sfr_register(&lvdcon,          0xfd2,porv,"lvdcon");

  add_sfr_register( osccon,          0xfd3,RegisterValue(0x40,0),"osccon");

  add_sfr_register(&t0con,          0xfd5,RegisterValue(0xff,0),"t0con");
  add_sfr_register(&tmr0l,          0xfd6,porv,"tmr0l");
  add_sfr_register(&tmr0h,          0xfd7,porv,"tmr0h");
  t0con.put(0xff);  /**FIXME - need a way to set this to 0xff at reset*/

  add_sfr_register(status,       0xfd8);
  status->set_rcon(&rcon);

  add_sfr_register(&ind2.fsrl,          0xfd9,porv,"fsr2l");
  add_sfr_register(&ind2.fsrh,    0xfda,porv,"fsr2h");
  add_sfr_register(&ind2.plusw,   0xfdb,porv,"plusw2");
  add_sfr_register(&ind2.preinc,  0xfdc,porv,"preinc2");
  add_sfr_register(&ind2.postdec, 0xfdd,porv,"postdec2");
  add_sfr_register(&ind2.postinc, 0xfde,porv,"postinc2");
  add_sfr_register(&ind2.indf,    0xfdf,porv,"indf2");

  add_sfr_register(&bsr,          0xfe0,porv, "bsr");

  add_sfr_register(&ind1.fsrl,          0xfe1,porv,"fsr1l");
  add_sfr_register(&ind1.fsrh,    0xfe2,porv,"fsr1h");
  add_sfr_register(&ind1.plusw,   0xfe3,porv,"plusw1");
  add_sfr_register(&ind1.preinc,  0xfe4,porv,"preinc1");
  add_sfr_register(&ind1.postdec, 0xfe5,porv,"postdec1");
  add_sfr_register(&ind1.postinc, 0xfe6,porv,"postinc1");
  add_sfr_register(&ind1.indf,    0xfe7,porv,"indf1");

  add_sfr_register(Wreg,            0xfe8);

  add_sfr_register(&ind0.fsrl,          0xfe9,porv,"fsr0l");
  add_sfr_register(&ind0.fsrh,    0xfea,porv,"fsr0h");
  add_sfr_register(&ind0.plusw,   0xfeb,porv,"plusw0");
  add_sfr_register(&ind0.preinc,  0xfec,porv,"preinc0");
  add_sfr_register(&ind0.postdec, 0xfed,porv,"postdec0");
  add_sfr_register(&ind0.postinc, 0xfee,porv,"postinc0");
  add_sfr_register(&ind0.indf,    0xfef,porv,"indf0");

  add_sfr_register(&intcon3, 0xff0, porv,"intcon3");
  porv2.data = 0xF5;
  add_sfr_register(&intcon2, 0xff1, porv2,"intcon2");
  add_sfr_register(&intcon,  0xff2, porv,"intcon");

  add_sfr_register(&prodl, 0xff3, porv,"prodl");
  add_sfr_register(&prodh, 0xff4, porv,"prodh");

  add_sfr_register(&tbl.tablat,  0xff5, porv,"tablat");
  add_sfr_register(&tbl.tblptrl, 0xff6, porv,"tblptrl");
  add_sfr_register(&tbl.tblptrh, 0xff7, porv,"tblptrh");
  add_sfr_register(&tbl.tblptru, 0xff8, porv,"tblptru");

  if(pcl)
    delete pcl;
  pcl = new PCL16(this,"pcl", "Program Counter Low byte");

  add_sfr_register(pcl,     0xff9);
  add_sfr_register(pclath,  0xffa);
  add_sfr_register(&pclatu, 0xffb, porv, "pclatu");
  pclath->mValidBits = 0xFF;    // Data sheet implies does not depend on memory size

  Stack16 *stack16 = static_cast<Stack16 *>(stack);
  add_sfr_register(&stack16->stkptr,  0xffc,porv,"stkptr");
  add_sfr_register(&stack16->tosl,    0xffd,porv,"tosl");
  add_sfr_register(&stack16->tosh,    0xffe,porv,"tosh");
  add_sfr_register(&stack16->tosu,    0xfff,porv,"tosu");
  stack16->stack_mask = 31;


  EEPROM *e = get_eeprom();

  if (e) {
    add_sfr_register(e->get_reg_eedata(), 0xfa8);
    add_sfr_register(e->get_reg_eeadr(), 0xfa9);
    if ( e->get_reg_eeadrh() )
        add_sfr_register(e->get_reg_eeadrh(), 0xfaa);
    add_sfr_register(e->get_reg_eecon1(), 0xfa6, RegisterValue(0,0));
    add_sfr_register(e->get_reg_eecon2(), 0xfa7);
  }

  // Initialize all of the register cross linkages
  pir_set_def.set_pir1(&pir1);

  tmr2.ssp_module[0] = &ssp;

  tmr1l.tmrh   = &tmr1h;
  tmr1l.t1con  = t1con;
  tmr1l.setInterruptSource(new InterruptSource(&pir1, PIR1v1::TMR1IF));
//  tmr1l.ccpcon = &ccp1con;

  tmr1h.tmrl  = &tmr1l;

  t1con->tmrl  = &tmr1l;

  t2con.tmr2  = &tmr2;
  tmr2.pir_set = &pir_set_def; //get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.add_ccp ( &ccp1con );
  tmr2.add_ccp ( &ccp2con );
  pr2.tmr2    = &tmr2;


  tmr3l.tmrh  = &tmr3h;
  tmr3l.t1con = t3con;
//  tmr3l.ccpcon = &ccp1con;

  tmr3h.tmrl  = &tmr3l;

  t3con->tmrl  = &tmr3l;
  if (T3HasCCP())
  {
    t3con->tmr1l = &tmr1l;
    t3con->ccpr1l = &ccpr1l;
    t3con->ccpr2l = &ccpr2l;
    t3con->t1con = t1con;
  }

  ccp1con.setCrosslinks(&ccpr1l, &pir1, PIR1v2::CCP1IF, &tmr2);
  ccp1con.setIOpin(&((*m_portc)[2]));
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  pir1.set_intcon(&intcon);
  pir1.set_pie(&pie1);
  pir1.set_ipr(&ipr1);
  pie1.setPir(&pir1);

  // All of the status bits on the 16bit core are writable
  status->write_mask = 0xff;

  // AN5,AN6 and AN7 exist only on devices with a PORTE.
}

void _16bit_processor::init_pir2(PIR *pir2, uint bitMask)
{
  RegisterValue porv(0,0);

  tmr3l.setInterruptSource(new InterruptSource(pir2, bitMask));
  pir_set_def.set_pir2(pir2);
  pir2->set_intcon(&intcon);
  pir2->set_pie(&pie2);
  pir2->set_ipr(&ipr2);
  pie2.setPir(pir2);
  add_sfr_register(pir2, 0xfa1,porv,"pir2");
}

//-------------------------------------------------------------------
//
//    create
//
//  The purpose of this member function is to 'create' those things
// that are unique to the 16-bit core processors.

void _16bit_processor :: create ()
{
  fast_stack.init(this);
  /*
  ind0.init(this);
  ind1.init(this);
  ind2.init(this);
  */
  pic_processor::create();
  osccon = getOSCCON();
  create_sfr_map();

  tmr0l.initialize();

  intcon.set_rcon(&rcon);
  intcon.set_intcon2(&intcon2);
  intcon.set_pir_set(&pir_set_def);

  //tbl.initialize(this);
  tmr0l.start(0);

  if(pma) {

    pma->SpecialRegisters.push_back(&bsr);
    rma.SpecialRegisters.push_back(&bsr);
  }
}

//-------------------------------------------------------------------
// void _16bit_processor::interrupt ()
//
//  When the virtual function cpu->interrupt() is called during
// pic_processor::run() AND the cpu gpsim is simulating is an 18cxxx
// device then we end up here. For an interrupt to have occured,
// the interrupt processing logic must have just ran. One of the
// responsibilities of that logic is to determine at what address
// the interrupt should begin executing. That address is placed
// in 'interrupt_vector'.
//
//-------------------------------------------------------------------
void _16bit_processor::interrupt ()
{
  bp.clear_interrupt();

  stack->push(pc->value);

  // Save W,status, and BSR if this is a high priority interrupt.
  fast_stack.push();

  intcon.in_interrupt = true; // Mask interrupts
  pc->interrupt(intcon.get_interrupt_vector());
}

//-------------------------------------------------------------------
void _16bit_processor::option_new_bits_6_7(uint bits)
{
  //portb.rbpu_intedg_update(bits);
  //cout << "16bit, option bits 6 and/or 7 changed\n";
}

//-------------------------------------------------------------------
void _16bit_processor::enter_sleep()
{
    tmr0l.sleep();
    pic_processor::enter_sleep();
}

//-------------------------------------------------------------------
void _16bit_processor::exit_sleep()
{
  if (m_ActivityState == ePASleeping)
  {
      tmr0l.wake();
      pic_processor::exit_sleep();
  }
}

//------------------------------------------------------------------
// It is assummed that this will only be set to true for processors
// that support extended instructions
//
void _16bit_processor::set_extended_instruction(bool v)
{
   extended_instruction_flag = v;
}

//-------------------------------------------------------------------
// Fetch the rom contents at a particular address.
uint _16bit_processor::get_program_memory_at_address(uint address)
{
  uint uIndex = map_pm_address2index(address);

  if (uIndex < program_memory_size())
    return  program_memory[uIndex] ? program_memory[uIndex]->get_opcode() : 0xffffffff;

  if (address >= CONFIG1L && address <= 0x30000D)
    return get_config_word(address);

  uIndex = (address - 0x200000) >> 1;  // Look to see if it's an ID location
  
  if( uIndex < IdentMemorySize() )  return idloc[uIndex];

//  static const uint DEVID1 = 0x3ffffe;
 // static const uint DEVID2 = 0x3fffff;
#define  DEVID1 0x3ffffe
  if ((address & DEVID1) == DEVID1)
  {
        return get_device_id();
  }

  return 0xffffffff;
}

uint _16bit_processor::get_config_word(uint address)
{
  if (!(address >= CONFIG1L && address <= 0x30000D))
    return 0xffffffff;

  address -= CONFIG1L;

  if (m_configMemory) {
    address &= 0xfffe; // Clear LSB
    uint ret = 0xffff;

    if (m_configMemory->getConfigWord(address))
      ret = (ret & 0xff00) |  (((uint )(m_configMemory->getConfigWord(address)->getVal())) & 0x00ff);

    address++;

    if (m_configMemory->getConfigWord(address))
      ret = (ret & 0x00ff) |  ((((uint )(m_configMemory->getConfigWord(address)->getVal()))<<8) & 0xff00);

    return ret;
  }
  return 0xffffffff;
}

bool  _16bit_processor::set_config_word(uint address, uint cfg_word)
{
  if (address >= CONFIG1L && address <= 0x30000D) 
  {
    address -= CONFIG1L;

    if (m_configMemory) 
    {
      address &= 0xfffe;

      if (m_configMemory->getConfigWord(address))
        m_configMemory->getConfigWord(address)->set((int)(cfg_word&0xff));

      address++;

      if (m_configMemory->getConfigWord(address))
        m_configMemory->getConfigWord(address)->set((int)((cfg_word>>8)&0xff));

      return true;
    }
    else cout << "Setting config word no m_configMemory\n";
  }
  return false;
}

void _16bit_processor::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,configMemorySize());
  m_configMemory->addConfigWord(CONFIG1H-CONFIG1L,new Config1H(this, CONFIG1H));
  m_configMemory->addConfigWord(CONFIG2H-CONFIG1L,new Config2H(this, CONFIG2H));
  m_configMemory->addConfigWord(CONFIG4L-CONFIG1L,new Config4L(this, CONFIG4L));
}

void _16bit_processor::set_out_of_range_pm(uint address, uint value)
{
    // This method is only called by Processor::init_program_memory which writes words
    if ( get_eeprom()
    && (address>= 0xf00000)
    && (address < 0xf00000 + get_eeprom()->get_rom_size()))
    {
        get_eeprom()->change_rom(1 + address - 0xf00000, value >> 8);
        get_eeprom()->change_rom(address - 0xf00000, value & 0xff);
    }
    else if( (address>= 0x200000) && (address < 0x200008) ) {
        idloc[(address - 0x200000) >> 1] = value;
    }
}

void _16bit_processor::osc_mode(uint value)
{
  IOPIN *m_pin;
  uint pin_Number =  get_osc_pin_Number(0);

  if (pin_Number < 253)
  {
        m_pin = package->get_pin(pin_Number);
  }
  if ( (pin_Number =  get_osc_pin_Number(1)) < 253 &&
        (m_pin = package->get_pin(pin_Number)))
  {
        pll_factor = 0;
        if (value < 5)
        {
            set_clk_pin(pin_Number, m_osc_Monitor[1], "OSC2", true,
                m_porta, m_trisa, m_lata);
        }
        else if(value == 6 )
        {
            pll_factor = 2;
            set_clk_pin(pin_Number, m_osc_Monitor[1], "CLKO", false,
                m_porta, m_trisa, m_lata);
        }
        else
        {
            clr_clk_pin(pin_Number, m_osc_Monitor[1],
                m_porta, m_trisa, m_lata);
        }
  }
}

void _16bit_processor::create_iopin_map()
{
  cout << "_16bit_processor::create_iopin_map WARNING --- not creating package \n";
}

void _16bit_compat_adc::create()
{
    adcon0 = new ADCON0(this, "adcon0", "A2D control register"),
    adcon1 = new ADCON1(this, "adcon1", "A2D control register"),

    _16bit_processor::create();

    a2d_compat();
}

void _16bit_compat_adc::create_sfr_map()
{
    _16bit_processor::create_sfr_map();
}

void _16bit_compat_adc :: a2d_compat()
{
  RegisterValue porv(0,0);

  add_sfr_register(adcon1, 0xfc1,porv,"adcon1");
  add_sfr_register(adcon0, 0xfc2,porv,"adcon0");

  adcon0->setAdresLow(&adresl);
  adcon0->setAdres(&adresh);
  adcon0->setAdcon1(adcon1);
  adcon0->setIntcon(&intcon);
  adcon0->setPir(&pir1);
  adcon0->setChannel_Mask(7); // Greater than 4 channels
  adcon0->setA2DBits(10);

  adcon1->setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 |
                         ADCON1::PCFG2 | ADCON1::PCFG3,0);

  adcon1->setChannelConfiguration(0, 0xff);
  adcon1->setChannelConfiguration(1, 0xff);
  adcon1->setChannelConfiguration(2, 0x1f);
  adcon1->setChannelConfiguration(3, 0x1f);
  adcon1->setChannelConfiguration(4, 0x0b);
  adcon1->setChannelConfiguration(5, 0x0b);
  adcon1->setChannelConfiguration(6, 0x00);
  adcon1->setChannelConfiguration(7, 0x00);
  adcon1->setChannelConfiguration(8, 0xff);
  adcon1->setChannelConfiguration(9, 0x3f);
  adcon1->setChannelConfiguration(10, 0x3f);
  adcon1->setChannelConfiguration(11, 0x3f);
  adcon1->setChannelConfiguration(12, 0x1f);
  adcon1->setChannelConfiguration(13, 0x0f);
  adcon1->setChannelConfiguration(14, 0x01);
  adcon1->setChannelConfiguration(15, 0x0d);

  adcon1->setVrefHiConfiguration(1, 3);
  adcon1->setVrefHiConfiguration(3, 3);
  adcon1->setVrefHiConfiguration(5, 3);
  adcon1->setVrefHiConfiguration(8, 3);
  adcon1->setVrefHiConfiguration(10, 3);
  adcon1->setVrefHiConfiguration(11, 3);
  adcon1->setVrefHiConfiguration(12, 3);
  adcon1->setVrefHiConfiguration(13, 3);
  adcon1->setVrefHiConfiguration(15, 3);

  adcon1->setVrefLoConfiguration(8, 2);
  adcon1->setVrefLoConfiguration(11, 2);
  adcon1->setVrefLoConfiguration(12, 2);
  adcon1->setVrefLoConfiguration(13, 2);
  adcon1->setVrefLoConfiguration(15, 2);

  adcon1->setNumberOfChannels(5);
  adcon1->setIOPin(0, &(*m_porta)[0]);
  adcon1->setIOPin(1, &(*m_porta)[1]);
  adcon1->setIOPin(2, &(*m_porta)[2]);
  adcon1->setIOPin(3, &(*m_porta)[3]);
  adcon1->setIOPin(4, &(*m_porta)[5]);

}
_16bit_compat_adc::_16bit_compat_adc(const char *_name, const char *desc)
        : _16bit_processor(_name, desc), adcon0(0), adcon1(0)
{
}
_16bit_compat_adc::~_16bit_compat_adc()
{
    if(adcon0) delete_sfr_register(adcon0);
    if(adcon1) delete_sfr_register(adcon1);
}
void _16bit_v2_adc::create(int nChannels)
{
    adcon0 = new ADCON0_V2(this, "adcon0", "A2D control register");
    adcon1 = new ADCON1_V2(this, "adcon1", "A2D control register");
    adcon2 = new ADCON2_V2(this, "adcon2", "A2D control register");

    RegisterValue porv(0,0);

    add_sfr_register(adcon2,          0xfc0,porv,"adcon2");
    add_sfr_register(adcon1,          0xfc1,porv,"adcon1");
    add_sfr_register(adcon0,          0xfc2,porv,"adcon0");

    adcon0->setAdresLow(&adresl);
    adcon0->setAdres(&adresh);
    adcon0->setAdcon1(adcon1);
    adcon0->setAdcon2(adcon2);
    adcon0->setIntcon(&intcon);
    adcon0->setPir(&pir1);
    adcon0->setChannel_Mask(0xf); // upto 16 channels
    adcon0->setA2DBits(10);

    adcon1->setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 |
                            ADCON1::PCFG2 | ADCON1::PCFG3,0);

    adcon1->setNumberOfChannels(nChannels);
    adcon1->setChanTable(0x1fff, 0x1fff, 0x1fff, 0x0fff,
        0x07ff, 0x03ff, 0x01ff, 0x00ff, 0x007f, 0x003f,
        0x001f, 0x000f, 0x0007, 0x0003, 0x0001, 0x0000);
    adcon1->setVrefHiChannel(3);
    adcon1->setVrefLoChannel(2);

    adcon1->setIOPin(0, &(*m_porta)[0]);
    adcon1->setIOPin(1, &(*m_porta)[1]);
    adcon1->setIOPin(2, &(*m_porta)[2]);
    adcon1->setIOPin(3, &(*m_porta)[3]);
}

_16bit_v2_adc::_16bit_v2_adc(const char *_name, const char *desc)
        : _16bit_processor(_name, desc), adcon0(0), adcon1(0), adcon2(0)
{
}
_16bit_v2_adc::~_16bit_v2_adc()
{
    if(adcon0) delete_sfr_register(adcon0);
    if(adcon1) delete_sfr_register(adcon1);
    if(adcon2) delete_sfr_register(adcon2);
}

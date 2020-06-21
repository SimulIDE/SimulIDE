/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2010 Roy R Rankin

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
#include <string>

#include "config.h"
#include "p18x.h"
#include "pic-ioports.h"
#include "packages.h"
#include "stimuli.h"

/* Config Word defines */
#define MCLRE 	(1<<7)
#define P2BMX 	(1<<5)
#define T3CMX	(1<<4)
#define HFOFST	(1<<3)
#define LPT1OSC (1<<2)
#define CCP3MX 	(1<<2)
#define PBADEN 	(1<<1)
#define CCP2MX 	(1<<0)


/**
 *  A special variant of the Config3H class that includes all the bits that
 *  config register does on PIC18F2x21 and derivatives (including 4620). Note
 *  that the "set" method requires that the parent processor is an instance
 *  of the P18F2x21 class (or a derived variant thereof).
 */
class Config3H_2x21 : public ConfigWord
{
public:
  Config3H_2x21(_16bit_processor *pCpu, uint addr, uint def_val)
    : ConfigWord("CONFIG3H", ~def_val & 0xfff, "Config Reg 3H", pCpu, addr)
  {
    set(def_val);
  }

  virtual void set(int64_t v)
  {
    int64_t i64;
    get(i64);
    int diff = (i64 ^ v) &0xfff;
    Integer::set(v);

    if (m_pCpu)
    {
      P18F2x21 *pCpu21 = (P18F2x21*)m_pCpu;

      if (diff & MCLRE)
        (v & MCLRE) ? m_pCpu->assignMCLRPin(1) : m_pCpu->unassignMCLRPin();
      if ( pCpu21->adcon1 )
      {
        uint pcfg = (v & PBADEN) ? 0 
                           : (ADCON1::PCFG0 | ADCON1::PCFG1 | ADCON1::PCFG2);
        pCpu21->adcon1->por_value=RegisterValue(pcfg,0);
      }
      if ( diff & CCP2MX )
      {
        if ( v & CCP2MX )
          pCpu21->ccp2con.setIOpin(&((*pCpu21->m_portc)[1]));
        else
          pCpu21->ccp2con.setIOpin(&((*pCpu21->m_portb)[3]));
      }
    }
  }

  virtual string toString()
  {
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];
    snprintf(buff, sizeof(buff), "$%04x\n"
	" MCLRE=%d - %s\n"
	" LPT1OSC=%d - Timer1 configured for %s operation\n"
	" PBADEN=%d - PORTB<4:0> pins %s\n"
	" CCP2MX=%d - CCP2 I/O is muxed with %s\n",
	i,
	(i & MCLRE) ? 1:0, (i & MCLRE) ? "Pin is MCLRE" : "Pin is RE3",
	(i & LPT1OSC) ? 1:0, (i & LPT1OSC) ? "low-power" : "higher power",
	(i & PBADEN) ?1:0, 
		(i & PBADEN) ? "analog on Reset" : "digital I/O on reset",
	(i & CCP2MX) ? 1:0, (i & CCP2MX) ? "RC1" : "RB3"
    );
    return string(buff);
  }

};

//----------------------------------------------------------------------
// For only MCLRE in CONFIG3H and using pin 4 (RA5)
//
class Config3H_1x20 : public ConfigWord
{
public:
  Config3H_1x20(_16bit_processor *pCpu, uint addr, uint def_val)
    : ConfigWord("CONFIG3H", ~def_val & 0xfff, "Config Reg 3H", pCpu, addr)
  {
	set(def_val);
  }

  virtual void set(int64_t v)
  {
    int64_t i64;
    get(i64);
    int diff = (i64 ^ v) &0xfff;
    Integer::set(v);

    if (m_pCpu)
    {
	if (diff & MCLRE)
	    (v & MCLRE) ? m_pCpu->assignMCLRPin(4) : m_pCpu->unassignMCLRPin();
    }
  }
    virtual string toString()
  {
    int64_t i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];
    snprintf(buff, sizeof(buff), "$%04x\n"
	" MCLRE=%d - %s\n",
	i,
	(i & MCLRE) ? 1:0, (i & MCLRE) ? "Pin is MCLRE" : "Pin is RA5"
    );
    return string(buff);
  }
};

//========================================================================
//
void P18C2x2::create()
{
  create_iopin_map();
  _16bit_compat_adc::create();
  osccon->value = RegisterValue(0x00,0);
  osccon->por_value = RegisterValue(0x00,0);
  init_pir2(pir2, PIR2v2::TMR3IF);
}

//------------------------------------------------------------------------
void P18C2x2::create_iopin_map()
{
  package = new Package(28);

  if(!package)
    return;

  // Build the links between the I/O Ports and their tris registers.

  //package->assign_pin(1, 0);  // /MCLR
  createMCLRPin(1);

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

  package->assign_pin(8, 0);  // Vss
  package->assign_pin(9, 0);  // OSC1

  package->assign_pin(10, m_porta->addPin(new IO_bi_directional("porta6"),6));

  package->assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));

  package->assign_pin(19, 0);  // Vss
  package->assign_pin(20, 0);  // Vdd

  package->assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  tmr1l.setIOpin(&(*m_portc)[0]);
  ssp.initialize(&pir_set_def,    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // i2c tris port
		SSP_TYPE_MSSP
       );
  set_osc_pin_Number(0,9, NULL);
  set_osc_pin_Number(1,10, &(*m_porta)[6]);
  //1portc.usart = &usart16;
}

P18C2x2::P18C2x2(const char *_name, const char *desc)
  : _16bit_compat_adc(_name,desc)
{
}

//------------------------------------------------------------------------
//
P18C242::P18C242(const char *_name, const char *desc)
  : P18C2x2(_name,desc)
{
}

void P18C242::create()
{
  P18C2x2::create();
}

Processor * P18C242::construct(const char *name)
{
  P18C242 *p = new P18C242(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18C252::P18C252(const char *_name, const char *desc)
  : P18C242(_name,desc)
{
}

void P18C252::create()
{
  P18C242::create();
}

Processor * P18C252::construct(const char *name)
{
  P18C252 *p = new P18C252(name);;

  p->create();
  p->create_invalid_registers();

  return p;
}

//========================================================================
//
void P18C4x2::create()
{
  create_iopin_map();

  _16bit_compat_adc::create();
  osccon->value = RegisterValue(0x00,0);
  osccon->por_value = RegisterValue(0x00,0);
}
//------------------------------------------------------------------------
void P18C4x2::create_iopin_map()
{
  package = new Package(40);

  if(!package)
    return;

  createMCLRPin(1);

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
  package->assign_pin(13, new IOPIN("OSC1"));
  package->assign_pin(14, m_porta->addPin(new IO_bi_directional("porta6"),6));

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


  psp.initialize(&pir_set_def,    // PIR
                m_portd,           // Parallel port
                m_trisd,           // Parallel tris
                m_trise,           // Control tris
                &(*m_porte)[0],    // NOT RD
                &(*m_porte)[1],    // NOT WR
                &(*m_porte)[2]);   // NOT CS

  tmr1l.setIOpin(&(*m_portc)[0]);

  ssp.initialize(&pir_set_def,    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // i2c tris port
		SSP_TYPE_MSSP
       );
  //1portc.ccp1con = &ccp1con;
  //1portc.usart = &usart16;
}

P18C4x2::P18C4x2(const char *_name, const char *desc)
  : _16bit_compat_adc(_name,desc)
{
  m_portd = new PicPSP_PortRegister(this,"portd","",8,0xff);
  m_trisd = new PicTrisRegister(this,"trisd","", (PicPortRegister *)m_portd, false);
  m_latd  = new PicLatchRegister(this,"latd","",m_portd);

  m_porte = new PicPortRegister(this,"porte","",8,0x07);
  m_trise = new PicPSP_TrisRegister(this,"trise","", m_porte, false);
  m_late  = new PicLatchRegister(this,"late","",m_porte);

}
P18C4x2::~P18C4x2()
{
  delete_sfr_register(m_portd);
  delete_sfr_register(m_porte);

  delete_sfr_register(m_latd);
  delete_sfr_register(m_late);

  delete_sfr_register(m_trisd);
  delete_sfr_register(m_trise);
}


void P18C4x2::create_sfr_map()
{
  _16bit_processor::create_sfr_map();

  RegisterValue porv(0,0);

  // Assume this should follow the old behaviour as it's an old chip
  osccon->por_value = porv;

  add_sfr_register(m_portd,       0xf83,porv);
  add_sfr_register(m_porte,       0xf84,porv);

  add_sfr_register(m_latd,        0xf8c,porv);
  add_sfr_register(m_late,        0xf8d,porv);

  add_sfr_register(m_trisd,       0xf95,RegisterValue(0xff,0));
  add_sfr_register(m_trise,       0xf96,RegisterValue(0x07,0));

  // rest of configureation in parent class
  adcon1->setNumberOfChannels(8);
  adcon1->setIOPin(5, &(*m_porte)[0]);
  adcon1->setIOPin(6, &(*m_porte)[1]);
  adcon1->setIOPin(7, &(*m_porte)[2]);
  init_pir2(pir2, PIR2v2::TMR3IF); 
  tmr3l.setIOpin(&(*m_portc)[0]);

  //1 usart16.initialize_16(this,&pir_set_def,&portc);
}

//------------------------------------------------------------------------
//
P18C442::P18C442(const char *_name, const char *desc)
  : P18C4x2(_name,desc)
{
}

void P18C442::create()
{
  P18C4x2::create();

  set_osc_pin_Number(0,13, NULL);
  set_osc_pin_Number(1,14, &(*m_porta)[6]);
}

Processor * P18C442::construct(const char *name)
{
  P18C442 *p = new P18C442(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18C452::P18C452(const char *_name, const char *desc)
  : P18C442(_name,desc)
{
}

void P18C452::create()
{
  P18C442::create();
}

Processor * P18C452::construct(const char *name)
{
  P18C452 *p = new P18C452(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F242::P18F242(const char *_name, const char *desc)
  : P18C242(_name,desc)
{
}

void P18F242::create()
{
  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, false);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);

  P18C242::create();
}

Processor * P18F242::construct(const char *name)
{
  P18F242 *p = new P18F242(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F252::P18F252(const char *_name, const char *desc)
  : P18F242(_name,desc)
{
}

void P18F252::create()
{
  P18F242::create();
}
Processor * P18F252::construct(const char *name)
{
  P18F252 *p = new P18F252(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F442::P18F442(const char *_name, const char *desc)
  : P18C442(_name,desc)
{
}

void P18F442::create()
{
  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, false);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);

  P18C442::create();
}

Processor * P18F442::construct(const char *name)
{
  P18F442 *p = new P18F442(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F248::P18F248(const char *_name, const char *desc)
  : P18F242(_name,desc)
{
}

void P18F248::create()
{
  P18F242::create();
}

Processor * P18F248::construct(const char *name)
{
  P18F248 *p = new P18F248(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F258::P18F258(const char *_name, const char *desc)
  : P18F252(_name,desc)
{
}

void P18F258::create()
{
  P18F252::create();
}

Processor * P18F258::construct(const char *name)
{
  P18F258 *p = new P18F258(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F448::P18F448(const char *_name, const char *desc)
  : P18F442(_name,desc)
{
}

void P18F448::create()
{
  P18F442::create();
}

Processor * P18F448::construct(const char *name)
{
  P18F448 *p = new P18F448(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F452::P18F452(const char *_name, const char *desc)
  : P18F442(_name,desc)
{
}

void P18F452::create()
{
  P18F442::create();
}

Processor * P18F452::construct(const char *name)
{
  P18F452 *p = new P18F452(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F458::P18F458(const char *_name, const char *desc)
  : P18F452(_name,desc)
{
}

void P18F458::create()
{
  P18F452::create();
}

Processor * P18F458::construct(const char *name)
{
  P18F458 *p = new P18F458(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2455::P18F2455(const char *_name, const char *desc)
  : P18F2x21(_name,desc),
      ufrml(this, "ufrml", "USB Frame Number register Low"      ),
      ufrmh(this, "ufrmh", "USB Frame Number register High"     ),
      uir  (this, "uir"  , "USB Interrupt Status register"      ),
      uie  (this, "uie"  , "USB Interrupt Enable register"      ),
      ueir (this, "ueir" , "USB Error Interrupt Status register"),
      ueie (this, "ueie" , "USB Error Interrupt Enable register"),
      ustat(this, "ustat", "USB Transfer Status register"       ),
      ucon (this, "ucon" , "USB Control register"               ),
      uaddr(this, "uaddr", "USB Device Address register"        ),
      ucfg (this, "ucfg" , "USB Configuration register"         ),
      uep0 (this, "uep0" , "USB Endpoint 0 Enable register"     ),
      uep1 (this, "uep1" , "USB Endpoint 1 Enable register"     ),
      uep2 (this, "uep2" , "USB Endpoint 2 Enable register"     ),
      uep3 (this, "uep3" , "USB Endpoint 3 Enable register"     ),
      uep4 (this, "uep4" , "USB Endpoint 4 Enable register"     ),
      uep5 (this, "uep5" , "USB Endpoint 5 Enable register"     ),
      uep6 (this, "uep6" , "USB Endpoint 6 Enable register"     ),
      uep7 (this, "uep7" , "USB Endpoint 7 Enable register"     ),
      uep8 (this, "uep8" , "USB Endpoint 8 Enable register"     ),
      uep9 (this, "uep9" , "USB Endpoint 9 Enable register"     ),
      uep10(this, "uep10", "USB Endpoint 10 Enable register"    ),
      uep11(this, "uep11", "USB Endpoint 11 Enable register"    ),
      uep12(this, "uep12", "USB Endpoint 12 Enable register"    ),
      uep13(this, "uep13", "USB Endpoint 13 Enable register"    ),
      uep14(this, "uep14", "USB Endpoint 14 Enable register"    ),
      uep15(this, "uep15", "USB Endpoint 15 Enable register"    )
{
  cout << "\nP18F2455 does not support USB registers and functionality\n\n";
}

P18F2455::~P18F2455()
{
  remove_sfr_register(&ufrml);
  remove_sfr_register(&ufrmh);
  remove_sfr_register(&uir  );
  remove_sfr_register(&uie  );
  remove_sfr_register(&ueir );
  remove_sfr_register(&ueie );
  remove_sfr_register(&ustat);
  remove_sfr_register(&ucon );
  remove_sfr_register(&uaddr);
  remove_sfr_register(&ucfg );
  remove_sfr_register(&uep0 );
  remove_sfr_register(&uep1 );
  remove_sfr_register(&uep2 );
  remove_sfr_register(&uep3 );
  remove_sfr_register(&uep4 );
  remove_sfr_register(&uep5 );
  remove_sfr_register(&uep6 );
  remove_sfr_register(&uep7 );
  remove_sfr_register(&uep8 );
  remove_sfr_register(&uep9 );
  remove_sfr_register(&uep10);
  remove_sfr_register(&uep11);
  remove_sfr_register(&uep12);
  remove_sfr_register(&uep13);
  remove_sfr_register(&uep14);
  remove_sfr_register(&uep15);
}
void P18F2455::create_sfr_map()
{
  P18F2x21::create_sfr_map();
  package->destroy_pin(14);
  package->assign_pin(14, 0, false);          // Vusb

  /* The MSSP/I2CC pins are different on this chip. */
  ssp.initialize(&pir_set_def,         // PIR
                 &(*m_portb)[1],       // SCK
                 &(*m_porta)[5],       // SS
                 &(*m_portc)[7],       // SDO
                 &(*m_portb)[0],       // SDI
                 m_trisb,              // i2c tris port
                 SSP_TYPE_MSSP
       );
  add_sfr_register(&ufrml,0x0F66, RegisterValue(0,0),"ufrm");
  add_sfr_register(&ufrmh,0X0F67, RegisterValue(0,0));
  add_sfr_register(&uir  ,0x0F68, RegisterValue(0,0));
  add_sfr_register(&uie  ,0x0F69, RegisterValue(0,0));
  add_sfr_register(&ueir ,0x0F6A, RegisterValue(0,0));
  add_sfr_register(&ueie ,0x0F6B, RegisterValue(0,0));
  add_sfr_register(&ustat,0X0F6C, RegisterValue(0,0));
  add_sfr_register(&ucon ,0x0F6D, RegisterValue(0,0));
  add_sfr_register(&uaddr,0X0F6E, RegisterValue(0,0));
  add_sfr_register(&ucfg ,0x0F6F, RegisterValue(0,0));
  add_sfr_register(&uep0 ,0x0F70, RegisterValue(0,0));
  add_sfr_register(&uep1 ,0x0F71, RegisterValue(0,0));
  add_sfr_register(&uep2 ,0x0F72, RegisterValue(0,0));
  add_sfr_register(&uep3 ,0x0F73, RegisterValue(0,0));
  add_sfr_register(&uep4 ,0x0F74, RegisterValue(0,0));
  add_sfr_register(&uep5 ,0x0F75, RegisterValue(0,0));
  add_sfr_register(&uep6 ,0x0F76, RegisterValue(0,0));
  add_sfr_register(&uep7 ,0x0F77, RegisterValue(0,0));
  add_sfr_register(&uep8 ,0x0F78, RegisterValue(0,0));
  add_sfr_register(&uep9 ,0x0F79, RegisterValue(0,0));
  add_sfr_register(&uep10,0x0F7A, RegisterValue(0,0));
  add_sfr_register(&uep11,0x0F7B, RegisterValue(0,0));
  add_sfr_register(&uep12,0x0F7C, RegisterValue(0,0));
  add_sfr_register(&uep13,0x0F7D, RegisterValue(0,0));
  add_sfr_register(&uep14,0x0F7E, RegisterValue(0,0));
  add_sfr_register(&uep15,0x0F7F, RegisterValue(0,0));
}

Processor * P18F2455::construct(const char *name)
{
  P18F2455 *p = new P18F2455(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
// P18F2550	- 28 pin
// 

P18F2550::P18F2550(const char *_name, const char *desc)
  : P18F2x21(_name,desc),
      ufrml(this, "ufrml", "USB Frame Number register Low"      ),
      ufrmh(this, "ufrmh", "USB Frame Number register High"     ),
      uir  (this, "uir"  , "USB Interrupt Status register"      ),
      uie  (this, "uie"  , "USB Interrupt Enable register"      ),
      ueir (this, "ueir" , "USB Error Interrupt Status register"),
      ueie (this, "ueie" , "USB Error Interrupt Enable register"),
      ustat(this, "ustat", "USB Transfer Status register"       ),
      ucon (this, "ucon" , "USB Control register"               ),
      uaddr(this, "uaddr", "USB Device Address register"        ),
      ucfg (this, "ucfg" , "USB Configuration register"         ),
      uep0 (this, "uep0" , "USB Endpoint 0 Enable register"     ),
      uep1 (this, "uep1" , "USB Endpoint 1 Enable register"     ),
      uep2 (this, "uep2" , "USB Endpoint 2 Enable register"     ),
      uep3 (this, "uep3" , "USB Endpoint 3 Enable register"     ),
      uep4 (this, "uep4" , "USB Endpoint 4 Enable register"     ),
      uep5 (this, "uep5" , "USB Endpoint 5 Enable register"     ),
      uep6 (this, "uep6" , "USB Endpoint 6 Enable register"     ),
      uep7 (this, "uep7" , "USB Endpoint 7 Enable register"     ),
      uep8 (this, "uep8" , "USB Endpoint 8 Enable register"     ),
      uep9 (this, "uep9" , "USB Endpoint 9 Enable register"     ),
      uep10(this, "uep10", "USB Endpoint 10 Enable register"    ),
      uep11(this, "uep11", "USB Endpoint 11 Enable register"    ),
      uep12(this, "uep12", "USB Endpoint 12 Enable register"    ),
      uep13(this, "uep13", "USB Endpoint 13 Enable register"    ),
      uep14(this, "uep14", "USB Endpoint 14 Enable register"    ),
      uep15(this, "uep15", "USB Endpoint 15 Enable register"    )
{
  cout << "\nP18F2550 does not support USB registers and functionality\n\n";
}

P18F2550::~P18F2550()
{
  remove_sfr_register(&ufrml);
  remove_sfr_register(&ufrmh);
  remove_sfr_register(&uir  );
  remove_sfr_register(&uie  );
  remove_sfr_register(&ueir );
  remove_sfr_register(&ueie );
  remove_sfr_register(&ustat);
  remove_sfr_register(&ucon );
  remove_sfr_register(&uaddr);
  remove_sfr_register(&ucfg );
  remove_sfr_register(&uep0 );
  remove_sfr_register(&uep1 );
  remove_sfr_register(&uep2 );
  remove_sfr_register(&uep3 );
  remove_sfr_register(&uep4 );
  remove_sfr_register(&uep5 );
  remove_sfr_register(&uep6 );
  remove_sfr_register(&uep7 );
  remove_sfr_register(&uep8 );
  remove_sfr_register(&uep9 );
  remove_sfr_register(&uep10);
  remove_sfr_register(&uep11);
  remove_sfr_register(&uep12);
  remove_sfr_register(&uep13);
  remove_sfr_register(&uep14);
  remove_sfr_register(&uep15);
}
void P18F2550::create_sfr_map()
{
  P18F2x21::create_sfr_map();
  package->destroy_pin(14);
  package->assign_pin(14, 0, false);          // Vusb

  /* The MSSP/I2CC pins are different on this chip. */
  ssp.initialize(&pir_set_def,         // PIR
                 &(*m_portb)[1],       // SCK
                 &(*m_porta)[5],       // SS
                 &(*m_portc)[7],       // SDO
                 &(*m_portb)[0],       // SDI
                 m_trisb,              // i2c tris port
                 SSP_TYPE_MSSP
       );
  add_sfr_register(&ufrml,0x0F66, RegisterValue(0,0),"ufrm");
  add_sfr_register(&ufrmh,0X0F67, RegisterValue(0,0));
  add_sfr_register(&uir  ,0x0F68, RegisterValue(0,0));
  add_sfr_register(&uie  ,0x0F69, RegisterValue(0,0));
  add_sfr_register(&ueir ,0x0F6A, RegisterValue(0,0));
  add_sfr_register(&ueie ,0x0F6B, RegisterValue(0,0));
  add_sfr_register(&ustat,0X0F6C, RegisterValue(0,0));
  add_sfr_register(&ucon ,0x0F6D, RegisterValue(0,0));
  add_sfr_register(&uaddr,0X0F6E, RegisterValue(0,0));
  add_sfr_register(&ucfg ,0x0F6F, RegisterValue(0,0));
  add_sfr_register(&uep0 ,0x0F70, RegisterValue(0,0));
  add_sfr_register(&uep1 ,0x0F71, RegisterValue(0,0));
  add_sfr_register(&uep2 ,0x0F72, RegisterValue(0,0));
  add_sfr_register(&uep3 ,0x0F73, RegisterValue(0,0));
  add_sfr_register(&uep4 ,0x0F74, RegisterValue(0,0));
  add_sfr_register(&uep5 ,0x0F75, RegisterValue(0,0));
  add_sfr_register(&uep6 ,0x0F76, RegisterValue(0,0));
  add_sfr_register(&uep7 ,0x0F77, RegisterValue(0,0));
  add_sfr_register(&uep8 ,0x0F78, RegisterValue(0,0));
  add_sfr_register(&uep9 ,0x0F79, RegisterValue(0,0));
  add_sfr_register(&uep10,0x0F7A, RegisterValue(0,0));
  add_sfr_register(&uep11,0x0F7B, RegisterValue(0,0));
  add_sfr_register(&uep12,0x0F7C, RegisterValue(0,0));
  add_sfr_register(&uep13,0x0F7D, RegisterValue(0,0));
  add_sfr_register(&uep14,0x0F7E, RegisterValue(0,0));
  add_sfr_register(&uep15,0x0F7F, RegisterValue(0,0));
}

Processor * P18F2550::construct(const char *name)
{
  P18F2550 *p = new P18F2550(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
// P18F4455
// 

P18F4455::P18F4455(const char *_name, const char *desc)
  : P18F4x21(_name,desc),
      ufrml(this, "ufrml", "USB Frame Number register Low"      ),
      ufrmh(this, "ufrmh", "USB Frame Number register High"     ),
      uir  (this, "uir"  , "USB Interrupt Status register"      ),
      uie  (this, "uie"  , "USB Interrupt Enable register"      ),
      ueir (this, "ueir" , "USB Error Interrupt Status register"),
      ueie (this, "ueie" , "USB Error Interrupt Enable register"),
      ustat(this, "ustat", "USB Transfer Status register"       ),
      ucon (this, "ucon" , "USB Control register"               ),
      uaddr(this, "uaddr", "USB Device Address register"        ),
      ucfg (this, "ucfg" , "USB Configuration register"         ),
      uep0 (this, "uep0" , "USB Endpoint 0 Enable register"     ),
      uep1 (this, "uep1" , "USB Endpoint 1 Enable register"     ),
      uep2 (this, "uep2" , "USB Endpoint 2 Enable register"     ),
      uep3 (this, "uep3" , "USB Endpoint 3 Enable register"     ),
      uep4 (this, "uep4" , "USB Endpoint 4 Enable register"     ),
      uep5 (this, "uep5" , "USB Endpoint 5 Enable register"     ),
      uep6 (this, "uep6" , "USB Endpoint 6 Enable register"     ),
      uep7 (this, "uep7" , "USB Endpoint 7 Enable register"     ),
      uep8 (this, "uep8" , "USB Endpoint 8 Enable register"     ),
      uep9 (this, "uep9" , "USB Endpoint 9 Enable register"     ),
      uep10(this, "uep10", "USB Endpoint 10 Enable register"    ),
      uep11(this, "uep11", "USB Endpoint 11 Enable register"    ),
      uep12(this, "uep12", "USB Endpoint 12 Enable register"    ),
      uep13(this, "uep13", "USB Endpoint 13 Enable register"    ),
      uep14(this, "uep14", "USB Endpoint 14 Enable register"    ),
      uep15(this, "uep15", "USB Endpoint 15 Enable register"    ),
      sppcon(this, "sppcon", "Streaming Parallel port control register"),
      sppcfg(this, "sppcfg", "Streaming Parallel port configuration register"),
      sppeps(this, "sppeps", "SPP ENDPOINT ADDRESS AND STATUS REGISTER"),
      sppdata(this, "sppdata", "Streaming Parallel port data register")

{
  cout << "\nP18F4455 does not support USB registers and functionality\n\n";
}

P18F4455::~P18F4455()
{
  remove_sfr_register(&ufrml);
  remove_sfr_register(&ufrmh);
  remove_sfr_register(&uir  );
  remove_sfr_register(&uie  );
  remove_sfr_register(&ueir );
  remove_sfr_register(&ueie );
  remove_sfr_register(&ustat);
  remove_sfr_register(&ucon );
  remove_sfr_register(&uaddr);
  remove_sfr_register(&ucfg );
  remove_sfr_register(&uep0 );
  remove_sfr_register(&uep1 );
  remove_sfr_register(&uep2 );
  remove_sfr_register(&uep3 );
  remove_sfr_register(&uep4 );
  remove_sfr_register(&uep5 );
  remove_sfr_register(&uep6 );
  remove_sfr_register(&uep7 );
  remove_sfr_register(&uep8 );
  remove_sfr_register(&uep9 );
  remove_sfr_register(&uep10);
  remove_sfr_register(&uep11);
  remove_sfr_register(&uep12);
  remove_sfr_register(&uep13);
  remove_sfr_register(&uep14);
  remove_sfr_register(&uep15);
  remove_sfr_register(&sppcon);
  remove_sfr_register(&sppcfg);
  remove_sfr_register(&sppeps);
  remove_sfr_register(&sppdata);
}

void P18F4455::create()
{
  P18F4x21::create();

  package->destroy_pin(18);
  package->assign_pin(18, 0, false);          // Vusb

  /* The MSSP/I2CC pins are different on this chip. */
  ssp.initialize(&pir_set_def,         // PIR
                 &(*m_portb)[1],       // SCK
                 &(*m_porta)[5],       // SS
                 &(*m_portc)[7],       // SDO
                 &(*m_portb)[0],       // SDI
                 m_trisb,              // i2c tris port
                 SSP_TYPE_MSSP
       );

  // RP: RRR commented out comparator.cmcon.set_eccpas(&eccpas);  ??
  // Streaming Parallel port (SPP)
  spp.initialize(&pir_set_def,         // PIR
		m_portd,		//Parallel port
		m_trisd,		//Parallel port tristate register
		&sppcon,
		&sppcfg,
		&sppeps,
		&sppdata,
		&(*m_porte)[0],		// CLK1SPP
		&(*m_porte)[1],		// CLK2SPP
		&(*m_porte)[2],		// OESPP
		&(*m_portb)[4]		// CSSPP
	);

  add_sfr_register(&sppdata,0x0F62, RegisterValue(0,0));
  add_sfr_register(&sppcfg,0x0F63, RegisterValue(0,0));
  add_sfr_register(&sppeps,0x0F64, RegisterValue(0,0));
  add_sfr_register(&sppcon,0x0F65, RegisterValue(0,0));
  add_sfr_register(&ufrml,0x0F66, RegisterValue(0,0),"ufrm");
  add_sfr_register(&ufrmh,0X0F67, RegisterValue(0,0));
  add_sfr_register(&uir  ,0x0F68, RegisterValue(0,0));
  add_sfr_register(&uie  ,0x0F69, RegisterValue(0,0));
  add_sfr_register(&ueir ,0x0F6A, RegisterValue(0,0));
  add_sfr_register(&ueie ,0x0F6B, RegisterValue(0,0));
  add_sfr_register(&ustat,0X0F6C, RegisterValue(0,0));
  add_sfr_register(&ucon ,0x0F6D, RegisterValue(0,0));
  add_sfr_register(&uaddr,0X0F6E, RegisterValue(0,0));
  add_sfr_register(&ucfg ,0x0F6F, RegisterValue(0,0));
  add_sfr_register(&uep0 ,0x0F70, RegisterValue(0,0));
  add_sfr_register(&uep1 ,0x0F71, RegisterValue(0,0));
  add_sfr_register(&uep2 ,0x0F72, RegisterValue(0,0));
  add_sfr_register(&uep3 ,0x0F73, RegisterValue(0,0));
  add_sfr_register(&uep4 ,0x0F74, RegisterValue(0,0));
  add_sfr_register(&uep5 ,0x0F75, RegisterValue(0,0));
  add_sfr_register(&uep6 ,0x0F76, RegisterValue(0,0));
  add_sfr_register(&uep7 ,0x0F77, RegisterValue(0,0));
  add_sfr_register(&uep8 ,0x0F78, RegisterValue(0,0));
  add_sfr_register(&uep9 ,0x0F79, RegisterValue(0,0));
  add_sfr_register(&uep10,0x0F7A, RegisterValue(0,0));
  add_sfr_register(&uep11,0x0F7B, RegisterValue(0,0));
  add_sfr_register(&uep12,0x0F7C, RegisterValue(0,0));
  add_sfr_register(&uep13,0x0F7D, RegisterValue(0,0));
  add_sfr_register(&uep14,0x0F7E, RegisterValue(0,0));
  add_sfr_register(&uep15,0x0F7F, RegisterValue(0,0));

  // Initialize the register cross linkages
  init_pir2(pir2, PIR2v4::TMR3IF);

  //new InterruptSource(pir2, PIR2v4::USBIF);
}

Processor * P18F4455::construct(const char *name)
{
  P18F4455 *p = new P18F4455(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F4550::P18F4550(const char *_name, const char *desc)
  : P18F4x21(_name,desc),
      ufrml(this, "ufrml", "USB Frame Number register Low"      ),
      ufrmh(this, "ufrmh", "USB Frame Number register High"     ),
      uir  (this, "uir"  , "USB Interrupt Status register"      ),
      uie  (this, "uie"  , "USB Interrupt Enable register"      ),
      ueir (this, "ueir" , "USB Error Interrupt Status register"),
      ueie (this, "ueie" , "USB Error Interrupt Enable register"),
      ustat(this, "ustat", "USB Transfer Status register"       ),
      ucon (this, "ucon" , "USB Control register"               ),
      uaddr(this, "uaddr", "USB Device Address register"        ),
      ucfg (this, "ucfg" , "USB Configuration register"         ),
      uep0 (this, "uep0" , "USB Endpoint 0 Enable register"     ),
      uep1 (this, "uep1" , "USB Endpoint 1 Enable register"     ),
      uep2 (this, "uep2" , "USB Endpoint 2 Enable register"     ),
      uep3 (this, "uep3" , "USB Endpoint 3 Enable register"     ),
      uep4 (this, "uep4" , "USB Endpoint 4 Enable register"     ),
      uep5 (this, "uep5" , "USB Endpoint 5 Enable register"     ),
      uep6 (this, "uep6" , "USB Endpoint 6 Enable register"     ),
      uep7 (this, "uep7" , "USB Endpoint 7 Enable register"     ),
      uep8 (this, "uep8" , "USB Endpoint 8 Enable register"     ),
      uep9 (this, "uep9" , "USB Endpoint 9 Enable register"     ),
      uep10(this, "uep10", "USB Endpoint 10 Enable register"    ),
      uep11(this, "uep11", "USB Endpoint 11 Enable register"    ),
      uep12(this, "uep12", "USB Endpoint 12 Enable register"    ),
      uep13(this, "uep13", "USB Endpoint 13 Enable register"    ),
      uep14(this, "uep14", "USB Endpoint 14 Enable register"    ),
      uep15(this, "uep15", "USB Endpoint 15 Enable register"    ),
      sppcon(this, "sppcon", "Streaming Parallel port control register"),
      sppcfg(this, "sppcfg", "Streaming Parallel port configuration register"),
      sppeps(this, "sppeps", "SPP ENDPOINT ADDRESS AND STATUS REGISTER"),
      sppdata(this, "sppdata", "Streaming Parallel port data register")

{
  cout << "\nP18F4550 does not support USB registers and functionality\n\n";
}

P18F4550::~P18F4550()
{
  remove_sfr_register(&ufrml);
  remove_sfr_register(&ufrmh);
  remove_sfr_register(&uir  );
  remove_sfr_register(&uie  );
  remove_sfr_register(&ueir );
  remove_sfr_register(&ueie );
  remove_sfr_register(&ustat);
  remove_sfr_register(&ucon );
  remove_sfr_register(&uaddr);
  remove_sfr_register(&ucfg );
  remove_sfr_register(&uep0 );
  remove_sfr_register(&uep1 );
  remove_sfr_register(&uep2 );
  remove_sfr_register(&uep3 );
  remove_sfr_register(&uep4 );
  remove_sfr_register(&uep5 );
  remove_sfr_register(&uep6 );
  remove_sfr_register(&uep7 );
  remove_sfr_register(&uep8 );
  remove_sfr_register(&uep9 );
  remove_sfr_register(&uep10);
  remove_sfr_register(&uep11);
  remove_sfr_register(&uep12);
  remove_sfr_register(&uep13);
  remove_sfr_register(&uep14);
  remove_sfr_register(&uep15);
  remove_sfr_register(&sppcon);
  remove_sfr_register(&sppcfg);
  remove_sfr_register(&sppeps);
  remove_sfr_register(&sppdata);
}

void P18F4550::create()
{
  P18F4x21::create();

  package->destroy_pin(18);
  package->assign_pin(18, 0, false);          // Vusb

  /* The MSSP/I2CC pins are different on this chip. */
  ssp.initialize(&pir_set_def,         // PIR
                 &(*m_portb)[1],       // SCK
                 &(*m_porta)[5],       // SS
                 &(*m_portc)[7],       // SDO
                 &(*m_portb)[0],       // SDI
                 m_trisb,              // i2c tris port
                 SSP_TYPE_MSSP
       );

  // Streaming Parallel port (SPP)
  spp.initialize(&pir_set_def,         // PIR
		m_portd,		//Parallel port
		m_trisd,		//Parallel port tristate register
		&sppcon,
		&sppcfg,
		&sppeps,
		&sppdata,
		&(*m_porte)[0],		// CLK1SPP
		&(*m_porte)[1],		// CLK2SPP
		&(*m_porte)[2],		// OESPP
		&(*m_portb)[4]		// CSSPP
	);

  // RP: RRR commented out comparator.cmcon.set_eccpas(&eccpas);  ??
  add_sfr_register(&sppdata,0x0F62, RegisterValue(0,0));
  add_sfr_register(&sppcfg,0x0F63, RegisterValue(0,0));
  add_sfr_register(&sppeps,0x0F64, RegisterValue(0,0));
  add_sfr_register(&sppcon,0x0F65, RegisterValue(0,0));
  add_sfr_register(&ufrml,0x0F66, RegisterValue(0,0),"ufrm");
  add_sfr_register(&ufrmh,0X0F67, RegisterValue(0,0));
  add_sfr_register(&uir  ,0x0F68, RegisterValue(0,0));
  add_sfr_register(&uie  ,0x0F69, RegisterValue(0,0));
  add_sfr_register(&ueir ,0x0F6A, RegisterValue(0,0));
  add_sfr_register(&ueie ,0x0F6B, RegisterValue(0,0));
  add_sfr_register(&ustat,0X0F6C, RegisterValue(0,0));
  add_sfr_register(&ucon ,0x0F6D, RegisterValue(0,0));
  add_sfr_register(&uaddr,0X0F6E, RegisterValue(0,0));
  add_sfr_register(&ucfg ,0x0F6F, RegisterValue(0,0));
  add_sfr_register(&uep0 ,0x0F70, RegisterValue(0,0));
  add_sfr_register(&uep1 ,0x0F71, RegisterValue(0,0));
  add_sfr_register(&uep2 ,0x0F72, RegisterValue(0,0));
  add_sfr_register(&uep3 ,0x0F73, RegisterValue(0,0));
  add_sfr_register(&uep4 ,0x0F74, RegisterValue(0,0));
  add_sfr_register(&uep5 ,0x0F75, RegisterValue(0,0));
  add_sfr_register(&uep6 ,0x0F76, RegisterValue(0,0));
  add_sfr_register(&uep7 ,0x0F77, RegisterValue(0,0));
  add_sfr_register(&uep8 ,0x0F78, RegisterValue(0,0));
  add_sfr_register(&uep9 ,0x0F79, RegisterValue(0,0));
  add_sfr_register(&uep10,0x0F7A, RegisterValue(0,0));
  add_sfr_register(&uep11,0x0F7B, RegisterValue(0,0));
  add_sfr_register(&uep12,0x0F7C, RegisterValue(0,0));
  add_sfr_register(&uep13,0x0F7D, RegisterValue(0,0));
  add_sfr_register(&uep14,0x0F7E, RegisterValue(0,0));
  add_sfr_register(&uep15,0x0F7F, RegisterValue(0,0));

  // Initialize the register cross linkages

  //new InterruptSource(pir2, PIR2v4::USBIF);
}

Processor * P18F4550::construct(const char *name)
{
  P18F4550 *p = new P18F4550(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
Processor * P18F1220::construct(const char *name)
{
  P18F1220 *p = new P18F1220(name);
  p->create();
  p->create_invalid_registers();

  return p;
}

void P18F1220::create()
{
  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, false);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);

  create_iopin_map();

  _16bit_processor::create();
  _16bit_v2_adc::create(7);
  osccon->value = RegisterValue(0x00,0);
  osccon->por_value = RegisterValue(0x00,0);
  osccon->has_iofs_bit = true;
  usart.txsta.setIOpin(&(*m_portb)[1]);
  usart.rcsta.setIOpin(&(*m_portb)[4]);
  adcon1->setIOPin(4, &(*m_portb)[0]);
  adcon1->setIOPin(5, &(*m_portb)[1]);
  adcon1->setIOPin(6, &(*m_portb)[4]);
  adcon1->setValidCfgBits(0x7f, 0);
  adcon0->setChannel_Mask(0x7);
  adcon1->setAdcon0(adcon0);	// VCFG0, VCFG1 in adcon0
  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(&ssp.sspcon);
  remove_sfr_register(&ssp.sspstat);
  remove_sfr_register(&ssp.sspadd);
  remove_sfr_register(&ssp.sspbuf);

  add_sfr_register(&osctune,      0xf9b,RegisterValue(0,0));
  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);


  set_osc_pin_Number(0,16, &(*m_porta)[7]);
  set_osc_pin_Number(1,15, &(*m_porta)[6]);
  m_configMemory->addConfigWord(CONFIG1H-CONFIG1L,new Config1H_4bits(this, CONFIG1H, 0xcf));
  m_configMemory->addConfigWord(CONFIG3H-CONFIG1L,new Config3H_1x20(this, CONFIG3H, 0x80));

  add_sfr_register(&usart.spbrgh,   0xfb0,RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.baudcon,  0xfaa,RegisterValue(0,0),"baudctl");
  usart.set_eusart(true);

  add_sfr_register(&pwm1con, 0xfb7, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0xfb6, RegisterValue(0,0));
  ccp1con.setBitMask(0xff);
  ccp1con.setCrosslinks(&ccpr1l, &pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  eccpas.setIOpin(&(*m_portb)[1], &(*m_portb)[2], &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setIOpin(&((*m_portb)[3]), &((*m_portb)[2]), &((*m_portb)[6]), &((*m_portb)[7]));
  init_pir2(pir2, PIR2v2::TMR3IF);
  tmr3l.setIOpin(&(*m_portb)[6]);
}
//------------------------------------------------------------------------
void P18F1220::create_iopin_map()
{
  package = new Package(18);

  if(!package)
    return;

  package->assign_pin( 1, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 6, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta4"),4));
  package->assign_pin( 4, m_porta->addPin(new IO_open_collector("porta5"),5));
  package->assign_pin(15, m_porta->addPin(new IO_bi_directional("porta6"),6));
  package->assign_pin(16, m_porta->addPin(new IO_bi_directional("porta7"),7));

  package->assign_pin( 8, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin( 9, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(17, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(18, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  package->assign_pin(5, 0);
  package->assign_pin(14, 0);
}

P18F1220::P18F1220(const char *_name, const char *desc)
  : _16bit_v2_adc(_name,desc),
    osctune(this, "osctune", "OSC Tune"),
    eccpas(this, "eccpas", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register")
{
}

P18F1220::~P18F1220()
{
  remove_sfr_register(&usart.spbrgh);
  remove_sfr_register(&usart.baudcon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&eccpas);
}

void P18F1220::osc_mode(uint value)
{
  IOPIN *m_pin;
  uint pin_Number =  get_osc_pin_Number(0);
  uint fosc = value & (FOSC3 | FOSC2 | FOSC1 | FOSC0);
  
 if (osccon)
  {
      osccon->set_config_irc(fosc >= 8 && fosc <= 9);
      osccon->set_config_xosc(fosc > 9 || fosc < 3 || fosc == 6);
      osccon->set_config_ieso(value & IESO);
  }

  value &= (FOSC3 | FOSC2 | FOSC1 | FOSC0);
  set_int_osc(false);
  if (pin_Number < 253)
  {
	m_pin = package->get_pin(pin_Number);
	if (value == 8 || value == 9)
	{
	    clr_clk_pin(pin_Number, get_osc_PinMonitor(0), 
		m_porta, m_trisa, m_lata);
	    set_int_osc(true);
	}
	else
	{
	    set_clk_pin(pin_Number, get_osc_PinMonitor(0), "OSC1", true,
		 m_porta, m_trisa, m_lata);
	}
  }
  if ( (pin_Number =  get_osc_pin_Number(1)) < 253 &&
	(m_pin = package->get_pin(pin_Number)))
  {
	pll_factor = 0;
	switch(value)
	{
	case 6:
	    pll_factor = 2;
	case 0:
	case 1:
	case 2:
	    set_clk_pin(pin_Number, get_osc_PinMonitor(1), "OSC2", true,
		m_porta, m_trisa, m_lata);
	    break;

	case 4:
	case 9:
	case 12:
	case 13:
	case 14:
	case 15:
	    cout << "CLKO not simulated\n";
	    set_clk_pin(pin_Number, get_osc_PinMonitor(1) , "CLKO", false,
		m_porta, m_trisa, m_lata);
	    break;

	default:
	    clr_clk_pin(pin_Number, get_osc_PinMonitor(1),
		m_porta, m_trisa, m_lata);
	    break;
	}
  }
  
}

//------------------------------------------------------------------------
//
P18F1320::P18F1320(const char *_name, const char *desc)
  : P18F1220(_name,desc)
{
}

void P18F1320::create()
{
  P18F1220::create();
}

Processor * P18F1320::construct(const char *name)
{
  P18F1320 *p = new P18F1320(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//========================================================================
//
void P18F2x21::create()
{
  delete pir2;
  pir2 = (PIR2v2 *)(new PIR2v4(this, "pir2" , "Peripheral Interrupt Register",0,0  ));

  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, false);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);
  create_iopin_map();

  _16bit_processor::create();

  m_configMemory->addConfigWord(CONFIG3H-CONFIG1L,new Config3H_2x21(this, CONFIG3H, 0x83));
  m_configMemory->addConfigWord(CONFIG1H-CONFIG1L,new Config1H_4bits(this, CONFIG1H, 0x07));

  set_osc_pin_Number(0, 9, &(*m_porta)[7]);
  set_osc_pin_Number(1,10, &(*m_porta)[6]);



  /// @bug registers not present on 28 pin according to table 5-1 of the
  /// data sheet, but bit-restricted according to section 16.4.7
  add_sfr_register(&pwm1con, 0xfb7, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0xfb6, RegisterValue(0,0));  

  eccpas.setBitMask(0xfc);
  eccpas.setIOpin(0, 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  comparator.cmcon.set_eccpas(&eccpas);
  ccp1con.setBitMask(0x3f);
  ccp1con.setCrosslinks(&ccpr1l, &pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setIOpin(&((*m_portc)[2]), 0, 0, 0);
  pwm1con.setBitMask(0x80);
}

//------------------------------------------------------------------------
void P18F2x21::create_iopin_map()
{
  package = new Package(28);

  if(!package)
    return;

  // Build the links between the I/O Ports and their tris registers.

  package->assign_pin( 1, m_porte->addPin(new IO_bi_directional("porte3"),3));

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));  // %%%FIXME - is this O/C ?
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));



  package->assign_pin(8, 0);  // Vss
  package->assign_pin(9, m_porta->addPin(new IO_bi_directional("porta7"),7));  // OSC1

  package->assign_pin(10, m_porta->addPin(new IO_bi_directional("porta6"),6));

  package->assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));

  package->assign_pin(19, 0);  // Vss
  package->assign_pin(20, 0);  // Vdd

  package->assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  tmr1l.setIOpin(&(*m_portc)[0]);
  ssp.initialize(&pir_set_def,    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // i2c tris port
		SSP_TYPE_MSSP
       );


  //1portc.usart = &usart16;
}

P18F2x21::P18F2x21(const char *_name, const char *desc)
  : _16bit_v2_adc(_name,desc),
    eccpas(this, "eccp1as", "ECCP Auto-Shutdown Control Register"),
    pwm1con(this, "eccp1del", "Enhanced PWM Control Register"),
    osctune(this, "osctune", "OSC Tune"),
    comparator(this)
{
    m_porte = new PicPortRegister(this,"porte","",8,0x08);
    // No TRIS register for port E on 28-pin devices
}

P18F2x21::~P18F2x21()
{
    delete_sfr_register(m_porte);
    remove_sfr_register(&usart.spbrgh);
    remove_sfr_register(&usart.baudcon);
    remove_sfr_register(&osctune);
    remove_sfr_register(&comparator.cmcon);
    remove_sfr_register(&comparator.vrcon);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&eccpas);
}

void P18F2x21::create_sfr_map()
{
  _16bit_processor::create_sfr_map();
  _16bit_v2_adc::create(13);

  RegisterValue porv(0,0);

  add_sfr_register(m_porte,       0xf84,porv);

  adcon1->setIOPin(4, &(*m_porta)[5]);
/*  Not on 28 pin processors
  adcon1->setIOPin(5, &(*m_porte)[0]);
  adcon1->setIOPin(6, &(*m_porte)[1]);
  adcon1->setIOPin(7, &(*m_porte)[2]);
*/
  adcon1->setIOPin(8, &(*m_portb)[2]);
  adcon1->setIOPin(9, &(*m_portb)[3]);
  adcon1->setIOPin(10, &(*m_portb)[1]);
  adcon1->setIOPin(11, &(*m_portb)[4]);
  adcon1->setIOPin(12, &(*m_portb)[0]);

  add_sfr_register(&osctune,      0xf9b,porv);
  osccon->set_osctune(&osctune);
  osccon->has_iofs_bit = true;
  osctune.set_osccon(osccon);

  // rest of configuration in parent class

  // Link the comparator and voltage ref to porta
  comparator.initialize(&pir_set_def, &(*m_porta)[2], &(*m_porta)[0], 
	&(*m_porta)[1], &(*m_porta)[2], &(*m_porta)[3], &(*m_porta)[4],
	&(*m_porta)[5]);

  comparator.cmcon.set_configuration(1, 0, AN0, AN3, AN0, AN3, ZERO);
  comparator.cmcon.set_configuration(2, 0, AN1, AN2, AN1, AN2, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 1, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(1, 2, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 2, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 3, AN1, AN2, AN1, AN2, OUT1);
  comparator.cmcon.set_configuration(1, 4, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN1, AN3, AN1, AN3, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 5, AN1, AN3, AN1, AN3, OUT1);
  comparator.cmcon.set_configuration(1, 6, AN0, VREF, AN3, VREF, NO_OUT);
  comparator.cmcon.set_configuration(2, 6, AN1, VREF, AN2, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0xfb4, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0xfb5, RegisterValue(0,0),"cvrcon");

  ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v2::CCP2IF, &tmr2);
//  ccp2con.setIOpin(&((*m_portc)[1]));     // handled by Config3H_2x21
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  //1 usart16.initialize_16(this,&pir_set_def,&portc);
  add_sfr_register(&usart.spbrgh,   0xfb0,porv,"spbrgh");
  add_sfr_register(&usart.baudcon,  0xfb8,porv,"baudcon");
  usart.set_eusart(true);
  init_pir2(pir2, PIR2v4::TMR3IF); 
  tmr3l.setIOpin(&(*m_portc)[0]);
}

void P18F2x21::osc_mode(uint value)
{
  IOPIN *m_pin;
  uint pin_Number =  get_osc_pin_Number(0);
  uint fosc = value & (FOSC3 | FOSC2 | FOSC1 | FOSC0);

  if (osccon)
  {
      osccon->set_config_irc(fosc >= 8 && fosc <= 11);
      osccon->set_config_xosc(fosc > 11 || fosc < 4);
      osccon->set_config_ieso(value & IESO);
  }
  
  value &= (FOSC3 | FOSC2 | FOSC1 | FOSC0);
  set_int_osc(false);
  if (pin_Number < 253)
  {
	m_pin = package->get_pin(pin_Number);
	if (value == 8 || value == 9)	// internal RC clock
	{
	    clr_clk_pin(pin_Number, get_osc_PinMonitor(0),
		m_porta, m_trisa, m_lata);
	    set_int_osc(true);
	}
	else
        {
	    set_clk_pin(pin_Number, get_osc_PinMonitor(0), "OSC1", true,
		m_porta, m_trisa, m_lata);
	    set_int_osc(false);
	}
  }
  if ( (pin_Number =  get_osc_pin_Number(1)) < 253 &&
	(m_pin = package->get_pin(pin_Number)))
  {
	pll_factor = 0;
	switch(value)
	{
	case 6:
	    pll_factor = 2;
	case 0:
	case 1:
	case 2:
	    set_clk_pin(pin_Number, get_osc_PinMonitor(1), "OSC2", true,
		m_porta, m_trisa, m_lata);
	    break;

	case 3:
	case 4:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	    cout << "CLKO not simulated\n";
	    set_clk_pin(pin_Number, get_osc_PinMonitor(1) , "CLKO", false,
		m_porta, m_trisa, m_lata);
	    break;

	default:
	    clr_clk_pin(pin_Number, get_osc_PinMonitor(1),
		m_porta, m_trisa, m_lata);
	    break;
	}
  }
  
}

//------------------------------------------------------------------------
//
P18F2221::P18F2221(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2221::construct(const char *name)
{
  P18F2221 *p = new P18F2221(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2321::P18F2321(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2321::construct(const char *name)
{
  P18F2321 *p = new P18F2321(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2420::P18F2420(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2420::construct(const char *name)
{
  P18F2420 *p = new P18F2420(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2520::P18F2520(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2520::construct(const char *name)
{
  P18F2520 *p = new P18F2520(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2525::P18F2525(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2525::construct(const char *name)
{
  P18F2525 *p = new P18F2525(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F2620::P18F2620(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
}

Processor * P18F2620::construct(const char *name)
{
  P18F2620 *p = new P18F2620(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//=======================================================================
//
void P18F4x21::create()
{
  delete pir2;
  pir2 = (PIR2v2 *)(new PIR2v4(this, "pir2" , "Peripheral Interrupt Register",0,0  ));

  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, false);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);

  create_iopin_map();

  _16bit_processor::create();

  m_configMemory->addConfigWord(CONFIG3H-CONFIG1L,new Config3H_2x21(this, CONFIG3H, 0x83));
  m_configMemory->addConfigWord(CONFIG1H-CONFIG1L,new Config1H_4bits(this, CONFIG1H, 0x07));

  set_osc_pin_Number(0, 13, &(*m_porta)[7]);
  set_osc_pin_Number(1,14, &(*m_porta)[6]);

  add_sfr_register(&pwm1con, 0xfb7, RegisterValue(0,0));
  add_sfr_register(&eccpas, 0xfb6, RegisterValue(0,0));
  eccpas.setIOpin(0, 0, &(*m_portb)[0]);
  eccpas.link_registers(&pwm1con, &ccp1con);
  comparator.cmcon.set_eccpas(&eccpas);
  ccp1con.setBitMask(0xff);
  ccp1con.setCrosslinks(&ccpr1l, &pir1, PIR1v2::CCP1IF, &tmr2, &eccpas);
  ccp1con.pwm1con = &pwm1con;
  ccp1con.setIOpin(&((*m_portc)[2]), &((*m_portd)[5]), &((*m_portd)[6]), &((*m_portd)[7]));

}

//------------------------------------------------------------------------
void P18F4x21::create_iopin_map()
{
  package = new Package(40);

  if(!package)
    return;

  // Build the links between the I/O Ports and their tris registers.

  package->assign_pin( 1, m_porte->addPin(new IO_bi_directional("porte3"),3));

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

  package->assign_pin( 8, m_porte->addPin(new IO_bi_directional("porte0"),0));
  package->assign_pin( 9, m_porte->addPin(new IO_bi_directional("porte1"),1));
  package->assign_pin(10, m_porte->addPin(new IO_bi_directional("porte2"),2));


  package->assign_pin(11, 0);   // Vdd
  package->assign_pin(12, 0);   // Vss
  package->assign_pin(13, m_porta->addPin(new IO_bi_directional("porta7"),7));
  package->assign_pin(14, m_porta->addPin(new IO_bi_directional("porta6"),6));

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

  package->assign_pin(31, 0);   // Vss
  package->assign_pin(32, 0);   // Vdd

  package->assign_pin(33, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(34, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(35, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(36, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(37, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(38, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(39, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(40, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  tmr1l.setIOpin(&(*m_portc)[0]);

  ssp.initialize(&pir_set_def,    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // i2c tris port
		SSP_TYPE_MSSP
       );

  //1portc.ccp1con = &ccp1con;
  //1portc.usart = &usart16;
}

P18F4x21::P18F4x21(const char *_name, const char *desc)
  : P18F2x21(_name,desc)
{
  m_portd = new PicPSP_PortRegister(this,"portd","",8,0xFF);
  m_trisd = new PicTrisRegister(this,"trisd","", (PicPortRegister *)m_portd, false);
  m_latd  = new PicLatchRegister(this,"latd","",m_portd);

//  m_porte = new PicPortRegister(this,"porte","",8,0x07);
  m_porte->setEnableMask(0x07);     // It's been created by the P18F2x21 constructor, but with the wrong enables
  m_trise = new PicPSP_TrisRegister(this,"trise","", m_porte, false);
  m_late  = new PicLatchRegister(this,"late","",m_porte);

}
P18F4x21::~P18F4x21()
{
  delete_sfr_register(m_portd);
  delete_sfr_register(m_trisd);
  delete_sfr_register(m_latd);
  delete_sfr_register(m_trise);
  delete_sfr_register(m_late);
  remove_sfr_register(&pwm1con);
  remove_sfr_register(&eccpas);
}

void P18F4x21::create_sfr_map()
{
  _16bit_processor::create_sfr_map();
  _16bit_v2_adc::create(13);

  RegisterValue porv(0,0);

  add_sfr_register(m_portd,       0xf83,porv);
  add_sfr_register(m_porte,       0xf84,porv);

  add_sfr_register(m_latd,        0xf8c,porv);
  add_sfr_register(m_late,        0xf8d,porv);

  add_sfr_register(m_trisd,       0xf95,RegisterValue(0xff,0));
  add_sfr_register(m_trise,       0xf96,RegisterValue(0x07,0));

  add_sfr_register(&osctune,      0xf9b,porv);
  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);

  adcon1->setIOPin(4, &(*m_porta)[5]);
  adcon1->setIOPin(5, &(*m_porte)[0]);
  adcon1->setIOPin(6, &(*m_porte)[1]);
  adcon1->setIOPin(7, &(*m_porte)[2]);
  adcon1->setIOPin(8, &(*m_portb)[2]);
  adcon1->setIOPin(9, &(*m_portb)[3]);
  adcon1->setIOPin(10, &(*m_portb)[1]);
  adcon1->setIOPin(11, &(*m_portb)[4]);
  adcon1->setIOPin(12, &(*m_portb)[0]);
/*
  adcon1->setChanTable(0x1ff, 0x1fff, 0x1fff, 0x0fff,
	0x07ff, 0x03ff, 0x01ff, 0x00ff, 0x007f, 0x003f,
	0x001f, 0x000f, 0x0007, 0x0003, 0x0001, 0x0000);
  adcon1->setVrefHiChannel(3);
  adcon1->setVrefLoChannel(2);
*/





  // Link the comparator and voltage ref to porta
  comparator.initialize(&pir_set_def, &(*m_porta)[2], &(*m_porta)[0], 
	&(*m_porta)[1], &(*m_porta)[2], &(*m_porta)[3], &(*m_porta)[4],
	&(*m_porta)[5]);

  comparator.cmcon.set_configuration(1, 0, AN0, AN3, AN0, AN3, ZERO);
  comparator.cmcon.set_configuration(2, 0, AN1, AN2, AN1, AN2, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 1, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(1, 2, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 2, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 3, AN1, AN2, AN1, AN2, OUT1);
  comparator.cmcon.set_configuration(1, 4, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN1, AN3, AN1, AN3, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, AN0, AN3, AN0, AN3, OUT0);
  comparator.cmcon.set_configuration(2, 5, AN1, AN3, AN1, AN3, OUT1);
  comparator.cmcon.set_configuration(1, 6, AN0, VREF, AN3, VREF, NO_OUT);
  comparator.cmcon.set_configuration(2, 6, AN1, VREF, AN2, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0xfb4, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0xfb5, RegisterValue(0,0),"cvrcon");

  ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v2::CCP2IF, &tmr2);
//  ccp2con.setIOpin(&((*m_portc)[1]));     // Handled by Config3H_2x21::set
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  //1 usart16.initialize_16(this,&pir_set_def,&portc);
  add_sfr_register(&usart.spbrgh,   0xfb0,porv,"spbrgh");
  add_sfr_register(&usart.baudcon,  0xfb8,porv,"baudcon");
  usart.set_eusart(true);
  init_pir2(pir2, PIR2v4::TMR3IF); 
  tmr3l.setIOpin(&(*m_portc)[0]);
}

//------------------------------------------------------------------------
//
P18F4221::P18F4221(const char *_name, const char *desc)
  : P18F4x21(_name,desc)
{
}

Processor * P18F4221::construct(const char *name)
{
  P18F4221 *p = new P18F4221(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F4321::P18F4321(const char *_name, const char *desc)
  : P18F4x21(_name,desc)
{
}

Processor * P18F4321::construct(const char *name)
{
  P18F4321 *p = new P18F4321(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F4420::P18F4420(const char *_name, const char *desc)
  : P18F4x21(_name,desc)
{
}

Processor * P18F4420::construct(const char *name)
{
  P18F4420 *p = new P18F4420(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F4520::P18F4520(const char *_name, const char *desc)
  : P18F4x21(_name,desc)
{
}

Processor * P18F4520::construct(const char *name)
{
  P18F4520 *p = new P18F4520(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//------------------------------------------------------------------------
//
P18F4620::P18F4620(const char *_name, const char *desc)
  : P18F4x21(_name,desc)
{
}

Processor * P18F4620::construct(const char *name)
{
  P18F4620 *p = new P18F4620(name);

  p->create();
  p->create_invalid_registers();

  return p;
}

//========================================================================
//
void P18F6x20::create()
{
  tbl.initialize ( eeprom_memory_size(), 32, 4, CONFIG1L, true);
  tbl.set_intcon(&intcon);
  set_eeprom_pir(&tbl);
  tbl.set_pir(pir2);
  tbl.eecon1.set_valid_bits(0xbf);

  create_iopin_map();

  _16bit_processor::create();

  m_configMemory->addConfigWord(CONFIG1H-CONFIG1L,new Config1H_4bits(this, CONFIG1H, 0x27));
  init_pir2(pir2, PIR2v2::TMR3IF);
  tmr3l.setIOpin(&(*m_portc)[0]);
}

//------------------------------------------------------------------------
void P18F6x20::create_iopin_map()
{
  package = new Package(64);

  if(!package)
    return;

  // Build the links between the I/O Ports and their tris registers.

  package->assign_pin( 1, m_porte->addPin(new IO_bi_directional("porte1"),1));
  package->assign_pin( 2, m_porte->addPin(new IO_bi_directional("porte0"),0));

  package->assign_pin( 3, m_portg->addPin(new IO_bi_directional("portg0"),0));
  package->assign_pin( 4, m_portg->addPin(new IO_bi_directional("portg1"),1));
  package->assign_pin( 5, m_portg->addPin(new IO_bi_directional("portg2"),2));
  package->assign_pin( 6, m_portg->addPin(new IO_bi_directional("portg3"),3));

  createMCLRPin(7);

  package->assign_pin( 8, m_portg->addPin(new IO_bi_directional("portg4"),4));

  package->assign_pin( 9, 0);  // Vss
  package->assign_pin(10, 0);  // Vdd

  package->assign_pin(11, m_portf->addPin(new IO_bi_directional("portf7"),7));
  package->assign_pin(12, m_portf->addPin(new IO_bi_directional("portf6"),6));
  package->assign_pin(13, m_portf->addPin(new IO_bi_directional("portf5"),5));
  package->assign_pin(14, m_portf->addPin(new IO_bi_directional("portf4"),4));
  package->assign_pin(15, m_portf->addPin(new IO_bi_directional("portf3"),3));
  package->assign_pin(16, m_portf->addPin(new IO_bi_directional("portf2"),2));
  package->assign_pin(17, m_portf->addPin(new IO_bi_directional("portf1"),1));
  package->assign_pin(18, m_portf->addPin(new IO_bi_directional("portf0"),0));

  package->assign_pin(19, 0);  // AVdd
  package->assign_pin(20, 0);  // AVss

  package->assign_pin(21, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin(22, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin(23, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin(24, m_porta->addPin(new IO_bi_directional("porta0"),0));

  package->assign_pin(25, 0);  // Vss
  package->assign_pin(26, 0);  // Vdd

  package->assign_pin(27, m_porta->addPin(new IO_bi_directional("porta5"),5));
  package->assign_pin(28, m_porta->addPin(new IO_open_collector("porta4"),4));

  package->assign_pin(29, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(30, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(31, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(32, m_portc->addPin(new IO_bi_directional("portc7"),7));
  package->assign_pin(33, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(34, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(35, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(36, m_portc->addPin(new IO_bi_directional("portc5"),5));

  package->assign_pin(37, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  package->assign_pin(38, 0);  // Vdd
  package->assign_pin(39, 0);  // OSC1/CLKI

  package->assign_pin(40, m_porta->addPin(new IO_bi_directional("porta6"),6));

  package->assign_pin(41, 0);  // Vss

  package->assign_pin(42, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(43, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(44, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(45, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(46, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(47, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(48, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));

  package->assign_pin(49, m_portd->addPin(new IO_bi_directional("portd7"),7));
  package->assign_pin(50, m_portd->addPin(new IO_bi_directional("portd6"),6));
  package->assign_pin(51, m_portd->addPin(new IO_bi_directional("portd5"),5));
  package->assign_pin(52, m_portd->addPin(new IO_bi_directional("portd4"),4));
  package->assign_pin(53, m_portd->addPin(new IO_bi_directional("portd3"),3));
  package->assign_pin(54, m_portd->addPin(new IO_bi_directional("portd2"),2));
  package->assign_pin(55, m_portd->addPin(new IO_bi_directional("portd1"),1));

  package->assign_pin(56, 0);  // Vss
  package->assign_pin(57, 0);  // Vdd

  package->assign_pin(58, m_portd->addPin(new IO_bi_directional("portd0"),0));

  package->assign_pin(59, m_porte->addPin(new IO_bi_directional("porte7"),7));
  package->assign_pin(60, m_porte->addPin(new IO_bi_directional("porte6"),6));
  package->assign_pin(61, m_porte->addPin(new IO_bi_directional("porte5"),5));
  package->assign_pin(62, m_porte->addPin(new IO_bi_directional("porte4"),4));
  package->assign_pin(63, m_porte->addPin(new IO_bi_directional("porte3"),3));
  package->assign_pin(64, m_porte->addPin(new IO_bi_directional("porte2"),2));

  psp.initialize(&pir_set_def,    // PIR
                m_portd,           // Parallel port
                m_trisd,           // Parallel tris
                pspcon,           // Control register
                &(*m_porte)[0],    // NOT RD
                &(*m_porte)[1],    // NOT WR
                &(*m_porte)[2]);   // NOT CS

  tmr1l.setIOpin(&(*m_portc)[0]);
  ssp.initialize(&pir_set_def,    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_portf)[7],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,         // i2c tris port
		SSP_TYPE_MSSP
       );


  set_osc_pin_Number(0,39, NULL);
  set_osc_pin_Number(1,40, &(*m_porta)[6]);
}

P18F6x20::P18F6x20(const char *_name, const char *desc)
  : _16bit_v2_adc(_name,desc),
    t4con(this, "t4con", "TMR4 Control"),
    pr4(this, "pr4", "TMR4 Period Register"),
    tmr4(this, "tmr4", "TMR4 Register"),
    pir3(this,"pir3","Peripheral Interrupt Register",0,0),
    pie3(this, "pie3", "Peripheral Interrupt Enable"),
    ipr3(this, "ipr3", "Interrupt Priorities"),
    ccp3con(this, "ccp3con", "Capture Compare Control"),
    ccpr3l(this, "ccpr3l", "Capture Compare 3 Low"),
    ccpr3h(this, "ccpr3h", "Capture Compare 3 High"),
    ccp4con(this, "ccp4con", "Capture Compare Control"),
    ccpr4l(this, "ccpr4l", "Capture Compare 4 Low"),
    ccpr4h(this, "ccpr4h", "Capture Compare 4 High"),
    ccp5con(this, "ccp5con", "Capture Compare Control"),
    ccpr5l(this, "ccpr5l", "Capture Compare 5 Low"),
    ccpr5h(this, "ccpr5h", "Capture Compare 5 High"),
    usart2(this), comparator(this)
{
  m_portd = new PicPSP_PortRegister(this,"portd","",8,0xFF);
  m_trisd = new PicTrisRegister(this,"trisd","", (PicPortRegister *)m_portd, false);
  m_latd  = new PicLatchRegister(this,"latd","",m_portd);

  m_porte = new PicPortRegister(this,"porte","",8,0xFF);
  m_trise = new PicTrisRegister(this,"trise","", m_porte, false);
  m_late  = new PicLatchRegister(this,"late","",m_porte);

  m_portf = new PicPortRegister(this,"portf","",8,0xFF);
  m_trisf = new PicTrisRegister(this,"trisf","", m_portf, false);
  m_latf  = new PicLatchRegister(this,"latf","",m_portf);

  m_portg = new PicPortRegister(this,"portg","",8,0x1F);
  m_trisg = new PicTrisRegister(this,"trisg","", m_portg, false);
  m_latg  = new PicLatchRegister(this,"latg","",m_portg);

  pspcon = new PSPCON(this, "pspcon","");


}

P18F6x20::~P18F6x20()
{
  delete_sfr_register(m_portd);
  delete_sfr_register(m_porte);
  delete_sfr_register(m_portf);
  delete_sfr_register(m_portg);

  delete_sfr_register(m_latd);
  delete_sfr_register(m_late);
  delete_sfr_register(m_latf);
  delete_sfr_register(m_latg);

  delete_sfr_register(m_trisd);
  delete_sfr_register(m_trise);
  delete_sfr_register(m_trisf);
  delete_sfr_register(m_trisg);
  delete_sfr_register(pspcon);
  delete_sfr_register(usart2.txreg);
  delete_sfr_register(usart2.rcreg);

  remove_sfr_register(&pie3);
  remove_sfr_register(&pir3);
  remove_sfr_register(&ipr3);
  remove_sfr_register(&usart2.rcsta);
  remove_sfr_register(&usart2.txsta);
  remove_sfr_register(&usart2.spbrg);
  remove_sfr_register(&ccp4con);
  remove_sfr_register(&ccpr4l);
  remove_sfr_register(&ccpr4h);
  remove_sfr_register(&ccp5con);
  remove_sfr_register(&ccpr5l);
  remove_sfr_register(&ccpr5h);
  remove_sfr_register(&t4con);
  remove_sfr_register(&pr4);
  remove_sfr_register(&tmr4);
  remove_sfr_register(&ccp3con);
  remove_sfr_register(&ccpr3l);
  remove_sfr_register(&ccpr3h);
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);

}

void P18F6x20::create_sfr_map()
{
  _16bit_processor::create_sfr_map();
  _16bit_v2_adc::create(12);

  RegisterValue porv(0,0);

  osccon->por_value.put(0x01,0x01);

  // cout << "Create extra ports\n";
  add_sfr_register(m_portd,       0xf83,porv);
  add_sfr_register(m_porte,       0xf84,porv);
  add_sfr_register(m_portf,       0xf85,porv);
  add_sfr_register(m_portg,       0xf86,porv);

  add_sfr_register(m_latd,        0xf8c,porv);
  add_sfr_register(m_late,        0xf8d,porv);
  add_sfr_register(m_latf,        0xf8e,porv);
  add_sfr_register(m_latg,        0xf8f,porv);

  add_sfr_register(m_trisd,       0xf95,RegisterValue(0xff,0));
  add_sfr_register(m_trise,       0xf96,RegisterValue(0xff,0));
  add_sfr_register(m_trisf,       0xf97,RegisterValue(0xff,0));
  add_sfr_register(m_trisg,       0xf98,RegisterValue(0x1f,0));

  add_sfr_register(&pie3,	  0xfa3,porv,"pie3");
  add_sfr_register(&pir3,	  0xfa4,porv,"pir3");
  add_sfr_register(&ipr3,	  0xfa5,porv,"ipr3");



  add_sfr_register(pspcon,       0xfb0,RegisterValue(0x00,0));

  // cout << "Assign ADC pins to " << adcon1 << "\n";
  adcon1->setIOPin(4, &(*m_porta)[5]);
  adcon1->setIOPin(5, &(*m_portf)[0]);
  adcon1->setIOPin(6, &(*m_portf)[1]);
  adcon1->setIOPin(7, &(*m_portf)[2]);
  adcon1->setIOPin(8, &(*m_portf)[3]);
  adcon1->setIOPin(9, &(*m_portf)[4]);
  adcon1->setIOPin(10, &(*m_portf)[5]);
  adcon1->setIOPin(11, &(*m_portf)[6]);
//  adcon1->setIOPin(12, &(*m_portb)[0]);
/*
  adcon1->setChanTable(0x1ff, 0x1fff, 0x1fff, 0x0fff,
	0x07ff, 0x03ff, 0x01ff, 0x00ff, 0x007f, 0x003f,
	0x001f, 0x000f, 0x0007, 0x0003, 0x0001, 0x0000);
  adcon1->setVrefHiChannel(3);
  adcon1->setVrefLoChannel(2);
*/


  // Link the comparator and voltage ref to portf
  comparator.initialize(&pir_set_def, &(*m_portf)[5], 
	0, 0, 0, 0,
	&(*m_portf)[2], &(*m_portf)[1]);

  // set anx for input pins
  comparator.cmcon.setINpin(0, &(*m_portf)[6], "an11");
  comparator.cmcon.setINpin(1, &(*m_portf)[5], "an10");
  comparator.cmcon.setINpin(2, &(*m_portf)[4], "an9");
  comparator.cmcon.setINpin(3, &(*m_portf)[3], "an8");


  comparator.cmcon.set_configuration(1, 0, AN0, AN1, AN0, AN1, ZERO);
  comparator.cmcon.set_configuration(2, 0, AN2, AN3, AN2, AN3, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN0, AN1, AN0, AN1, OUT0);
  comparator.cmcon.set_configuration(2, 1, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(1, 2, AN0, AN1, AN0, AN1, NO_OUT);
  comparator.cmcon.set_configuration(2, 2, AN2, AN3, AN2, AN3, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN0, AN1, AN0, AN1, OUT0);
  comparator.cmcon.set_configuration(2, 3, AN2, AN3, AN2, AN3, OUT1);
  comparator.cmcon.set_configuration(1, 4, AN0, AN1, AN0, AN1, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN2, AN1, AN2, AN1, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, AN0, AN1, AN0, AN1, OUT0);
  comparator.cmcon.set_configuration(2, 5, AN2, AN1, AN2, AN1, OUT1);
  comparator.cmcon.set_configuration(1, 6, AN0, VREF, AN1, VREF, NO_OUT);
  comparator.cmcon.set_configuration(2, 6, AN2, VREF, AN3, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0xfb4, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0xfb5, RegisterValue(0,0),"cvrcon");


  // cout << "Setting CCP cross-links\n";
  ccp2con.setCrosslinks(&ccpr2l, pir2, PIR2v2::CCP2IF, &tmr2);
  ccp2con.setIOpin(&((*m_portc)[1]));
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  add_sfr_register(&ccp3con,	  0xfb7,porv,"ccp3con");
  add_sfr_register(&ccpr3l,	  0xfb8,porv,"ccpr3l");
  add_sfr_register(&ccpr3h,	  0xfb9,porv,"ccpr3h");
  add_sfr_register(&ccp4con,	  0xf73,porv,"ccp4con");
  add_sfr_register(&ccpr4l,	  0xf74,porv,"ccpr4l");
  add_sfr_register(&ccpr4h,	  0xf75,porv,"ccpr4h");
  add_sfr_register(&ccp5con,	  0xf70,porv,"ccp5con");
  add_sfr_register(&ccpr5l,	  0xf71,porv,"ccpr5l");
  add_sfr_register(&ccpr5h,	  0xf72,porv,"ccpr5h");

  add_sfr_register(&t4con,	  0xf76,porv,"t4con");
  add_sfr_register(&pr4,	  0xf77,RegisterValue(0xff,0),"pr4");
  add_sfr_register(&tmr4,	  0xf78,porv,"tmr4");

  ccp3con.setCrosslinks(&ccpr3l, &pir3, PIR3v1::CCP3IF, &tmr2);
  ccp3con.setIOpin(&((*m_portg)[0]));
  ccpr3l.ccprh  = &ccpr3h;
  ccpr3l.tmrl   = &tmr1l;
  ccpr3h.ccprl  = &ccpr3l;
  tmr2.add_ccp ( &ccp3con );

  ccp4con.setCrosslinks(&ccpr4l, &pir3, PIR3v1::CCP4IF, &tmr2);
  ccp4con.setIOpin(&((*m_portg)[3]));
  ccpr4l.ccprh  = &ccpr4h;
  ccpr4l.tmrl   = &tmr1l;
  ccpr4h.ccprl  = &ccpr4l;
  tmr2.add_ccp ( &ccp4con );

  ccp5con.setCrosslinks(&ccpr5l, &pir3, PIR3v1::CCP5IF, &tmr2);
  ccp5con.setIOpin(&((*m_portg)[4]));
  ccpr5l.ccprh  = &ccpr5h;
  ccpr5l.tmrl   = &tmr1l;
  ccpr5h.ccprl  = &ccpr5l;
  tmr2.add_ccp ( &ccp5con );

  //cout << "Create second USART\n";
  usart2.initialize(&pir3,&(*m_portg)[1], &(*m_portg)[2],
	            new _TXREG(this,"txreg2", "USART Transmit Register", &usart2), 
                    new _RCREG(this,"rcreg2", "USART Receiver Register", &usart2));

  add_sfr_register(&usart2.rcsta,    0xf6b,porv,"rcsta2");
  add_sfr_register(&usart2.txsta,    0xf6c,RegisterValue(0x02,0),"txsta2");
  add_sfr_register(usart2.txreg,     0xf6d,porv,"txreg2");
  add_sfr_register(usart2.rcreg,     0xf6e,porv,"rcreg2");
  add_sfr_register(&usart2.spbrg,    0xf6f,porv,"spbrg2");

  t4con.tmr2  = &tmr4;
  tmr4.pir_set = &pir_set_def; //get_pir_set();
  tmr4.pr2    = &pr4;
  tmr4.t2con  = &t4con;
  tmr4.add_ccp ( &ccp1con );
  tmr4.add_ccp ( &ccp2con );
  pr4.tmr2    = &tmr4;

  pir3.set_intcon(&intcon);
  pir3.set_pie(&pie3);
  pir3.set_ipr(&ipr3);
  pie3.setPir(&pir3);
  //pie3.new_name("pie3");

}

//------------------------------------------------------------------------
//
P18F6520::P18F6520(const char *_name, const char *desc)
  : P18F6x20(_name,desc)
{
}

Processor * P18F6520::construct(const char *name)
{
  P18F6520 *p = new P18F6520(name);

  p->create();
  p->create_invalid_registers();

  return p;
}


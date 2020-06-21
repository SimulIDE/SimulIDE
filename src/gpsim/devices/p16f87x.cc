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

// p16f87x
//
//  This file supports:
//    P16F874  P16F877
//    P16F871  P16F873
//    P16F876  P16F873A
//    P14F874A P16F876A
//    P16F877A

#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"
#include "p16f87x.h"
#include "pic-ioports.h"
#include "stimuli.h"

//-------------------------------------------------------------------

void P16F871::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void P16F871::create_sfr_map()
{
  add_sfr_register(pir2,    0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));

  // Parent classes just set PIR1
  pir_set_2_def.set_pir2(pir2_2_reg);

  usart.initialize(pir1,&(*m_portc)[6], &(*m_portc)[7],
		   new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");

  intcon = &intcon_reg;

  if (pir2) {
    pir2->set_intcon(&intcon_reg);
    pir2->set_pie(&pie2);
  }
  pie2.setPir(get_pir2());

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));

  // Enable program memory reads and writes.
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);

  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedatah(), 0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),  0x10f);

  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  alias_file_registers(0x00,0x04,0x100);
  alias_file_registers(0x80,0x84,0x100);

  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x86,0x86,0x100);

  //alias_file_registers(0x0a,0x0b,0x080);  //already called
  alias_file_registers(0x0a,0x0b,0x100);
  alias_file_registers(0x0a,0x0b,0x180);

  //alias_file_registers(0x20,0x7f,0x100);  // already called
  alias_file_registers(0xa0,0xbf,0x100);

  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));
  add_sfr_register(&adres,  0x1e, RegisterValue(0,0));
  add_sfr_register(&adresl, 0x9e, RegisterValue(0,0));

  //1adcon0.analog_port = porta;
  //1adcon0.analog_port2 = porte;

  adcon0.setAdres(&adres);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(7);
  adcon0.setAdresLow(&adresl);
  adcon0.setA2DBits(10);

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 | 
			 ADCON1::PCFG2 | ADCON1::PCFG3, 0);
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
  adcon1.setChannelConfiguration(8, 0xff);
  adcon1.setChannelConfiguration(9, 0x3f);
  adcon1.setChannelConfiguration(10, 0x3f);
  adcon1.setChannelConfiguration(11, 0x3f);
  adcon1.setChannelConfiguration(12, 0x1f);
  adcon1.setChannelConfiguration(13, 0x0f);
  adcon1.setChannelConfiguration(14, 0x01);
  adcon1.setChannelConfiguration(15, 0x0d);

  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);
  adcon1.setVrefHiConfiguration(8, 3);
  adcon1.setVrefHiConfiguration(10, 3);
  adcon1.setVrefHiConfiguration(11, 3);
  adcon1.setVrefHiConfiguration(12, 3);
  adcon1.setVrefHiConfiguration(13, 3);
  adcon1.setVrefHiConfiguration(15, 3);

  adcon1.setVrefLoConfiguration(8, 2);
  adcon1.setVrefLoConfiguration(11, 2);
  adcon1.setVrefLoConfiguration(12, 2);
  adcon1.setVrefLoConfiguration(13, 2);
  adcon1.setVrefLoConfiguration(15, 2);
}

void P16F871::create()
{
  P16C64::create();

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eeprom_memory_size());
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F871::create_sfr_map();
}

Processor * P16F871::construct(const char *name)
{
  P16F871 *p = new P16F871(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

//========================================================================
P16F871::P16F871(const char *_name, const char *desc)
  : P16C64(_name,desc) ,
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result"),
    adresl(this,"adresl", "A2D Result Low"),
    usart(this)
{
  //pir2 = &pir2_2_reg;
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);
  delete pir2;
  pir2 = pir2_2_reg;
}

P16F871::~P16F871()
{
  remove_sfr_register(&pie2);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&adres);
  remove_sfr_register(&adresl);
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  delete get_eeprom();
}

//-------------------------------------------------------
void P16F873::set_out_of_range_pm(uint address, uint value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void P16F873::create_sfr_map()
{
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));

  // Enable program memory reads and writes.
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);

  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedatah(), 0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),  0x10f);

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

  // The rest of the A/D definition in 16C73
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  adcon0.setAdresLow(&adresl);
  adcon0.setA2DBits(10);
  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 | 
			 ADCON1::PCFG2 | ADCON1::PCFG3 , 0);

  adcon1.setChannelConfiguration(0, 0x1f);
  adcon1.setChannelConfiguration(1, 0x1f);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setChannelConfiguration(8, 0x1f);
  adcon1.setChannelConfiguration(9, 0x1f);
  adcon1.setChannelConfiguration(10, 0x1f);
  adcon1.setChannelConfiguration(11, 0x1f);
  adcon1.setChannelConfiguration(12, 0x1f);
  adcon1.setChannelConfiguration(13, 0x1f);
  adcon1.setChannelConfiguration(14, 0x01);
  adcon1.setChannelConfiguration(15, 0x0d);

  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);
  adcon1.setVrefHiConfiguration(8, 3);
  adcon1.setVrefHiConfiguration(10, 3);
  adcon1.setVrefHiConfiguration(11, 3);
  adcon1.setVrefHiConfiguration(12, 3);
  adcon1.setVrefHiConfiguration(13, 3);
  adcon1.setVrefHiConfiguration(15, 3);

  adcon1.setVrefLoConfiguration(8, 2);
  adcon1.setVrefLoConfiguration(11, 2);
  adcon1.setVrefLoConfiguration(12, 2);
  adcon1.setVrefLoConfiguration(13, 2);
  adcon1.setVrefLoConfiguration(15, 2);
  add_sfr_register(&ssp.sspcon2,  0x91, RegisterValue(0,0) ,"sspcon2");

  ssp.initialize(
		get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,    	  // i2c tris port
		SSP_TYPE_MSSP
        );
}

void P16F873::create()
{
  P16C73::create();

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eeprom_memory_size());
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F873::create_sfr_map();
}

//========================================================================
Processor * P16F873::construct(const char *name)
{
  P16F873 *p = new P16F873(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F873::P16F873(const char *_name, const char *desc)
  : P16C73(_name,desc),
    adresl(this,"adresl", "A2D Result Low")
{
  set_hasSSP();
}

P16F873::~P16F873()
{
  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(&adresl);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  delete get_eeprom();
}

void P16F873A::create()
{
  P16F873::create();
  P16F873A::create_sfr_map();
}

void P16F873A::create_sfr_map()
{
  // Link the comparator and voltage ref to porta
  comparator.initialize(get_pir_set(), 
	&(*m_porta)[2], &(*m_porta)[0],
        &(*m_porta)[1], &(*m_porta)[2], 
	&(*m_porta)[3], &(*m_porta)[4], &(*m_porta)[5]);

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
  

  add_sfr_register(&comparator.cmcon, 0x9c, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0x9d, RegisterValue(0,0),"vrcon");

}
Processor * P16F873A::construct(const char *name)
{
  P16F873A *p = new P16F873A(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F873A::P16F873A(const char *_name, const char *desc)
  : P16F873(_name,desc),
    comparator(this)
{
}

P16F873A::~P16F873A()
{
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);
}


Processor * P16F876::construct(const char *name)
{
  P16F876 *p = new P16F876(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

void P16F876::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void P16F876::create_sfr_map()
{
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));

  // Enable program memory reads and writes.
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);

  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedatah(), 0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),  0x10f);

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


  add_file_registers(0x110, 0x16f, 0);
  add_file_registers(0x190, 0x1ef, 0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  // The rest of the A/D definition in 16C73
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  adcon0.setAdresLow(&adresl);
  adcon0.setA2DBits(10);
  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 | 
			 ADCON1::PCFG2 | ADCON1::PCFG3 , 0);

  adcon1.setChannelConfiguration(0, 0x1f);
  adcon1.setChannelConfiguration(1, 0x1f);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setChannelConfiguration(8, 0x1f);
  adcon1.setChannelConfiguration(9, 0x1f);
  adcon1.setChannelConfiguration(10, 0x1f);
  adcon1.setChannelConfiguration(11, 0x1f);
  adcon1.setChannelConfiguration(12, 0x1f);
  adcon1.setChannelConfiguration(13, 0x1f);
  adcon1.setChannelConfiguration(14, 0x01);
  adcon1.setChannelConfiguration(15, 0x0d);

  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);
  adcon1.setVrefHiConfiguration(8, 3);
  adcon1.setVrefHiConfiguration(10, 3);
  adcon1.setVrefHiConfiguration(11, 3);
  adcon1.setVrefHiConfiguration(12, 3);
  adcon1.setVrefHiConfiguration(13, 3);
  adcon1.setVrefHiConfiguration(15, 3);

  adcon1.setVrefLoConfiguration(8, 2);
  adcon1.setVrefLoConfiguration(11, 2);
  adcon1.setVrefLoConfiguration(12, 2);
  adcon1.setVrefLoConfiguration(13, 2);
  adcon1.setVrefLoConfiguration(15, 2);
  add_sfr_register(&ssp.sspcon2,  0x91, RegisterValue(0,0) ,"sspcon2");

  ssp.initialize(
		get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,    	  // i2c tris port
		SSP_TYPE_MSSP
        );
}

void P16F876::create()
{
  P16C73::create();

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eeprom_memory_size());
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F876::create_sfr_map();
}

P16F876::P16F876(const char *_name, const char *desc)
  : P16C73(_name,desc),
    adresl(this,"adresl", "A2D Result Low")
{
}

P16F876::~P16F876()
{
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  delete get_eeprom();
  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(&adresl);

  delete_file_registers(0x110, 0x16f);
  delete_file_registers(0x190, 0x1ef);
}

Processor * P16F876A::construct(const char *name)
{
  P16F876A *p = new P16F876A(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

void P16F876A::create_sfr_map()
{
}

void P16F876A::create()
{
  P16F873A::create();

  // get rid of aliases
  delete_file_registers(0x20,0x7f);	// get rid of aliases
  delete_file_registers(0xa0,0xff);	// ""
  add_file_registers(0x20,0x7f, 0);
  add_file_registers(0xa0, 0xef,0);
  add_file_registers(0x110, 0x16f, 0);
  add_file_registers(0x190, 0x1ef, 0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  P16F876A::create_sfr_map();
}

P16F876A::P16F876A(const char *_name, const char *desc)
  : P16F873A(_name,desc),
    comparator(this)
{
}

P16F876A::~P16F876A()
{
  delete_file_registers(0x110, 0x16f);
  delete_file_registers(0x190, 0x1ef);
}

//-------------------------------------------------------

void P16F874::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void P16F874::create_sfr_map()
{
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));

  // Enable program memory reads and writes.
  get_eeprom()->get_reg_eecon1()->set_bits(EECON1::EEPGD);

  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  get_eeprom()->get_reg_eedatah()->new_name("eedath");
  add_sfr_register(get_eeprom()->get_reg_eedatah(), 0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),  0x10f);


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

  // The rest of the A/D definition in 16C74
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  adcon0.setA2DBits(10);
  adcon0.setAdresLow(&adresl);


  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 | 
			 ADCON1::PCFG2 | ADCON1::PCFG3, 0);

  adcon1.setChannelConfiguration(0, 0xff);
  adcon1.setChannelConfiguration(1, 0xff);
  adcon1.setChannelConfiguration(2, 0x1f);
  adcon1.setChannelConfiguration(3, 0x1f);
  adcon1.setChannelConfiguration(4, 0x0b);
  adcon1.setChannelConfiguration(5, 0x0b);
  adcon1.setChannelConfiguration(6, 0x00);
  adcon1.setChannelConfiguration(7, 0x00);
  adcon1.setChannelConfiguration(8, 0xff);
  adcon1.setChannelConfiguration(9, 0x3f);
  adcon1.setChannelConfiguration(10, 0x3f);
  adcon1.setChannelConfiguration(11, 0x3f);
  adcon1.setChannelConfiguration(12, 0x3f);
  adcon1.setChannelConfiguration(13, 0x1f);
  adcon1.setChannelConfiguration(14, 0x01);
  adcon1.setChannelConfiguration(15, 0x0d);

  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);
  adcon1.setVrefHiConfiguration(8, 3);
  adcon1.setVrefHiConfiguration(10, 3);
  adcon1.setVrefHiConfiguration(11, 3);
  adcon1.setVrefHiConfiguration(12, 3);
  adcon1.setVrefHiConfiguration(13, 3);
  adcon1.setVrefHiConfiguration(15, 3);

  adcon1.setVrefLoConfiguration(8, 2);
  adcon1.setVrefLoConfiguration(11, 2);
  adcon1.setVrefLoConfiguration(12, 2);
  adcon1.setVrefLoConfiguration(13, 2);
  adcon1.setVrefLoConfiguration(15, 2);
  add_sfr_register(&ssp.sspcon2,  0x91, RegisterValue(0,0) ,"sspcon2");
  ssp.initialize(
		get_pir_set(),    // PIR
                &(*m_portc)[3],   // SCK
                &(*m_porta)[5],   // SS
                &(*m_portc)[5],   // SDO
                &(*m_portc)[4],   // SDI
                m_trisc,    	  // i2c tris port
		SSP_TYPE_MSSP
        );
}

void P16F874::create()
{
  P16C74::create();

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eeprom_memory_size());

  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F874::create_sfr_map();
}

Processor * P16F874::construct(const char *name)
{
  P16F874 *p = new P16F874(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F874::P16F874(const char *_name, const char *desc)
  : P16C74(_name,desc),
    comparator(this),
    adresl(this,"adresl", "A2D Result Low")
{
  set_hasSSP();
}

P16F874::~P16F874()
{
  remove_sfr_register(&adresl);
  remove_sfr_register(&ssp.sspcon2);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  delete get_eeprom();
}

//------------------------------------------------------
void P16F874A::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void P16F874A::create_sfr_map()
{
  // Link the comparator and voltage ref to porta
  comparator.initialize(get_pir_set(), 
	&(*m_porta)[2], &(*m_porta)[0],
        &(*m_porta)[1], &(*m_porta)[2], 
	&(*m_porta)[3], &(*m_porta)[4], &(*m_porta)[5]);

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

  add_sfr_register(&comparator.cmcon, 0x9c, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0x9d, RegisterValue(0,0),"vrcon");
}

void P16F874A::create()
{
  P16F874::create();
  P16F874A::create_sfr_map();
}

Processor * P16F874A::construct(const char *name)
{
  P16F874A *p = new P16F874A(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F874A::P16F874A(const char *_name, const char *desc)
  : P16F874(_name,desc),
    comparator(this)
{
}

P16F874A::~P16F874A()
{
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);
}

void P16F877::create_sfr_map()
{
}

void P16F877::create()
{
  P16F874::create();

  delete_file_registers(0x20, 0x7f); // get rid of alias registers
  delete_file_registers(0xa0, 0xff); //		""
  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xef, 0);
  add_file_registers(0x110, 0x16f, 0);
  add_file_registers(0x190, 0x1ef, 0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  P16F877::create_sfr_map();
}

Processor * P16F877::construct(const char *name)
{
  P16F877 *p = new P16F877(name);

  p->create();
  p->create_invalid_registers ();

  return p;
}

P16F877::P16F877(const char *_name, const char *desc)
  : P16F874(_name,desc)
{
} 

P16F877::~P16F877()
{
  delete_file_registers(0x110, 0x16f);
  delete_file_registers(0x190, 0x1ef);
}

void P16F877A::create_sfr_map()
{
}

void P16F877A::create()
{
  P16F874A::create();

  delete_file_registers(0x20, 0x7f);	// get rid of alias registers
  delete_file_registers(0xa0, 0xff);	// ""
  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xef, 0);

  add_file_registers(0x110, 0x16f, 0);
  add_file_registers(0x190, 0x1ef, 0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  P16F877A::create_sfr_map();
}

Processor * P16F877A::construct(const char *name)
{
  P16F877A *p = new P16F877A(name);

  p->create();
  p->create_invalid_registers ();
  return p;
}

P16F877A::P16F877A(const char *_name, const char *desc)
  : P16F874A(_name,desc),
    comparator(this)
{
}

P16F877A::~P16F877A()
{
  delete_file_registers(0x110, 0x16f);
  delete_file_registers(0x190, 0x1ef);
}

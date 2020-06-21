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
// p16f62x
//
//  This file supports:
//    PIC16F627
//    PIC16F628
//    PIC16F648
//

#include <stdio.h>
#include <iostream>
#include <string>

//#include "config.h"

#include "stimuli.h"
#include "p16f62x.h"
#include "pic-ioports.h"
#include "packages.h"

P16F62x::P16F62x(const char *_name, const char *desc)
  : P16X6X_processor(_name,desc),
    usart(this),
    comparator(this)
{
}

P16F62x::~P16F62x()
{
  delete_file_registers(0xc0, 0xef);
  delete_file_registers(0x120,0x14f);

  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);

  delete eeprom;
  eeprom = 0;
}

void P16F62x::create_iopin_map()
{
  package = new Package(18);
  if(!package) return;

  // Now Create the package and place the I/O pins

  package->assign_pin(17, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin(18, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 1, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 3, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta5"),5));
  package->assign_pin(15, m_porta->addPin(new IO_bi_directional("porta6"),6));
  package->assign_pin(16, m_porta->addPin(new IO_bi_directional("porta7"),7));

  package->assign_pin(5, 0);  // Vss
  package->assign_pin( 6, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin( 7, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin( 8, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin( 9, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));
  package->assign_pin(14, 0);  // Vdd
}

void P16F62x::create_sfr_map()
{
  add_file_registers(0xc0, 0xef, 0);   // 0xa0 - 0xbf are created in the P16X6X_processor class
  add_file_registers(0x120,0x14f,0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);
  alias_file_registers(0x0,0x0,0x100);      // INDF exists in all four pages, 16x6x did the first two
  alias_file_registers(0x0,0x0,0x180);
  alias_file_registers(0x01,0x04,0x100);
  alias_file_registers(0x81,0x84,0x100);
  remove_sfr_register(m_trisa);
  add_sfr_register(m_trisa, 0x85, RegisterValue(0xff,0));

  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x86,0x86,0x100);

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x9a);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x9b);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x9c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x9d);

  // PCLATH
  alias_file_registers(0x0a,0x0a,0x100);
  alias_file_registers(0x0a,0x0a,0x180);

  alias_file_registers(0x0b,0x0b,0x100);
  alias_file_registers(0x0b,0x0b,0x180);

  usart.initialize(pir1,&(*m_portb)[2], &(*m_portb)[1],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart),
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  // Link the comparator and voltage ref to porta
  comparator.initialize(get_pir_set(), &(*m_porta)[2], &(*m_porta)[0],
        &(*m_porta)[1], &(*m_porta)[2], &(*m_porta)[3], &(*m_porta)[3],
        &(*m_porta)[4]);

  comparator.cmcon.set_configuration(1, 0, AN0, AN3, AN0, AN3, ZERO);
  comparator.cmcon.set_configuration(2, 0, AN1, AN2, AN1, AN2, ZERO);
  comparator.cmcon.set_configuration(1, 1, AN0, AN2, AN3, AN2, NO_OUT);
  comparator.cmcon.set_configuration(2, 1, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 2, AN0, VREF, AN3, VREF, NO_OUT);
  comparator.cmcon.set_configuration(2, 2, AN1, VREF, AN2, VREF, NO_OUT);
  comparator.cmcon.set_configuration(1, 3, AN0, AN2, AN0, AN2, NO_OUT);
  comparator.cmcon.set_configuration(2, 3, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 4, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 5, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 6, AN0, AN2, AN0, AN2, OUT0);
  comparator.cmcon.set_configuration(2, 6, AN1, AN2, AN1, AN2, OUT1);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0x1f, RegisterValue(0,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0x9f, RegisterValue(0,0),"vrcon");

  comparator.cmcon.put(0);

  // Link ccp1 onto portb
  ccp1con.setIOpin(&((*m_portb)[3]));
}

void P16F62x::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

//========================================================================
bool P16F62x::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<4,
    CFG_MCLRE = 1<<5
  };
  // Let the base class do most of the work:

  if (pic_processor::set_config_word(address, cfg_word)) 
  {
    uint valid_pins = m_porta->getEnableMask();

    set_int_osc(false);
    // Careful these bits not adjacent
    switch(cfg_word & (CFG_FOSC0 | CFG_FOSC1 | CFG_FOSC2)) 
    {
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

    // If the /MCLRE bit is set then RA5 is the MCLR pin, otherwise it's
    // a general purpose I/O pin.

    if (! (cfg_word & CFG_MCLRE)) 
    {
      unassignMCLRPin();
      valid_pins |= ( 1<< 5);           // porta5 IO port
    }
    else assignMCLRPin(4);         // pin 4

    //cout << " porta valid_iopins " << porta->valid_iopins <<
    //   "  tris valid io " << trisa.valid_iopins << '\n';

    if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
    {
        m_porta->setEnableMask(valid_pins);
        m_porta->setTris(m_trisa);
    }
    return true;
  }
  return false;
}

//========================================================================
void  P16F62x::create(int ram_top, uint eeprom_size)
{
  EEPROM_PIR *e;

  create_iopin_map();

  _14bit_processor::create();

  e = new EEPROM_PIR(this,pir1);
  e->initialize(eeprom_size);
  //e->set_pir_set(get_pir_set());
  e->set_intcon(&intcon_reg);

  // assign this eeprom to the processor
  set_eeprom_pir(e);

  P16X6X_processor::create_sfr_map();

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F62x::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  //1ccp1con.iopin = portb->pins[3];
}

//========================================================================
//
// Pic 16F627
//

Processor * P16F627::construct(const char *name)
{
  P16F627 *p = new P16F627(name);

  p->P16F62x::create(0x2f, 128);
  p->create_invalid_registers ();

  return p;
}

P16F627::P16F627(const char *_name, const char *desc)
  : P16F62x(_name,desc)
{
}

//========================================================================
//
Processor * P16F628::construct(const char *name)
{
  P16F628 *p = new P16F628(name);

  p->P16F62x::create(0x2f, 128);
  p->create_invalid_registers ();

  return p;
}

P16F628::P16F628(const char *_name, const char *desc)
  : P16F627(_name,desc)
{
}

P16F628::~P16F628()
{
}

//========================================================================
//
Processor * P16F648::construct(const char *name)
{
  P16F648 *p = new P16F648(name);

  p->P16F62x::create(0x2f, 256);
  p->create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F648::P16F648(const char *_name, const char *desc)
  : P16F628(_name,desc)
{
}

P16F648::~P16F648()
{
  delete_file_registers(0x150,0x16f);
}

void P16F648::create_sfr_map()
{
  add_file_registers(0x150,0x16f,0);
}

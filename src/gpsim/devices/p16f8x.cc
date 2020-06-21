/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2006 Roy R. Rankin

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
// p16f8x
//
//  This file supports:
//    PIC16F87
//    PIC16F88
//    PIC16F818
//    PIC16F819
//

#include <stdio.h>
#include <iostream>
#include <string>

//#include "config.h"

#include "stimuli.h"

#include "p16f8x.h"
#include "pic-ioports.h"
#include "packages.h"


//========================================================================
//
// Configuration Memory for the 16F8X devices.

class Config1 : public ConfigWord
{
public:
  Config1(pic_processor *pCpu)
    : ConfigWord("CONFIG1", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
  }

  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    WDTEN  = 1<<2,
    PWRTEN = 1<<3,

    FOSC2  = 1<<4,
    MCLRE  = 1<<5,
    BOREN  = 1<<6,
    LVP    = 1<<7,

    CPD    = 1<<8,
    WRT0   = 1<<9,
    WRT1   = 1<<10,
    DEBUG  = 1<<11,

    CCPMX  = 1<<12,
    CP     = 1<<13
  };

  virtual void set(int64_t v)
  {
    Integer::set(v);
    if (m_pCpu) 
        m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);
  }
};


//========================================================================

P16F8x::P16F8x(const char *_name, const char *desc)
  : P16X6X_processor(_name,desc),
    wdtcon(this, "wdtcon", "WDT Control", 0x1f),
    osccon(0),
    osctune(this, "osctune", "OSC Tune"),
    usart(this),
    comparator(this)
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);
  delete pir1;
  delete pir2;
  pir1 = pir1_2_reg;
  pir2 = pir2_2_reg;
}

P16F8x::~P16F8x()
{
  delete_file_registers(0xc0, 0xef);
  delete_file_registers(0x110,0x16f);
  delete_file_registers(0x190,0x1ef);
  remove_sfr_register(&comparator.cmcon);
  remove_sfr_register(&comparator.vrcon);
  remove_sfr_register(&wdtcon);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(&usart.rcsta);
  remove_sfr_register(&usart.txsta);
  remove_sfr_register(&usart.spbrg);
  delete_sfr_register(usart.txreg);
  delete_sfr_register(usart.rcreg);
  delete get_eeprom();
  remove_sfr_register(osccon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&pie2);
}

void P16F8x::create_iopin_map()
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

  if (hasSSP()) {
    ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portb)[4],   // SCK
                &(*m_portb)[5],   // SS
                &(*m_portb)[2],   // SDO
                &(*m_portb)[1],    // SDI
                m_trisb,          // i2c tris port
                SSP_TYPE_SSP
        );
  }
}

void P16F8x::create_sfr_map()
{
  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);
 
  add_file_registers(0xc0, 0xef, 0);
  add_file_registers(0x110,0x16f,0);
  add_file_registers(0x190,0x1ef,0);

  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x70,0x7f,0x180);

  add_sfr_register(get_pir2(),   0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));
                                                                                
  pir_set_def.set_pir2(pir2);
                                                                                
  pie2.setPir(get_pir2());
  alias_file_registers(0x00,0x04,0x100);
  alias_file_registers(0x80,0x84,0x100);

  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x86,0x86,0x100);

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x10f);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  //alias_file_registers(0x0a,0x0b,0x080); // Already done
  alias_file_registers(0x0a,0x0b,0x100);
  alias_file_registers(0x0a,0x0b,0x180);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  add_sfr_register(osccon, 0x8f, RegisterValue(0,0),"osccon");
  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");

  osccon->set_osctune(&osctune);
  osctune.set_osccon(osccon);
  osccon->write_mask = 0x73;
  osccon->has_iofs_bit = true;

  usart.initialize(pir1,&(*m_portb)[5], &(*m_portb)[2],
                   new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
                   new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");
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
  comparator.cmcon.set_configuration(2, 3, AN1, AN2, AN1, AN3, NO_OUT);
  comparator.cmcon.set_configuration(1, 4, AN0, AN3, AN0, AN3, NO_OUT);
  comparator.cmcon.set_configuration(2, 4, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 5, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 5, AN1, AN2, AN1, AN2, NO_OUT);
  comparator.cmcon.set_configuration(1, 6, AN0, AN2, AN0, AN2, OUT0);
  comparator.cmcon.set_configuration(2, 6, AN1, AN2, AN1, AN2, OUT1);
  comparator.cmcon.set_configuration(1, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);
  comparator.cmcon.set_configuration(2, 7, NO_IN, NO_IN, NO_IN, NO_IN, ZERO);

  add_sfr_register(&comparator.cmcon, 0x9c, RegisterValue(7,0),"cmcon");
  add_sfr_register(&comparator.vrcon, 0x9d, RegisterValue(0,0),"cvrcon");
  add_sfr_register(&wdtcon, 0x105, RegisterValue(0x08,0),"wdtcon");
}

void P16F8x::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

//========================================================================
bool P16F8x::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<4,
    CFG_MCLRE = 1<<5,
    CFG_CCPMX = 1<<12,
    CFG2_IESO = 1<<1
  };

  uint fosc;

  // Let the base class do most of the work:
  if (address == 0x2007)
  {
    pic_processor::set_config_word(address, cfg_word);

    uint valid_pins = m_porta->getEnableMask();

    set_int_osc(false);
    // Careful these bits not adjacent
    fosc = ((cfg_word & CFG_FOSC2) >> 2) | (cfg_word & (CFG_FOSC0 | CFG_FOSC1));
    if (osccon) 
    {
        osccon->set_config_xosc(fosc < 3);
        osccon->set_config_irc(fosc == 4 || fosc == 5);
    }
    switch(fosc)
    {
    case 0:  // LP oscillator: low power crystal is on RA6 and RA7
    case 1:     // XT oscillator: crystal/resonator is on RA6 and RA7
    case 2:     // HS oscillator: crystal/resonator is on RA6 and RA7
        break;

    case 3:     // EC:  RA6 is an I/O, RA7 is a CLKIN
    case 6:  // ER oscillator: RA6 is an I/O, RA7 is a CLKIN
        valid_pins =  (valid_pins & 0x7f)|0x40;
        break;

    case 4:  // INTRC: Internal Oscillator, RA6 and RA7 are I/O's
        set_int_osc(true);
        valid_pins |= 0xc0;
        break;

    case 5:  // INTRC: Internal Oscillator, RA7 is an I/O, RA6 is CLKOUT
        set_int_osc(true);
        valid_pins = (valid_pins & 0xbf)|0x80;
        break;

    case 7:  // ER oscillator: RA6 is CLKOUT, resistor (?) on RA7 
        break;
    }
    // If the /MCLRE bit is set then RA5 is the MCLR pin, otherwise it's 
    // a general purpose I/O pin.
    if ((cfg_word & CFG_MCLRE)) 
    {
        assignMCLRPin(4);
    }
    else
    {
        unassignMCLRPin();
    }

    if (cfg_word & CFG_CCPMX)
        ccp1con.setIOpin(&((*m_portb)[0]));
    else
        ccp1con.setIOpin(&((*m_portb)[3]));
        
    if (valid_pins != m_porta->getEnableMask()) // enable new pins for IO
    {
        m_porta->setEnableMask(valid_pins);
        m_porta->setTris(m_trisa);
    }
    return true;
  }
  else if (address == 0x2008 )
  {
    cout << "p16f8x 0x" << hex << address << " config word 0x" << cfg_word << '\n';
    if (osccon) osccon->set_config_ieso(cfg_word & CFG2_IESO);
    return true;
  }
  return false;
}

//========================================================================

void P16F8x::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,2);
  m_configMemory->addConfigWord(0,new Config1(this));
  m_configMemory->addConfigWord(1,new ConfigWord("CONFIG2", 0,"Configuration Word",this,0x2008));
  wdt.initialize(true); // default WDT enabled
  wdt.set_timeout(0.000035);
  set_config_word(0x2007, 0x3fff);
}


//========================================================================
void  P16F8x::create(int eesize)
{
  set_hasSSP();
  create_iopin_map();

  _14bit_processor::create();

  osccon = new OSCCON_1(this, "osccon", "OSC Control");

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eesize);
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  P16X6X_processor::create_sfr_map();

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F8x::create_sfr_map();

}

void P16F8x::enter_sleep()
{
    tmr1l.sleep();
    osccon->sleep();
    _14bit_processor::enter_sleep();
}

void P16F8x::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
        tmr1l.wake();
        osccon->wake();
        _14bit_processor::exit_sleep();
    }
}

//========================================================================

P16F81x::P16F81x(const char *_name, const char *desc)
  : P16X6X_processor(_name,desc),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low"),
    osccon(0),
    osctune(this, "osctune", "OSC Tune")
{
  pir1_2_reg = new PIR1v2(this,"pir1","Peripheral Interrupt Register",&intcon_reg,&pie1);
  pir2_2_reg = new PIR2v2(this,"pir2","Peripheral Interrupt Register",&intcon_reg,&pie2);
  delete pir1;
  delete pir2;
  pir1 = pir1_2_reg;
  pir2 = pir2_2_reg;
}

P16F81x::~P16F81x()
{
  remove_sfr_register(osccon);
  remove_sfr_register(&osctune);
  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(get_eeprom()->get_reg_eedata());
  remove_sfr_register(get_eeprom()->get_reg_eeadr());
  remove_sfr_register(get_eeprom()->get_reg_eedatah());
  remove_sfr_register(get_eeprom()->get_reg_eeadrh());
  remove_sfr_register(get_eeprom()->get_reg_eecon1());
  remove_sfr_register(get_eeprom()->get_reg_eecon2());
  remove_sfr_register(&pie2);
  delete get_eeprom();
}

void P16F81x::create_iopin_map()
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

  if (hasSSP()) {
    ssp.initialize(
                get_pir_set(),    // PIR
                &(*m_portb)[4],   // SCK
                &(*m_portb)[5],   // SS
                &(*m_portb)[2],   // SDO
                &(*m_portb)[1],    // SDI
                m_trisb,          // i2c tris port
                SSP_TYPE_SSP
        );
  }
}

void P16F81x::create_sfr_map()
{
  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);
 
  //add_file_registers(0xa0, 0xef, 0);
  //add_file_registers(0xc0, 0xef, 0);
  //add_file_registers(0x110,0x16f,0);
  //add_file_registers(0x190,0x1ef,0);

  add_sfr_register(get_pir2(),   0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));
                                                                                
  pir_set_def.set_pir2(pir2);
                                                                                
  pie2.setPir(get_pir2());
  alias_file_registers(0x00,0x04,0x100);
  alias_file_registers(0x80,0x84,0x100);

  alias_file_registers(0x06,0x06,0x100);
  alias_file_registers(0x86,0x86,0x100);

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x10c);
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x10d);
  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x10e);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x10f);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x18c, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x18d);

  //alias_file_registers(0x0a,0x0b,0x080);  //Already done
  alias_file_registers(0x0a,0x0b,0x100);
  alias_file_registers(0x0a,0x0b,0x180);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  add_sfr_register(osccon, 0x8f, RegisterValue(0,0),"osccon");
  add_sfr_register(&osctune, 0x90, RegisterValue(0,0),"osctune");

  osccon->set_osctune(&osctune);
  osccon->write_mask = 0x70;
  osctune.set_osccon(osccon);

  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));
                                                                                
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));
                                                                                
  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(7);
                                                                                
  adcon1.setNumberOfChannels(5);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[4]);

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
  adcon1.setChannelConfiguration(13, 0x0f);
  adcon1.setChannelConfiguration(14, 0x01);
  adcon1.setChannelConfiguration(15, 0x0d);

  // set a2d modes where an3 is Vref+ 
  adcon1.setVrefHiConfiguration(1, 3);
  adcon1.setVrefHiConfiguration(3, 3);
  adcon1.setVrefHiConfiguration(5, 3);
  adcon1.setVrefHiConfiguration(8, 3);
  adcon1.setVrefHiConfiguration(10, 3);
  adcon1.setVrefHiConfiguration(11, 3);
  adcon1.setVrefHiConfiguration(12, 3);
  adcon1.setVrefHiConfiguration(13, 3);
  adcon1.setVrefHiConfiguration(15, 3);

  // set a2d modes where an2 is Vref-
  adcon1.setVrefLoConfiguration(8, 2);
  adcon1.setVrefLoConfiguration(11, 2);
  adcon1.setVrefLoConfiguration(12, 2);
  adcon1.setVrefLoConfiguration(13, 2);
  adcon1.setVrefLoConfiguration(15, 2);

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1 |
                         ADCON1::PCFG2 | ADCON1::PCFG3 , 0);
}

void P16F81x::set_out_of_range_pm(uint address, uint value)
{
  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

//========================================================================
bool P16F81x::set_config_word(uint address, uint cfg_word)
{
  enum {
    CFG_FOSC0 = 1<<0,
    CFG_FOSC1 = 1<<1,
    CFG_FOSC2 = 1<<4,
    CFG_MCLRE = 1<<5,
    CFG_CCPMX = 1<<12
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
    if ((cfg_word & CFG_MCLRE)) 
    {
        assignMCLRPin(4);
    }
    else
    {
        unassignMCLRPin();
    }

    if (cfg_word & CFG_CCPMX)
    {
        ccp1con.setIOpin(&((*m_portb)[2]));
    }
    else
    {
        ccp1con.setIOpin(&((*m_portb)[3]));
    }

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

void P16F81x::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,2);
  m_configMemory->addConfigWord(0,new Config1(this));
  m_configMemory->addConfigWord(1,new ConfigWord("CONFIG2", 0,"Configuration Word",this,0x2008));
  wdt.initialize(true); // default WDT enabled
  wdt.set_timeout(0.000035);
  set_config_word(0x2007, 0x3fff);
}

//========================================================================
void  P16F81x::create(int eesize)
{
  set_hasSSP();
  create_iopin_map();

  _14bit_processor::create();

   osccon = new OSCCON_1(this, "osccon", "OSC Control");

  EEPROM_WIDE *e;
  e = new EEPROM_WIDE(this,pir2);
  e->initialize(eesize);
  e->set_intcon(&intcon_reg);
  set_eeprom_wide(e);

  P16X6X_processor::create_sfr_map();

  status->rp_mask = 0x60;  // rp0 and rp1 are valid.
  indf->base_address_mask1 = 0x80; // used for indirect accesses above 0x100
  indf->base_address_mask2 = 0x1ff; // used for indirect accesses above 0x100

  P16F81x::create_sfr_map();
}

//========================================================================
//
Processor * P16F87::construct(const char *name)
{
  P16F87 *p = new P16F87(name);

  p->P16F8x::create(256);
  p->P16F87::create_sfr_map();
  p->create_invalid_registers ();

  return p;
}

P16F87::P16F87(const char *_name, const char *desc)
  : P16F8x(_name,desc)
{
  m_porta->setEnableMask(0xff);
  // trisa5 is an input only pin
  m_trisa->setEnableMask(0xdf);
}

void P16F87::create_sfr_map()
{
}
//========================================================================
//
Processor * P16F88::construct(const char *name)
{
  P16F88 *p = new P16F88(name);

  p->P16F88::create();
  p->create_invalid_registers ();

  return p;
}

P16F88::P16F88(const char *_name, const char *desc)
  : P16F87(_name,desc),
    ansel(this,"ansel", "Analog Select"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low")
{
}

P16F88::~P16F88()
{
  remove_sfr_register(&adresl);
  remove_sfr_register(&adresh);
  remove_sfr_register(&adcon0);
  remove_sfr_register(&adcon1);
  remove_sfr_register(&ansel);
}
void  P16F88::create()
{
    P16F8x::create(256);
    P16F88::create_sfr_map();
}

void P16F88::create_sfr_map()
{
  add_sfr_register(&adresl,  0x9e, RegisterValue(0,0));
  add_sfr_register(&adresh,  0x1e, RegisterValue(0,0));
                                                                                
  add_sfr_register(&adcon0, 0x1f, RegisterValue(0,0));
  add_sfr_register(&adcon1, 0x9f, RegisterValue(0,0));
  add_sfr_register(&ansel, 0x9b, RegisterValue(0x7f,0));

  ansel.setAdcon1(&adcon1);
  adcon0.setAdresLow(&adresl);
  adcon0.setAdres(&adresh);
  adcon0.setAdcon1(&adcon1);
  adcon0.setIntcon(&intcon_reg);
  adcon0.setA2DBits(10);
  adcon0.setPir(pir1);
  adcon0.setChannel_Mask(7);
                                                                                
  adcon1.setNumberOfChannels(7);
  adcon1.setIOPin(0, &(*m_porta)[0]);
  adcon1.setIOPin(1, &(*m_porta)[1]);
  adcon1.setIOPin(2, &(*m_porta)[2]);
  adcon1.setIOPin(3, &(*m_porta)[3]);
  adcon1.setIOPin(4, &(*m_porta)[4]);
  adcon1.setIOPin(5, &(*m_portb)[6]);
  adcon1.setIOPin(6, &(*m_portb)[7]);

  adcon1.setVrefHiConfiguration(2, 3);
  adcon1.setVrefHiConfiguration(3, 3);
                                                                                
  adcon1.setVrefLoConfiguration(1, 2);
  adcon1.setVrefLoConfiguration(3, 2);

/* Channel Configuration done dynamiclly based on ansel */
                                                                                
  adcon1.setValidCfgBits(ADCON1::VCFG0 | ADCON1::VCFG1 , 4);

 // Link the A/D converter to the Capture Compare Module
  ccp2con.setADCON(&adcon0);
}

//========================================================================
//
Processor * P16F818::construct(const char *name)
{
  P16F818 *p = new P16F818(name);

  p->P16F818::create();
  p->create_invalid_registers ();

  return p;
}

P16F818::P16F818(const char *_name, const char *desc)
  : P16F81x(_name,desc)
{
}

void  P16F818::create()
{
    P16F81x::create(128);
    P16F818::create_sfr_map();
}

void P16F818::create_sfr_map()
{
  alias_file_registers(0x40,0x7f,0x80);
  alias_file_registers(0x20,0x7f,0x100);
  alias_file_registers(0x20,0x7f,0x180);

}

//========================================================================
//
Processor * P16F819::construct(const char *name)
{
  P16F819 *p = new P16F819(name);

  p->P16F819::create();
  p->create_invalid_registers ();

  return p;
}

P16F819::P16F819(const char *_name, const char *desc)
  : P16F81x(_name,desc)
{
}

P16F819::~P16F819()
{
  delete_file_registers(0xc0,0xef);
  delete_file_registers(0x120,0x16f);
}

void  P16F819::create()
{
    P16F81x::create(256);
    P16F819::create_sfr_map();
}

void P16F819::create_sfr_map()
{
  add_file_registers(0xc0,0xef, 0);
  add_file_registers(0x120,0x16f, 0);
  alias_file_registers(0x70,0x7f,0x80);
  alias_file_registers(0x70,0x7f,0x100);
  alias_file_registers(0x20,0x7f,0x180);
}

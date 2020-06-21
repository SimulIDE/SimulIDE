/*
   Copyright (C) 2000 T. Scott Dattalo, Daniel Schudel

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
// p16x5x
//
//  This file supports:
//    P16C54
//    P16C55
//    P16C56


#include <stdio.h>
#include <iostream>
#include <string>

#include "packages.h"
#include "p16x5x.h"
#include "pic-ioports.h"

void P16C54::create_iopin_map()
{
#ifdef USE_PIN_MODULE_FOR_TOCKI
    IOPIN * tockipin;
#endif

  package = new Package(18);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin(17, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin(18, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 1, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta3"),3));
#ifdef USE_PIN_MODULE_FOR_TOCKI
  // RCP - attempt to add TOCKI without port register
  tockipin = new IOPIN("tocki");
  m_tocki->setPin ( tockipin );
  package->assign_pin( 3, tockipin );
  // RCP - End new code
#else
  package->assign_pin( 3, m_tocki->addPin(new IOPIN("tocki"),0));
#endif
  package->assign_pin( 4, 0);
  package->assign_pin( 5, 0);
  package->assign_pin( 6, m_portb->addPin(new IO_bi_directional("portb0"),0));
  package->assign_pin( 7, m_portb->addPin(new IO_bi_directional("portb1"),1));
  package->assign_pin( 8, m_portb->addPin(new IO_bi_directional("portb2"),2));
  package->assign_pin( 9, m_portb->addPin(new IO_bi_directional("portb3"),3));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional("portb4"),4));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional("portb5"),5));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional("portb6"),6));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional("portb7"),7));
  package->assign_pin(14, 0);
  package->assign_pin(15, 0);
  package->assign_pin(16, 0);
}


void P16C55::create_iopin_map()
{
  package = new Package(28);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin( 6, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 8, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 9, m_porta->addPin(new IO_bi_directional("porta3"),3));
#ifdef USE_PIN_MODULE_FOR_TOCKI
  // RCP - attempt to add TOCKI without port register
  tockipin = new IOPIN("tocki");
  m_tocki->setPin ( tockipin );
  package->assign_pin( 1, tockipin );
  // RCP - End new code
#else
  package->assign_pin( 1, m_tocki->addPin(new IOPIN("tocki"),0));
#endif
  package->assign_pin( 2, 0);
  package->assign_pin( 3, 0);
  package->assign_pin( 4, 0);
  package->assign_pin( 5, 0);

  package->assign_pin(10, m_portb->addPin(new IO_bi_directional("portb0"),0));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional("portb1"),1));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional("portb2"),2));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional("portb3"),3));
  package->assign_pin(14, m_portb->addPin(new IO_bi_directional("portb4"),4));
  package->assign_pin(15, m_portb->addPin(new IO_bi_directional("portb5"),5));
  package->assign_pin(16, m_portb->addPin(new IO_bi_directional("portb6"),6));
  package->assign_pin(17, m_portb->addPin(new IO_bi_directional("portb7"),7));

  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(19, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(20, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(21, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(22, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(23, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(24, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(25, m_portc->addPin(new IO_bi_directional("portc7"),7));

  package->assign_pin(26, 0);
  package->assign_pin(27, 0);
  package->assign_pin(28, 0);

}

void P16C54::create_sfr_map()
{
  add_file_registers(0x07, 0x1f, 0x00);

  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,    0x04);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_portb, 0x06);

  add_sfr_register(option_reg,  0xffffffff, RegisterValue(0xff,0));
  add_sfr_register(m_trisa,  0xffffffff, RegisterValue(0x1f,0));
  add_sfr_register(m_trisb,  0xffffffff, RegisterValue(0xff,0));
#ifndef USE_PIN_MODULE_FOR_TOCKI
  add_sfr_register(m_tocki,  0xffffffff, RegisterValue(0x01,0));
  add_sfr_register(m_trist0, 0xffffffff, RegisterValue(0x01,0));
#endif
}

void P16C54::create()
{
  create_iopin_map();
  _12bit_processor::create();
}

Processor * P16C54::construct(const char *name)
{
  P16C54 *p = new P16C54(name);

  p->pc->set_reset_address(0x1ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();

  return p;
}

P16C54::P16C54(const char *_name, const char *desc)
  : _12bit_processor(_name,desc)
{
  m_porta = new PicPortRegister(this,"porta","",8,0x1f);
  m_trisa = new PicTrisRegister(this,"trisa","",m_porta, false);

  m_portb = new PicPortRegister(this,"portb","",8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb","",m_portb, false);

#ifdef USE_PIN_MODULE_FOR_TOCKI
//  RCP - Attempt to assign TOCKI without a port register
  m_tocki = new PinModule();
  cout << "c54 contructor assigning tmr0\n";
  tmr0.set_cpu(this, m_tocki);
#else
  m_tocki = new PicPortRegister(this,"tockiport","",8,0x01);
  m_trist0 = new PicTrisRegister(this,"trist0","",m_tocki, false);
//  cout << "c54 contructor assigning tmr0 to tocki register\n";
  tmr0.set_cpu(this, m_tocki, 0,option_reg);
#endif
  tmr0.start(0);
}

P16C54::~P16C54()
{
  delete_file_registers(0x07, 0x1f);

//  add_sfr_register(indf);

  remove_sfr_register(&tmr0);

//  add_sfr_register(pcl);
//  add_sfr_register(status);
//  add_sfr_register(fsr);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_portb);

//  delete_sfr_register(option_reg);
  delete_sfr_register(m_trisa);
  delete_sfr_register(m_trisb);
#ifndef USE_PIN_MODULE_FOR_TOCKI
  delete_sfr_register(m_tocki);
  delete_sfr_register(m_trist0);
#endif

}

void P16C54::tris_instruction(uint tris_register)
{
   switch (tris_register)
   {
      case 5:
        m_trisa->put(Wget());
        //trace.write_TRIS(m_trisa->value.get());
        break;
      case 6:
        m_trisb->put(Wget());
        //trace.write_TRIS(m_trisb->value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}

void P16C55::create_sfr_map()
{
  P16C54::create_sfr_map();

  delete_file_registers(0x07, 0x07);
  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc,  0xffffffff, RegisterValue(0xff,0));
}

void P16C55::create()
{
  P16C54::create();
}

Processor * P16C55::construct(const char *name)
{
  P16C55 *p = new P16C55(name);

  p->pc->set_reset_address(0x1ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();

  return p;
}

P16C55::P16C55(const char *_name, const char *desc)
  : P16C54(_name,desc)
{
  m_portc = new PicPortRegister(this,"portc","",8,0xff);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false);
}

P16C55::~P16C55()
{
  delete_sfr_register(m_trisc);
}

void P16C55::tris_instruction(uint tris_register)
{

   switch (tris_register)
   {
      case 5:
        m_trisa->put(Wget());
        //trace.write_TRIS(m_trisa->value.get());
        break;
      case 6:
        m_trisb->put(Wget());
        //trace.write_TRIS(m_trisb->value.get());
        break;
      case 7:
        m_trisc->put(Wget());
        //trace.write_TRIS(m_trisc->value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}

Processor * P16C56::construct(const char *name)
{
  P16C56 *p = new P16C56(name);

  p->pc->set_reset_address(0x3ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();

  return p;
}

P16C56::P16C56(const char *_name, const char *desc)
  : P16C54(_name,desc)
{
}


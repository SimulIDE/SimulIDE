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
#include <string>

#include "config.h"
#include "p17c75x.h"

#if 0
void _68pins::create_iopin_map(void)
{
  // ---- This is probably going to be moved:
  porta = new PORTA;
  portb = new PORTB;
  portc = new PORTC;
  portd = new PORTD;
  porte = new PORTE;
  portf = new PORTF;
  portg = new PORTG;

  if(verbose)
    cout << "Create i/o pin map\n";
  // Build the links between the I/O Ports and their tris registers.
  porta->tris = &ddra;

  portb->tris = &ddrb;
  ddrb.port = portb;

  portc->tris = &ddrc;
  ddrc.port = portc;

  portd->tris = &ddrd;
  ddrd.port = portd;

  porte->tris = &ddre;
  ddre.port = porte;

  portf->tris = &ddrf;
  ddrf.port = portf;

  portg->tris = &ddrg;
  ddrg.port = portg;

  // And give them a more meaningful name.
  ddrb.new_name("ddrb");
  ddrc.new_name("ddrc");
  ddrd.new_name("ddrd");
  ddre.new_name("ddre");
  ddrf.new_name("ddrf");
  ddrg.new_name("ddrg");

  // Define the valid I/O pins.
  porta->valid_iopins = 0x3f;
  portb->valid_iopins = 0xff;
  portc->valid_iopins = 0xff;
  portd->valid_iopins = 0xff;
  porte->valid_iopins = 0x0f;
  portf->valid_iopins = 0xff;
  portg->valid_iopins = 0xff;


  // Now Create the package and place the I/O pins

  create_pkg(68);

  // Vdd and Vss pins
  assign_pin(2, 0);
  assign_pin(20, 0);
  assign_pin(37, 0);
  assign_pin(49, 0);
  assign_pin(19, 0);
  assign_pin(36, 0);
  assign_pin(53, 0);
  assign_pin(68, 0);

  // AVdd and AVss pins
  assign_pin(29, 0);
  assign_pin(30, 0);

  // NC pins
  assign_pin(1, 0);
  assign_pin(18, 0);
  assign_pin(35, 0);
  assign_pin(52, 0);

  // Test pin
  assign_pin(17, 0);

  // Reset pin
  assign_pin(16, 0);
  
  // Oscillator pins
  assign_pin(50, 0);
  assign_pin(51, 0);

  assign_pin(60, new IOPIN(porta, 0));
  assign_pin(44, new IOPIN(porta, 1));
  assign_pin(45, new IO_bi_directional_pu(porta, 2));
  assign_pin(46, new IO_bi_directional_pu(porta, 3));
  assign_pin(43, new IO_bi_directional(porta, 4));
  assign_pin(42, new IO_bi_directional(porta, 5));

  assign_pin(59, new IO_bi_directional(portb, 0));
  assign_pin(58, new IO_bi_directional(portb, 1));
  assign_pin(54, new IO_bi_directional(portb, 2));
  assign_pin(57, new IO_bi_directional(portb, 3));
  assign_pin(56, new IO_bi_directional(portb, 4));
  assign_pin(55, new IO_bi_directional(portb, 5));
  assign_pin(47, new IO_bi_directional(portb, 6));
  assign_pin(48, new IO_bi_directional(portb, 7));

  assign_pin(3,  new IO_bi_directional(portc, 0));
  assign_pin(67, new IO_bi_directional(portc, 1));
  assign_pin(66, new IO_bi_directional(portc, 2));
  assign_pin(65, new IO_bi_directional(portc, 3));
  assign_pin(64, new IO_bi_directional(portc, 4));
  assign_pin(63, new IO_bi_directional(portc, 5));
  assign_pin(62, new IO_bi_directional(portc, 6));
  assign_pin(61, new IO_bi_directional(portc, 7));

  assign_pin(11, new IO_bi_directional(portd, 0));
  assign_pin(10, new IO_bi_directional(portd, 1));
  assign_pin(9,  new IO_bi_directional(portd, 2));
  assign_pin(8,  new IO_bi_directional(portd, 3));
  assign_pin(7,  new IO_bi_directional(portd, 4));
  assign_pin(6,  new IO_bi_directional(portd, 5));
  assign_pin(5,  new IO_bi_directional(portd, 6));
  assign_pin(4,  new IO_bi_directional(portd, 7));

  assign_pin(12, new IO_bi_directional(porte, 0));
  assign_pin(13, new IO_bi_directional(porte, 1));
  assign_pin(14, new IO_bi_directional(porte, 2));
  assign_pin(15, new IO_bi_directional(porte, 3));

  assign_pin(28,  new IO_bi_directional(portf, 0));
  assign_pin(27, new IO_bi_directional(portf, 1));
  assign_pin(26, new IO_bi_directional(portf, 2));
  assign_pin(25, new IO_bi_directional(portf, 3));
  assign_pin(24, new IO_bi_directional(portf, 4));
  assign_pin(23, new IO_bi_directional(portf, 5));
  assign_pin(22, new IO_bi_directional(portf, 6));
  assign_pin(21, new IO_bi_directional(portf, 7));

  assign_pin(34, new IO_bi_directional(portg, 0));
  assign_pin(33, new IO_bi_directional(portg, 1));
  assign_pin(32, new IO_bi_directional(portg, 2));
  assign_pin(31, new IO_bi_directional(portg, 3));
  assign_pin(38, new IO_bi_directional(portg, 4));
  assign_pin(39, new IO_bi_directional(portg, 5));
  assign_pin(41, new IO_bi_directional(portg, 6));
  assign_pin(40, new IO_bi_directional(portg, 7));

}
#endif


//========================================================================
//
// Pic 17C7xx
//


Processor * P17C7xx::construct(const char *name)
{

  P17C7xx *p = new P17C7xx;

  cout << " 17c7xx construct\n";

  p->create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c7xx");
  return p;

}

P17C7xx::P17C7xx()
  : cpusta(this,"cpusta","")
{
  //_16bit_processor::create();
  //  create_iopins(iopin_map, num_of_iopins);
  name_str = "p17c7xx";
}

void  P17C7xx::create(int ram_top)
{
  cout << "p17c7xx create\n";


  create_iopin_map();

  //_16bit_processor::create();

  // FIXME - TSD the 17c7xx is derived from the 16bit_processor,
  // but it can call the _16bit_processor::create member function
  // (because it assumes the 16bit processor is an 18cxxx device)

  pic_processor::create();

  fast_stack.init(this);
  /*
  ind0.init(this);
  ind1.init(this);
  ind2.init(this);
  */
  tmr0l.initialize();
  intcon.initialize();

  //usart.initialize(this);
  //tbl.initialize(this);
  //tmr0l.start(0);

  //  create_iopin_map();
  //  create_sfr_map();

  add_file_registers(0x0, ram_top, 0);
}

void P17C7xx::create_sfr_map()
{  
}

//========================================================================
//
Processor * P17C75x::construct(const char *name)
{
  P17C75x *p = new P17C75x;

  p->create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c75x");
  return p;
}

void P17C75x::create(int ram_top)
{
  P17C7xx::create(ram_top);

  cout << "p17c75x parent created\n";
  P17C75x::create_sfr_map();
  cout << "p17c75x sfr map created\n";
  cout << "p17c75x parent created\n";
}

P17C75x::P17C75x()
{
  //if(verbose)
    cout << "17c75x constructor, type = " << isa() << '\n';
}

void P17C75x::create_sfr_map()
{
#if 0 
  if (verbose) 
    cout << "creating p17c75x common registers\n";
  
  add_file_registers(0x01A, 0x01f, 0x100);
  alias_file_registers(0x1A, 0x1f, 0x200);
  alias_file_registers(0x1A, 0x1f, 0x300);
  alias_file_registers(0x1A, 0x1f, 0x400);
  alias_file_registers(0x1A, 0x1f, 0x500);
  alias_file_registers(0x1A, 0x1f, 0x600);
  alias_file_registers(0x1A, 0x1f, 0x700);
  
  add_file_registers(0x020, 0x0ff, 0);
  add_file_registers(0x120, 0x1ff, 0);
  add_file_registers(0x220, 0x2ff, 0);
  add_file_registers(0x320, 0x3ff, 0);

  //Unbanked registers
  add_sfr_register(&ind0.indf,    0x00,    0, "indf0");
  add_sfr_register(&ind0.fsrl,	  0x01,    0, "fsr0"); // Indirect addressing

  add_sfr_register(&pcl,          0x02,    0, "pcl");
  add_sfr_register(&pclath,       0x03,    0, "pclath");   // Program counter

  add_sfr_register(&status,       0x04, 0xf0, "alusta");  // ALU status
  //  add_sfr_register(&t0sta,        0x05,    0, "t0sta");  // Timer0 Status
  add_sfr_register(&cpusta,       0x06, 0x3C, "cpusta");  // CPU status
  //  add_sfr_register(&intsta,       0x07,    0, "intsta");  
  add_sfr_register(&ind1.indf,    0x08,    0, "indf1");
  add_sfr_register(&ind1.fsrl,    0x09,    0, "fsr1");
  add_sfr_register(&W,            0x0a,    0, "wreg");
  //  add_sfr_register(&tmr0l,        0x0b,    0, "tmr0l");  // Timer0 registers
  //  add_sfr_register(&tmr0h,        0x0c,    0, "tmr0h");
  //  add_sfr_register(&tblptrl,      0x0d,    0, "tblptrl");  // Program memory table pointer
  //  add_sfr_register(&tblptrh,      0x0e,    0, "tblptrh");
  add_sfr_register(&bsr,          0x0f,    0, "bsr");  // Bank select register

  add_sfr_register(&prodl,        0x18,    0, "prodl");  // 16 bit product registers
  add_sfr_register(&prodh,        0x19,    0, "prodh");
  alias_file_registers(0x18, 0x19, 0x100);
  alias_file_registers(0x18, 0x19, 0x200);
  alias_file_registers(0x18, 0x19, 0x300);
  alias_file_registers(0x18, 0x19, 0x400);
  alias_file_registers(0x18, 0x19, 0x500);
  alias_file_registers(0x18, 0x19, 0x600);
  alias_file_registers(0x18, 0x19, 0x700);

  // Bank 0
  add_sfr_register(porta,        0x10,    0, "porta");  // PortA bits
  add_sfr_register(&ddrb,         0x11, 0xff, "ddrb");  // Data direction register of portA
  add_sfr_register(portb,        0x12,    0, "portb");
  /*
  add_sfr_register(&rcsta1,       0x13,    0, "rcsta1");  // Serial port 1 rec. status register
  add_sfr_register(&rcreg1,       0x14,    0, "rcreg1");  // Serial port 1 receive register
  add_sfr_register(&txsta1,       0x15,    0, "txsta1");
  add_sfr_register(&txreg1,       0x16,    0, "txreg1");
  add_sfr_register(&spbrg1,       0x17,    0, "spbrg1");
  */
  // Bank 1
  add_sfr_register(&ddrc,         0x110, 0xff, "ddrc");
  add_sfr_register(portc,        0x111,    0, "portc");
  add_sfr_register(&ddrd,         0x112, 0xff, "ddrd");
  add_sfr_register(portd,        0x113,    0, "portd");
  add_sfr_register(&ddre,         0x114, 0x0f, "ddre");
  add_sfr_register(porte,        0x115,    0, "porte");
  add_sfr_register(&pir1,         0x116, 0x02, "pir1"); 
  add_sfr_register(&pie1,         0x117,    0, "pie1");

  // Bank 2
  /*
  add_sfr_register(&tmr1,         0x210,    0, "tmr1");
  add_sfr_register(&tmr2,         0x211,    0, "tmr2");
  add_sfr_register(&tmr3l,        0x212,    0, "tmr3l");
  add_sfr_register(&tmr3h,        0x213,    0, "tmr3h");
  add_sfr_register(&pr1,          0x214,    0, "pr1");   // Timer1's period register
  add_sfr_register(&pr2,          0x215,    0, "pr2");   // Timer2's period register
  add_sfr_register(&pr3l,         0x216,    0, "pr3l");   // Timer3's period registers
  add_sfr_register(&pr3h,         0x217,    0, "pr3h");   

  // Bank 3
  add_sfr_register(&pw1dcl,       0x310,    0, "pw1dcl");
  add_sfr_register(&pw2dcl,       0x311,    0, "pw2dcl");
  add_sfr_register(&pw1dch,       0x312,    0, "pw1dch");
  add_sfr_register(&pw2dch,       0x313,    0, "pw2dch");
  add_sfr_register(&ca2l,         0x314,    0, "ca2l");   // Capture2 registers
  add_sfr_register(&ca2h,         0x315,    0, "ca2h");
  add_sfr_register(&tcon1,        0x316,    0, "tcon1");
  add_sfr_register(&tcon2,        0x317,    0, "tcon2");
  */
  // Bank 4
  add_sfr_register(&pir2,         0x410,    0, "pir2");
  add_sfr_register(&pie2,         0x411,    0, "pie2");
  //add_sfr_register(&     ,        0x412);
  /*
  add_sfr_register(&rcsta2,       0x413,    0, "rcsta2");
  add_sfr_register(&rcreg2,       0x414,    0, "rcreg2");
  add_sfr_register(&txsta2,       0x415, 0x02, "txsta2");
  add_sfr_register(&txreg2,       0x416,    0, "txreg2");
  add_sfr_register(&spbrg2,       0x417,    0, "spbrg2");
  */
  // Bank 5
  add_sfr_register(&ddrf,         0x510, 0xff, "ddrf");
  add_sfr_register(portf,        0x511,    0, "portf");
  add_sfr_register(&ddrg,         0x512, 0xff, "ddrg");
  add_sfr_register(portg,        0x513,    0, "portg");
  /*
  add_sfr_register(&adcon0,       0x514,    0, "adcon0");
  add_sfr_register(&adcon1,       0x515,    0, "adcon1");
  add_sfr_register(&adresl,       0x516,    0, "adresl");
  add_sfr_register(&adresh,       0x517,    0, "adresh");
  */
  // Bank 6
  /*
  add_sfr_register(&sspadd,       0x610,    0, "sspadd");  // Synchronous serial port registers
  add_sfr_register(&sspcon1,      0x611,    0, "sspcon1");
  add_sfr_register(&sspcon2,      0x612,    0, "sspcon2");
  add_sfr_register(&sspstat,      0x613,    0, "sspstat");
  add_sfr_register(&sspbuf,       0x614,    0, "sspbuf");
  */
  //add_sfr_register(&              0x615);
  //add_sfr_register(&              0x616);
  //add_sfr_register(&              0x617);

  // Bank 7
  /*
  add_sfr_register(&pw3dcl,       0x710,    0, "pw3dcl");
  add_sfr_register(&pw3dch,       0x711,    0, "pw3dch");
  add_sfr_register(&ca3l,         0x712,    0, "ca3l");
  add_sfr_register(&ca3h,         0x713,    0, "ca3h");
  add_sfr_register(&ca4l,         0x714,    0, "ca4l");
  add_sfr_register(&ca4h,         0x715,    0, "ca4h");
  add_sfr_register(&tcon3,        0x716,    0, "tcon3");
  //add_sfr_register(&              0x717);
  */
  // Initialize all of the register cross linkages

  // All of the status bits on the 16bit core are writable
  status.write_mask = 0xff;

#endif
}

//========================================================================
//
Processor * P17C756::construct(const char *name)
{
  P17C756 *p = new P17C756;

  cout << " 17c756 construct\n";

  p->P17C7xx::create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c756");
  return p;
}

void P17C756::create()
{

  create_iopin_map();


  P17C756::create_sfr_map();
  //  create_iopin_map(&iopin_map, &num_of_iopins);

  _16bit_processor::create();

  //  create_iopins(iopin_map, num_of_iopins);

}

void P17C756::create_sfr_map()
{

  cout << "create_sfr_map P17C756\n";
}

P17C756::P17C756()
{
}

//------------------------------------------------------------------------
//
Processor * P17C756A::construct(const char *name)
{

  P17C756A *p = new P17C756A;

  cout << " 17c756a construct\n";

  p->P17C7xx::create(0x1fff);
  //p->create_invalid_registers ();

  p->new_name("p17c756a");
  return p;
}

P17C756A::P17C756A()
{
}

void P17C756A::create()
{
  create_iopin_map();

  P17C756A::create_sfr_map();
  //  create_iopin_map(&iopin_map, &num_of_iopins);

  _16bit_processor::create();

  //  create_iopins(iopin_map, num_of_iopins);
}

void P17C756A::create_sfr_map()
{
}

//========================================================================
//
Processor * P17C752::construct(const char *name)
{
  P17C752 *p = new P17C752;

  cout << " 17c752 construct\n";

  p->P17C7xx::create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c752");
  return p;
}

void P17C752::create()
{

  create_iopin_map();


  P17C752::create_sfr_map();
  //  create_iopin_map(&iopin_map, &num_of_iopins);

  _16bit_processor::create();

  //  create_iopins(iopin_map, num_of_iopins);

}

void P17C752::create_sfr_map()
{

  cout << "create_sfr_map P17C752\n";
}

P17C752::P17C752()
{
}

//========================================================================
//
Processor * P17C762::construct(const char *name)
{
  P17C762 *p = new P17C762;

  p->P17C7xx::create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c762");
  return p;
}

void P17C762::create()
{
  create_iopin_map();

  P17C762::create_sfr_map();
  //  create_iopin_map(&iopin_map, &num_of_iopins);

  _16bit_processor::create();

  //  create_iopins(iopin_map, num_of_iopins);
}

void P17C762::create_sfr_map()
{

  cout << "create_sfr_map P17C762\n";
}

P17C762::P17C762()
{
}

//========================================================================
//
Processor * P17C766::construct(const char *name)
{
  P17C766 *p = new P17C766;

  p->P17C7xx::create(0x1fff);
  p->create_invalid_registers ();

  p->new_name("p17c766");
  return p;
}

void P17C766::create()
{
  create_iopin_map();

  P17C766::create_sfr_map();
  //  create_iopin_map(&iopin_map, &num_of_iopins);

  _16bit_processor::create();

  //  create_iopins(iopin_map, num_of_iopins);
}

void P17C766::create_sfr_map()
{
}

P17C766::P17C766()
{
}




/*
   Copyright (C) 1998-2002 T. Scott Dattalo

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

#ifndef __P16F62X_H__
#define __P16F62X_H__

#include "p16x6x.h"

#include "eeprom.h"
#include "comparator.h"

/***************************************************************************
 *
 * Include file for:  P16F627, P16F628, P16F648
 *
 *
 * The F62x devices are quite a bit different from the other PICs. The class
 * heirarchy is similar to the 16F84.
 * 
 *
 ***************************************************************************/

class P16F62x : public P16X6X_processor
{
public:
  P16F62x(const char *_name=0, const char *desc=0);
  ~P16F62x();

  USART_MODULE usart;
  ComparatorModule comparator;

  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F627_;};

  virtual uint register_memory_size () const { return 0x200;};

  virtual uint program_memory_size() { return 0; };

  virtual void create_sfr_map();

  // The f628 (at least) I/O pins depend on the Fosc Configuration bits.
  virtual bool set_config_word(uint address, uint cfg_word);

  virtual void create(int ram_top, uint eeprom_size);
  virtual void create_iopin_map();

  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F62x expects to have a PIR capable EEPROM
    assert(0);
  }
  virtual void set_eeprom_pir(EEPROM_PIR *ep) {
    eeprom = ep;
  }
  virtual EEPROM_PIR *get_eeprom() { return ((EEPROM_PIR *)eeprom); }
};

class P16F627 : public P16F62x
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F627_;};

  virtual uint program_memory_size() const { return 0x400; };

  P16F627(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

class P16F628 : public P16F627
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F628_;};

  virtual uint program_memory_size() const { return 0x800; };

  P16F628(const char *_name=0, const char *desc=0);
  ~P16F628();
  static Processor *construct(const char *name);
};

class P16F648 : public P16F628
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F648_;};

  virtual uint program_memory_size() const { return 0x1000; };
  virtual void create_sfr_map();

  P16F648(const char *_name=0, const char *desc=0);
  ~P16F648();
  static Processor *construct(const char *name);
};

#endif

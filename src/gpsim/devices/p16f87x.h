/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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

#ifndef __P16F87X_H__
#define __P16F87X_H__

#include "p16x7x.h"

#include "eeprom.h"
#include "comparator.h"

class IOPORT;


class P16F871 : public P16C64   // The 74 has too much RAM and too many CCPs
{
 public:
  // XXX
  // This pir1_2, pir2_2 stuff is not particularly pretty.  It would be
  // better to just tell C++ to redefine pir1 and pir2 and PIR1v2 and
  // PIR2v2, but C++ only supports covariance in member function return
  // values.
  PIR2v2 *pir2_2_reg;

  virtual PIR *get_pir2() { return (pir2_2_reg); }

  ADCON0 adcon0;
  ADCON1 adcon1;
  sfr_register  adres;
  sfr_register  adresl;

  USART_MODULE usart;

  // That now brings us up to spec with the 74 as far as we need to be

  
  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F871_;};
  virtual uint program_memory_size() const { return 0x0800; };
  virtual uint eeprom_memory_size() const { return 64; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F871(const char *_name=0, const char *desc=0);
  ~P16F871();
  static Processor *construct(const char *name);

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F871 expect a wide EEPROM
    assert(0);

  }
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


private:

};




class P16F873 : public P16C73
{

 public:

  sfr_register adresl;

  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F873_;};
  virtual uint program_memory_size() const { return 0x1000; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F873(const char *_name=0, const char *desc=0);
  ~P16F873();

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F873 expect a wide EEPROM
    assert(0);
  }
  virtual uint eeprom_memory_size() const { return 128; };
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }
  static Processor *construct(const char *name);

private:

};


class P16F873A : public P16F873
{
public:
  ComparatorModule comparator;
  virtual PROCESSOR_TYPE isa(){return _P16F873A_;};

  void create_sfr_map();
  void create();

  P16F873A(const char *_name=0, const char *desc=0);
  ~P16F873A();
  static Processor *construct(const char *name);
};


class P16F876 : public P16C73
{
 public:

  sfr_register adresl;
  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F876_;};
  virtual uint program_memory_size() const { return 0x2000; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F876(const char *_name=0, const char *desc=0);
  ~P16F876();
  static Processor *construct(const char *name);

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F873 expect a wide EEPROM
    assert(0);
  }
  virtual uint eeprom_memory_size() const { return 256; };
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }
};

class P16F876A : public P16F873A
{
 public:
  ComparatorModule comparator;
  virtual PROCESSOR_TYPE isa(){return _P16F876A_;};
  virtual uint program_memory_size() const { return 0x2000; };
  virtual uint eeprom_memory_size() const { return 256; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F876A(const char *_name=0, const char *desc=0);
  ~P16F876A();
  static Processor *construct(const char *name);
};


class P16F874 : public P16C74
{
public:
  ComparatorModule comparator;

  sfr_register adresl;

  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F874_;};
  virtual uint program_memory_size() const { return 0x1000; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F874(const char *_name=0, const char *desc=0);
  ~P16F874();
  static Processor *construct(const char *name);

  virtual uint eeprom_memory_size() const { return 128; };
  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F873 expect a wide EEPROM
    assert(0);
  }
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }
  //virtual bool hasSSP() { return true;}
};

class P16F877 : public P16F874
{
public:
  virtual PROCESSOR_TYPE isa(){return _P16F877_;};
  virtual uint program_memory_size() const { return 0x2000; };
  virtual uint eeprom_memory_size() const { return 256; };

  void create_sfr_map();
  void create();

  P16F877(const char *_name=0, const char *desc=0);
  ~P16F877();
  static Processor *construct(const char *name);
};

class P16F874A : public P16F874
{
public:
  ComparatorModule comparator;

  virtual void set_out_of_range_pm(uint address, uint value);

  virtual PROCESSOR_TYPE isa(){return _P16F874A_;};
  virtual uint program_memory_size() const { return 0x1000; };
  virtual uint eeprom_memory_size() const { return 256; };

  void create_sfr_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};

  P16F874A(const char *_name=0, const char *desc=0);
  ~P16F874A();
  static Processor *construct(const char *name);

};

class P16F877A : public P16F874A
{
public:
  ComparatorModule comparator;
  virtual PROCESSOR_TYPE isa(){return _P16F877A_;};
  virtual uint program_memory_size() const { return 0x2000; };
  virtual uint eeprom_memory_size() const { return 256; };

  void create_sfr_map();
  void create();

  P16F877A(const char *_name=0, const char *desc=0);
  ~P16F877A();
  static Processor *construct(const char *name);
};

#endif

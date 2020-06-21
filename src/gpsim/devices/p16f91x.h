/*
   Copyright (C) 2017 Roy R. Rankin

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

#ifndef __P16F91X_H__
#define __P16F91X_H__

#include "p16x7x.h"

#include "eeprom.h"
#include "comparator.h"
#include "lcd_module.h"

class IOPORT;

class P16F91X : public  _14bit_processor
{
public:

   P16F91X(const char *_name=0, const char *desc=0);
   ~P16F91X();

  INTCON_14_PIR    intcon_reg;
  T1CON   t1con;
  PIR    *pir1;
  PIE     pie1;
  PIR    *pir2;
  PIE     pie2;
  T2CON   t2con;
  PR2     pr2;
  TMR2    tmr2;
  TMRL    tmr1l;
  TMRH    tmr1h;
  CCPCON  ccp1con;
  CCPRL   ccpr1l;
  CCPRH   ccpr1h;
  CCPCON  ccp2con;
  CCPRL   ccpr2l;
  CCPRH   ccpr2h;
  PCON    pcon;
  LVDCON_14  lvdcon;
  SSP_MODULE   ssp;
  PIR1v2 *pir1_2_reg;
  PIR_SET_2 pir_set_2_def;
  virtual PIR *get_pir1() { return (pir1_2_reg); }
  virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }
  PIR2v2 *pir2_2_reg;
  virtual PIR *get_pir2() { return (pir2_2_reg); }


  ADCON0_91X adcon0;
  ADCON1_16F adcon1;
  sfr_register  adres;
  sfr_register  adresl;
  ANSEL_P  ansel;

  USART_MODULE usart;

  LCD_MODULE  lcd_module;

  WDTCON	wdtcon;
  OSCCON  	*osccon;
  OSCTUNE	osctune;

  ComparatorModule comparator;



  PicPortRegister  *m_porta;
  PicTrisRegister  *m_trisa;

  PicPortBRegister *m_portb;
  PicTrisRegister  *m_trisb;
  WPU              *m_wpub;
  IOC              *m_iocb;

  PicPortRegister  *m_portc;
  PicTrisRegister  *m_trisc;

  PicPortRegister  *m_porte;
  PicTrisRegister  *m_trise;

  virtual void create_sfr_map();
  virtual void option_new_bits_6_7(uint bits);
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }
  virtual void update_vdd();
  virtual bool set_config_word(uint address, uint cfg_word);
  virtual void enter_sleep();
  virtual void exit_sleep();
};


class P16F91X_40 : public P16F91X   
{
 public:
  
  PicPortRegister  *m_portd;
  PicTrisRegister  *m_trisd;
  virtual void set_out_of_range_pm(uint address, uint value);

  virtual void create_sfr_map();
  virtual void create_iopin_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};


  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F917 expect a wide EEPROM
    assert(0);

  }

  P16F91X_40(const char *_name=0, const char *desc=0);
  ~P16F91X_40();

private:

};
class P16F91X_28 : public P16F91X   
{
 public:
  
  virtual void set_out_of_range_pm(uint address, uint value);

  virtual void create_sfr_map();
  virtual void create_iopin_map();
  void create();
  virtual uint register_memory_size () const { return 0x200;};


  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F917 expect a wide EEPROM
    assert(0);

  }

  P16F91X_28(const char *_name=0, const char *desc=0);
  ~P16F91X_28();

private:

};
class P16F917 : public P16F91X_40
{
 public:
  virtual PROCESSOR_TYPE isa(){return _P16F917_;};
  virtual uint program_memory_size() const { return 8192; };
  virtual uint register_memory_size () const { return 0x200;};

  P16F917(const char *_name=0, const char *desc=0);
  ~P16F917();
  static Processor *construct(const char *name);
  void create();
  void create_sfr_map();

private:

};
class P16F916 : public P16F91X_28
{
    public:
      virtual PROCESSOR_TYPE isa(){return _P16F916_;};
      virtual uint program_memory_size() const { return 8192; };
      virtual uint register_memory_size () const { return 0x200;};

      P16F916(const char *_name=0, const char *desc=0);
      ~P16F916();
      static Processor *construct(const char *name);
      void create();
      void create_sfr_map();

    private:
};


class P16F914 : public P16F91X_40
{
 public:
  virtual PROCESSOR_TYPE isa(){return _P16F914_;};
  virtual uint program_memory_size() const { return 4096; };
  virtual uint register_memory_size () const { return 0x200;};

  P16F914(const char *_name=0, const char *desc=0) : P16F91X_40(_name, desc) {};
  static Processor *construct(const char *name);
  void create();

private:

};

class P16F913 : public P16F91X_28
{
 public:
  virtual PROCESSOR_TYPE isa(){return _P16F913_;};
  virtual uint program_memory_size() const { return 4096; };
  virtual uint register_memory_size () const { return 0x200;};

  P16F913(const char *_name=0, const char *desc=0) : P16F91X_28(_name, desc) {};
  static Processor *construct(const char *name);
  void create();

private:

};

#endif

/*
   Copyright (C) 2010,2015	   Roy Rankin

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

#ifndef __P16F88X_H__
#define __P16F88X_H__

#include "p16x6x.h"


/***************************************************************************
 *
 * Include file for:  P16F887, P16F88
 *
 *
 *
 ***************************************************************************/

class P16F88x : public _14bit_processor
{
public:

  INTCON_14_PIR    intcon_reg;

  PicPortRegister  *m_porta;
  PicTrisRegister  *m_trisa;

  PicPortGRegister *m_portb;
  PicTrisRegister  *m_trisb;
  WPU              *m_wpu;
  IOC              *m_ioc;

  PicPortRegister  *m_portc;
  PicTrisRegister  *m_trisc;

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
  SSP_MODULE   ssp;
  PIR1v2 *pir1_2_reg;
  PIR2v3 *pir2_2_reg;
  PIR_SET_2 pir_set_2_def;

  OSCCON       *osccon;
  OSCTUNE      osctune;
  WDTCON       wdtcon;
  USART_MODULE usart;
  ComparatorModule2 comparator;
  VRCON   vrcon;
  SRCON   srcon;
  ANSEL  ansel;
  ANSEL_H  anselh;
  ADCON0 adcon0;
  ADCON1 adcon1;
  ECCPAS	eccpas;
  PWM1CON	pwm1con;
  PSTRCON	pstrcon;


  sfr_register  adresh;
  sfr_register  adresl;

  PicPortRegister  *m_porte;
  PicPSP_TrisRegister  *m_trise;


  P16F88x(const char *_name=0, const char *desc=0);
  ~P16F88x();
  virtual void set_out_of_range_pm(uint address, uint value);

//  virtual PROCESSOR_TYPE isa(){return _P16F88x_;};

  virtual uint register_memory_size () const { return 0x200;};

  virtual uint program_memory_size() { return 0; };
  virtual void option_new_bits_6_7(uint bits);

  virtual void create_sfr_map();

  // The f628 (at least) I/O pins depend on the Fosc Configuration bits.
  virtual bool set_config_word(uint address, uint cfg_word);


  virtual void create(int eesize);
  virtual void create_iopin_map();
  virtual void create_config_memory();

  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


  virtual PIR *get_pir1() { return (pir1); }
  virtual PIR *get_pir2() { return (pir2); }
  virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }
};


class P16F884 : public P16F88x
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F884_;};

  virtual uint program_memory_size() const { return 4096; };
  PicPSP_PortRegister  *m_portd;

  PicTrisRegister  *m_trisd;


  P16F884(const char *_name=0, const char *desc=0);
  ~P16F884();
  static Processor *construct(const char *name);

  virtual void create_sfr_map();
  virtual void create_iopin_map();
};

class P16F887 : public P16F884
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F887_;};

  virtual uint program_memory_size() const { return 8192; };


  P16F887(const char *_name=0, const char *desc=0);
  ~P16F887();
  static Processor *construct(const char *name);

  virtual void create_sfr_map();
};
class P16F882 : public P16F88x
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F882_;};

  virtual uint program_memory_size() const { return 2048; };


  P16F882(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);

  virtual void create_sfr_map();
  virtual void create_iopin_map();
};
class P16F883 : public P16F882
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F883_;};

  virtual uint program_memory_size() const { return 4096; };


  P16F883(const char *_name=0, const char *desc=0);
  ~P16F883();
  static Processor *construct(const char *name);

  virtual void create_sfr_map();
};

class P16F886 : public P16F882
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F886_;};

  virtual uint program_memory_size() const { return 8192; };

  P16F886(const char *_name=0, const char *desc=0);
  ~P16F886();
  static Processor *construct(const char *name);

  virtual void create_sfr_map();
};

class P16F631 :  public _14bit_processor
{
public:

  P16F631(const char *_name=0, const char *desc=0);
  virtual ~P16F631();

  T1CON   t1con;
  PIR    *pir1;
  PIR    *pir2;
  PIE     pie1;
  PIE     pie2;
  TMRL    tmr1l;
  TMRH    tmr1h;
  OSCTUNE  osctune;
  PCON    pcon;
  WDTCON  wdtcon;
  OSCCON  *osccon;
  VRCON_2   vrcon;
  SRCON   srcon;
  ANSEL  ansel;
  ComparatorModule2 comparator;
  ADCON0_12F  adcon0;
  ADCON1_16F  adcon1;


  EEPROM_WIDE *e;
  PIR1v2 *pir1_2_reg;
  PIR2v3 *pir2_3_reg;

  INTCON_14_PIR    intcon_reg;
  PIR_SET_2    pir_set_2_def;
  WPU              *m_wpua;
  WPU              *m_wpub;
  IOC              *m_ioca;
  IOC              *m_iocb;


  virtual PIR *get_pir2() { return (pir2); }
  virtual PIR *get_pir1() { return (pir1); }
  virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }



  PicPortGRegister  	*m_porta;
  PicTrisRegister  	*m_trisa;

  PicPortGRegister  	*m_portb;
  PicTrisRegister  	*m_trisb;

  PicPortRegister 	*m_portc;
  PicTrisRegister  	*m_trisc;

  a2d_stimulus 		*m_cvref;
  a2d_stimulus 		*m_v06ref;

  virtual PROCESSOR_TYPE isa(){return _P16F631_;}
  static Processor *construct(const char *name);
  void create(int);

  virtual void create_sfr_map();
  virtual void create_iopin_map();
  virtual void option_new_bits_6_7(uint bits);

  virtual uint program_memory_size() const { return 0x400; };
  virtual uint register_memory_size () const { return 0x200; }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) { eeprom = ep; }
  virtual void create_config_memory();
  virtual bool set_config_word(uint address, uint cfg_word);
};

class P16F677 : public P16F631
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F677_;};

  virtual uint program_memory_size() const { return 4096; };
  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


  P16F677(const char *_name=0, const char *desc=0);
  ~P16F677();
  static Processor *construct(const char *name);

  SSP_MODULE   ssp;

  ANSEL_H  anselh;
  sfr_register  adresh;
  sfr_register  adresl;
  virtual void create_sfr_map();
};

class P16F687 : public P16F677
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F687_;};

  virtual uint program_memory_size() const { return 2048; };
  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


  P16F687(const char *_name=0, const char *desc=0);
  ~P16F687();
  static Processor *construct(const char *name);
  TMRL    tmr1l;
  TMRH    tmr1h;
  PCON    pcon;

  USART_MODULE usart;

  virtual void create_sfr_map();
};

class P16F684 : public  _14bit_processor
{
public:

  ComparatorModule comparator;
  virtual PROCESSOR_TYPE isa(){return _P16F684_;};
  virtual uint program_memory_size() const { return 2048; };
  virtual uint register_memory_size () const { return 0x100;};
  virtual void create(int eesize);
  virtual void create_iopin_map();


  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }

  virtual void option_new_bits_6_7(uint bits);

  virtual void create_config_memory();
  virtual bool set_config_word(uint, uint);


  P16F684(const char *_name=0, const char *desc=0);
  virtual ~P16F684();
  static Processor *construct(const char *name);
  PicPortGRegister  *m_porta;
  PicTrisRegister  *m_trisa;

  PicPortRegister *m_portc;
  PicTrisRegister  *m_trisc;

  WPU              *m_wpua;
  IOC              *m_ioca;


  T1CON   t1con;
  T2CON   t2con;
  PIR1v3    *pir1;
  PIE     pie1;
  PR2     pr2;
  TMR2    tmr2;
  TMRL    tmr1l;
  TMRH    tmr1h;
  OSCTUNE  osctune;
  PCON    pcon;
  WDTCON  wdtcon;
  OSCCON  *osccon;
  ANSEL  ansel;
  ADCON0_12F  adcon0;
  ADCON1_16F  adcon1;
  sfr_register  adresh;
  sfr_register  adresl;

  CCPCON  ccp1con;
  CCPRL   ccpr1l;
  CCPRH   ccpr1h;
  ECCPAS	eccpas;
  PWM1CON	pwm1con;
  PSTRCON	pstrcon;
  PIR1v3 	*pir1_3_reg;
  INTCON_14_PIR    intcon_reg;
  PIR_SET_1     pir_set_def;
  IOC              *m_iocc;
  EEPROM_WIDE *e;

  virtual PIR_SET *get_pir_set() { return (&pir_set_def); }

  virtual void create_sfr_map();
};

class P16F685 : public P16F677
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F685_;};

  virtual uint program_memory_size() const { return 4096; };
  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


  P16F685(const char *_name=0, const char *desc=0);
  ~P16F685();
  static Processor *construct(const char *name);
  T2CON   t2con;
  PR2     pr2;
  TMR2    tmr2;
  TMRL    tmr1l;
  TMRH    tmr1h;
  CCPCON  ccp1con;
  CCPRL   ccpr1l;
  CCPRH   ccpr1h;
  PCON    pcon;
  ECCPAS	eccpas;
  PWM1CON	pwm1con;
  PSTRCON	pstrcon;

  virtual void create_sfr_map();
};

class P16F689 : public P16F687
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F689_;};

  virtual uint program_memory_size() const { return 4096; };
  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }


  P16F689(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};
class P16F690 : public P16F685
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F690_;};

  virtual uint program_memory_size() const { return 4096; };
  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F8x expects to have a PIR capable EEPROM
    assert(0);
  }

  virtual void set_eeprom_wide(EEPROM_WIDE *ep) { eeprom = ep; }
  
  virtual EEPROM_WIDE *get_eeprom() { return ((EEPROM_WIDE *)eeprom); }

  P16F690(const char *_name=0, const char *desc=0);
  ~P16F690();
  
  static Processor *construct(const char *name);
  CCPCON  ccp2con;
  CCPRL   ccpr2l;
  CCPRH   ccpr2h;

  USART_MODULE usart;

  virtual void create_sfr_map();
};
#endif

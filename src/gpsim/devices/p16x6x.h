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

#ifndef __P16X6X_H__
#define __P16X6X_H__

#include "14bit-processors.h"
#include "p16x8x.h"
#include "14bit-tmrs.h"
#include "intcon.h"
#include "pir.h"
#include "ssp.h"
#include "psp.h"
#include "eeprom.h"
#include "comparator.h"
#include "a2dconverter.h"


//
//   -- Define a class to contain most of the registers/peripherals
//      of a 16x6x device (where the second `x' is >= 3
//

class P16X6X_processor :  public Pic14Bit
{
public:

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
  PIR_SET_1 pir_set_def;
  SSP_MODULE   ssp;

  virtual uint program_memory_size() const { return 0x800; };
  virtual uint register_memory_size () const { return 0x100; }

  virtual void create_sfr_map();
  virtual PIR *get_pir2() { return (pir2); }
  virtual PIR *get_pir1() { return (pir1); }
  virtual PIR_SET *get_pir_set() { return (&pir_set_def); }

  P16X6X_processor(const char *_name=0, const char *desc=0);
  virtual ~P16X6X_processor();
};

/*********************************************************************
 *  class definitions for the 16c6x family of processors
 */

 class P16C61 : public P16X8X
{
    public:

        P16C61(const char *_name=0, const char *desc=0);
        virtual ~P16C61();

        virtual PROCESSOR_TYPE isa(){return _P16C61_;};
        virtual uint program_memory_size() const { return 0x400; };
        virtual void create();

        static Processor *construct(const char *name);

};

class P16C62 : public  P16X6X_processor
{
    public:

        P16C62(const char *_name=0, const char *desc=0);
        virtual ~P16C62();

        static Processor *construct(const char *name);

        TMR2_MODULE tmr2_module;
        virtual PROCESSOR_TYPE isa(){return _P16C62_;};

        virtual void create_sfr_map();

        virtual uint program_memory_size() const { return 0x800; };
        virtual void create_iopin_map();

        virtual void create();
};

class P16C63 : public  P16C62
{
    public:

        P16C63(const char *_name=0, const char *desc=0);
        virtual ~P16C63();

        USART_MODULE usart;

        virtual PROCESSOR_TYPE isa(){return _P16C63_;};

        virtual uint program_memory_size() const { return 0x1000; };

        static Processor *construct(const char *name);
        void create();
        void create_sfr_map();
};


class P16C64 : public  P16X6X_processor
{
    public:
    
        P16C64(const char *_name=0, const char *desc=0);
        virtual ~P16C64();
        
        // XXX
        // This pir1_2, pir2_2 stuff is not particularly pretty.  It would be
        // better to just tell C++ to redefine pir1 and pir2 and PIR1v2 and
        // PIR2v2, but C++ only supports covariance in member function return
        // values.
        PIR1v2 *pir1_2_reg;
        PIR_SET_2 pir_set_2_def;
        virtual PIR *get_pir1() { return (pir1_2_reg); }
        virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }


        PicPSP_PortRegister  *m_portd;

        PicTrisRegister  *m_trisd;

        PicPortRegister  *m_porte;
        PicPSP_TrisRegister  *m_trise;
        PSP               psp;

        static Processor *construct(const char *name);

        TMR2_MODULE tmr2_module;
        virtual PROCESSOR_TYPE isa(){return _P16C64_;};

        void create_sfr_map();

        virtual uint program_memory_size() const { return 0x800; };
        virtual void create();
        virtual void create_iopin_map();

        virtual bool hasSPS() {return false;}
};

class P16C65 : public  P16C64
{
    public:
    
        P16C65(const char *_name=0, const char *desc=0);
        virtual ~P16C65();

        USART_MODULE usart;

        virtual PROCESSOR_TYPE isa(){return _P16C65_;};

        virtual uint program_memory_size() const { return 0x1000; };

        static Processor *construct(const char *name);
        void create();
        void create_sfr_map();
};


class P16F630 :  public _14bit_processor
{
    public:

        P16F630(const char *_name=0, const char *desc=0);
        virtual ~P16F630();

        T1CON   t1con;
        PIR    *pir1;
        PIE     pie1;
        TMRL    tmr1l;
        TMRH    tmr1h;
        OSCCAL  osccal;

        EEPROM_WIDE *e;
        PIR1v3 *pir1_3_reg;

        INTCON_14_PIR    intcon_reg;
        ComparatorModule comparator;
        PIR_SET_1    pir_set_def;
        WPU              *m_wpu;
        IOC              *m_ioc;

        virtual PIR *get_pir2() { return (NULL); }
        virtual PIR *get_pir1() { return (pir1); }
        virtual PIR_SET *get_pir_set() { return (&pir_set_def); }

        PicPortGRegister  *m_porta;
        PicTrisRegister  *m_trisa;

        PicPortRegister *m_portc;
        PicTrisRegister  *m_trisc;

        virtual PROCESSOR_TYPE isa(){return _P16F630_;}
        static Processor *construct(const char *name);
        void create(int);

        virtual void create_sfr_map();
        virtual void create_iopin_map();
        virtual void option_new_bits_6_7(uint bits);

        virtual uint program_memory_size() const { return 0x400; };
        virtual uint register_memory_size () const { return 0x100; }

        virtual void set_eeprom_wide(EEPROM_WIDE *ep) { eeprom = ep; }
        virtual void create_config_memory();
        virtual bool set_config_word(uint address, uint cfg_word);
};

class P16F676 :  public P16F630
{
    public:
    
        P16F676(const char *_name=0, const char *desc=0);
        virtual ~P16F676();
        
        ANSEL  ansel;
        ADCON0_12F adcon0;
        ADCON1_16F adcon1;
        sfr_register  adresh;
        sfr_register  adresl;

        virtual PROCESSOR_TYPE isa(){return _P16F676_;}
        static Processor *construct(const char *name);
        virtual void create(int);
        virtual void create_sfr_map();
};
#endif

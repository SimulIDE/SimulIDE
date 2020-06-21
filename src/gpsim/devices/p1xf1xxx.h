/*
   Copyright (C) 2013,2014,2017 Roy R. Rankin

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
/****************************************************************
*                                                               *
*  Modified 2018 by Santiago Gonzalez    santigoro@gmail.com    *
*                                                               *
*****************************************************************/

#ifndef __P1xF1xxx_H__
#define __P1xF1xxx_H__

#include "p16f178x.h"
#include "p16f1503.h"

#include "14bit-processors.h"
#include "14bit-tmrs.h"
#include "intcon.h"
#include "pir.h"
#include "pie.h"
#include "eeprom.h"
#include "comparator.h"
#include "a2dconverter.h"
#include "pic-ioports.h"
#include "dsm_module.h"
#include "cwg.h"
#include "nco.h"
#include "clc.h"
#include "apfcon.h"

#define FOSC0 (1<<0)
#define FOSC1 (1<<1)
#define FOSC2 (1<<2)
#define IESO (1<<12)


class P12F1822 : public _14bit_e_processor
{
    public:
     ComparatorModule2 comparator;
      PIR_SET_2 pir_set_2_def;
      PIE     pie1;
      PIR    *pir1;
      PIE     pie2;
      PIR    *pir2;
      T2CON_64  t2con;
      PR2        pr2;
      TMR2    tmr2;
      T1CON_G   t1con_g;
      TMRL    tmr1l;
      TMRH    tmr1h;
      CCPCON      ccp1con;
      CCPRL            ccpr1l;
      CCPRH            ccpr1h;
      FVRCON      fvrcon;
      BORCON      borcon;
      ANSEL_P       ansela;
      ADCON0        adcon0;
      ADCON1_16F       adcon1;
      sfr_register  adresh;
      sfr_register  adresl;
      OSCCON_2        *osccon;
      OSCTUNE       osctune;
      OSCSTAT       oscstat;
      //OSCCAL  osccal;
      WDTCON        wdtcon;
      USART_MODULE       usart;
      SSP1_MODULE       ssp;
      APFCON      apfcon;
      PWM1CON      pwm1con;
      ECCPAS        ccp1as;
      PSTRCON       pstr1con;
      CPSCON0      cpscon0;
      CPSCON1      cpscon1;
      SR_MODULE      sr_module;
      EEPROM_EXTND *e;

      WPU              *m_wpua;
      IOC              *m_iocap;
      IOC              *m_iocan;
      IOCxF            *m_iocaf;
      PicPortIOCRegister  *m_porta;
      PicTrisRegister  *m_trisa;
      PicLatchRegister *m_lata;
      DACCON0         *m_daccon0;
      DACCON1         *m_daccon1;
      DSM_MODULE       dsm_module;

      virtual PIR *get_pir2() { return (NULL); }
      virtual PIR *get_pir1() { return (pir1); }
      virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }

      virtual EEPROM_EXTND *get_eeprom() { return ((EEPROM_EXTND *)eeprom); }

      virtual PROCESSOR_TYPE isa(){return _P12F1822_;};

      static Processor *construct(const char *name);
      P12F1822(const char *_name=0, const char *desc=0);
      ~P12F1822();
      virtual void create_sfr_map();

      virtual void set_out_of_range_pm(uint address, uint value);
      virtual void create_iopin_map();
      virtual void create(int ram_top, int eeprom_size, int dev_id);
      virtual uint register_memory_size () const { return 0x1000; }
      virtual void option_new_bits_6_7(uint bits);
      virtual uint program_memory_size() const { return 2048; }
      virtual void enter_sleep();
      virtual void exit_sleep();
      virtual void oscillator_select(uint mode, bool clkout);
      virtual void program_memory_wp(uint mode);
};

class P12LF1822 : public P12F1822
{
    public:

      virtual PROCESSOR_TYPE isa(){return _P12LF1822_;};

      static Processor *construct(const char *name);
      
      P12LF1822(const char *_name=0, const char *desc=0);
      ~P12LF1822();
      virtual void create(int ram_top, int eeprom_size, int dev_id);
};

class P12F1840 : public P12F1822
{
    public:
      static Processor *construct(const char *name);
      virtual uint program_memory_size() const { return 4096; }
      virtual void create(int ram_top, int eeprom_size, int dev_id);
      virtual PROCESSOR_TYPE isa(){return _P12F1840_;};
      
      P12F1840(const char *_name=0, const char *desc=0);
      ~P12F1840();

      sfr_register *vrefcon;
};

class P12LF1840 : public P12F1840
{
    public:
      static Processor *construct(const char *name);
      virtual void create(int ram_top, int eeprom_size, int dev_id);
      virtual PROCESSOR_TYPE isa(){return _P12LF1840_;};
      
      P12LF1840(const char *_name=0, const char *desc=0);
      ~P12LF1840();
};

class P16F1823 : public P12F1822
{
    public:
      ANSEL_P   anselc;
      virtual PROCESSOR_TYPE isa(){return _P16F1823_;};

     P16F1823(const char *_name=0, const char *desc=0);
      ~P16F1823();
      
      static Processor *construct(const char *name);
      virtual void create_sfr_map();
      virtual void create_iopin_map();
      virtual void create(int ram_top, int eeprom_size, int dev_id);

      PicPortBRegister *m_portc;
      PicTrisRegister  *m_trisc;
      PicLatchRegister *m_latc;
      WPU              *m_wpuc;
};

class P16LF1823 : public P16F1823
{
    public:
      virtual PROCESSOR_TYPE isa(){return _P16LF1823_;};

     P16LF1823(const char *_name=0, const char *desc=0);
      ~P16LF1823();
      
      static Processor *construct(const char *name);
      virtual void create(int ram_top, int eeprom_size, int dev_id);

};

class P16F1825 : public P16F1823
{
    public:
      static Processor *construct( const char *name );
      virtual uint program_memory_size() const { return 8*1024; }
      virtual void create( int ram_top, int eeprom_size, int dev_id );
      virtual PROCESSOR_TYPE isa(){ return _P16F1825_; };
      
      P16F1825( const char *_name=0, const char *desc=0 );
      ~P16F1825();
      
      PIE       pie3;
      PIR*      pir3;
      T2CON_64  t4con;
      PR2       pr4;
      TMR2      tmr4;
      T2CON_64  t6con;
      PR2       pr6;
      TMR2      tmr6;
      CCPCON    ccp2con;
      CCPRL     ccpr2l;
      CCPRH     ccpr2h;
      PWM1CON   pwm2con;
      ECCPAS    ccp2as;
      PSTRCON   pstr2con;
      CCPCON    ccp3con;
      CCPRL     ccpr3l;
      CCPRH     ccpr3h;
      CCPCON    ccp4con;
      CCPRL     ccpr4l;
      CCPRH     ccpr4h;
      CCPTMRS14 ccptmrs;
      APFCON    apfcon0;
      APFCON    apfcon1;
      sfr_register  inlvla;
      sfr_register  inlvlc;
};

class P16LF1825 : public P16F1825
{
    public:
      static Processor *construct(const char *name);
      virtual void create(int ram_top, int eeprom_size, int dev_id);
      virtual PROCESSOR_TYPE isa(){return _P16LF1825_;};
      
      P16LF1825(const char *_name=0, const char *desc=0);
      ~P16LF1825();
};
#endif //__P1xF1xxx_H__

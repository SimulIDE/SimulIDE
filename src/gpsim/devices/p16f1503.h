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

#ifndef __P16F1503_H__
#define __P16F1503_H__

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


class P16F1503 : public _14bit_e_processor
{
    public:
      ComparatorModule2 comparator;
      PIR_SET_2 pir_set_2_def;
      PIE     pie1;
      PIR    *pir1;
      PIE     pie2;
      PIR    *pir2;
      PIE     pie3;
      PIR    *pir3;
      T2CON_64	  t2con;
      PR2	  pr2;
      TMR2    tmr2;
      T1CON_G   t1con_g;
      TMRL    tmr1l;
      TMRH    tmr1h;
      FVRCON	fvrcon;
      BORCON	borcon;
      ANSEL_P 	ansela;
      ANSEL_P 	anselc;
      ADCON0  	adcon0;
      ADCON1_16F 	adcon1;
      ADCON2_TRIG	adcon2;
      sfr_register  adresh;
      sfr_register  adresl;
      OSCCON_2  	*osccon;
      OSCTUNE 	osctune;
      OSCSTAT 	oscstat;
      WDTCON  	wdtcon;
      SSP1_MODULE 	ssp;
      APFCON	apfcon1;
      PWMxCON	pwm1con;
      sfr_register  pwm1dcl;
      sfr_register  pwm1dch;
      PWMxCON	pwm2con;
      sfr_register  pwm2dcl;
      sfr_register  pwm2dch;
      PWMxCON	pwm3con;
      sfr_register  pwm3dcl;
      sfr_register  pwm3dch;
      PWMxCON	pwm4con;
      sfr_register  pwm4dcl;
      sfr_register  pwm4dch;
      CWG4		cwg;
      NCO		nco;
      CLCDATA	clcdata;
      CLC		clc1;
      CLC		clc2;
      OSC_SIM	frc;
      OSC_SIM	lfintosc;
      OSC_SIM	hfintosc;
    /* RRR
      ECCPAS        ccp1as;
      PSTRCON       pstr1con;
    */

      EEPROM_EXTND     *e;
      sfr_register     vregcon;
      WPU              *m_wpua;
      IOC              *m_iocap;
      IOC              *m_iocan;
      IOCxF            *m_iocaf;
      PicPortIOCRegister  *m_porta;
      PicTrisRegister  *m_trisa;
      PicLatchRegister *m_lata;
      DACCON0	   *m_daccon0;
      DACCON1	   *m_daccon1;

      PicPortBRegister  *m_portc;
      PicTrisRegister  *m_trisc;
      PicLatchRegister *m_latc;
      
      virtual PIR *get_pir2() { return (NULL); }
      virtual PIR *get_pir1() { return (pir1); }
      virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }
      virtual EEPROM_EXTND *get_eeprom() { return ((EEPROM_EXTND *)eeprom); }
      virtual uint program_memory_size() const { return 2048; }
      virtual uint register_memory_size () const { return 0x1000; }
      
     P16F1503(const char *_name=0, const char *desc=0);
      ~P16F1503();
      
      virtual void create_iopin_map();
      virtual void create_sfr_map();

      virtual void set_out_of_range_pm(uint address, uint value);
      virtual void create(int ram_top, int dev_id);
      virtual void option_new_bits_6_7(uint bits);
      virtual void enter_sleep();
      virtual void exit_sleep();
      virtual void oscillator_select(uint mode, bool clkout);
      virtual void program_memory_wp(uint mode);
      static Processor *construct(const char *name);

      uint ram_size;
};

class P16LF1503 : public P16F1503
{
    public:
      static Processor *construct(const char *name);
      
      P16LF1503(const char *_name=0, const char *desc=0);
      ~P16LF1503(){;}
};
#endif 

/*
   Copyright (C) 1998-2002 T. Scott Dattalo
   Copyright (C) 2006,2010,2013 Roy R. Rankin

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

#ifndef __COMPARATOR_H__
#define __COMPARATOR_H__

#include "14bit-tmrs.h"
#include <cstdio>
#include <math.h>

/***************************************************************************
 *
 * Include file for: Processors with dual comparators and Voltage Refarence
 *
 * 
 *
 ***************************************************************************/

#define CFG_MASK 0xf
#define CFG_SHIFT 4

class CMSignalSource;
class CMxSignalSource;
class VRSignalSource;
class CMCON;
class TMRL;
class ADCON2_TRIG;
class CLC;

  enum compare_inputs
   {
	AN0 = 0,
	AN1,
	AN2,
	AN3,
	AN4,
	AN5,
	VREF = 6,	// use reference voltage
	NO_IN = 7,	// no input port
  	V06 = 8		// Reference voltage 0.6 
  };
  enum compare_outputs
   {
	OUT0 = 0,
	OUT1,
	ZERO = 6,	// register value == 0
	NO_OUT = 7	// no ouput port
   };

class VRCON : public sfr_register
{
 public:

 CMCON *_cmcon;

  enum VRCON_bits
    {
      VR0 = 1<<0,	// VR0-3 Value selection
      VR1 = 1<<1,
      VR2 = 1<<2,
      VR3 = 1<<3,
      VRSS = 1<<4,	// Use external references (16f88x)
      VRR = 1<<5,	// Range select
      VROE = 1<<6,	// Output Reference to external pin 
      VREN = 1<<7	// Enable Vref 
    };

  VRCON(Processor *pCpu, const char *pName, const char *pDesc);
  ~VRCON();

  virtual void put(uint new_value);
  virtual void setIOpin(PinModule *);
  virtual double get_Vref();
  void setValidBits(uint mask) { valid_bits = mask;}
                                                                                
protected:
  uint		valid_bits;
  PinModule 		*vr_PinModule;
  double 		vr_Vref;
  stimulus		*vr_pu;
  stimulus		*vr_pd;
  stimulus		*vr_06v;
  double		vr_Rhigh;
  double		vr_Rlow;
  double		Vref_high;	// usually VDD
  double		Vref_low;	// usually VSS
  char			*pin_name;	// original name of pin

};

// VSCOM class with two comparators as per 16f690
//
class VRCON_2 : public sfr_register
{
 public:

 CMCON *_cmcon;

  enum VRCON_bits
    {
      VR0 = 1<<0,	// VR0-3 Value selection
      VR1 = 1<<1,
      VR2 = 1<<2,
      VR3 = 1<<3,
      VP6EN = 1<<4,	// 0.6V reference enable
      VRR = 1<<5,	// Range select
      C2VREN = 1<<6,	// Comparator 2 Reference enable
      C1VREN = 1<<7	// Comparator 1 Reference enable
    };

  VRCON_2(Processor *pCpu, const char *pName, const char *pDesc);
  ~VRCON_2();

  virtual void put(uint new_value);

protected:
  uint		valid_bits;
  PinModule 		*vr_PinModule;
  double 		vr_Vref;
  stimulus		*vr_pu;
  stimulus		*vr_pd;
  stimulus		*vr_06v;
  char			*pin_name;	// original name of pin

};

class CM_stimulus : public stimulus
{
   public:

	CM_stimulus(CMCON *arg, const char *n=0,
           double _Vth=0.0, double _Zth=1e12
           );
	~CM_stimulus();

    CMCON *_cmcon;

     virtual void   set_nodeVoltage(double v);

};
class CM2CON1_V2;

class CMv2_stimulus : public stimulus
{
   public:

	CMv2_stimulus(CM2CON1_V2 *arg, const char *n=0,
           double _Vth=0.0, double _Zth=1e12
           );
	~CMv2_stimulus();

    CM2CON1_V2 *_cm2con1;

     virtual void   set_nodeVoltage(double v);

};
class CMCON1 : public sfr_register
{
 public:
  enum CMCON1_bits
  {
	CMSYNC = 1<<0,
	T1GSS  = 1<<1
  };

  virtual void put(uint);
  void set_tmrl(TMRL *arg) { m_tmrl = arg; }
  CMCON1(Processor *pCpu, const char *pName, const char *pDesc);
  ~CMCON1();

 private:
  TMRL *m_tmrl;
  uint		valid_bits;

};

class CMCON : public sfr_register
{
 public:


  VRCON *_vrcon;
  enum CMCON_bits
    {
      CM0 = 1<<0,
      CM1 = 1<<1,
      CM2 = 1<<2,
      CIS = 1<<3,
      C1INV = 1<<4,
      C2INV = 1<<5,
      C1OUT = 1<<6,
      C2OUT = 1<<7,
    };


  virtual void setINpin(int i, PinModule *, const char *an);
  virtual void setOUTpin(int i, PinModule *);
  virtual void assign_pir_set(PIR_SET *new_pir_set);
  virtual uint get();
  virtual void rename_pins(uint) { puts("CMCON::rename_pins() should not be called");}
  virtual void put(uint);
  virtual void set_configuration(int comp, int mode, int il1, int ih1, int il2, int ih2, int out);
  virtual double comp_voltage(int ind, int invert);

  void releasePin(int);



  void set_tmrl(TMRL *arg) { m_tmrl = arg; }
  void set_eccpas(ECCPAS *_eccpas) { m_eccpas = _eccpas; }
  CMCON(Processor *pCpu, const char *pName, const char *pDesc);
  ~CMCON();

protected:
  PinModule *cm_input[4];
  PinModule *cm_output[2];
  char *cm_input_pin[4];
  char *cm_an[4];
  char *cm_output_pin[2];
  CMSignalSource *cm_source[2];
  bool	cm_source_active[2];
  uint m_CMval[2];
  PIR_SET *pir_set;
  TMRL *m_tmrl;
  CM_stimulus *cm_stimulus[6];
  ECCPAS 	*m_eccpas;

  static const int cMaxConfigurations=8;
  static const int cMaxComparators=2;

  uint32_t m_configuration_bits[cMaxComparators][cMaxConfigurations];

};

class ComparatorModule
{
 public:

  ComparatorModule(Processor *);
  void initialize( PIR_SET *pir_set, PinModule *pin_vr0, PinModule *pin_cm0, 
	PinModule *pin_cm1, PinModule *pin_cm2,
	PinModule *pin_cm3, PinModule *pin_cm4, PinModule *pin_cm5);
  //protected:
  CMCON cmcon;
  CMCON1 cmcon1;
  VRCON vrcon;

};

/*
 * Compare module for 16f88x processors
 */

class CM1CON0;
class CM1CON0_2;
class CM2CON0;

/*
 * SRCON SR Latch Control Register
 */
class SRCON  : public sfr_register
{
 public:


  enum SRCON_bits
  {
	FVREN = 1<<0,  // Fixed Voltage Reference Enable
	PULSR = 1<<2,  // Pulse Reset of SR latch
	PULSS = 1<<3,  // Pulse set of SR Latch
	C2REN = 1<<4,  // C2OUT resets SR latch
	C1SEN = 1<<5,  // C1OUT sets SR latch
	SR0   = 1<<6,  // MUX SR Q out and C1OUT
	SR1   = 1<<7   // MUX SR -Q out and C2OUT
  };
  int writable_bits;
  bool SR_Q;
  bool set;
  bool reset;

  virtual void put(uint new_value);

  SRCON(Processor *pCpu, const char *pName, const char *pDesc);
  ~SRCON(){}
};

/*
 * CM2CON1 Comparator control register 1
 */
class CM2CON1 : public sfr_register
{
 public:

  
  enum CM2CON1_bits
  {
	C2SYNC = 1<<0, //C2 Output sync bit
	T1GSS  = 1<<1, // Timer1 Gate Source Select bit
	C2RSEL = 1<<4, // C2 Reference Select bit
	C1RSEL = 1<<5, // C1 Reference Select bit
	MC2OUT = 1<<6, // Mirror C2OUT bit
	MC1OUT = 1<<7  // Mirror C1OUT bit
  };
  int writable_bits;
  CM1CON0 *m_cm1con0;
  CM2CON0 *m_cm2con0;

  virtual void put(uint new_value);
  void link_cm12con0(CM1CON0 *_cm1con0, CM2CON0 *_cm2con0);

  CM2CON1(Processor *pCpu, const char *pName, const char *pDesc);
  ~CM2CON1(){}
};

class CM12SignalSource;

// The following classes are for comparators which have 3 registers
//
class CMxCON1;
class ComparatorModule2;

class CMxCON0_base : public sfr_register
{
public:

    enum {
	ON = 1<<7,
	OE = 1<<5
    };
    virtual uint get();
    virtual double get_Vpos(){return 0.;}
    virtual double get_Vneg(){return 0.;}
    virtual void put(uint)  { puts("Help");}
//    virtual int get(){return 0;}
    virtual void setBitMask(uint bm) { mValidBits = bm; }
    virtual void setIntSrc(InterruptSource *_IntSrc) { IntSrc = _IntSrc;}
    virtual double CVref(){return 0.;}
    virtual void notify(){;}
    virtual bool output_active() { return value.get() & (ON | OE); }
    virtual double get_hysteresis(){ return 0.;}
    virtual bool output_high() { return false;}
    virtual void set_output(bool output) { ;}
    virtual bool is_on(){return false;}
    virtual bool out_invert(){ return true;}
    virtual void releasePin(){ cm_source_active = false;}

  CMxCON0_base(Processor *pCpu, const char *pName, const char *pDesc,
	uint _cm, ComparatorModule2 *cmModule);
  ~CMxCON0_base();


  uint mValidBits;
  PinModule     *cm_input[5];
  PinModule     *cm_output;
  CM2CON1       *m_cm2con1;
  SRCON         *m_srcon;
//  PIR_SET       *pir_set;
  InterruptSource *IntSrc;
//  TMRL          *m_tmrl;
  CM_stimulus   *cm_stimulus[2];
  Stimulus_Node *cm_snode[2];
  ECCPAS        *m_eccpas;
  uint      cm;	// comparator number
  CMxCON1	    *m_cmxcon1;
  ComparatorModule2 *m_cmModule;
  CMxSignalSource   *cm_source;
  bool              cm_source_active;

};

class CMxCON0 : public CMxCON0_base
{
 public:

  enum 
    {
      CxSYNC 	= 1<<0,	// Output Synchronous Mode bit
      CxHYS 	= 1<<1,	// Hysteresis Enable bit
      CxSP   	= 1<<2,	// Speed/Power Select bit
      CxZLF	= 1<<3, // Zero Latency Filter Enable bit
      CxPOL 	= 1<<4,	// Output polarity select bit
      CxOE  	= 1<<5,	// Output enable
      CxOUT 	= 1<<6,	// Output bit 
      CxON  	= 1<<7,	// Enable bit
    };

  CMxCON0(Processor *pCpu, const char *pName, const char *pDesc, uint x, ComparatorModule2 *);
  ~CMxCON0();
  void put(uint);
  virtual double get_Vpos();
  virtual double get_Vneg();
  void setBitMask(uint bm) { mValidBits = bm; }
  virtual bool is_on() { return (value.get() & CxON);}
  virtual bool out_invert() { return value.get() & CxPOL;}
  virtual double get_hysteresis();
  virtual void set_output(bool output);
  virtual bool output_high() { return value.get() & CxOUT; }
};

class CMxCON0_V2 : public CMxCON0_base
{
 public:

  enum 
    {
      CxCH0 = 1<<0,	// Channel select bit 0
      CxCH1 = 1<<1,	// Channel select bit 1
      CxR   = 1<<2,	// Reference select bit (non-inverting input)
      CxPOL = 1<<4,	// Output polarity select bit
      CxOE  = 1<<5,	// Output enable
      CxOUT = 1<<6,	// Output bit 
      CxON  = 1<<7,	// Enable bit
      NEG	= 0,
      POS	= 1,
    };

  CMxCON0_V2(Processor *pCpu, const char *pName, const char *pDesc, 
	uint _cm, ComparatorModule2 *cmModule);
  ~CMxCON0_V2();
  virtual void put(uint);
  virtual double get_Vpos();
  virtual double get_Vneg();
  void setBitMask(uint bm) { mValidBits = bm; }
  virtual bool is_on() { return (value.get() & CxON);}
  virtual bool out_invert() { return value.get() & CxPOL;}
  virtual double get_hysteresis();
  virtual void set_output(bool output);
  virtual bool output_high() { return value.get() & CxOUT; }

  PinModule		*stimulus_pin[2];
};

class CMxCON1_base : public sfr_register
{
 public:

    enum {
      NEG	= 0,
      POS	= 1

    };
  CMxCON1_base(Processor *pCpu, const char *pName, const char *pDesc, uint _cm, ComparatorModule2 *);
  ~CMxCON1_base();

  void setBitMask(uint bm) { mValidBits = bm; }
  PinModule *output_pin(int cm=0) { return cm_output[cm]; }
  virtual void put(uint new_value){}
  virtual double get_Vpos(uint arg=0, uint arg2=0){ return 0.;}
  virtual double get_Vneg(uint arg=0, uint arg2=0){ return 0.;}
  virtual void setPinStimulus(PinModule *, int);
  virtual void set_INpinNeg(PinModule *pin_cm0, PinModule *pin_cm1, 
		PinModule *pin_cm2=0,  PinModule *pin_cm3=0,  
		PinModule *pin_cm4=0);
  virtual void set_OUTpin(PinModule *pin_cm0, PinModule *pin_cm1=0);
  virtual void set_INpinPos(PinModule *pin_cm0, PinModule *pin_cm1=0);
  virtual bool hyst_active(uint cm) { return false;}
  virtual void set_vrcon(VRCON *vrcon) {;}
  virtual void set_vrcon(VRCON_2 *vrcon) {;}
  virtual void tmr_gate(uint cm, bool output) {;}

protected:

  uint cm;	// comparator number
  CM_stimulus 	    *cm_stimulus[4];     // stimuli to monitor input pin
  PinModule	    *stimulus_pin[4];    // monitor stimulus loaded on this pin
  PinModule	    *ctmu_stimulus_pin;  // ctmu stimulus pin
  ComparatorModule2 *m_cmModule;
  PinModule 	    *cm_inputNeg[5];
  PinModule 	    *cm_inputPos[2];
  PinModule 	    *cm_output[2];
};
// CMxCON1 only uses 1 0r 2 of Negative select bits and 2 Positive select bits
class CMxCON1 : public CMxCON1_base
{
 public:

  enum 
    {
      CxNCH0 	= 1<<0,	//  Negative Input Channel Select bits
      CxNCH1 	= 1<<1,	//  Negative Input Channel Select bits
      CxNCH2 	= 1<<2,	//  Negative Input Channel Select bits
      CxPCH0	= 1<<3, //  Positive Input Channel Select bits
      CxPCH1	= 1<<4, //  Positive Input Channel Select bits
      CxPCH2	= 1<<5, //  Positive Input Channel Select bits
      CxINTN 	= 1<<6,	//  Interrupt on Negative Going Edge Enable bits
      CxINTP  	= 1<<7,	//  Interrupt on Positive Going Edge Enable bits
      CxNMASK = (CxNCH0 | CxNCH1 | CxNCH2),
      CxPMASK = (CxPCH0 | CxPCH1 | CxPCH2)
    };
  CMxCON1(Processor *pCpu, const char *pName, const char *pDesc, uint _x, ComparatorModule2 *);
  ~CMxCON1();

  virtual void put(uint new_value);
  virtual double get_Vpos(uint arg=0, uint arg2=0);
  virtual double get_Vneg(uint arg=0, uint arg2 = 0);

};

class CTMU;
/*  two comparators with common CM2CON1 and no COUT register, hyteresis, 
    C1, C2 possible T1,3,5  gate, FVR or  DAC for voltage reference,
    used by 18f26k22.
*/
class CM2CON1_V2 : public CMxCON1_base
{
 public:

  enum 
    {
	C2SYNC = 1<<0,
	C1SYNC = 1<<1,
	C2HYS  = 1<<2,
	C1HYS  = 1<<3,
	C2RSEL = 1<<4,
	C1RSEL = 1<<5,
	MC2OUT = 1<<6,
	MC1OUT = 1<<7
    };


  CM2CON1_V2(Processor *pCpu, const char *pName, const char *pDesc, 
		ComparatorModule2 * cmModule);
  ~CM2CON1_V2();
  
  virtual void put(uint new_value);
  virtual double get_Vpos(uint cm, uint cmxcon0);
  virtual double get_Vneg(uint cm, uint cmxcon0);
  virtual bool hyst_active(uint cm);
  virtual void tmr_gate(uint cm, bool output);
  void set_ctmu_stim(stimulus *_ctmu_stim, CTMU *_ctmu_module);
  void attach_ctmu_stim();
  void detach_ctmu_stim();

private:
  stimulus          *ctmu_stim;
  stimulus          *comp_input_cap;
  bool	            ctmu_attached;
};
/*  two comparators, no hyteresis, cm2con1 controls t1 gate,
    C2 possible T1 gate, vrcon for voltage reference
    used by 16f882.
*/
    
class CM2CON1_V3 : public CMxCON1_base
{
 public:

  enum 
    {
	C2SYNC = 1<<0,
	T1GSS  = 1<<1,
	C2RSEL = 1<<4,
	C1RSEL = 1<<5,
	MC2OUT = 1<<6,
	MC1OUT = 1<<7
    };


  CM2CON1_V3(Processor *pCpu, const char *pName, const char *pDesc, 
		uint _cm, ComparatorModule2 * cmModule) : 
		CMxCON1_base(pCpu, pName, pDesc, _cm, cmModule), m_vrcon(0){}
  ~CM2CON1_V3(){}
  virtual void put(uint new_value);
  virtual double get_Vpos(uint cm, uint cmxcon0);
  virtual double get_Vneg(uint cm, uint cmxcon0);
  virtual bool hyst_active(uint cm) { return false;}
  void set_vrcon(VRCON * _vrcon) { m_vrcon = _vrcon; }
  virtual void tmr_gate(uint cm, bool output);

protected:
  VRCON 	*m_vrcon;

};
/*  two comparators, no hyteresis, cm2con1 controls t1 gate,
    C2 possible T1 gate, VRCON for voltage reference
    Like CM2CON1_V3 without C1RSEL, C2RSEL
    used by 16f690.
*/
    
class CM2CON1_V4 : public CM2CON1_V3
{
 public:


  CM2CON1_V4(Processor *pCpu, const char *pName, const char *pDesc, 
		uint _cm, ComparatorModule2 * cmModule) ; 
  ~CM2CON1_V4();
  virtual void put(uint new_value);
  virtual double get_Vpos(uint cm, uint cmxcon0);
  void set_vrcon(VRCON_2 * _vrcon) { m_vrcon = _vrcon; }

protected:
  VRCON_2 	*m_vrcon;
  CM_stimulus 	*cm1_cvref;
  CM_stimulus 	*cm1_v06ref;
  CM_stimulus 	*cm2_cvref;
  CM_stimulus 	*cm2_v06ref;
};
class CMOUT : public sfr_register
{
 public:

    void put(uint val) { return;} // Read only by user

    CMOUT(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc) {}

};

// uses CMxCON0, CMxCON1, CMOUT
class ComparatorModule2
{
 public:

  ComparatorModule2(Processor *);
  ~ComparatorModule2();

  void run_get(uint comp) { cmxcon0[comp]->get();}


  void set_DAC_volt(double);
  void set_FVR_volt(double);
  void set_cmout(uint bit, bool value);
  void set_if(uint);
  void assign_pir_set(PIR_SET *new_pir_set){ pir_set = new_pir_set;}
  void assign_tmr1l(TMRL *t1, TMRL *t3 = 0, TMRL *t5 = 0) 
  { 
	tmr1l[0] = t1;
	tmr1l[1] = t3;
	tmr1l[2] = t5;
  }
  void assign_t1gcon(T1GCON *t1g, T1GCON *t3g = 0, T1GCON *t5g = 0) 
  { 
	t1gcon[0] = t1g;
	t1gcon[1] = t3g;
	t1gcon[2] = t5g;
  }
  void assign_sr_module(SR_MODULE *_sr_module) { sr_module = _sr_module;}

  void assign_eccpsas(ECCPAS *a1, ECCPAS *a2=0, ECCPAS *a3=0)
  {
	eccpas[0] = a1;
	eccpas[1] = a2;
	eccpas[2] = a3;
  }


  CMxCON0_base 	*cmxcon0[4];
  CMxCON1_base 	*cmxcon1[4];
  CMOUT		*cmout;

//protected:
  double 	DAC_voltage;
  double 	FVR_voltage;
  PIR_SET 	*pir_set;
  TMRL		*tmr1l[3];
  T1GCON	*t1gcon[3];
  SR_MODULE	*sr_module;
  CTMU		*ctmu_module;
  ECCPAS 	*eccpas[3];
  ADCON2_TRIG   *m_adcon2;
  CLC		*m_clc[4];
};
#endif // __COMPARATOR_H__

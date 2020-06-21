/*
   Copyright (C) 2015	Roy R Rankin

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

#ifndef __CTMU_H__
#define __CTMU_H__

//#include "trace.h"


class CTMU;


class CTMUCONH : public sfr_register
{
public:
    CTMUCONH(Processor *pCpu, const char *pName, const char *pDesc=0, CTMU *_ctmu=0);

    enum 
    {
	CTTRIG   = 1<<0,	// CTMU Special Event Trigger Control Bit
	IDISSEN  = 1<<1,	// Analog Current Source Control bit
	EDGSEQEN = 1<<2,	// Edge Sequence Enable bit
	EDGEN    = 1<<3,	// Edge Enable bit
	TGEN     = 1<<4,	// Time Generation Enable bit
	CTMUSIDL = 1<<5,	// Stop in Idle Mode bit
	CTMUEN   = 1<<7		// CTMU Enable bit
    };
	

    void put(uint new_value);

    CTMU *ctmu;

};

class CTMUCONL : public sfr_register
{
public:

    CTMUCONL(Processor *pCpu, const char *pName, const char *pDesc=0, CTMU *_ctmu=0);
    enum
    {
	EDG1STAT = 1<<0,	// Edge 1 Status bit
	EDG2STAT = 1<<1,	// Edge 2 Status bit
	EDG1SEL0 = 1<<2,	// Edge 1 Source Select bit 0
	EDG1SEL1 = 1<<3,	// Edge 1 Source Select bit 1
	EDG1POL  = 1<<4,	// Edge 1 Polarity Select bit
	EDG2SEL0 = 1<<5,	// Edge 2 Source Select bit 0
	EDG2SEL1 = 1<<6,	// Edge 2 Source Select bit 1
	EDG2POL  = 1<<7		// Edge 2 Polarity Select bit
    };

    void put(uint new_value);

    CTMU *ctmu;
};

class CTMUICON : public sfr_register
{
public:

    CTMUICON(Processor *pCpu, const char *pName, const char *pDesc=0, CTMU *_ctmu=0);

    void put(uint new_value);
    enum
    {
	IRNG0    = 1<<0,	// Current Source Range Select bit 0
	IRNG1    = 1<<1,	// Current Source Range Select bit 1
	ITRIM0   = 1<<2,	// Current Source Trim bit 0
	ITRIM1   = 1<<3,	// Current Source Trim bit 1
	ITRIM2   = 1<<4,	// Current Source Trim bit 2
	ITRIM3   = 1<<5,	// Current Source Trim bit 3
	ITRIM4   = 1<<6,	// Current Source Trim bit 4
	ITRIM5   = 1<<7  	// Current Source Trim bit 5
    };
    CTMU *ctmu;
};
class ctmu_stimulus : public stimulus
{
public:

  ctmu_stimulus(Processor *pCpu, const char *n=0, double _Vth=5.0, 
	double _Zth=1e3) : stimulus(n, _Vth, _Zth), cpu(pCpu)
  {
  }

  /* A current source is simulated by using a 200 V source so
     between 0-5 V the current should change < 2%. 
     The maximum voltage to a pin is clamped to be below Vdd-0.5 volts.
     Thus when the node voltage >= Vdd-0.6 we drop the drive voltage
     is reduced to Vdd-0.6. This can cause an overshoot to the node voltage.
  */
  virtual double get_Vth() 
  { 
        double max_volt = cpu->get_Vdd() - 0.6;
        if (get_nodeVoltage() >= max_volt)
	    return max_volt;
	return Vth; 
  }
private:
  Processor *cpu;
};

#define Vsrc 200.
class CTMU_SignalSink;

class CTMU
{
public:

    CTMU(Processor *pCpu);
    void new_current(double I);
    void enable(uint value);
    void disable();
    void current_off();
    void stat_change();
    void idissen(bool ground);
    void set_eepas(ECCPAS *_e1, ECCPAS *_e2) { m_eccpas1 = _e1; m_eccpas2=_e2;}
    void set_IOpins(PinModule *_pm1, PinModule *_pm2, PinModule *_pout) 
	{m_cted1 = _pm1; m_cted2 = _pm2; m_ctpls = _pout;}
    void new_edge();
    void tgen_on();
    void tgen_off();
    void syncC2out(bool high);
    


    double      current;
    double      resistance;
    bool        cted1_state;
    bool        cted2_state;
    ctmu_stimulus	*ctmu_stim;
    PinModule	*m_cted1;
    PinModule	*m_cted2;
    PinModule   *m_ctpls;
    ECCPAS	*m_eccpas1;
    ECCPAS	*m_eccpas2;
    CTMU_SignalSink *ctmu_cted1_sink;
    CTMU_SignalSink *ctmu_cted2_sink;
    PeripheralSignalSource *ctpls_source;

    CTMUCONH 	*ctmuconh;
    CTMUCONL 	*ctmuconl;
    CTMUICON 	*ctmuicon;
    ADCON0_V2   *adcon0;
    ADCON1_2B   *adcon1;
    CM2CON1_V2  *cm2con1;
    Processor   *cpu;
    
};
#endif // __CTMU_H__

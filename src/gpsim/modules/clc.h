/*
   Copyright (C) 2017 Roy R Rankin

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

// CONFIGURABLE LOGIC CELL (CLC)

#ifndef __CLC_h__
#define __CLC_h__

#include "registers.h"
#include "apfcon.h"

class CLC;
class OSC_SIM;
class NCO;
class INxSignalSink;
class CLCSigSource;

class CLCxCON : public sfr_register
{
  public:
    CLCxCON(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc), write_mask(0xdf)
    {
    }
    void put(uint);

  private:
    CLC *m_clc;
    uint write_mask;
};

class CLCxPOL : public sfr_register
{
  public:
    CLCxPOL(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc), write_mask(0x8f)

    {
    }

    void put(uint);

  private:
    CLC *m_clc;
    uint write_mask;
};

class CLCxSEL0 : public sfr_register
{
  public:
    CLCxSEL0(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc);

    void put(uint);

  private:
   CLC *m_clc;
    uint write_mask;
    
};

class CLCxSEL1 : public sfr_register
{
  public:
    CLCxSEL1(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc);
    void put(uint);

  private:
    CLC *m_clc;
    uint write_mask;
};

class CLCxGLS0 : public sfr_register
{
  public:
    CLCxGLS0(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc)
    {
    }

    void put(uint);

  private:
    CLC *m_clc;
};

class CLCxGLS1 : public sfr_register
{
  public:
    CLCxGLS1(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc)

    {
    }

    void put(uint);

  private:
    CLC *m_clc;
};

class CLCxGLS2 : public sfr_register
{
  public:
    CLCxGLS2(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc)

    {
    }

    void put(uint);

  private:
    CLC *m_clc;
};

class CLCxGLS3 : public sfr_register
{
  public:
    CLCxGLS3(CLC *_clc, Processor *pCpu, const char *pName, const char *pDesc) :
    sfr_register(pCpu, pName, pDesc), m_clc(_clc)

    {
    }

    void put(uint);

  private:
    CLC *m_clc;
};

class CLCDATA : public sfr_register
{
  public:
    CLCDATA(Processor *pCpu, const char *pName = 0, const char *pDesc = 0) :
    sfr_register(pCpu, pName, pDesc)
    {
    for(int i = 0; i < 4; i++)
        m_clc[i] = 0;
    }


    void put(uint val){;}
    void set_bit(bool bit_val, uint pos);
    void set_clc(CLC *clc1, CLC *clc2=0, CLC *clc3=0, CLC *clc4 = 0)
    {
    m_clc[0] = clc1;
    m_clc[1] = clc2;
    m_clc[2] = clc3;
    m_clc[3] = clc4;
    }

  private:
    CLC *m_clc[4];
};

class CLC : public apfpin
{
  public:
    enum {
    // CLCxCON
    LCxEN   = (1<<7),
    LCxOE   = (1<<6),
    LCxOUT  = (1<<5),
    LCxINTP = (1<<4),
    LCxINTN = (1<<3),
    LCxMODE = 0x7,

    // CLCxPOL
    LCxPOL  = (1<<7),
    };
    enum data_in {
    UNUSED = 0,
    LC1,
    LC2,
    LC3,
    LC4,
    CLCxIN0,    // 5
    CLCxIN1,
    PWM1,
    PWM2,
    PWM3,
    PWM4,        //10
    NCOx,
    FOSCLK,
    LFINTOSC,
    HFINTOSC,
    FRC_IN,        //15
    T0_OVER,
    T1_OVER,
    T2_MATCH,
    C1OUT,
    C2OUT,        //20
    };
    
    enum {
    CLCout_PIN=0,
    CLCin1_PIN,
    CLCin2_PIN
    };
    
    CLC(Processor *_cpu, uint _index, CLCDATA *_clcdata);
    ~CLC();
    
    bool CLCenabled() { return clcxcon.value.get() & LCxEN; }
    void setCLCxPin( PinModule *alt_pin );
    void enableINxpin( int, bool );
    virtual void setIOpin( int data, PinModule *pin );
    virtual void D1S( int select );
    virtual void D2S( int select );
    virtual void D3S( int select );
    virtual void D4S( int select );
    void t0_overflow();
    void t1_overflow();
    void t2_match();
    void osc_out( bool level, int kind );
    void out_pwm( bool level, int id );
    void NCO_out( bool level );
    void CxOUT_sync(bool output, int cm);
    void set_clcPins(PinModule *IN0, PinModule *IN1, PinModule *_CLCx)
    { pinCLCxIN[0] = IN0; pinCLCxIN[1] = IN1, pinCLCx = _CLCx;}
    void setState(char new3State, int index);
    void releasePinSource(PinModule *pin);
    void oeCLCx(bool on);
    void update_clccon(uint diff);
    void config_inputs(bool on);
    void compute_gates();
    void cell_function();
    bool cell_1_in_flipflop();
    bool cell_2_in_flipflop();
    bool cell_sr_latch();
    bool JKflipflop();
    bool transparent_D_latch();
    void lcxupdate(bool bit_val, uint pos);
    virtual void setInterruptSource(InterruptSource * _int) 
        { m_Interrupt = _int;}
     void outputCLC(bool out);


    uint  index;
    CLCxCON  clcxcon;
    CLCxPOL  clcxpol;
    CLCxSEL0 clcxsel0;
    CLCxSEL1 clcxsel1;
    CLCxGLS0 clcxgls0;
    CLCxGLS1 clcxgls1;
    CLCxGLS2 clcxgls2;
    CLCxGLS3 clcxgls3;
    CLCDATA  *clcdata;
    OSC_SIM  *frc;
    OSC_SIM  *lfintosc;
    OSC_SIM  *hfintosc;
    NCO      *p_nco;
    data_in      DxS_data[4];

  private:
    PinModule     *pinCLCx;
    CLCSigSource  *CLCxsrc;
    string        CLCxgui;
    bool          srcCLCxactive;
    INxSignalSink *INxsink[2];
    int          INxactive[2];
    bool      INxstate[2];
    PinModule     *pinCLCxIN[2];
    string        INxgui[2];
    bool      pwmx_level[4];
    bool      CMxOUT_level[4];
    bool      frc_level;
    bool      NCO_level;
    bool      lcxdT[4];        // incoming data
    bool      lcxg[4];        // Data gate output
    InterruptSource *m_Interrupt;
    bool      Doutput;
    bool      Dclock;
    bool      FRCactive;
    bool      LFINTOSCactive;
    bool      HFINTOSCactive;
};

class CLC1 : public CLC
{
  public:
    CLC1(Processor *_cpu, uint _index, CLCDATA *_clcdata);
    virtual void D1S(int select);
    virtual void D2S(int select);
    virtual void D3S(int select);
    virtual void D4S(int select);
};

// RC clock 600KHz used with ADC, CLC
class OSC_SIM : public TriggerObject
{
public:
    OSC_SIM(double _freq, int _data_in );
    
    void start_osc_sim(bool on);
    
    void set_clc(CLC *clc1, CLC *clc2=0, CLC *clc3=0, CLC *clc4 = 0)
    {
    m_clc[0] = clc1; m_clc[1] = clc2; m_clc[2] = clc3; m_clc[3] = clc4;
    }
    void callback();
private:
    double     frequency;
    int        data_in;
    int        active;
    CLC     *m_clc[4];
    bool     level;
    int      next_cycle;
    uint64_t     future_cycle;
    int64_t      adjust_cycles;
};
#endif // __CLC_h__

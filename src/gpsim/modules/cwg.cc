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

// Complimentry Waveform Generated (CWG) module

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

#include "pic-processor.h"
#include "cwg.h"

// Set pin direction
class TristateControl : public SignalControl
{
    public:
      TristateControl(CWG *_cwg, PinModule *_pin) : m_cwg(_cwg), m_pin(_pin){ }
      ~TristateControl() { }
      
      void set_pin_direction(char _direction) { direction = _direction;}
      virtual char getState() { return direction; }
      virtual void release()  { m_cwg->releasePin(m_pin); }
      
    private:
      CWG *m_cwg;
      PinModule *m_pin;
      char        direction;
};

class CWGSignalSource : public SignalControl
{
    public:
      CWGSignalSource(CWG *_cwg, PinModule *_pin)
        : m_cwg(_cwg),
        m_state('?'), m_pin(_pin)
      {
        assert(m_cwg);
      }
      virtual ~CWGSignalSource() { }

      void setState(char _state) { m_state = _state; }
      virtual char getState() { return m_state; }
      virtual void release() {
        Dprintf(("CWGSignalSource release\n"));
        m_cwg->releasePinSource(m_pin); }

    private:
      CWG *m_cwg;
      char m_state;
      PinModule *m_pin;
};

// Report state changes on incoming FLT pin
class FLTSignalSink : public SignalSink
{
    public:
      FLTSignalSink(CWG *_cwg)
        : m_cwg(_cwg) { }

      virtual void setSinkState(char new3State) {m_cwg->setState(new3State); }
      virtual void release() {delete this; }
      
    private:
      CWG *m_cwg;
};

CWG::CWG(Processor *pCpu) :
    cwg1con0(this, pCpu, "cwg1con0", "CWG Control Register 0"),
    cwg1con1(this, pCpu, "cwg1con1", "CWG Control Register 1"),
    cwg1con2(this, pCpu, "cwg1con2", "CWG Control Register 2"),
    cwg1dbf(this, pCpu, "cwg1dbf", "CWG Falling Dead-Band Count Register"),
    cwg1dbr(this, pCpu, "cwg1dbr", "CWG Rising Dead-Band Count Register"),
        con0_value(0), con1_value(0), con2_value(0), shutdown_active(false),
        cpu(pCpu),
        pinA(0), pinB(0), Atri(0), Btri(0), Asrc(0), Bsrc(0), FLTsink(0),
        pinAactive(false), pinBactive(false),
        srcAactive(false), srcBactive(false),
        active_next_edge(false),
        FLTstate(false)
    {
        nco_state = false;
        for(int i = 0; i<4; i++)
        {
            pwm_state[i] = false;
            clc_state[i] = false;
        }
    }

CWG::~CWG()
{
    if (Atri)
    {
        if (pinAactive) releasePin(pinA);

        delete Atri;
        delete Asrc;
        Atri = 0;
        Asrc = 0;
    }
    if (Btri)
    {
        if (pinBactive) releasePin(pinB);

        delete Btri;
        delete Bsrc;
        Btri = 0;
        Bsrc = 0;
    }
}


void CWG::setState(char state)
{
    Dprintf(("CWG::setState state=%c\n", state));
    if (state == '0' && FLTstate)        // new low edge
    {
        con2_value |= GxASE;
        cwg1con2.put_value(con2_value);
        autoShutEvent(true);
        active_next_edge = false;
    }
    else if (state == '1' && !FLTstate)
    {
        con2_value &= ~GxASE;
        cwg1con2.put_value(con2_value);
        active_next_edge = true;
    }
    FLTstate = (state != '0');
}

void CWG::set_IOpins(PinModule *p1, PinModule *p2, PinModule *_pinFLT)
{
    pinA = p1;
    pinB = p2;
    pinFLT = _pinFLT;
    if (Atri)
    {
        delete Atri;
        delete Asrc;
    }
    Atri = new TristateControl(this, pinA);
    Asrc = new CWGSignalSource (this, pinA);
    if (Btri)
    {
        delete Btri;
        delete Bsrc;
    }
    Btri = new TristateControl(this, pinB);
    Bsrc = new CWGSignalSource (this, pinB);
}
void CWG::oeA()
{
    Dprintf(("CWG::oeA() %u %u\n", (con0_value & GxEN), (con0_value & GxOEA)));
    if ((con0_value & GxEN) && (con0_value & GxOEA))
    {
        if (!pinAactive)
        {
            Atri->set_pin_direction('0');
            pinA->setControl(Atri);
            pinA->setSource(Asrc);
            pinA->updatePinModule();
            pinAactive = true;
            srcAactive = true;
        }
    }
    else if (pinAactive)
    {
        pinA->setControl(0);
        pinA->setSource(0);
        pinA->updatePinModule();
        pinAactive = false;
        srcAactive = false;
    }
}

void CWG::oeB()
{
    if ((con0_value & GxEN) && (con0_value & GxOEB))
    {
        if (!pinBactive)
        {
            Btri->set_pin_direction('0');
            pinB->setControl(Btri);
            pinB->setSource(Bsrc);
            pinB->updatePinModule();
            pinBactive = true;
            srcBactive = true;
        }
    }
    else if (pinBactive)
    {
        pinB->setControl(0);
        pinB->setSource(0);
        pinB->updatePinModule();
        pinBactive = false;
        srcBactive = false;
    }
}

void CWG::cwg_con0(uint value)
{
    uint diff = con0_value ^ value;
    con0_value = value;
    if (diff & (GxEN | GxOEA)) oeA();
    if (diff & (GxEN | GxOEB)) oeB();
}

void CWG::cwg_con1(uint value)
{
    con1_value = value;
}

void CWG::cwg_con2(uint value)
{
    uint diff = value ^ con2_value;
    con2_value = value;
    if (diff & GxASE)
    {
        if (value & GxASE)
        {
            if (value & GxARSEN)
                active_next_edge = true;
            autoShutEvent(true);
        }
        else
        {
            if (shutdown_active)
            {
                active_next_edge = true;
                autoShutEvent(false);
            }
        }
    }
    if (diff & GxASDFLT) enableAutoShutPin(value & GxASDFLT);
}

void CWG::autoShutEvent(bool on)
{
    if (on)
    {
        Dprintf(("CWG::autoShutEvent on A 0x%x\n", con1_value & (GxASDLA0|GxASDLA1)));
        switch(con1_value & (GxASDLA0|GxASDLA1))
        {
            case 0:                // to inactive state
                cwg1dbr.new_edge(false, 0.);
                break;

            case GxASDLA0:           // pin tristated
                cwg1dbr.kill_callback();
                Atri->set_pin_direction('1');
                pinA->updatePinModule();
                break;

            case GxASDLA1:                 // pin to 0
                cwg1dbr.kill_callback();
                    Asrc->setState('0');
                    pinA->updatePinModule();
                break;

            case GxASDLA0|GxASDLA1: // pin to 1
                cwg1dbr.kill_callback();
                    Asrc->setState('1');
                    pinA->updatePinModule();
                break;
        }
        Dprintf(("CWG::autoShutEvent on B 0x%x\n", con1_value & (GxASDLB0|GxASDLB1)));
        switch(con1_value & (GxASDLB0|GxASDLB1))
        {
            case 0:                // to inactive state
                cwg1dbf.new_edge(true, 0.);
                break;

            case GxASDLB0:           // pin tristated
                cwg1dbf.kill_callback();
                Btri->set_pin_direction('1');
                pinB->updatePinModule();
                break;

            case GxASDLB1:                 // pin to 0
                cwg1dbf.kill_callback();
                    Bsrc->setState('0');
                    pinB->updatePinModule();
                break;

            case GxASDLB0|GxASDLB1: // pin to 1
                cwg1dbf.kill_callback();
                    Bsrc->setState('1');
                    pinB->updatePinModule();
                break;
        }
        shutdown_active = true;
    }
    else
    {
        shutdown_active = false;
        Atri->set_pin_direction('0');
        pinA->updatePinModule();
        Btri->set_pin_direction('0');
        pinB->updatePinModule();
    }
}

void CWG::enableAutoShutPin(bool on)
{
    if (on)
    {
        if (!FLTsink)
        {
            FLTsink = new  FLTSignalSink(this);
            pinFLT->addSink(FLTsink);
            FLTstate = pinFLT->getPin().getState();
            Dprintf(("CWG::enableAutoShutPin FLTstate=%x\n", FLTstate));
        }
    }
    else
    {
        if (FLTsink)
        {
            pinFLT->removeSink(FLTsink);
            FLTsink->release();
            FLTsink = 0;
        }
    }
}

void CWG::releasePin(PinModule *pin)
{
    if (pin)
    {
        Dprintf(("CWG::releasePin %s pinAactive %d pinBactive %d\n", pin->getPin().name().c_str(), pinAactive, pinBactive));
        pin->setControl(0);
        if (pin == pinA) pinAactive = false;
        if (pin == pinB) pinBactive = false;
    }
}
void CWG::releasePinSource(PinModule *pin)
{
    Dprintf(("CWG::releasePinSource %p\n", pin));
    if (pin)
    {
        if (pin == pinA) srcAactive = false;
        if (pin == pinB) srcBactive = false;
        //pin->setSource(0);
    }
}
void CWG::input_source(bool level)
{
    if (level && active_next_edge)
    {
        con2_value &= ~GxASE;
        cwg1con2.put_value(con2_value);
        autoShutEvent(false);
        active_next_edge = false;
    }
    if (!shutdown_active)
    {
        double mult = (con0_value & GxCS0) ? 16e6/cpu->get_frequency() : 1;
        cwg1dbr.new_edge(level, mult);
        cwg1dbf.new_edge(!level, mult);
    }
}
void CWG::out_pwm(bool level, char index)
{
    if (index >= 2) return;
    if ((level != pwm_state[index-1])
         && (con0_value & GxEN)
         && ((int)(con1_value & (GxIS0|GxIS1)) == index-1))
    {
        Dprintf(("CWG::out_pwm level=%d shutdown_active=%d con2=0x%x\n", level, shutdown_active, con2_value));
           input_source(level);
    }
    pwm_state[index-1] = level;
}

void CWG::out_CLC(bool level, char index)
{
    assert(index > 1);
    if ((level != clc_state[index-1])
         && (con0_value & GxEN)
         && ((int)(con1_value & (GxIS0|GxIS1)) == 3))
    {
        Dprintf(("CWG::out_clc level=%d shutdown_active=%d con2=0x%x\n", level, shutdown_active, con2_value));
           input_source(level);
    }
    clc_state[index-1] = level;
}

void CWG::out_NCO(bool level)
{
    if ((level != nco_state)
         && (con0_value & GxEN)
         && ((int)(con1_value & (GxIS0|GxIS1)) == 2))
    {
        Dprintf(("CWG::out_NCO level=%d shutdown_active=%d con2=0x%x\n", level, shutdown_active, con2_value));
           input_source(level);
    }
    nco_state = level;
}

void CWG::set_outA(bool level)
{
    bool invert = con0_value & GxPOLA;
    Dprintf(("CWG::set_outA now=%" PRINTF_GINT64_MODIFIER "d level=%d invert=%d out=%d\n", get_cycles().get(), level, invert, level^invert));
    Asrc->setState((level^invert)?'1':'0');
    pinA->updatePinModule();
}

void CWG::set_outB(bool level)
{
    bool invert = con0_value & GxPOLB;
    Dprintf(("CWG::set_outB now=%" PRINTF_GINT64_MODIFIER "d level=%d invert=%d out=%d\n", get_cycles().get(), level, invert, level^invert));
    Bsrc->setState(level^invert?'1':'0');
    pinB->updatePinModule();
}

CWG4::CWG4(Processor *pCpu) : CWG(pCpu)
{
    cwg1con1.set_con1_mask(0xf7);
}

void CWG4::out_pwm(bool level, char index)
{
    if (index >= 4) return;
    if ((level != pwm_state[index-1])
         && (con0_value & GxEN)
         && ((int)(con1_value & (GxIS0|GxIS1|GxIS2)) == index-1))
    {
        Dprintf(("CWG4::out_pwm level=%d shutdown_active=%d con2=0x%x\n", level, shutdown_active, con2_value));
           input_source(level);
    }
    pwm_state[index-1] = level;
}

void CWG4::out_NCO(bool level)
{
    if ((level != nco_state)
         && (con0_value & GxEN)
         && ((int)(con1_value & (GxIS0|GxIS1|GxIS2)) == 6))
    {
        Dprintf(("CWG4::out_NCO level=%d shutdown_active=%d con2=0x%x\n", level, shutdown_active, con2_value));
           input_source(level);
    }
    nco_state = level;
}

CWGxCON0::CWGxCON0(CWG* pt, Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), pt_cwg(pt), con0_mask(0xf9)
    {}

void CWGxCON0::put(uint new_value)
{
    new_value &= con0_mask;
    if (!(new_value ^ value.get())) return;

    value.put(new_value);
    pt_cwg->cwg_con0(new_value);
}

CWGxCON1::CWGxCON1(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), pt_cwg(pt), con1_mask(0xf3)
    {}

void CWGxCON1::put(uint new_value)
{
    new_value &= con1_mask;
    uint diff = new_value ^ value.get();
    if (!diff) return;

    value.put(new_value);
    pt_cwg->cwg_con1(new_value);
}

CWGxCON2::CWGxCON2(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), pt_cwg(pt), con2_mask(0xc3)
    {}

void CWGxCON2::put(uint new_value)
{
    new_value &= con2_mask;
    uint diff = new_value ^ value.get();
    if (!diff) return;

    value.put(new_value);
    pt_cwg->cwg_con2(new_value);
}

CWGxDBF::CWGxDBF(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), pt_cwg(pt), future_cycle(0),
        next_level(false)
    {}
void CWGxDBF::callback()
{
        Dprintf(("CWGxDBF::callback() %" PRINTF_GINT64_MODIFIER "d\n", get_cycles().get()));
        pt_cwg->set_outB(next_level);
        future_cycle = 0;
}
void CWGxDBF::callback_print()
{
  cout << "CWGxDBF " << name() << " CallBack ID " << CallBackID << '\n';
}
void CWGxDBF::kill_callback()
{
    if (future_cycle)
    {
        Dprintf(("CWGxDBF::kill_callback() clear future_cycle=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle));
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
}

void CWGxDBF::new_edge(bool level, double multi)
{
    /* gpsim delay increment is Fosc/4 which is 1/4
       resolution of deadband, so deadband is approximate
    */
    int delay = (value.get() * multi + 2)/4;
    next_level = level;
    Dprintf(("CWGxDBF::new_edge now=%" PRINTF_GINT64_MODIFIER "d f=%.0f level=%d delay=%d\n", get_cycles().get(), ((Processor *)cpu)->get_frequency(), level, delay));
    if (future_cycle)
    {
        Dprintf(("\t clear future_cycle=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle));
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    if (!delay || !level) pt_cwg->set_outB(next_level);
    else
    {
        future_cycle = get_cycles().get() + delay;
        get_cycles().set_break(future_cycle, this);
    }
}

CWGxDBR::CWGxDBR(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc), pt_cwg(pt), future_cycle(0),
        next_level(false)
    {}

void CWGxDBR::kill_callback()
{
    if (future_cycle)
    {
        Dprintf(("CWGxDBR::kill_callback() clear future_cycle=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle));
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
}
void CWGxDBR::new_edge(bool level, double multi)
{
    /* gpsim delay increment is Fosc/4 which is 1/4
       resolution of deadband, so deadband is approximate
    */
    int delay = (value.get() * multi + 2)/4;
    next_level = level;
    Dprintf(("CWGxDBR::new_edge now=%" PRINTF_GINT64_MODIFIER "d f=%.0f level=%d delay=%d\n", get_cycles().get(), ((Processor *)cpu)->get_frequency(), level, delay));
    if (future_cycle)
    {
        Dprintf(("clear future_cycle=%" PRINTF_GINT64_MODIFIER "d\n", future_cycle));
        get_cycles().clear_break(future_cycle);
        future_cycle = 0;
    }
    if (!delay || !level) pt_cwg->set_outA(next_level);
    else
    {
        future_cycle = get_cycles().get() + delay;
        get_cycles().set_break(future_cycle, this);
    }
}

void CWGxDBR::callback()
{
    Dprintf(("CWGxDBR::callback() %" PRINTF_GINT64_MODIFIER "d\n", get_cycles().get()));
    pt_cwg->set_outA(next_level);
    future_cycle = 0;
}

void CWGxDBR::callback_print()
{
  cout << "CWGxDBR " << name() << " CallBack ID " << CallBackID << '\n';
}


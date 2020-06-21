/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2006,2009,2010,2013,2015,2017 Roy R Rankin

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

#include <stdio.h>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <string>

#include "config.h"
#include "14bit-tmrs.h"
#include "stimuli.h"
#include "a2dconverter.h"
#include "clc.h"

//
// 14bit-tmrs.cc
//
// Timer 1&2  modules for the 14bit core pic processors.
//
//#define DEBUG

#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


//--------------------------------------------------
// CCPRL
//--------------------------------------------------

CCPRL::CCPRL(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    ccprh(0), ccpcon(0), tmrl(0)
{
}

bool CCPRL::test_compare_mode()
{
    return tmrl && ccpcon && ccpcon->test_compare_mode();
}

void CCPRL::put(uint new_value)
{
  value.put(new_value);

  if(test_compare_mode())
    start_compare_mode();   // Actually, re-start with new capture value.
}

void CCPRL::capture_tmr()
{
  tmrl->get_low_and_high();

  value.put(tmrl->value.get());

  ccprh->value.put(tmrl->tmrh->value.get());

  //int c = value.get() + 256*ccprh->value.get();
}

void CCPRL::start_compare_mode(CCPCON *ref)
{
  int capture_value = value.get() + 256*ccprh->value.get();

  if ( ref ) ccpcon = ref;
  if ( ccpcon ) tmrl->set_compare_event ( capture_value, ccpcon );
  else          cout << "CPRL: Attempting to set a compare callback with no CCPCON\n";
}

void CCPRL::stop_compare_mode()
{
  // If this CCP is in the compare mode, then change to non-compare and cancel
  // the tmr breakpoint.

  if(test_compare_mode())
  {
    tmrl->clear_compare_event ( ccpcon );
  }
  ccpcon = 0;
}

void CCPRL::start_pwm_mode()
{
  //cout << "CCPRL: starting pwm mode\n";

  ccprh->pwm_mode = 1;
}
void CCPRL::stop_pwm_mode()
{
  //cout << "CCPRL: stopping pwm mode\n";

  ccprh->pwm_mode = 0;
}

//--------------------------------------------------
// assign_tmr - assign a new timer to the capture compare module
//
// This was created for the 18f family where it's possible to dynamically
// choose which clock is captured during an event.
//
void CCPRL::assign_tmr(TMRL *ptmr)
{
  if(ptmr) {
    cout << "Reassigning CCPRL clock source\n";

    tmrl = ptmr;
  }

}

//--------------------------------------------------
// CCPRH
//--------------------------------------------------

CCPRH::CCPRH(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    ccprl(0),pwm_mode(0),pwm_value(0)
{
}

// put_value allows PWM code to put data
void CCPRH::put_value(uint new_value)
{
  value.put(new_value);
}

void CCPRH::put(uint new_value)
{

  //cout << "CCPRH put \n";

  if(pwm_mode == 0)   // In pwm_mode, CCPRH is a read-only register.
    {
      put_value(new_value);

      if(ccprl && ccprl->test_compare_mode())
        ccprl->start_compare_mode();   // Actually, re-start with new capture value.

    }
}

uint CCPRH::get()
{
  //cout << "CCPRH get\n";
  return value.get();
}

//--------------------------------------------------
//
//--------------------------------------------------
class CCPSignalSource : public SignalControl
{
public:
  CCPSignalSource(CCPCON *_ccp, int _index)
    : m_ccp(_ccp),
    state('?'), index(_index)
  {
    assert(m_ccp);
  }
  virtual ~CCPSignalSource() { }

  void setState(char m_state) { state = m_state; }
  virtual char getState() { return state; }
  virtual void release() { m_ccp->releasePins(index); }

private:
  CCPCON *m_ccp;
  char state;
  int index;
};

//--------------------------------------------------
//
//--------------------------------------------------

class CCPSignalSink : public SignalSink
{
    public:
      CCPSignalSink(CCPCON *_ccp, int _index)
        : m_ccp(_ccp), index(_index)
      {
        assert(_ccp);
      }

      virtual ~CCPSignalSink(){}
      virtual void release() { m_ccp->releaseSink(); }
      void setSinkState(char new3State)
      {
        m_ccp->new_edge( new3State=='1' || new3State=='W');
      }
    private:
      CCPCON *m_ccp;
      int index;
    };

    class Tristate : public SignalControl
    {
    public:
      Tristate() { }
      ~Tristate() { }
      char getState() { return '1'; }        // set port to high impedance by setting it to input
      virtual void release() { }
};
//--------------------------------------------------
// CCPCON
//--------------------------------------------------
CCPCON::CCPCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    pstrcon(0),
    pwm1con(0),
    eccpas(0),
    m_sink(0),
    m_tristate(0),
    m_bInputEnabled(false),
    m_bOutputEnabled(false),
    m_cOutputState('?'),
    edges(0), delay_source0(false), delay_source1(false),
    bridge_shutdown(false),
    ccprl(0), pir(0), tmr2(0), adcon0(0)
{
  for(int i=0; i<4; i++)
  {
    m_PinModule[i] = 0;
    m_source[i] = 0;
    source_active[i] = false;
  }
  mValidBits = 0x3f;
}

CCPCON::~CCPCON()
{


    for(int i = 0; i<4; i++)
    {
        if (m_source[i])
        {
            if ( source_active[i] && m_PinModule[i] )
            {
                m_PinModule[i]->setSource(0);
            }
            delete m_source[i];
        }
    }

    if (m_tristate) delete m_tristate;
    if (m_PinModule[0] && m_sink) m_PinModule[0]->removeSink(m_sink);
    if (m_sink) delete m_sink;
}

void CCPCON::setIOPin1(PinModule *p1)
{
  if (p1 && &(p1->getPin()))
  {
    if (m_PinModule[0])
    {
        if (m_PinModule[0] != p1)
        {
            m_PinModule[0]->removeSink(m_sink);
            m_PinModule[0] = p1;
            p1->addSink(m_sink);
        }
    }
    else
    {
        m_PinModule[0] = p1;
        m_sink = new CCPSignalSink(this, 0);
        m_tristate = new Tristate();
        m_source[0] = new CCPSignalSource(this, 0);
        p1->addSink(m_sink);
    }
  }
}

void CCPCON::setIOPin2(PinModule *p2)
{
  if (p2)
  {
    m_PinModule[1] = p2;
    if (!m_source[1])
        m_source[1] = new CCPSignalSource(this, 1);
  }
  else
  {
        if (m_source[1])
        {
                delete m_source[1];
                m_source[1] = 0;
        }
        m_PinModule[1] = 0;
  }
}

void CCPCON::setIOPin3(PinModule *p3)
{
  if (p3)
  {
    m_PinModule[2] = p3;
    if (!m_source[2]) m_source[2] = new CCPSignalSource(this, 2);
  }
  else
  {
    if (m_source[2])
    {
            delete m_source[2];
            m_source[2] = 0;
    }
    m_PinModule[2] = 0;
  }
}

void CCPCON::setIOPin4(PinModule *p4)
{
  if (p4)
  {
    m_PinModule[3] = p4;
    if (!m_source[3])  m_source[3] = new CCPSignalSource(this, 3);
  }
  else
  {
    if (m_source[3])
    {
        delete m_source[3];
        m_source[3] = 0;
    }
  }
}

void CCPCON::setIOpin(int data, PinModule *pin)
{
    switch(data)
    {
    case CCP_PIN:
        setIOPin1(pin);
        break;

    case PxB_PIN:
        setIOPin2(pin);
        break;
    
    case PxC_PIN:
        setIOPin3(pin);
        break;
    
    case PxD_PIN:
        setIOPin4(pin);
        break;
    }
}

// EPWM has four outputs PWM 1
void CCPCON::setIOpin(PinModule *p1, PinModule *p2, PinModule *p3, PinModule *p4)
{
    Dprintf(("%s::setIOpin %s %s %s %s\n", name().c_str(), 
        (p1 && &(p1->getPin())) ? p1->getPin().name().c_str():"unknown",
        (p2 && &(p2->getPin())) ? p2->getPin().name().c_str():"unknown",
        (p3 && &(p3->getPin())) ? p3->getPin().name().c_str():"unknown",
        (p4 && &(p4->getPin())) ? p4->getPin().name().c_str():"unknown"
        ));
    if (p1 && !&(p1->getPin()))
    {
        Dprintf(("FIXME %s::setIOpin called where p1 has unassigned pin\n", name().c_str()));
        return;
    }
    
    setIOPin1(p1);
    setIOPin2(p2);
    setIOPin3(p3);
    setIOPin4(p4);
}

void CCPCON::setCrosslinks ( CCPRL *_ccprl, PIR *_pir, uint _mask,
                             TMR2 *_tmr2, ECCPAS *_eccpas )
{
  ccprl = _ccprl;
  pir = _pir;
  tmr2 = _tmr2;
  eccpas = _eccpas;
  pir_mask = _mask;
}

void CCPCON::setADCON(ADCON0 *_adcon0)
{
  adcon0 = _adcon0;
}

char CCPCON::getState()
{
  return m_bOutputEnabled ?  m_cOutputState : '?';
}

void CCPCON::new_edge(uint level)
{
  Dprintf(("%s::new_edge() level=%u\n",name().c_str(), level));

  switch(value.get() & (CCPM3 | CCPM2 | CCPM1 | CCPM0))
    {
    case ALL_OFF0:
    case ALL_OFF1:
    case ALL_OFF2:
    case ALL_OFF3:
      Dprintf(("--CCPCON not enabled\n"));
      return;

    case CAP_FALLING_EDGE:
      if (level == 0 && ccprl) {
        ccprl->capture_tmr();
        pir->set(pir_mask);
        Dprintf(("--CCPCON caught falling edge\n"));
      }
      break;

    case CAP_RISING_EDGE:
      if (level && ccprl) {
        ccprl->capture_tmr();
        pir->set(pir_mask);
        Dprintf(("--CCPCON caught rising edge\n"));
      }
      break;

    case CAP_RISING_EDGE4:
      if (level && --edges <= 0) {
        ccprl->capture_tmr();
        pir->set(pir_mask);
        edges = 4;
        Dprintf(("--CCPCON caught 4th rising edge\n"));
      }
        //else cout << "Saw rising edge, but skipped\n";
      break;


    case CAP_RISING_EDGE16:
      if (level && --edges <= 0) {
        ccprl->capture_tmr();
        pir->set(pir_mask);
        edges = 16;
        Dprintf(("--CCPCON caught 4th rising edge\n"));
      }
      //else cout << "Saw rising edge, but skipped\n";
      break;

    case COM_SET_OUT:
    case COM_CLEAR_OUT:
    case COM_INTERRUPT:
    case COM_TRIGGER:
    case PWM0:
    case PWM1:
    case PWM2:
    case PWM3:
      //cout << "CCPCON is set up as an output\n";
      return;
    }
}

void CCPCON::compare_match()
{

  Dprintf(("%s::compare_match()\n", name().c_str()));

  switch(value.get() & (CCPM3 | CCPM2 | CCPM1 | CCPM0))
    {
    case ALL_OFF0:
    case ALL_OFF1:
    case ALL_OFF2:
    case ALL_OFF3:
      Dprintf(("-- CCPCON not enabled\n"));
      return;

    case CAP_FALLING_EDGE:
    case CAP_RISING_EDGE:
    case CAP_RISING_EDGE4:
    case CAP_RISING_EDGE16:
      Dprintf(("-- CCPCON is programmed for capture. bug?\n"));
      break;

    case COM_SET_OUT:
      m_cOutputState = '1';
      m_source[0]->setState('1');
      m_PinModule[0]->updatePinModule();
      if (pir)
        pir->set(pir_mask);
      Dprintf(("-- CCPCON setting compare output to 1\n"));
      break;

    case COM_CLEAR_OUT:
      m_cOutputState = '0';
      m_source[0]->setState('0');
      m_PinModule[0]->updatePinModule();
      if (pir)
        pir->set(pir_mask);
      Dprintf(("-- CCPCON setting compare output to 0\n"));
      break;

    case COM_INTERRUPT:
      if (pir)
        pir->set(pir_mask);
      Dprintf(("-- CCPCON setting interrupt\n"));
      break;

    case COM_TRIGGER:
      if (ccprl)
        ccprl->tmrl->clear_timer();
      if (pir)
        pir->set(pir_mask);
      if(adcon0)
        adcon0->start_conversion();

      Dprintf(("-- CCPCON triggering an A/D conversion ccprl %p\n", ccprl));
      break;

    case PWM0:
    case PWM1:
    case PWM2:
    case PWM3:
      //cout << "CCPCON is set up as an output\n";
      return;

    }
}

// handle dead-band delay in half-bridge mode
void CCPCON::callback()
{

    if(delay_source0)
    {
               m_source[0]->setState('1');
            m_PinModule[0]->updatePinModule();
        delay_source0 = false;
    }
    if(delay_source1)
    {
               m_source[1]->setState('1');
            m_PinModule[1]->updatePinModule();
        delay_source1 = false;
    }
}

void CCPCON::releaseSink()
{
    delete m_sink;
    m_sink = 0;
}
void CCPCON::releasePins(int i)
{
    source_active[i] = false;
}

void CCPCON::pwm_match(int level)
{
  uint new_value = value.get();
  Dprintf(("%s::pwm_match() level=%d pwm1con = %p now=0x%" PRINTF_GINT64_MODIFIER "x\n", name().c_str(), level, pwm1con, get_cycles().get()));


  // if the level is 1, then tmr2 == pr2 and the pwm cycle
  // is starting over. In which case, we need to update the duty
  // cycle by reading ccprl and the ccp X & Y and caching them
  // in ccprh's pwm slave register.
  if(level == 1) {
      // Auto shutdown comes off at start of PWM if ECCPASE clear
      if (bridge_shutdown && (!eccpas || !(eccpas->get_value() & ECCPAS::ECCPASE)))
      {
          Dprintf(("bridge_shutdown=%d eccpas=%p ECCPAS=%x\n", bridge_shutdown, eccpas,
                eccpas ? eccpas->get_value() & ECCPAS::ECCPASE: 0));
          for(int i = 0; i < 4; i++)
          {
            if(m_PinModule[i])
            {
                 m_PinModule[i]->setControl(0); //restore default pin direction
                 source_active[i] = false;
                 m_PinModule[i]->updatePinModule();
            }
          }
          bridge_shutdown = false;
      }

      tmr2->pwm_dc(pwm_latch_value(), address);
      ccprl->ccprh->put_value(ccprl->value.get());
  }
  if( !pwm1con) { // simple PWM

    if (bridge_shutdown == false) // some processors use shutdown and simple PWM
    {
        m_cOutputState = level ? '1' : '0';
        m_source[0]->setState(level ? '1' : '0');
        m_PinModule[0]->setSource(m_source[0]);
        source_active[0] = true;


        if(level && !pwm_latch_value())  // if duty cycle == 0 output stays low
            m_source[0]->setState('0');

        m_PinModule[0]->updatePinModule();

        //cout << "iopin should change\n";
    }
  }
  else        // EPWM
  {


      if (!bridge_shutdown)
              drive_bridge(level, new_value);
  }
}
//
//  Drive PWM bridge
//
void CCPCON::drive_bridge(int level, int new_value)
{
      uint pstrcon_value;
      // pstrcon allows port steering for "single output"
      // if it is not defined, just use the first port
      if (pstrcon)
          pstrcon_value = pstrcon->value.get();
      else
          pstrcon_value = 1;
      int pwm_width;
      int delay = pwm1con->value.get() & ~PWM1CON::PRSEN;

      bool active_high[4];
      switch (new_value & (CCPM3|CCPM2|CCPM1|CCPM0))
      {
        case PWM0:        //P1A, P1C, P1B, P1D active high
        active_high[0] = true;
        active_high[1] = true;
        active_high[2] = true;
        active_high[3] = true;
            break;

        case PWM1:        // P1A P1C active high P1B P1D active low
        active_high[0] = true;
        active_high[1] = false;
        active_high[2] = true;
        active_high[3] = false;
            break;

        case PWM2:         // P1A P1C active low P1B P1D active high
        active_high[0] = false;
        active_high[1] = true;
        active_high[2] = false;
        active_high[3] = true;
            break;

        case PWM3:        // //P1A, P1C, P1B, P1D active low
        active_high[0] = false;
        active_high[1] = false;
        active_high[2] = false;
        active_high[3] = false;
            break;

        default:
            cout << "not pwm mode. bug?\n";
            return;
            break;
      }
      switch((new_value & (P1M1|P1M0))>>6) // ECCP bridge mode
      {
            case 0:        // Single
                Dprintf(("Single bridge %s pstrcon=0x%x\n", name().c_str(), pstrcon_value));
                for (int i = 0; i <4; i++)
                {
                    if (pstrcon_value & (1<<i) && m_PinModule[i])
                    {
                        m_PinModule[i]->setSource(m_source[i]);
                        source_active[i] = true;
                        // follow level except where duty cycle = 0
                        if (level && pwm_latch_value())
                              m_source[i]->setState(active_high[i]?'1':'0');
                        else
                              m_source[i]->setState(active_high[i]?'0':'1');

                            m_PinModule[i]->updatePinModule();
                    }
                    else if (m_PinModule[i])
                    {
                        m_PinModule[i]->setSource(0);
                        source_active[i] = false;
                    }
                }
                break;

            case 2:        // Half-Bridge
                Dprintf(("half-bridge %s\n", name().c_str()));
                m_PinModule[0]->setSource(m_source[0]);
                source_active[0] = true;
                m_PinModule[1]->setSource(m_source[1]);
                source_active[1] = true;
                if (m_PinModule[2])
                {
                    m_PinModule[2]->setSource(0);
                    source_active[2] = false;
                }
                if (m_PinModule[3])
                {
                    m_PinModule[3]->setSource(0);
                    source_active[3] = false;
                }
                delay_source0 = false;
                delay_source1 = false;
                // FIXME need to add deadband
                // follow level except where duty cycle = 0
                pwm_width = level ?
                        pwm_latch_value() :
                        ((tmr2->pr2->value.get()+1)*4)-pwm_latch_value();
                if (!(level^active_high[0]) && pwm_latch_value())
                {
                    // No delay, change state
                    if (delay == 0)
                                   m_source[0]->setState('1');
                    else if (delay < pwm_width) // there is a delay
                    {
                        future_cycle = get_cycles().get() + delay;
                          get_cycles().set_break(future_cycle, this);
                        delay_source0 = true;
                    }
                }
                else
                {
                           m_source[0]->setState('0');
                }
                if (!(level^active_high[1]) && pwm_latch_value())
                {
                           m_source[1]->setState('0');
                }
                else
                {
                    // No delay, change state
                    if (delay == 0)
                                   m_source[1]->setState('1');
                    else if (delay < pwm_width) // there is a delay
                    {
                        future_cycle = get_cycles().get() + delay;
                          get_cycles().set_break(future_cycle, this);
                        delay_source1 = true;
                    }
                }
                    m_PinModule[0]->updatePinModule();
                    m_PinModule[1]->updatePinModule();
                break;

            case 1:        // Full bidge Forward
                Dprintf(("full-bridge %s, forward\n", name().c_str()));
                if (m_PinModule[0])
                {
                    m_PinModule[0]->setSource(m_source[0]);
                    source_active[0] = true;
                    // P1A High (if active high)
                      m_source[0]->setState(active_high[0]?'1':'0');
                        m_PinModule[0]->updatePinModule();
                }
                if (m_PinModule[1])
                {
                    m_PinModule[1]->setSource(m_source[1]);
                    source_active[1] = true;
                    // P1B, P1C low (if active high)
                      m_source[1]->setState(active_high[1]?'0':'1');
                        m_PinModule[1]->updatePinModule();
                }
                if (m_PinModule[2])
                {
                    m_PinModule[2]->setSource(m_source[2]);
                    source_active[2] = true;
                    // P1B, P1C low (if active high)
                      m_source[2]->setState(active_high[2]?'0':'1');
                        m_PinModule[2]->updatePinModule();
                }
                if (m_PinModule[3])
                {
                    m_PinModule[3]->setSource(m_source[3]);
                    source_active[3] = true;
                    // P1D toggles
                    if (level && pwm_latch_value())
                          m_source[3]->setState(active_high[3]?'1':'0');
                    else
                          m_source[3]->setState(active_high[3]?'0':'1');
                        m_PinModule[3]->updatePinModule();
                }
                break;

            case 3:        // Full bridge reverse
                Dprintf(("full-bridge reverse %s\n", name().c_str()));
                if (m_PinModule[0])
                {
                    m_PinModule[0]->setSource(m_source[0]);
                    source_active[0] = true;
                    // P1A, P1D low (if active high)
                      m_source[0]->setState(active_high[0]?'0':'1');
                        m_PinModule[0]->updatePinModule();
                }
                if (m_PinModule[1])
                {
                    m_PinModule[1]->setSource(m_source[1]);
                    source_active[1] = true;
                    // P1B toggles
                    if (level && pwm_latch_value())
                          m_source[1]->setState(active_high[1]?'1':'0');
                    else
                          m_source[1]->setState(active_high[1]?'0':'1');
                        m_PinModule[1]->updatePinModule();
                }
                if (m_PinModule[2])
                {
                    m_PinModule[2]->setSource(m_source[2]);
                    source_active[2] = true;
                    // P1C High (if active high)
                      m_source[2]->setState(active_high[2]?'1':'0');
                        m_PinModule[2]->updatePinModule();
                }
                if (m_PinModule[3])
                {
                    m_PinModule[3]->setSource(m_source[3]);
                    source_active[3] = true;
                    // P1A, P1D low (if active high)
                      m_source[3]->setState(active_high[3]?'0':'1');
                        m_PinModule[3]->updatePinModule();
                }
                break;

            default:
                printf("%s::pwm_match impossible ECCP bridge mode\n", name().c_str());
                break;
       }

}
//
// Set PWM bridge into shutdown mode
//
void CCPCON::shutdown_bridge(int eccpas)
{
    bridge_shutdown = true;

    Dprintf(("eccpas=0x%x\n", eccpas));

    switch(eccpas & (ECCPAS::PSSBD0 | ECCPAS::PSSBD1))
    {
    case 0:        // B D output 0
        if (m_source[1]) m_source[1]->setState('0');
        if (m_source[3]) m_source[3]->setState('0');
        break;

    case 1:        // B, D output 1
        if (m_source[1]) m_source[1]->setState('1');
        if (m_source[3]) m_source[3]->setState('1');
        break;

    default:        // Tristate B & D
        if(m_PinModule[1])  m_PinModule[1]->setControl(m_tristate);
        if(m_PinModule[3])  m_PinModule[3]->setControl(m_tristate);
        break;
    }
    switch(eccpas & ((ECCPAS::PSSAC0 | ECCPAS::PSSAC1) >> 2))
    {
    case 0:        // A, C output 0
        m_source[0]->setState('0');
        if (m_source[2]) m_source[2]->setState('0');
        break;

    case 1:        // A, C output 1
        m_source[0]->setState('1');
        if (m_source[2]) m_source[2]->setState('1');
        break;

    default:        // Tristate A & C
        m_PinModule[0]->setControl(m_tristate);
        if(m_PinModule[2])  m_PinModule[2]->setControl(m_tristate);
        break;
    }
    m_PinModule[0]->updatePinModule();
    if (m_PinModule[1]) m_PinModule[1]->updatePinModule();
    if (m_PinModule[2]) m_PinModule[2]->updatePinModule();
    if (m_PinModule[3]) m_PinModule[3]->updatePinModule();
}

void CCPCON::put(uint new_value)
{

  uint old_value =  value.get();
  new_value &= mValidBits;

  Dprintf(("%s::put() new_value=0x%x\n",name().c_str(), new_value));

  value.put(new_value);
  if (!ccprl || !tmr2)
    return;

  // Return if no change other than possibly the duty cycle
  if (((new_value ^ old_value) & ~(CCPY|CCPX)) == 0)
    return;

  bool oldbInEn  = m_bInputEnabled;
  bool oldbOutEn = m_bOutputEnabled;

  switch(value.get() & (CCPM3 | CCPM2 | CCPM1 | CCPM0))
    {
    case ALL_OFF0:
    case ALL_OFF1:
    case ALL_OFF2:
    case ALL_OFF3:
      if (ccprl)
      {
        ccprl->stop_compare_mode();
        ccprl->stop_pwm_mode();
      }
      if (tmr2)
        tmr2->stop_pwm(address);
      m_bInputEnabled = false;
      m_bOutputEnabled = false;

      // RP - According to 16F87x data sheet section 8.2.1 clearing CCPxCON also clears the latch
      m_cOutputState = '0';
      m_source[0]->setState('0');
      break;

    case CAP_FALLING_EDGE:
    case CAP_RISING_EDGE:
      edges = 0;
      ccprl->stop_compare_mode();
      ccprl->stop_pwm_mode();
      tmr2->stop_pwm(address);
      m_bInputEnabled = true;
      m_bOutputEnabled = false;
      break;

    case CAP_RISING_EDGE4:
      edges &= 3;
      ccprl->stop_compare_mode();
      ccprl->stop_pwm_mode();
      tmr2->stop_pwm(address);
      m_bInputEnabled = true;
      m_bOutputEnabled = false;
      break;

    case CAP_RISING_EDGE16:
      ccprl->stop_compare_mode();
      ccprl->stop_pwm_mode();
      tmr2->stop_pwm(address);
      m_bInputEnabled = true;
      m_bOutputEnabled = false;
      break;

    case COM_SET_OUT:
    case COM_CLEAR_OUT:
      m_bOutputEnabled = true;
    case COM_INTERRUPT:
    case COM_TRIGGER:
      ccprl->start_compare_mode(this);
      ccprl->stop_pwm_mode();
      tmr2->stop_pwm(address);

      // RP - just writing CCP2CON doesn't trigger the ADC; that only happens on a match
      //if(adcon0)
      //  adcon0->start_conversion();

      m_bInputEnabled = false;
      //if(adcon0) cout << "CCP triggering an A/D\n";

      break;

    case PWM0:
    case PWM1:
    case PWM2:
    case PWM3:
      ccprl->stop_compare_mode();
/* do this when TMR2 == PR2
      ccprl->start_pwm_mode();
      tmr2->pwm_dc( pwm_latch_value(), address);
*/
      m_bInputEnabled = false;
      m_bOutputEnabled = false;        // this is done in pwm_match
      m_cOutputState = '0';
      if ((old_value & P1M0) && (new_value & P1M0)) // old and new full-bridge
      {                // need to adjust timer if P1M1 also changed
        Dprintf(("full bridge repeat old=0x%x new=%x\n", old_value, new_value));
      }
      else
          pwm_match(0);
          //RRRpwm_match(2);
      return;
      break;

    }

    if (oldbOutEn != m_bOutputEnabled && m_PinModule)
    {
        if (m_bOutputEnabled)
        {
            m_PinModule[0]->setSource(m_source[0]);
            source_active[0] = true;
        }
        else
        {
            m_PinModule[0]->setSource(0);
            m_source[0]->setState('?');
            source_active[0] = false;
        }
    }

    if ((oldbInEn  != m_bInputEnabled  ||
       oldbOutEn != m_bOutputEnabled)
      && m_PinModule[0])
        m_PinModule[0]->updatePinModule();

}

bool CCPCON::test_compare_mode()
{
  switch(value.get() & (CCPM3 | CCPM2 | CCPM1 | CCPM0))
  {
    case ALL_OFF0:
    case ALL_OFF1:
    case ALL_OFF2:
    case ALL_OFF3:
    case CAP_FALLING_EDGE:
    case CAP_RISING_EDGE:
    case CAP_RISING_EDGE4:
    case CAP_RISING_EDGE16:
    case PWM0:
    case PWM1:
    case PWM2:
    case PWM3:
      return false;
      break;

    case COM_SET_OUT:
    case COM_CLEAR_OUT:
    case COM_INTERRUPT:
    case COM_TRIGGER:
      return true;
      break;
  }
  return false;
}

PWMxCON::PWMxCON(Processor *pCpu, const char *pName, const char *pDesc, char _index)
  : CCPCON(pCpu, pName, pDesc),
        pwmdcl(0), pwmdch(0), m_cwg(0), index(_index)
{
        mValidBits = 0xd0;
        for(int i=0; i<4; i++)
            m_clc[i] = 0;
}
void PWMxCON::put(uint new_value)
{
        new_value &= mValidBits;
        put_value(new_value);
}

void PWMxCON::put_value(uint new_value)
{
        uint diff = value.get() ^ new_value;
        Dprintf(("PWMxCON::put %s new 0x%x diff 0x%x\n", name().c_str(), new_value, diff));
        if (!diff) return;

        value.put(new_value);
        if (diff & PWMxEN)
        {
            if (new_value & PWMxEN) // Turn on PWM
            {
                    pwm_match(0);
            }
            else                        // Turn off PWM
            {
                    tmr2->stop_pwm(address);
            }
        }
}
/*
 * level == 0 duty cycle match
 * level == 1 tmr2 == PR2
 * level == 2
 */
void PWMxCON::pwm_match(int level)
{
        uint reg = value.get();

        if (!(reg & PWMxEN))
            return;

        Dprintf(("%s::pwm_match() level=%d now=%" PRINTF_GINT64_MODIFIER "d\n", name().c_str(), level, get_cycles().get()));

        if (level == 1)
        {
             tmr2->pwm_dc(pwm_latch_value(), address);
             if(!pwm_latch_value()) // if duty cycle == 0 output stays low
                         level = 0;

        }
        if (reg & PWMxPOL)        // inverse output
        {
            level = level ? 0 : 1;
            Dprintf(("invert output\n"));
        }
        if (level)
            reg |= PWMxOUT;
        else
            reg &= ~PWMxOUT;
        Dprintf(("reg 0x%x old 0x%x\n", reg, value.get()));
        if (reg != value.get())
            put_value(reg);
        if (m_cwg) m_cwg->out_pwm(level, index);
        for(int i = 0; i<4; i++)
            if (m_clc[i]) m_clc[i]->out_pwm(level, index);
        if (reg & PWMxOE)
        {
            m_cOutputState = level ? '1' : '0';
            m_source[0]->setState(level ? '1' : '0');
            m_PinModule[0]->setSource(m_source[0]);
            m_PinModule[0]->updatePinModule();
            source_active[0] = true;
            Dprintf(("PWMxOE level %c\n", m_cOutputState));
        }
}



TRISCCP::TRISCCP(Processor *pCpu, const char *pName, const char *pDesc) :
        sfr_register(pCpu, pName), first(true)
{
}
void TRISCCP::put(uint new_value)
{
  if (first)
  {
      first = false;
      cout << name() << " not implemented, if required, file feature request\n";
  }
  value.put(new_value);
}

DATACCP::DATACCP(Processor *pCpu, const char *pName, const char *pDesc) :
        sfr_register(pCpu, pName), first(true)
{
}

void DATACCP::put(uint new_value)
{
  if (first)
  {
      first = false;
      cout << name() << " not implemented, if required, file feature request\n";
  }
  value.put(new_value);
}

// Attribute for frequency of external Timer1 oscillator
class TMR1_Freq_Attribute : public Float
{
    public:
      TMR1_Freq_Attribute(Processor * _cpu, double freq, const char *name = "tmr1_freq");

      virtual void set(double d);
      double get_freq();

    private:
      Processor * cpu;
};

TMR1_Freq_Attribute::TMR1_Freq_Attribute(Processor * _cpu, double freq, const char *name)
  : Float(name, freq, " Tmr oscillator frequency."),
    cpu(_cpu)
{

}

double TMR1_Freq_Attribute::get_freq()
{
    double d;
    Float::get(d);
    return(d);
}
void TMR1_Freq_Attribute::set(double d)
{
    Float::set ( d );
}

//--------------------------------------------------
// T1CON
//--------------------------------------------------
T1CON::T1CON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    tmrl(0), cpu(pCpu)
{
    char freq_name[] = "tmr1_freq";
    if (*(pName+1) >= '1' && *(pName+1) <= '9') freq_name[3] = *(pName+1);
    freq_attribute = new TMR1_Freq_Attribute(pCpu, 32768., freq_name);
}
T1CON::~T1CON()
{
    delete freq_attribute;
}

void T1CON::put(uint new_value)
{
  uint diff = value.get() ^ new_value;
  value.put(new_value);
  
  if (!tmrl) return;
  
  // First, check the tmr1 clock source bit to see if we are  changing from
  // internal to external (or vice versa) clocks.
  if( diff & (TMR1CS | T1OSCEN)) tmrl->new_clock_source();

  if( diff & TMR1ON)
    tmrl->on_or_off(value.get() & TMR1ON);
  else  if( diff & (T1CKPS0 | T1CKPS1 | TMR1GE | T1GINV))
    tmrl->update();
}

uint T1CON::get()
{
  return(value.get());
}

uint T1CON::get_prescale()
{
  return( ((value.get() &(T1CKPS0 | T1CKPS1)) >> 4) );
}

double T1CON::t1osc()
{
    return (value.get() & T1OSCEN) ? freq_attribute->get_freq() : 0.;
}
//--------------------------------------------------
//
//--------------------------------------------------

//
//  Signal T1GCon on change of state of Gate pin
//
class T1GCon_GateSignalSink : public SignalSink
{
public:
  T1GCon_GateSignalSink(T1GCON *_t1gcon)
    : m_t1gcon(_t1gcon)
  {
    assert(_t1gcon);
  }
  virtual ~T1GCon_GateSignalSink()
  {
  }

  virtual void release() {delete this; }
  virtual void setSinkState(char new3State)
  {
    m_t1gcon->PIN_gate( new3State=='1' || new3State=='W');
  }
private:
  T1GCON *m_t1gcon;
};

T1GCON::T1GCON(Processor *pCpu, const char *pName, const char *pDesc, T1CON_G *_t1con_g)
  : sfr_register(pCpu, pName, pDesc), sink(0), write_mask(0xfb), tmrl(0),
    t1con_g(_t1con_g), m_Interrupt(0),
    PIN_gate_state(false), T0_gate_state(false), CM1_gate_state(false),
    CM2_gate_state(false), last_t1g_in(false), gate_pin(0)
{
}

T1GCON::~T1GCON()
{
    if (m_Interrupt)
       m_Interrupt->release();
}

bool T1GCON::tmr1_isON()
{
    if (t1con_g)
        return t1con_g->get_tmr1on();

    if (tmrl->t1con)
        return tmrl->t1con->get_tmr1on();

    cerr << "Error " << name() << " get_tmr1on() not found\n";
    return false;
}


void T1GCON::put(uint new_value)
{
  uint old_value = value.get();
  new_value = (new_value & write_mask) | (old_value & ~write_mask);
  uint diff = new_value ^ old_value;
  bool t1ggo = new_value & T1GGO;

  assert(m_Interrupt);
  assert(tmrl);

  if (!diff) return;

  value.put(new_value);

  if (diff & (T1GSS1 | T1GSS0 | T1GPOL | TMR1GE))
  {
      switch(new_value & (T1GSS1 | T1GSS0))
      {
      case 0:
        new_gate(PIN_gate_state);
        break;

      case 1:
        new_gate(T0_gate_state);
        break;

      case 2:
        new_gate(CM1_gate_state);
        break;

      case 3:
        new_gate(CM2_gate_state);
        break;
      }
      // Dont't allow gate change to clear new T1GG0
      if((diff & T1GGO) && t1ggo)
        value.put(value.get() | T1GGO);
  }
  // T1GGO  set and Single pulse mode
  if ((diff & T1GGO) && (value.get() & (T1GGO | T1GSPM)))
  {
        //tmrl->IO_gate(true);
        if (value.get() & T1GVAL)
        {
            value.put(value.get() & ~T1GVAL);
            //tmrl->IO_gate(true);
            tmrl->IO_gate(false);
        }
  }
  if (diff & T1GTM)
  {
        if ((value.get() & T1GTM)) // T1GTM going high, set t1g_in to 0
        {
           if(value.get() & T1GVAL)
           {
                value.put(value.get() & ~(T1GVAL));
                m_Interrupt->Trigger();
           }
           tmrl->IO_gate(false); // Counting should be stopped
        }
  }
  tmrl->update();
}

void T1GCON::setGatepin(PinModule *pin)
{


    if (pin != gate_pin)
    {
        if(sink)
        {
            gate_pin->removeSink(sink);
        }
          else
            sink = new T1GCon_GateSignalSink(this);

        gate_pin = pin;
        Dprintf(("T1GCON::setGatepin %s\n", pin->getPin().name().c_str()));
        pin->addSink(sink);
    }
}

// The following 4 functions are called on a state change.
// They pass the state to new_gate if that input is selected.
void T1GCON::PIN_gate(bool state)
{
    PIN_gate_state = state;
    if((value.get() & (T1GSS0|T1GSS1)) == 0)
        new_gate(state);
}
void T1GCON::T0_gate(bool state)
{
    T0_gate_state = state;
    if((value.get() & (T1GSS0|T1GSS1)) == 1)
        new_gate(state);
}
// T[246] = PR[246]
// overloads T0_gate_state
void T1GCON::T2_gate(bool state)
{
    T0_gate_state = state;
    if((value.get() & (T1GSS0|T1GSS1)) == 1)
        new_gate(state);
}
void T1GCON::CM1_gate(bool state)
{
    CM1_gate_state = state;
    if((value.get() & (T1GSS0|T1GSS1)) == 2)
    {
        new_gate(state);
    }
}
void T1GCON::CM2_gate(bool state)
{
    CM2_gate_state = state;

    if((value.get() & (T1GSS0|T1GSS1)) == 3)
    {
        new_gate(state);
    }
}
void T1GCON::new_gate(bool state)
{
    // TMR1 counts when state low (unless t1gpol is set)
    // t1g_in is inverted as per XOR in spec sheet flow chart
    bool t1g_in = (!get_t1GPOL()) ^ state ;
    bool t1g_val = value.get() & T1GVAL;
    uint reg_value = value.get();


    if ((t1g_in == last_t1g_in) && (t1g_in == t1g_val)) // no state change, do nothing
    {
  //          tmrl->IO_gate(t1g_val);
        return;
    }


    last_t1g_in = t1g_in;

    if ( reg_value & T1GTM) // Toggle mode
    {
        t1g_val = reg_value & T1GVAL;
        if (t1g_in)        // rising edge
        {
            t1g_val = ! t1g_val;                // t1gval changes state
        }
        else
        {
            return;
        }
    }
    else        // Gate directly in control
    {
        t1g_val = t1g_in;
    }

    if (reg_value & T1GSPM)        // Single pulse mode
    {
        if (!(reg_value & T1GGO))  // do nothing if T1GGO clear
            return;

        if (!t1g_val)                // End of gate
        {
            reg_value &= ~T1GGO;  //set done
        }
        else                        // Start of gate
        {
        }
        //t1g_val = t1g_in;
    }


    if (t1g_val)
    {
        reg_value |= T1GVAL;
    }
    else
    {
        if (reg_value & T1GVAL)        // interrupt on T1GVAL negative edge
        {
            m_Interrupt->Trigger();
        }
        reg_value &= ~T1GVAL;
    }

    value.put(reg_value);
    tmrl->IO_gate(t1g_val);
}

//--------------------------------------------------
// T1CON_G
//--------------------------------------------------
T1CON_G::T1CON_G(Processor *pCpu, const char *pName, const char *pDesc)
  //: sfr_register(pCpu, pName, pDesc),
  : T1CON(pCpu, pName, pDesc),
    tmrl(0), freq_attribute(nullptr),
    t1gcon(pCpu, "t1gcon", "TM1 Gate Control Register", this)
{
  new_name("T1CON");
}

T1CON_G::~T1CON_G()
{
}

void T1CON_G::put(uint new_value)
{
  uint diff = value.get() ^ new_value;
  value.put(new_value);
  
  if (!tmrl) return;
  
  // First, check the tmr1 clock source bit to see if we are  changing from
  // internal to external (or vice versa) clocks.
  if( diff & (TMR1CS0 | TMR1CS1 | T1OSCEN))
    tmrl->new_clock_source();

  if( diff & TMR1ON)
    tmrl->on_or_off(value.get() & TMR1ON);
  else  if( diff & (T1CKPS0 | T1CKPS1 ))
    tmrl->update();
}

// If Cap. sensing oscillator T1 clock source, pass to T1
void T1CON_G::t1_cap_increment()
{
        if (get_tmr1cs() == 3)        // T1 input Cap. sensing oscillator
            tmrl->increment();
}
//--------------------------------------------------
// member functions for the TMRH base class
//--------------------------------------------------
TMRH::TMRH(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    tmrl(0)
{

  value.put(0);

}

void TMRH::put(uint new_value)
{
  if(!tmrl)
  {
    value.put(new_value & 0xff);
    return;
  }

  tmrl->set_ext_scale();
  value.put(new_value & 0xff);
  tmrl->synchronized_cycle = get_cycles().get();
  tmrl->last_cycle = tmrl->synchronized_cycle
        - (int64_t)((tmrl->value.get() + (value.get()<<8)
        * tmrl->prescale * tmrl->ext_scale) +0.5);

  if(tmrl->t1con->get_tmr1on()) tmrl->update();
}

uint TMRH::get()
{
  return get_value();
}

// For the gui and CLI
uint TMRH::get_value()
{
  // If the TMR1 is being read immediately after being written, then
  // it hasn't had enough time to synchronize with the PIC's clock.

  if(get_cycles().get() <= tmrl->synchronized_cycle)
    return value.get();

  // If the TMR is not running then return.
  if(!tmrl->t1con->get_tmr1on())
    return value.get();

  tmrl->current_value();

  return(value.get());

}


//--------------------------------------------------
//
//--------------------------------------------------

class TMRl_GateSignalSink : public SignalSink
{
public:
  TMRl_GateSignalSink(TMRL *_tmr1l)
    : m_tmr1l(_tmr1l)
  {
    assert(_tmr1l);
  }

  virtual void release() { delete this; }
  void setSinkState(char new3State)
  {
    m_tmr1l->IO_gate( new3State=='1' || new3State=='W');
  }
private:
  TMRL *m_tmr1l;
};

//--------------------------------------------------
// trivial class to represent a compare event reference
//--------------------------------------------------

class TMR1CapComRef
{
    public:
        TMR1CapComRef * next;

        CCPCON * ccpcon;
        uint value;

        TMR1CapComRef ( CCPCON * c, uint v ) : ccpcon(c), value(v) {};
};

//--------------------------------------------------
// member functions for the TMRL base class
//--------------------------------------------------
TMRL::TMRL(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    value_16bit(0),
    m_cState('?'), m_GateState(false), m_compare_GateState(true),
    m_io_GateState(true), m_bExtClkEnabled(false),
    m_sleeping(false), m_t1gss(true), m_Interrupt(0)
{
  value.put(0);
  synchronized_cycle = 0;
  prescale_counter = prescale = 1;
  break_value = 0x10000;
  last_cycle = 0;
  future_cycle = 0;

  ext_scale = 1.;
  tmrh    = 0;
  t1con   = 0;
  compare_queue = 0;
  for (int i = 0; i < 4; i++) m_clc[i] = 0;
}

TMRL::~TMRL()
{
  if (m_Interrupt) m_Interrupt->release();
}
/*
 * If we are similating an external RTC crystal for timer1,
 * compute scale factor between crsytal speed and processor
 * instruction cycle rate
 *
 * If tmr1cs = 1 Fosc is 4 x normal speed so reduce ticks by 1/4
 */
void TMRL::set_ext_scale()
{
    current_value();
    if (t1con->get_t1oscen() && (t1con->get_tmr1cs() == 2)) // external clock
    {
        ext_scale = get_cycles().instruction_cps()/
        t1con->freq_attribute->get_freq();
    }
    else if (t1con->get_tmr1cs() == 1) // Fosc
         ext_scale = 0.25;
    else ext_scale = 1.;

    if (future_cycle)
      last_cycle = get_cycles().get() - (int64_t)(value_16bit *( prescale * ext_scale) + 0.5);
}

void TMRL::release()
{
}

void TMRL::setIOpin(PinModule *extClkSource)
{
  Dprintf(("%s::setIOpin %s\n", name().c_str(), extClkSource?extClkSource->getPin().name().c_str():""));

  if (extClkSource)
    extClkSource->addSink(this);
}


void TMRL::setSinkState(char new3State)
{
  if (new3State != m_cState) {
    m_cState = new3State;
    if (m_bExtClkEnabled && (m_cState == '1' || m_cState == 'W'))
      increment();
  }
}

void TMRL::set_compare_event ( uint value, CCPCON *host )
{
  TMR1CapComRef * event = compare_queue;

  if ( host )
  {
    while ( event )
    {
      if ( event->ccpcon == host )
      {
        event->value = value;
        update();
        return;
      }
      event = event->next;
    }
    event = new TMR1CapComRef ( host, value );
    event->next = compare_queue;
    compare_queue = event;
    update();
  }
  else
      cout << "TMRL::set_compare_event called with no CAPCOM\n";
}

void TMRL::clear_compare_event ( CCPCON *host )
{
  TMR1CapComRef * event = compare_queue;
  TMR1CapComRef * * eptr = &compare_queue;

  while ( event )
  {
    if ( event->ccpcon == host )
    {
      *eptr = event->next;
      delete event;
      update();
      return;
    }
    eptr = &event->next;
    event = event->next;
  }
}

void TMRL::setGatepin(PinModule *extGateSource)
{
  Dprintf(("TMRL::setGatepin\n"));

  if (extGateSource)
    extGateSource->addSink(new TMRl_GateSignalSink(this));
}

void TMRL::set_T1GSS(bool arg)
{

    m_t1gss = arg;
    if (m_t1gss)
        IO_gate(m_io_GateState);
    else
        compare_gate(m_compare_GateState);
}
void TMRL::compare_gate(bool state)
{
  m_compare_GateState = state;
  if (!m_t1gss && m_GateState != state)
  {
    m_GateState = state;

    Dprintf(("TMRL::compare_gate state %d \n", state));

    if (t1con->get_tmr1GE())
    {
        update();
    }
  }
}
void TMRL::IO_gate(bool state)
{
  m_io_GateState = state;

  if (m_t1gss && (m_GateState != state))
  {
    m_GateState = state;

    Dprintf(("TMRL::IO_gate state %d \n", state));

    if (t1con->get_tmr1GE())
    {
        update();
    }
  }
}

//------------------------------------------------------------
// setInterruptSource()
//
// This Timer can be an interrupt source. When the interrupt
// needs to be generated, then the InterruptSource object will
// direct the interrupt to where it needs to go (usually this
// is the Peripheral Interrupt Register).

void TMRL::setInterruptSource(InterruptSource *_int)
{
  m_Interrupt = _int;
}

void TMRL::increment()
{
  Dprintf(("TMRL increment because of external clock\n"));

  if(--prescale_counter == 0) {
    prescale_counter = prescale;

  // In synchronous counter mode prescaler works but rest of tmr1 does not
  if (t1con->get_t1sync() == 0 && m_sleeping)
    return;

  // prescaler works but rest of timer turned off
  if (!t1con->get_tmr1on()) return;

    // If TMRH/TMRL have been manually changed, we'll want to
    // get the up-to-date values;

    current_value();

    value_16bit = 0xffff & ( value_16bit + 1);

    tmrh->value.put((value_16bit >> 8) & 0xff);
    value.put(value_16bit & 0xff);
    if(value_16bit == 0 && m_Interrupt)
    {
      m_Interrupt->Trigger();
      for(int i=0; i<4; i++)
        if (m_clc[i]) m_clc[i]->t1_overflow();
    }
  }

}

void TMRL::on_or_off(int new_state)
{

  if(new_state) {

    Dprintf(("%s is being turned on\n", name().c_str()));

    // turn on the timer

    // Effective last cycle
    // Compute the "effective last cycle", i.e. the cycle
    // at which TMR1 was last 0 had it always been counting.

    last_cycle = (int64_t)(get_cycles().get() -
        (value.get() + (tmrh->value.get()<<8)) * prescale * ext_scale + 0.5);
    update();
  }
  else {

    Dprintf(("%s is being turned off\n", name().c_str()));

    // turn off the timer and save the current value
    current_value();
    if (future_cycle)
    {
        get_cycles().clear_break(this);
        future_cycle = 0;
    }
  }

}
//
// If anything has changed to affect when the next TMR1 break point
// will occur, this routine will make sure the break point is moved
// correctly.
//

void TMRL::update()
{

  Dprintf(("TMR1 %s update now=0x%" PRINTF_GINT64_MODIFIER "x\n",name().c_str(), get_cycles().get()));
  // if t1con->get_t1GINV() is false, timer can run if m_GateState == 0

  bool gate = t1con->get_t1GINV() ? m_GateState : !m_GateState;
  Dprintf(("TMRL::update gate %d GateState %d inv %d get_tmr1on %x tmr1GE %x tmr1cs %x t1oscen %x\n", gate, m_GateState, t1con->get_t1GINV(), t1con->get_tmr1on(), t1con->get_tmr1GE(), t1con->get_tmr1cs(), t1con->get_t1oscen()));
  /* When tmr1 is on, and t1con->get_tmr1GE() is true,
     gate == 1 allows timer to run, gate == 0 stops timer.
     However, if t1con->get_tmr1GE() is false gate has no
     effect on timer running or not.
  */
  if(t1con->get_tmr1on() && (t1con->get_tmr1GE() ? gate : true))
  {
    switch(t1con->get_tmr1cs())
    {
    case 0:        // internal clock Fosc/4
        break;

    case 1:        // internal clock Fosc
        break;

    case 2:        // External clock
        if (t1con->get_t1oscen())        // External clock enabled
        {
        /*
         external timer1 clock runs off a crystal which is typically
         32768 Hz and is independant on the instruction clock, but
         gpsim runs on the instruction clock. Ext_scale is the ratio
         of these two clocks so the breakpoint can be adjusted to be
         triggered at the correct time.
        */
        }
        else                        // External stimuli(pin)
        {
              prescale = 1 << t1con->get_prescale();
              prescale_counter = prescale;
              set_ext_scale();
              return;
        }
        break;

     case 3:                        // Cap. sensing oscillator
        prescale = 1 << t1con->get_prescale();
        prescale_counter = prescale;
        set_ext_scale();
        return;
        break;

     default:
        cout << "TMR1SC reserved value " << t1con->get_tmr1cs() << endl;
        break;
     }

     set_ext_scale();


      // Note, unlike TMR0, anytime something is written to TMRL, the
      // prescaler is unaffected on the P18 processors. However, it is
      // reset on the p16f88 processor, which is how the current code
      // works. This only effects the external drive mode.

      prescale = 1 << t1con->get_prescale();
      prescale_counter = prescale;

      //  synchronized_cycle = cycles.get() + 2;
      synchronized_cycle = get_cycles().get();


      last_cycle = synchronized_cycle
                        - (int64_t)(value_16bit *( prescale * ext_scale) + 0.5);


      break_value = 0x10000;  // Assume that a rollover will happen first.

      for ( TMR1CapComRef * event = compare_queue; event; event = event->next )
      {
        if ( event->value > value_16bit && event->value < break_value )
        {
            // A compare interrupt is going to happen before the timer
            // will rollover.
            break_value = event->value;
        }
      }
      uint64_t fc = get_cycles().get()
                + (uint64_t)((break_value - value_16bit) * prescale * ext_scale);

      if(future_cycle)
        get_cycles().reassign_break(future_cycle, fc, this);
      else
        get_cycles().set_break(fc, this);

      future_cycle = fc;
    }
  else
    {
      // turn off the timer and save the current value
      if (future_cycle)
      {
        current_value();
        get_cycles().clear_break(this);
        future_cycle = 0;
      }
    }
}

void TMRL::put(uint new_value)
{
  set_ext_scale();

  value.put(new_value & 0xff);

  if (!tmrh || !t1con) return;

  synchronized_cycle = get_cycles().get();
  last_cycle = synchronized_cycle - (int64_t)(( value.get()
        + (tmrh->value.get()<<8)) * prescale * ext_scale + 0.5);

  current_value();

  if(t1con->get_tmr1on()) update();
}

uint TMRL::get()
{
  return get_value();
}

// For the gui and CLI
uint TMRL::get_value()
{
  // If the TMRL is being read immediately after being written, then
  // it hasn't had enough time to synchronize with the PIC's clock.
  if(get_cycles().get() <= synchronized_cycle)
    return value.get();

  // If TMRL is not on, then return the current value
  if(!t1con->get_tmr1on())
    return value.get();

  current_value();

  return(value.get());
}

//%%%FIXME%%% inline this
// if break inactive (future_cycle == 0), just read the TMR1H and TMR1L
// registers otherwise compute what the register should be and then
// update TMR1H and TMR1L.
// RP: Using future_cycle here is not strictly right. What we really want is
// the condition "TMR1 is running on a GPSIM-generated clock" (as opposed
// to being off, or externally clocked by a stimulus). The presence of a
// breakpoint is _usually_ a good indication of this, but not while we're
// actually processing that breakpoint. For the time being, we work around
// this by calling current_value "redundantly" in callback()
//
void TMRL::current_value()
{
  if (!tmrh)
    return;
  if (future_cycle == 0)
      value_16bit = tmrh->value.get() * 256 + value.get();
  else
  {
    value_16bit = (uint64_t)((get_cycles().get() - last_cycle)/
                (prescale* ext_scale));


    if (value_16bit > 0x10000)
        cerr << "overflow TMRL " << name() << " " << value_16bit << endl;

    value.put(value_16bit & 0xff);
    tmrh->value.put((value_16bit>>8) & 0xff);
  }
}

uint TMRL::get_low_and_high()
{
  // If the TMRL is being read immediately after being written, then
  // it hasn't had enough time to synchronize with the PIC's clock.
  if(get_cycles().get() <= synchronized_cycle)
    return value.get();

  current_value();

  return(value_16bit);
}

// set m_bExtClkEnable is tmr1 is being clocked by an external stimulus
void TMRL::new_clock_source()
{

    m_bExtClkEnabled = false;

    current_value();

    switch(t1con->get_tmr1cs())
    {
    case 0:        // Fosc/4
        put(value.get());
        break;

    case 1:        // Fosc
        put(value.get());
        break;

    case 2:        // External pin or crystal

        if(t1con->get_t1oscen())        // External crystal, simulate
        {
            put(value.get());    // let TMRL::put() set a cycle counter break point
        }
        else        // external pin
        {
            if (future_cycle)
            {
              // Compute value_16bit with old prescale and ext_scale
                current_value();
                get_cycles().clear_break(this);
                future_cycle = 0;
            }
            prescale = 1 << t1con->get_prescale();
            prescale_counter = prescale;
            set_ext_scale();
            m_bExtClkEnabled = true;
        }
        break;

    case 3:        // Capacitor sense oscillator
        if (future_cycle)
        {
          // Compute value_16bit with old prescale and ext_scale
                current_value();
                get_cycles().clear_break(this);
                future_cycle = 0;
        }
        prescale = 1 << t1con->get_prescale();
        prescale_counter = prescale;
        set_ext_scale();
        break;
    }

}

//
// clear_timer - This is called by either the CCP or PWM modules to
// reset the timer to zero. This is rather easy since the current TMR
// value is always referenced to the cpu cycle counter.
//

void TMRL::clear_timer()
{

  synchronized_cycle = get_cycles().get();
  last_cycle = synchronized_cycle;
  value.put(0);
  tmrh->value.put(0);
}

// TMRL callback is called when the cycle counter hits the break point that
// was set in TMRL::put. The cycle counter will clear the break point, so
// we don't need to worry about it. At this point, TMRL is rolling over.

void TMRL::callback()
{
  // If TMRL is being clocked by the external clock, then at some point
  // the simulate code must have switched from the internal clock to
  // external clock. The cycle break point was still set, so just ignore it.
  if((t1con->get_tmr1cs() == 2) && ! t1con->get_t1oscen())
    {
      value.put(0);
      tmrh->value.put(0);
      future_cycle = 0;  // indicates that TMRL no longer has a break point
      return;
    }

  current_value();      // Because this relies on future_cycle, we must call it before clearing that

  future_cycle = 0;     // indicate that there's no break currently set

  if(break_value < 0x10000)
  {
      // The break was due to a "compare"

      if ( value_16bit != break_value )
          cout << "TMR1 compare break: value=" << value_16bit << " but break_value=" << break_value << '\n';

      for ( TMR1CapComRef * event = compare_queue; event; event = event->next )
      {
        if ( event->value == break_value )
        {
            // This CCP channel has a compare at this time
          event->ccpcon->compare_match();
        }
      }
    }
  else
    {

      // The break was due to a roll-over

      //cout<<"TMRL rollover: " << hex << cycles.get() << '\n';
      if (m_Interrupt)
        m_Interrupt->Trigger();

      for(int i=0; i<4; i++)
        if (m_clc[i]) m_clc[i]->t1_overflow();

      // Reset the timer to 0.

      synchronized_cycle = get_cycles().get();
      last_cycle = synchronized_cycle;
      value.put(0);
      tmrh->value.put(0);
    }

  update();

}

//---------------------------

void TMRL::callback_print()
{
  cout << "TMRL " << name() << " CallBack ID " << CallBackID << '\n';

}


//---------------------------

void TMRL::sleep()
{
    m_sleeping = true;
    Dprintf(("TMRL::sleep t1sysc %d\n", t1con->get_t1sync()));
    // If tmr1 is running off Fosc/4 or Fosc this assumes Fosc stops during sleep

    if (  t1con->get_tmr1on() && t1con->get_tmr1cs() < 2)
    {
      if (future_cycle)
      {
        current_value();
        get_cycles().clear_break(this);
        future_cycle = 0;
      }
    }
}

//---------------------------

void TMRL::wake()
{
    m_sleeping = false;
    Dprintf(("TMRL::wake\n"));
    if (  t1con->get_tmr1on() && t1con->get_tmr1cs() < 2)
    {
        update();
    }
}

//--------------------------------------------------
// member functions for the PR2 base class
//--------------------------------------------------

PR2::PR2(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    tmr2(0)
{
}

void PR2::put(uint new_value)
{
  Dprintf(("PR2:: put %x\n", new_value));

    if(value.get() != new_value)
    {
      if (tmr2) tmr2->new_pr2(new_value);
      value.put(new_value);
    }
    else value.put(new_value);
}

//--------------------------------------------------
// member functions for the T2CON base class
//--------------------------------------------------

T2CON::T2CON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    tmr2(0)
{
}

void T2CON::put(uint new_value)
{
  uint diff = value.get() ^ new_value;
  value.put(new_value);

    if (tmr2) 
    {
        tmr2->new_pre_post_scale();

        if( diff & TMR2ON) tmr2->on_or_off(value.get() & TMR2ON);
    }
}


TMR2::TMR2(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    pwm_mode(0),
    update_state(TMR2_ANY_PWM_UPDATE | TMR2_PR2_UPDATE),
    last_update(0),
    prescale(1),
    prescale_counter(0), break_value(0), post_scale(0),
    last_cycle(0),
    pr2(0), pir_set(0), t2con(0), m_txgcon(0), m_Interrupt(0),
    tmr2_interface(0)
{
  ssp_module[0] = ssp_module[1] = 0;
  value.put(0);
  future_cycle = 0;

  std::fill_n(duty_cycle, MAX_PWM_CHANS, 0);
  for (int i = 0; i < 4; i++)
    m_clc[i] = 0;
  for (int cc = 0; cc < MAX_PWM_CHANS; cc++ )
    ccp[cc] = 0;
}

TMR2::~TMR2()
{
  if (m_Interrupt) m_Interrupt->release();
}


void TMR2::callback_print()
{
  cout << "TMR2 " << name() << " CallBack ID " << CallBackID << '\n';
}

void TMR2::start()
{
  value.put(0);
  prescale = 0;
  last_cycle = 0;
  future_cycle = 0;
}

bool TMR2::add_ccp ( CCPCON * _ccp )
{
  int cc;

  for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
  {
    if ( ccp[cc] == 0 || ccp[cc] == _ccp )
    {
        ccp[cc] = _ccp;
        return true;
    }
  }
  return false;
}

bool TMR2::rm_ccp ( CCPCON * _ccp )
{
  int cc;

  for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
  {
    if ( ccp[cc] == _ccp )
    {
        ccp[cc] = 0;
        return true;
    }
  }
  return false;
}


void TMR2::on_or_off(int new_state)
{
  if(new_state) 
  {
    Dprintf(("TMR2 is being turned on\n"));

    // turn on the timer

    // Effective last cycle
    // Compute the "effective last cycle", i.e. the cycle
    // at which TMR2 was last 0 had it always been counting.

    last_cycle = get_cycles().get() - value.get()*prescale;
    update();
  }
  else
  {
    Dprintf(("TMR2 is being turned off\n"));
    current_value(); // turn off the timer and save the current value
  }
}

//
// pwm_dc - set PWM duty cycle
//
void TMR2::pwm_dc(uint dc, uint ccp_address)
{
    int modeMask = TMR2_PWM1_UPDATE;
    bool found = false;
    int cc;

  for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
  {
    if ( ccp[cc] && ( ccp_address == ccp[cc]->address ) )
    {
      //cout << "TMR2:  pwm mode with ccp1. duty cycle = " << hex << dc << '\n';
      Dprintf(("TMR2::pwm_dc duty cycle 0x%x ccp_addres 0x%x\n", dc, ccp_address));
      duty_cycle[cc] = dc;
      pwm_mode |= modeMask;
      return;
    }
    modeMask <<= 1;
  }
  if ( ! found )
  {
    cout << name() <<": error bad ccpxcon address while in pwm_dc()\n";
    cout << "ccp_address = " << ccp_address << " expected one of";
    for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
      if ( ccp[cc] )
        cout << " " << ccp[cc]->address;
    cout << '\n';
  }
}

//
// stop_pwm
//

void TMR2::stop_pwm(uint ccp_address)
{
    int modeMask = TMR2_PWM1_UPDATE;
    int cc;
    int old_pwm = pwm_mode;

  for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
  {
    if ( ccp[cc] && ( ccp_address == ccp[cc]->address ) )
    {
      // cout << "TMR2:  stopping pwm mode with ccp" << cc+1 << ".\n";
      pwm_mode &= ~modeMask;
      if(last_update & modeMask)
         update_state &= (~modeMask);
    }
    modeMask <<= 1;
  }

  if((pwm_mode ^ old_pwm) && future_cycle && t2con->get_tmr2on())
    update(update_state);
}

//
// update
//  This member function will determine if/when there is a TMR2 break point
// that needs to be set and will set/move it if so.
//  There are two different types of break sources:
//     1) TMR2 matching PR2
//     2) TMR2 matching one of the ccp registers in pwm mode
//

void TMR2::update(int ut)
{
    int modeMask = TMR2_PWM1_UPDATE;
    int cc;

  //cout << "TMR2 update. cpu cycle " << hex << cycles.get() <<'\n';

  if(t2con->get_tmr2on())
  {
    if(future_cycle)
    {
      // If TMR2 is enabled (i.e. counting) then 'future_cycle' is non-zero,
      // which means there's a cycle break point set on TMR2 that needs to
      // be moved to a new cycle.

      current_value();

      // Assume that we are not in pwm mode (and hence the next break will
      // be due to tmr2 matching pr2)

      break_value = 1 + pr2->value.get();
      uint64_t fc = get_cycles().get() + (break_value - value.get()) * prescale;

      last_update = TMR2_PR2_UPDATE;

      // RP - I strongly suspect this is now entirely redundant, as I think the
      // result comes out the same as above.
      if (pwm_mode)
      {
          fc = last_cycle + break_value * prescale;
      }

      for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
      {
        if ( pwm_mode & ut & modeMask )
        {
          /* We are in pwm mode... So let's see what happens first: a pr2
             compare or a duty cycle compare. The duty cycle is 10-bits,
             but the two least significant match against the prescaler
             rather than TMR2.
          */

          if ( (duty_cycle[cc] > (value.get()*4) ) && ( duty_cycle[cc] < break_value*4 ) )
          {
            uint64_t nc = last_cycle + ( duty_cycle[cc] * prescale ) / 4;

            // cout << "TMR2:PWM" << cc+1 << " update at " << hex << nc <<
            //         ", dc=" << duty_cycle[cc] << "\n";
            if ( nc < fc )      /// @bug not robust against wrap-around of uint64_t
            {
              last_update = modeMask;
              fc = nc;
            }
            else if ( nc == fc )
            {
              last_update |= modeMask;
            }
          }
        }
        modeMask <<= 1;
      }
      if (fc != future_cycle)
      {
          // cout << "TMR2: update new break at cycle "<<hex<<fc<<'\n';
          get_cycles().reassign_break(future_cycle, fc, this);
          future_cycle = fc;
      }
    }
    else
    {
      cerr << "TMR2 BUG!! tmr2 is on but has no cycle_break set on it\n";
    }
  }
  else
  {
    // cout << "TMR2 is not running (no update occurred)\n";
  }
}


void TMR2::put(uint new_value)
{

  uint old_value = get_value();

  value.put(new_value & 0xff);

    if(future_cycle)
    {
      // If TMR2 is enabled (i.e. counting) then 'future_cycle' is non-zero,
      // which means there's a cycle break point set on TMR2 that needs to
      // be moved to a new cycle.

      uint64_t current_cycle = get_cycles().get();
      uint delta = (future_cycle - last_cycle);
      int shift = (new_value - old_value) * prescale;

//      printf ( "TMR2::put(%02X) with future_cycle -\n", new_value );

//      printf ( "   move break from 0x%" PRINTF_INT64_MODIFIER "X by %d\n", current_cycle, shift );

      // set cycle when tmr2 would have been zero

      last_cycle = current_cycle - shift;
      uint now = (current_cycle - last_cycle);

//      printf ( "   value now is 0x%X\n", now );

      uint64_t fc;

      /*
        Three possible cases
           1> TMR2 is still before the next break point.
                Adjust the breakpoint to occur at correct TMR2 value
           2> TMR2 is now greater the PR2
                Assume TMR2 must count up to 0xff, roll-over and then
                we are back in business. High CCP outputs stay high.
           3> TMR2 is now less than PR2 but greater than a CCP duty cycle point.
                The CCP output stays as the duty cycle comparator does not
                match on this cycle.
      */

      if (now < delta) // easy case, just shift break.
      {
          fc = last_cycle + delta;

//          printf ( "   now < delta (0x%X), set future_cycle to 0x%" PRINTF_INT64_MODIFIER "X\n", delta, fc );

          get_cycles().reassign_break(future_cycle, fc, this);
          future_cycle = fc;
      }
      else if (now >= break_value * prescale)  // TMR2 now greater than PR2
      {
        // set break to when TMR2 will overflow
        last_update |= TMR2_WRAP;
        fc = last_cycle + 0x100 * prescale;

//        printf ( "   now > break (0x%X), set future_cycle to 0x%" PRINTF_INT64_MODIFIER "X\n", break_value * prescale, fc );

        get_cycles().reassign_break(future_cycle, fc, this);
        future_cycle = fc;
      }
      else        // new break < PR2 but > duty cycle break
      {
//          printf ( "   new break < PR2 but > duty cycle\n" );
          update(update_state);
      }



     /*
        'clear' the post scale counter. (I've actually implemented the
        post scale counter as a count-down counter, so 'clearing it'
        means resetting it to the starting point.
     */
      if (t2con)
        post_scale = t2con->get_post_scale();
    }
}

uint TMR2::get()
{
    if(t2con->get_tmr2on()) current_value();

    return(value.get());
}

uint TMR2::get_value()
{
    if(t2con->get_tmr2on()) current_value();

    return(value.get());
}

void TMR2::new_pre_post_scale()
{
  //cout << "T2CON was written to, so update TMR2 " << t2con->get_tmr2on() << "\n";

  if(!t2con->get_tmr2on()) {
    // TMR2 is not on. If has just been turned off, clear the callback breakpoint.

    if(future_cycle) {
      get_cycles().clear_break(this);
      future_cycle = 0;
    }
    return;
  }

  uint old_prescale = prescale;
  prescale = t2con->get_pre_scale();
  post_scale = t2con->get_post_scale();

  if(future_cycle)
    {
      // If TMR2 is enabled (i.e. counting) then 'future_cycle' is non-zero,
      // which means there's a cycle break point set on TMR2 that needs to
      // be moved to a new cycle.


      if (prescale != old_prescale)        // prescaler value change
      {
        // togo is number of cycles to next callback based on new prescaler.
        uint64_t togo = (future_cycle - get_cycles().get()) * prescale / old_prescale;

        if (!togo)        // I am not sure this can happen RRR
            callback();
        else
        {
          uint64_t fc = togo + get_cycles().get();

          get_cycles().reassign_break(future_cycle, fc, this);
          future_cycle = fc;
        }
      }
    }
  else
    {
      //cout << "TMR2 was off, but now it's on.\n";

      if (value.get() == pr2->value.get()) // TMR2 == PR2
      {
        future_cycle = get_cycles().get();
        get_cycles().set_break(future_cycle, this);
        callback();
      }
      else if (value.get() > pr2->value.get()) // TMR2 > PR2
      {
        cout << "Warning TMR2 turned on with TMR2 greater than PR2\n";
        // this will cause TMR2 to wrap
        future_cycle  = get_cycles().get() +
                (1 + pr2->value.get() + (0x100 -  value.get())) * prescale;
        get_cycles().set_break(future_cycle, this);
      }
      else
      {
        future_cycle = get_cycles().get() + 1;
          get_cycles().set_break(future_cycle, this);
          last_cycle = get_cycles().get() - value.get();
          update(update_state);
      }
  }

}

void TMR2::new_pr2(uint new_value)
{
  Dprintf(("TMR2::new_pr2 on=%u\n", t2con->get_tmr2on()));

  if(t2con->get_tmr2on())
    {
      Dprintf(( "TMR2::new_pr2(0x%02X) with timer at 0x%02X -\n", new_value, value.get() ));

      uint cur_break = (future_cycle - last_cycle)/prescale;
      uint new_break = 1 + new_value;
      uint now_cycle = (get_cycles().get() - last_cycle) / prescale;

      uint64_t fc = last_cycle;
      Dprintf(( "   cur_break = 0x%X,  new_break = 0x%X,  now = 0x%X\n", cur_break, new_break, now_cycle ));
      Dprintf(( "   last_cycle = 0x%" PRINTF_GINT64_MODIFIER "X\n", fc ));

      /*
        PR2 change cases

        1> TMR2 greater than new PR2
                TMR2 wraps through 0xff
        2> New PR2 breakpoint less than current breakpoint or
           current break point due to PR2
                Change breakpoint to new value based on PR2
        3> Other breakpoint < PR2 breakpoint
                No need to do anything.
     */


      if (now_cycle > new_break)        // TMR2 > PR2 do wrap
      {
        // set break to when TMR2 will overflow
        last_update |= TMR2_WRAP;
        fc += 0x100 * prescale;
        Dprintf(( "   now > new, set future_cycle to 0x%" PRINTF_GINT64_MODIFIER "X\n", fc ));
        get_cycles().reassign_break(future_cycle, fc, this);
        future_cycle = fc;
      }
      else if (cur_break == break_value ||        // breakpoint due to pr2
               new_break < cur_break)                // new break less than current
      {
        fc += new_break * prescale;
        Dprintf(( "   new<break, set future_cycle to 0x%" PRINTF_GINT64_MODIFIER "X\n", fc ));
        if ( cur_break != break_value )
            last_update = TMR2_PR2_UPDATE;      // RP : fix bug 3092906
        get_cycles().reassign_break(future_cycle, fc, this);
        future_cycle = fc;
        Dprintf(( "   next event mask %02X\n", last_update ));
      }
    }
}

void TMR2::current_value()
{
  uint tmr2_val = (get_cycles().get() - last_cycle)/ prescale;
  if (future_cycle)
        tmr2_val = (get_cycles().get() - last_cycle)/ prescale;
  else
        tmr2_val = value.get();

  if (tmr2_val == max_counts())
  {
    // tmr2 is about to roll over. However, the user code
    // has requested the current value before the callback function
    // has been invoked. So do callback and return 0.
     if (future_cycle)
     {
        future_cycle = 0;
        get_cycles().clear_break(this);
        callback();
     }
     tmr2_val = 0;
  }

  value.put(tmr2_val & (max_counts() - 1));


  if(tmr2_val >= max_counts())        // Can get to max_counts  during transition
  {
   cerr << "TMR2 BUG!! value = 0x" << tmr2_val << " which is greater than 0x";
   cerr << max_counts() << endl;
  }
}

// TMR2 callback is called when the cycle counter hits the break point that
// was set in TMR2::put. The cycle counter will clear the break point, so
// we don't need to worry about it. At this point, TMR2 is equal to PR2.

void TMR2::callback()
{
  int cc;

  //cout<<"TMR2 callback cycle: " << hex << cycles.value << '\n';

  // If tmr2 is still enabled, then set up for the next break.
  // If tmr2 was disabled then ignore this break point.
  if(t2con->get_tmr2on())
    {

      // What caused the callback: PR2 match or duty cyle match ?

      if (last_update & TMR2_WRAP) // TMR2 > PR2
      {
        last_update &= ~TMR2_WRAP;
        // This (implicitly) resets the timer to zero:
        last_cycle = get_cycles().get();
      }
      else if ( last_update & TMR2_ANY_PWM_UPDATE )
      {
        int modeMask = TMR2_PWM1_UPDATE;

        for ( cc=0; cc<MAX_PWM_CHANS && last_update; cc++ )
        {
          if ( last_update & modeMask )
          {
            // duty cycle match
            //cout << name() << ": duty cycle match for pwm" << cc+1 << "\n";
            update_state &= (~modeMask);
            last_update &= ~modeMask;
            if ( ccp[cc] )      // shouldn't be needed
                ccp[cc]->pwm_match(0);
            else
                cout << "TMR2::callback() found update of non-existent CCP\n";
          }
          modeMask <<= 1;
        }
      }
      else
      {
        // matches PR2

        //cout << "TMR2: PR2 match. pwm_mode is " << pwm_mode <<'\n';

        // This (implicitly) resets the timer to zero:
        last_cycle = get_cycles().get();
        for(int i =0; i<4; i++)
            if(m_clc[i]) m_clc[i]->t2_match();

        if (ssp_module[0])
             ssp_module[0]->tmr2_clock();
        if (ssp_module[1])
             ssp_module[1]->tmr2_clock();
        if (m_txgcon)        // toggle T2_gate, if present
        {
            m_txgcon->T2_gate(1);
            m_txgcon->T2_gate(0);
        }

        for ( cc=0; cc<MAX_PWM_CHANS; cc++ )
        {
          // RRR FIX
          if ( ccp[cc] &&  ccp[cc]->is_pwm())
          {
             ccp[cc]->pwm_match(1);
          }
        }

        if(--post_scale < 0)
        {
          //cout << "setting IF\n";
          if (pir_set)
              pir_set->set_tmr2if();
          else if (m_Interrupt) // for multiple T2 (T2, T4, T6)
              m_Interrupt->Trigger();

          post_scale = t2con->get_post_scale();
        }

        update_state = TMR2_ANY_PWM_UPDATE | TMR2_PR2_UPDATE;

      }
      update(update_state);

    }
  else
    future_cycle = 0;
}


//------------------------------------------------------------------------
// TMR2_MODULE
//
//

TMR2_MODULE::TMR2_MODULE()
{

  t2con = 0;
  pr2   = 0;
  tmr2  = 0;
  cpu   = 0;
  name_str = 0;

}

void TMR2_MODULE::initialize(T2CON *t2con_, PR2 *pr2_, TMR2  *tmr2_)
{

  t2con = t2con_;
  pr2   = pr2_;
  tmr2  = tmr2_;

}

//--------------------------------------------------
//
//--------------------------------------------------

class INT_SignalSink : public SignalSink
{
public:
  INT_SignalSink(ECCPAS *_eccpas, int _index)
    : m_eccpas(_eccpas), m_index(_index)
  {
    assert(_eccpas);
  }

  virtual void release() { delete this; }
  void setSinkState(char new3State)
  {
    m_eccpas->set_trig_state( m_index, new3State=='0' || new3State=='w');
  }
private:
  ECCPAS *m_eccpas;
  int     m_index;
};

//--------------------------------------------------
// ECCPAS
//--------------------------------------------------
ECCPAS::ECCPAS(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc),
    pwm1con(0), ccp1con(0),
    m_PinModule(0)
{
    trig_state[0] = trig_state[1] = trig_state[2] = false;
    mValidBits = 0xff;
}

ECCPAS::~ECCPAS()
{
}
void ECCPAS::link_registers(PWM1CON *_pwm1con, CCPCON *_ccp1con)
{
        pwm1con = _pwm1con;
        ccp1con = _ccp1con;
}
void ECCPAS::put(uint new_value)
{
  Dprintf(("ECCPAS::put() new_value=0x%x\n",new_value));

  put_value(new_value);
}
void ECCPAS::put_value(uint new_value)
{

  int old_value = value.get();
  new_value &= mValidBits;


  // Auto-shutdown trigger active
  //   make sure ECCPASE is set
  //   if change in shutdown status, drive bridge outputs as per current flags
  if (shutdown_trigger(new_value))
  {
        new_value |= ECCPASE;
        if ((new_value ^ old_value) &  (ECCPASE|PSSAC1|PSSAC0|PSSBD1|PSSBD0))
            ccp1con->shutdown_bridge(new_value);
  }
  else // no Auto-shutdown triggers active
  {
      if (pwm1con->value.get() & PWM1CON::PRSEN) // clear ECCPASE bit
        new_value &= ~ ECCPASE;
  }
  value.put(new_value);
}
// Return true is shutdown trigger is active
bool ECCPAS::shutdown_trigger(int key)
{

  if ((key & ECCPAS0) && trig_state[0])
        return true;

  if ((key & ECCPAS1) && trig_state[1])
        return true;

  if ((key & ECCPAS2) && trig_state[2])
        return true;

  return false;
}
// connect IO pins to shutdown trigger source
void ECCPAS::setIOpin(PinModule *p0, PinModule *p1, PinModule *p2)
{
    if (p0)
    {
        m_PinModule = p0;
        m_sink = new INT_SignalSink(this, 0);
        p0->addSink(m_sink);
    }
    if (p1)
    {
        m_PinModule = p1;
        m_sink = new INT_SignalSink(this, 1);
        p1->addSink(m_sink);
    }
    if (p2)
    {
        m_PinModule = p2;
        m_sink = new INT_SignalSink(this, 2);
        p2->addSink(m_sink);
    }
}

// set shutdown trigger states
void ECCPAS::set_trig_state(int index, bool state)
{
    if (trig_state[index] != state)
    {
        Dprintf(("index=%d state=%d old=%d\n", index, state, trig_state[index]));
        trig_state[index] = state;
        put_value(value.get());
    }
}
// Trigger state from comparator 1
void ECCPAS::c1_output(int state)
{
    set_trig_state(0, state);
}
// Trigger state from comparator 2
void ECCPAS::c2_output(int state)
{
    set_trig_state(1, state);
}
//--------------------------------------------------
// PWM1CON
//--------------------------------------------------
PWM1CON::PWM1CON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
  mValidBits = 0xff;
}

PWM1CON::~PWM1CON()
{
}
void PWM1CON::put(uint new_value)
{
  new_value &= mValidBits;
  Dprintf(("PWM1CON::put() new_value=0x%x\n",new_value));

  value.put(new_value);
}
//--------------------------------------------------
// PSTRCON
//--------------------------------------------------
PSTRCON::PSTRCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
}

PSTRCON::~PSTRCON()
{
}
void PSTRCON::put(uint new_value)
{
  Dprintf(("PSTRCON::put() new_value=0x%x\n",new_value));
  new_value &= STRSYNC|STRD|STRC|STRB|STRA;

  value.put(new_value);
}

//--------------------------------------------------------------------
//  PWM TIMER SELECTION CONTROL REGISTER
//--------------------------------------------------------------------
CCPTMRS14::CCPTMRS14(Processor *pCpu, const char *pName, const char *pDesc)
         : sfr_register(pCpu, pName, pDesc)
{
    for(int i=0; i<4; i++) ccp[i] = 0;
    t2=t4=t6=0;
}

void CCPTMRS14::put(unsigned int new_value)
{
    TMR2* tx;
    value.put(new_value);
    
    for(int i=0; i<4; i++)
    {
        switch(new_value & 0x3)
        {
            case 0:
                tx = t2;
                break;
                
            case 1:
                tx = t4;
                break;
                
            case 2:
                tx = t6;
                break;
                
            default:
                tx = 0;
                break;
        }
        if (ccp[i] && tx)
        {
            ccp[i]->set_tmr2(tx);
            tx->add_ccp(ccp[i]);
        }
        new_value >>= 2;
    }
}

/*
   Copyright (C) 2015        Roy R Rankin

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

#include "config.h"
#include "14bit-processors.h"
#include "14bit-registers.h"
#include "a2d_v2.h"
#include "ctmu.h"

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif


CTMUCONH::CTMUCONH(Processor *pCpu, const char *pName, 
            const char *pDesc, CTMU *_ctmu)
    : sfr_register(pCpu,pName,pDesc),
    ctmu(_ctmu)
{
    ctmu->ctmuconh = this;
}


void CTMUCONH::put(uint new_value)
{
    uint diff = value.get() ^ new_value;

    value.put(new_value);
    if (diff & CTMUEN)        // on or off
    {
        if (new_value & CTMUEN)        // enable CTMU
            ctmu->enable(new_value);
        else
            ctmu->disable();        // disable CTMU
    }
    if ((diff & TGEN) || (diff & CTMUEN))        // Pulse generation
    {
       if ((new_value & TGEN) && (new_value & CTMUEN))
            ctmu->tgen_on();
       else
            ctmu->tgen_off();
    }
    if (diff & IDISSEN)
        ctmu->idissen(new_value & IDISSEN);
}

CTMUCONL::CTMUCONL(Processor *pCpu, const char *pName, 
            const char *pDesc, CTMU *_ctmu)
    : sfr_register(pCpu,pName,pDesc),
    ctmu(_ctmu)
{
    ctmu->ctmuconl = this;
}


void CTMUCONL::put(uint new_value)
{
    uint diff = value.get() ^ new_value;

    value.put(new_value);
    if (diff)
        ctmu->stat_change();
}

CTMUICON::CTMUICON(Processor *pCpu, const char *pName, 
            const char *pDesc, CTMU *_ctmu)
    : sfr_register(pCpu,pName,pDesc),
    ctmu(_ctmu)
{
    ctmu->ctmuicon = this;
}

void CTMUICON::put(uint new_value)
{
    uint diff = value.get() ^ new_value;
    int adj= ((new_value & 0xfc) >>2);
    double I;

    value.put(new_value);

    if (!diff) return;

    if (new_value & ITRIM5) adj -= 0x40;

    switch(new_value & (IRNG0|IRNG1))
    {
    case 0:         // Current off
        I = 0.;
        break;

    case 1:                        // Base current
        I = 0.55e-6;        // 0.55 uA
            break;

    case 2:                        // 10x Range
        I = 5.5e-6;                //5.5 uA
        break;

    case 3:                        // 100x Range
        I = 55e-6;                // 55 uA
    }
    // AN1250 page 4 says adjustment steps 2%, no value found in 18f26k22
    //        spec sheet
    I += I * adj * 0.02;

    ctmu->new_current(I);
}


class CTMU_SignalSink : public SignalSink
{
    public:
      CTMU_SignalSink(CTMU *_ctmu)
        : m_state(false), m_ctmu(_ctmu)
      {
        assert(_ctmu);
      }

      virtual void setSinkState(char new3State) 
      { 
            m_state = ((new3State == '0') | (new3State == 'w'))?false:true; 
            m_ctmu->new_edge();
      }
      virtual void release() { delete this; }
      bool get_state() { return m_state;}
      
    private:
      bool m_state;
      CTMU *m_ctmu;
};


CTMU::CTMU(Processor *pCpu): cted1_state(false), cted2_state(false),
                ctmu_stim(0), m_cted1(0), m_cted2(0), 
                ctmu_cted1_sink(0),ctmu_cted2_sink(0),
                ctpls_source(0), cpu(pCpu)
{
}
void CTMU::enable(uint value)
{
    if (!ctmu_cted1_sink)
    {
        ctmu_cted1_sink = new CTMU_SignalSink(this);
        ctmu_cted2_sink = new CTMU_SignalSink(this);
    }
    m_cted1->addSink(ctmu_cted1_sink);
    m_cted2->addSink(ctmu_cted2_sink);

    idissen(value & CTMUCONH::IDISSEN);
    stat_change();
}


void CTMU::disable()
{
    current_off();
    if (ctmu_cted1_sink)
    {
        m_cted1->removeSink(ctmu_cted1_sink);
        m_cted2->removeSink(ctmu_cted2_sink);
        delete ctmu_cted1_sink; ctmu_cted1_sink = 0;
        delete ctmu_cted2_sink; ctmu_cted2_sink = 0;
    }
}
void CTMU::current_off()
{
        ctmu_stim->set_Vth(cpu->get_Vdd());
        ctmu_stim->set_Zth(1e12);
        ctmu_stim->updateNode();
}
void CTMU::new_current(double I)
{
    current = I;
    if (I)
        resistance = Vsrc / I;
    else
        resistance = 1e12;
}

void CTMU::stat_change()
{
    uint value = ctmuconl->value.get();
    bool edg1 = value & CTMUCONL::EDG1STAT;
    bool edg2 = value & CTMUCONL::EDG2STAT;

    // Don't do anything is CTMU not enables
    if (! (ctmuconh->value.get() & CTMUCONH::CTMUEN))
        return;

    /* either edg1 or edg2 set, but not both */
    if(edg1 ^ edg2)
    {
        ctmu_stim->set_Vth(Vsrc);
        ctmu_stim->set_Zth(resistance);
        ctmu_stim->updateNode();
        if (ctmuconh->value.get() & CTMUCONH::TGEN)
            ctpls_source->putState('1');
    }
    else
    {
        current_off();
        if (ctmuconh->value.get() & CTMUCONH::TGEN)
            ctpls_source->putState('0');
        if (ctmuconh->value.get() & CTMUCONH::CTTRIG)
        {
            adcon1->ctmu_trigger();
        }
    }
}
#define EDG1_SEL(x) ((x) & (CTMUCONL::EDG1SEL0 | CTMUCONL::EDG1SEL1))
#define EDG2_SEL(x) ((x) & (CTMUCONL::EDG2SEL0 | CTMUCONL::EDG2SEL1))
void CTMU::new_edge()
{
   uint value = ctmuconl->value.get();
   bool state1 = ctmu_cted1_sink->get_state();
   bool state2 = ctmu_cted2_sink->get_state();


   // return if edges are blocked
   if (!(ctmuconh->value.get() & CTMUCONH::EDGEN)) 
   {
       cted1_state = state1;
       cted2_state = state2;
       return;
   }
   if (state1 != cted1_state)        // state change on cted1
   {
        //using CTED1
        if (EDG1_SEL(value) == (CTMUCONL::EDG1SEL0 | CTMUCONL::EDG1SEL1))
        {
            // positive edge active 
            if (value & CTMUCONL::EDG1POL)
            {
                if (state1) value |= CTMUCONL::EDG1STAT;
            }
            // negative edge
            else
            {
                if (!state1) value |= CTMUCONL::EDG1STAT;
            }
            ctmuconl->put(value);
        }
        //using CTED1
        if (EDG2_SEL(value) == (CTMUCONL::EDG2SEL0 | CTMUCONL::EDG2SEL1))
        {
            // positive edge active 
            if (value & CTMUCONL::EDG2POL)
            {
                if (state1) value |= CTMUCONL::EDG2STAT;
            }
            // negative edge
            else
            {
                if (!state1) value |= CTMUCONL::EDG2STAT;
            }
            ctmuconl->put(value);
        }
        cted1_state = state1;
   }
   if (state2 != cted2_state)        // state change on cted2
   {
        //using CTED2
        if (EDG1_SEL(value) == (CTMUCONL::EDG1SEL1))
        {
            // positive edge active 
            if (value & CTMUCONL::EDG1POL)
            {
                if (state2) value |= CTMUCONL::EDG1STAT;
            }
            // negative edge
            else
            {
                if (!state2) value |= CTMUCONL::EDG1STAT;
            }
            ctmuconl->put(value);
        }
        //using CTED2
        if (EDG2_SEL(value) == (CTMUCONL::EDG2SEL1))
        {
            // positive edge active 
            if (value & CTMUCONL::EDG2POL)
            {
                if (state2) value |= CTMUCONL::EDG2STAT;
            }
            // negative edge
            else
            {
                if (!state2) value |= CTMUCONL::EDG2STAT;
            }
            ctmuconl->put(value);
        }
        cted2_state = state2;
   }
}

//Status from comparator module for C2
void CTMU::syncC2out(bool high)
{
    if ((ctmuconh->value.get() & CTMUCONH::TGEN) && high)
    {
        uint value = ctmuconl->value.get();
        value |= CTMUCONL::EDG2STAT;
        ctmuconl->put(value);
    }
}

void CTMU::tgen_on()
{
    cm2con1->set_ctmu_stim(ctmu_stim, this);

    if (!ctpls_source) ctpls_source = new PeripheralSignalSource(m_ctpls);
    m_ctpls->setSource(ctpls_source);
}

void CTMU::tgen_off()
{
    cm2con1->set_ctmu_stim(0, 0);

    if (ctpls_source) m_ctpls->setSource(0);
}

void CTMU::idissen(bool ground)
{
    // Don't do anything is CTMU not enables
    if (! (ctmuconh->value.get() & CTMUCONH::CTMUEN)) return;

    if (ground)
    {
        ctmu_stim->set_Vth(0.);
        ctmu_stim->set_Zth(300.0);
        ctmu_stim->updateNode();
    }
    else stat_change();
}


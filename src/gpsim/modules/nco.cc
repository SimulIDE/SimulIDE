/*
   Copyright (C) 2017,2018 Roy R. Rankin
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

// NUMERICALLY CONTROLLED OSCILLATOR (NCO) MODULE


//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

#include "pic-processor.h"
#include "packages.h"
#include "apfcon.h"
#include "nco.h"
#include "clc.h"
#include "cwg.h"

class NCOSigSource:public SignalControl
{
  public:
    NCOSigSource (NCO * _nco, PinModule * _pin):m_nco (_nco),
        m_state ('?'), m_pin (_pin)
    {
        assert (m_nco);
    }
    virtual ~ NCOSigSource ()
    {
    }

    void setState (char _state)
    {
        m_state = _state;
    }
    virtual char getState ()
    {
        return m_state;
    }
    virtual void release ()
    {
        m_nco->releasePinSource (m_pin);
    }

  private:
    NCO * m_nco;
    char m_state;
    PinModule *m_pin;
};

// Report state changes on incoming CLK pin
class ncoCLKSignalSink:public SignalSink
{
  public:
    ncoCLKSignalSink (NCO * _nco):m_nco (_nco)
    {
    }

    virtual void setSinkState (char new3State)
    {
        m_nco->setState (new3State);
    }
    virtual void release ()
    {
        delete this;
    }
  private:
    NCO * m_nco;
};


NCO::NCO (Processor * pCpu):
    nco1con (this, pCpu, "nco1con", "NCOx Control Register"),
    nco1clk (this, pCpu, "nco1clk", "NCOx Input Clock Control Register"),
    nco1acch (this, pCpu, "nco1acch", "NCOx Accumulator Register-High Byte"),
    nco1accl (this, pCpu, "nco1accl", "NCOx Accumulator Register-Low Byte"),
    nco1accu (this, pCpu, "nco1accu", "NCOx Accumulator Register-Upper Byte"),
    nco1inch (this, pCpu, "nco1inch", "NCOx Increment Register-High Byte"),
    nco1incl (this, pCpu, "nco1incl", "NCOx Increment Register-Low Byte"),
    pir (0), m_NCOif (0),
    cpu (pCpu), pinNCOclk (0), pinNCO1 (0), NCO1src (0), srcNCO1active (false),
    inc_load (0), inc (1), acc (0), future_cycle (0), last_cycle (0),
    CLKsink (0), CLKstate (false), NCOoverflow (false),
    accFlag (false), pulseWidth (0), m_cwg(0)
{
    acc_hold[0] = acc_hold[1] = acc_hold[2] = 0;
    for (int i = 0; i < 4; i++)
        m_clc[i] = 0;
}

NCO::~NCO ()
{
    if (NCO1src)
    {
        delete NCO1src;
        NCO1src = 0;
    }
}

// Process change in nconcon register
void NCO::update_ncocon (uint diff)
{
    uint value = nco1con.value.get ();
    Dprintf (("NCO::update_ncocon diff=0x%x value=0x%x\n", diff, value));
    if ((diff & NxEN) && (value & NxEN))        //NCO turning on
    {
        Dprintf (("NCO::update_ncocon ON nco1con=0x%x nco1clk=0x%x\n", value,
                  nco1clk.value.get ()));
        pulseWidth = 0;
        /*if (!nco_interface)
        {
            nco_interface = new NCO_Interface (this);
            get_interface ().prepend_interface (nco_interface);
        }*/
        if (value & NxOE)
            oeNCO1 (true);

        // force clock to current state
        update_ncoclk (NxCLKS_mask);
    }
    else if ((diff & NxEN) && !(value & NxEN))        //NCO turning off
    {
        Dprintf (("NCO::update_ncocon OFF nco1con=0x%x nco1clk=0x%x acc=0x%x\n", value,
                  nco1clk.value.get (), acc));
        pulseWidth = 0;
        oeNCO1 (false);
        current_value ();
        if (future_cycle)
        {
            get_cycles ().clear_break (future_cycle);
            future_cycle = 0;
        }
        if (acc >= (1 << 20))
                acc -= (1 << 20);
    }
    else if (value & NxEN)        // currently running
    {
        if ((diff & NxOE))
            oeNCO1 (value & NxOE);

        if (diff & NxPOL)
            outputNCO1 (value & NxOUT);
    }
}

/* this does a manual update of the accumulator register
   and is used when the LCx_out or NCO1CLK are used as the
   clock source
*/
void NCO::NCOincrement ()
{
    uint value;

    // Load nco1inc on second + clock edge
    if (inc_load && !--inc_load)
    {
        set_inc_buf ();
        Dprintf (("NCO::NCOincrement() loading inc with 0x%x\n", inc));
    }
    // Turn off output if pulsewidth goes to zero
    if (pulseWidth && !--pulseWidth)
    {
        nco1con.value.put (nco1con.value.get () & ~NxOUT);
        outputNCO1 (false);
    }

    // Overflow was on last edge
    if (NCOoverflow)
    {
        value = nco1con.value.get ();
        if (!(value & NxPFM))        // Fixed duty cycle
            value = (value & NxOUT) ? value & ~NxOUT : value | NxOUT;
        else
        {
            pulseWidth = 1 << ((nco1clk.value.get () & NxPW_mask) >> 5);
            value = value | NxOUT;
        }
        nco1con.value.put (value);
        NCOoverflow = false;
        outputNCO1 (value & NxOUT);
        Dprintf(("m_NCOif=%p pir=%p\n", m_NCOif,pir));
        if (m_NCOif)
            m_NCOif->Trigger ();
        else if (pir)
            pir->set_nco1if ();
        else
            fprintf (stderr, "NCO interrupt method not configured\n");
    }
    acc += inc;
    Dprintf (("NCO::NCOincrement() acc=0x%x inc=0x%x\n", acc, inc));
    if (acc >= (1 << 20))        // overflow
    {
        Dprintf (("NCO::NCOincrement() acc overflow acc=0x%x\n", acc));
        acc -= (1 << 20);
        NCOoverflow = true;
    }
}

void NCO::callback ()
{

    current_value ();
    future_cycle = 0;
    uint value = nco1con.value.get ();

    if (acc >= (1 << 20))
    {
        acc -= (1 << 20);
        uint value = nco1con.value.get ();
        if (!(value & NxPFM))        // Fixed duty cycle
        {
            value = (value & NxOUT) ? value & ~NxOUT : value | NxOUT;
            Dprintf (("call simulate_clock\n"));
            simulate_clock (true);
        }
        else
        {
            uint cps = cpu->get_ClockCycles_per_Instruction ();
            pulseWidth = 1 << ((nco1clk.value.get () & NxPW_mask) >> 5);
            Dprintf (("NCO::callback raw pulseWidth=%u ", pulseWidth));
            value = value | NxOUT;
            if (clock_src () == HFINTOSC)
                pulseWidth *= cpu->get_frequency () / (16e6);
            int rem = pulseWidth % cps;
            pulseWidth /= cps;
            if (!pulseWidth || rem)
                pulseWidth++;
            Dprintf (("pulseWidth=%u rem = %d value=0x%x\n", pulseWidth, rem,
                      value));
            last_cycle = get_cycles ().get ();
            future_cycle = last_cycle + pulseWidth;
            get_cycles ().set_break (future_cycle, this);
        }
        nco1con.value.put (value);
        outputNCO1 (value & NxOUT);
        Dprintf(("m_NCOif=%p pir=%p\n", m_NCOif,pir));
        if (m_NCOif)
            m_NCOif->Trigger ();
        else if (pir)
            pir->set_nco1if ();
        else
            fprintf (stderr, "NCO interrupt method not configured\n");
    }
    else if (pulseWidth)
    {
        value &= ~NxOUT;
        nco1con.value.put (value);
        outputNCO1 (value & NxOUT);

        Dprintf (("call simulate_clock\n"));
        simulate_clock (true);
    }
    else
    {
        Dprintf (("call simulate_clock\n"));
        simulate_clock (true);
    }
}

// Use callback to simulate NCO driven by internal clock
void NCO::simulate_clock (bool on)
{
    Dprintf(("on=%d inc=%u clock=%s\n", on, inc, clock_src() ? "Fosc" : "HFINTOSC"));
    if (on && inc)
    {
        int64_t delta;
        uint cps = cpu->get_ClockCycles_per_Instruction ();
        uint rem = 0;

        if (future_cycle)
        {
            current_value ();
            get_cycles ().clear_break (future_cycle);
        }
        delta = ((1 << 20) - acc) / inc;
        if (delta <= 0)
            delta = 1;
        else
            rem = ((1 << 20) - acc) % inc;
        if (rem)
            delta++;

        if (clock_src () == HFINTOSC)
        {
            delta *= cpu->get_frequency() / (16e6);
            Dprintf(("delta=%ld cpu=%.3fMHz HFINTOC=%.3fMHz\n", delta, cpu->get_frequency()/1e6, 16e6/1e6));
        }

        rem = delta % cps;        // if rem != 0 timing is approximate
        delta /= cps;
        if ((delta <= 0) || rem > 0)
            delta++;
        Dprintf (("NCO::simulate_clock clock=%.2e acc=0x%x delta = %"
                  PRINTF_GINT64_MODIFIER "d rem=%u\n",
                  (clock_src () == HFINTOSC) ? 16e6 : cpu->get_frequency (),
                  acc, delta, rem));


        last_cycle = get_cycles ().get ();
        future_cycle = last_cycle + delta;
        get_cycles ().set_break (future_cycle, this);
    }
    else                        // clock off
    {
        current_value ();
        if (future_cycle)
        {
            current_value ();
            get_cycles ().clear_break (future_cycle);
            future_cycle = 0;
        }

    }
}

// Set output value for output pin
void NCO::outputNCO1 (bool level)
{
    level = (nco1con.value.get () & NxPOL) ? !level : level;
    Dprintf (("NCO::outputNCO1 level=%d\n", level));
    for (int i = 0; i < 4; i++)
        if (m_clc[i])
            m_clc[i]->NCO_out (level);

    if (m_cwg)
        m_cwg->out_NCO(level);

    if (NCO1src)
    {
        NCO1src->setState (level ? '1' : '0');
        pinNCO1->updatePinModule ();
    }
}

// Enable/disable output pin
void NCO::oeNCO1 (bool on)
{
    if (on)
    {
        if (!srcNCO1active)
        {
            if (!NCO1src) NCO1src = new NCOSigSource (this, pinNCO1);
            
            pinNCO1->setSource (NCO1src);
            srcNCO1active = true;
            NCO1src->setState ((nco1con.value.get () & NxOUT) ? '1' : '0');
            pinNCO1->updatePinModule ();
        }
    }
    else if (srcNCO1active)
    {
        pinNCO1->setSource (0);
        srcNCO1active = false;
        pinNCO1->updatePinModule ();
    }
}

void NCO::enableCLKpin (bool on)
{
    if (on)
    {
        if (!CLKsink) CLKsink = new ncoCLKSignalSink (this);
        
        pinNCOclk->addSink (CLKsink);
        CLKstate = pinNCOclk->getPin ().getState ();
    }
    else
    {
        if (CLKsink) pinNCOclk->removeSink (CLKsink);
    }
}

// new value for NCO1CLK register
void NCO::update_ncoclk (uint diff)
{
    Dprintf(("nco1con=0x%x diff=0x%x\n", nco1con.value.get(), diff));
    if ((nco1con.value.get () & NxEN) && (diff & NxCLKS_mask))
    {
        enableCLKpin (false);
        if (future_cycle)
        {
            simulate_clock (false);
        }
        Dprintf(("clk=%d\n", clock_src()));
        switch (clock_src ())
        {
        case HFINTOSC:
            simulate_clock (true);
            break;

        case FOSC:
            simulate_clock (true);        //FIXME FOSC different HFINTOSC
            break;

        case LC1_OUT:
            break;

        case NCO1CLK:
            enableCLKpin (true);
            break;
        }
    }
}

// return pseudo clock codes (this for 16f1503)
int NCO::clock_src ()
{
    switch (nco1clk.value.get () & NxCLKS_mask)
    {
    case 0:                        //HFINTOSC
        return HFINTOSC;
        break;

    case 1:                        //FOSC
        return FOSC;
        break;

    case 2:                        // LC1_OUT
        return LC1_OUT;
        break;

    case 3:                        // NCO1CLK pin
        return NCO1CLK;
        break;
    }
    return -1;
}

void NCO::setIOpins (PinModule * pIN, PinModule * pOUT)
{
    pinNCOclk = pIN;
    pinNCO1 = pOUT;
}

void NCO::setIOpin(int data, PinModule *pin)
{
    if (data == NCOout_PIN) setNCOxPin(pin);
    else fprintf(stderr, "NCO::setIOpin unexpected data=%d\n", data);
}

// remap NCO1 pin, called from APFCON
void NCO::setNCOxPin (PinModule * pNCOx)
{
    if (pNCOx == pinNCO1)
        return;
    if (srcNCO1active)                // old pin active disconnect
    {
        oeNCO1 (false);
        if (NCO1src)
            delete NCO1src;
        NCO1src = 0;
    }
    pinNCO1 = pNCOx;
    if (nco1con.value.get () & NxOE)
        oeNCO1 (true);
}

// link from Configurable Logic Cell
void NCO::link_nco (bool level, char index)
{
    // Active?
    if (clock_src () == LC1_OUT)
    {
        Dprintf (("NCO::link_nco level=%d index=%d edge=%d\n", level, index,
                  (bool) (level & !CLKstate)));
        if (level & !CLKstate)        // new edge
            NCOincrement ();
        CLKstate = level;
    }
}

// Save acc buffer into accx registers,
// but if clock is simulated, first compute value of acc buffer.
void NCO::current_value ()
{
    if (future_cycle && (get_cycles ().get () - last_cycle))
    {
        uint cps = cpu->get_ClockCycles_per_Instruction ();
        uint32_t delta_acc = (get_cycles ().get () - last_cycle) * inc * cps;
        if (clock_src () == HFINTOSC)
            delta_acc *= 16e6 / cpu->get_frequency ();
        acc += delta_acc;
        last_cycle = get_cycles ().get ();
    }
    nco1accu.value.put ((acc >> 16) & 0x0f);
    nco1acch.value.put ((acc >> 8) & 0xff);
    nco1accl.value.put (acc & 0xff);
}

// transfer accx registers to acc buffer
void NCO::set_acc_buf ()
{
    acc = ((acc_hold[2] & 0x0f) << 16) | (acc_hold[1] << 8) | acc_hold[0];

    NCOoverflow = false;

    if ((clock_src () == FOSC || clock_src () == HFINTOSC) &&
        (nco1con.value.get () & NxEN))
    {
        set_inc_buf ();
        Dprintf (("call simulate_clock\n"));
        simulate_clock (true);
    }
}

/*
   Documentation indicates the increment buffer is loaded
   on second rising edge of the source clock;
*/
void NCO::newINCL ()
{
    Dprintf(("newINCL clock=%d\n", clock_src()));
    // If NCO is not enables, inc buffer loaded immediately
    if (!(nco1con.value.get () & NxEN))
        set_inc_buf ();
    // If simulating clock, load will be too early or late,
    // so do it now (to simplify code)
    else if (clock_src () == FOSC || clock_src () == HFINTOSC)
    {
        current_value ();
        set_inc_buf ();
        Dprintf (("call simulate_clock\n"));
        simulate_clock (true);
    }
    else
        inc_load = 2;
}

// load inc buffer from registers
void NCO::set_inc_buf ()
{
    inc = (nco1inch.value.get () << 8) | nco1incl.value.get ();
    Dprintf (("NCO::set_inc_buf inc=0x%x\n", inc));
}

// process input from clock pin
void NCO::setState (char new3State)
{
    if (clock_src () == NCO1CLK)
    {
        if (new3State == '1' && !CLKstate)        //new edge
        {
            CLKstate = true;
            NCOincrement ();
        }
        else if (new3State == '0' && CLKstate)
        {
            CLKstate = false;
        }
    }
}

void NCO::sleep(bool on)
{
    if (clock_src() == FOSC)
    {
        // pause FOSC on sleep, restart on wakeup
        simulate_clock(!on);
    }
}

void NCO::releasePinSource (PinModule * pin)
{
    if (pin)
    {

        if (pin == pinNCO1)
            srcNCO1active = false;
    }
}


NCOxCON::NCOxCON (NCO * pt, Processor * pCpu, const char *pName,
                  const char *pDesc):
        sfr_register (pCpu, pName, pDesc), con_mask (0xd1), pt_nco (pt)
{
}

void NCOxCON::put (uint new_value)
{
    new_value &= con_mask;
    uint diff = new_value ^ value.get ();
    if (!diff) return;

    value.put (new_value);
    pt_nco->update_ncocon (diff);
}
// make sure acc reset after con
void NCOxCON::reset(RESET_TYPE r)
{
    putRV(por_value);
    pt_nco->nco1accu.reset(r);
    pt_nco->nco1acch.reset(r);
    pt_nco->nco1accl.reset(r);
}


NCOxCLK::NCOxCLK (NCO * pt, Processor * pCpu, const char *pName,
                const char *pDesc):
        sfr_register (pCpu, pName, pDesc), clk_mask (0xe3), pt_nco (pt)
{
}

void NCOxCLK::put (uint new_value)
{
    new_value &= clk_mask;
    uint diff = new_value ^ value.get ();
    if (!diff) return;

    value.put (new_value);
    pt_nco->update_ncoclk (diff);
}

NCOxACCH::NCOxACCH (NCO * pt, Processor * pCpu, const char *pName,
                    const char *pDesc):
        sfr_register (pCpu, pName, pDesc), pt_nco (pt)
{
}

void NCOxACCH::put (uint new_value)
{
    uint diff = new_value ^ value.get ();

    pt_nco->set_hold_acc (new_value, 1);
    pt_nco->set_accFlag (true);
    if (!diff) return;

    value.put (new_value);
}

NCOxACCL::NCOxACCL (NCO * pt, Processor * pCpu, const char *pName,
                    const char *pDesc):
        sfr_register (pCpu, pName, pDesc), pt_nco (pt)
{
}

void NCOxACCL::put (uint new_value)
{
    uint diff = new_value ^ value.get ();
    pt_nco->set_hold_acc (new_value, 0);
    pt_nco->set_accFlag (true);
    
    if (diff) value.put (new_value);

    if (pt_nco->get_accFlag ())
    {
        pt_nco->set_acc_buf ();
        pt_nco->set_accFlag (false);
    }
}

NCOxACCU::NCOxACCU (NCO * pt, Processor * pCpu, const char *pName,
                    const char *pDesc):
        sfr_register (pCpu, pName, pDesc), pt_nco (pt)
{
}

void NCOxACCU::put (uint new_value)
{
    uint diff = new_value ^ value.get ();
    pt_nco->set_hold_acc (new_value, 2);
    pt_nco->set_accFlag (true);
    
    if (!diff) return;

    value.put (new_value);
}

NCOxINCH::NCOxINCH (NCO * pt, Processor * pCpu, const char *pName,
                const char *pDesc):
        sfr_register (pCpu, pName, pDesc), pt_nco (pt)
{
}

void NCOxINCH::put (uint new_value)
{
    uint diff = new_value ^ value.get ();
    if (!diff) return;

    value.put (new_value);
}

NCOxINCL::NCOxINCL (NCO * pt, Processor * pCpu, const char *pName,
                const char *pDesc):
        sfr_register (pCpu, pName, pDesc), pt_nco (pt)
{
}

void NCOxINCL::put (uint new_value)
{
    value.put (new_value);
    pt_nco->newINCL ();
}

NCO2::NCO2 (Processor * pCpu):NCO (pCpu)
{
}

// return pseudo clock codes (this for 10f320)
int NCO2::clock_src ()
{
    switch (nco1clk.value.get () & NxCLKS_mask)
    {
    case 2:                        //HFINTOSC
        return HFINTOSC;
        break;

    case 1:                        //FOSC
        return FOSC;
        break;

    case 3:                        // LC1_OUT
        return LC1_OUT;
        break;

    case 0:                        // NCO1CLK pin
        return NCO1CLK;
        break;
    }
    return -1;
}

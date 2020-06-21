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

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

#include "pic-processor.h"
#include "clc.h"
#include "nco.h"

  // Report state changes on incoming INx pins
class INxSignalSink:public SignalSink
{
  public:
    INxSignalSink (CLC * _clc, int _index):m_clc (_clc), index (_index)
    {
    }

    virtual void setSinkState (char new3State)
    {
        m_clc->setState (new3State, index);
    }
    virtual void release ()
    {
        delete this;
    }
  private:
    CLC * m_clc;
    int index;
};

class CLCSigSource:public SignalControl
{
  public:
    CLCSigSource (CLC * _clc, PinModule * _pin):m_clc (_clc),
        m_state ('?'), m_pin (_pin)
    {
        assert (m_clc);
    }
    virtual ~ CLCSigSource ()
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
        m_clc->releasePinSource (m_pin);
    }

  private:
    CLC * m_clc;
    char m_state;
    PinModule *m_pin;
};


void CLCxCON::put (uint new_value)
{
    new_value &= write_mask;
    new_value |= (value.get () & ~write_mask);
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff)
        return;
    m_clc->update_clccon (diff);
}

void CLCxPOL::put (uint new_value)
{
    new_value &= write_mask;
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff)
        return;
    m_clc->compute_gates ();
}

CLCxSEL0::CLCxSEL0 (CLC * _clc, Processor * pCpu, const char *pName,
        const char *pDesc):
        sfr_register (pCpu, pName, pDesc), m_clc (_clc), write_mask (0x77)
{
    m_clc->D1S (0);
    m_clc->D2S (0);
}

void CLCxSEL0::put (uint new_value)
{
    new_value &= write_mask;

    uint diff = new_value ^ value.get ();
    value.put (new_value);
    if (diff & 0xf)
        m_clc->D1S (new_value & 0xf);
    if (diff & 0xf0)
        m_clc->D2S ((new_value & 0xf0) >> 4);
    if (diff && m_clc->CLCenabled ())
        m_clc->config_inputs (true);
}

CLCxSEL1::CLCxSEL1 (CLC * _clc, Processor * pCpu, const char *pName,
        const char *pDesc):
        sfr_register (pCpu, pName, pDesc), m_clc (_clc), write_mask (0x77)
{
    m_clc->D3S (0);
    m_clc->D4S (0);
}

void CLCxSEL1::put (uint new_value)
{
    new_value &= write_mask;

    uint diff = new_value ^ value.get ();
    value.put (new_value);
    if (diff & 0xf)
        m_clc->D3S (new_value & 0xf);
    if (diff & 0xf0)
        m_clc->D4S ((new_value & 0xf0) >> 4);
    if (diff && m_clc->CLCenabled ())
        m_clc->config_inputs (true);
}

void CLCxGLS0::put (uint new_value)
{
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff)
        return;
    if (m_clc->CLCenabled ())
        m_clc->config_inputs (true);
    m_clc->compute_gates ();
}

void CLCxGLS1::put (uint new_value)
{
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff)
        return;
    if (m_clc->CLCenabled ())
        m_clc->config_inputs (true);
    m_clc->compute_gates ();
}

void CLCxGLS2::put (uint new_value)
{
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff) return;
    if (m_clc->CLCenabled ())
        m_clc->config_inputs (true);
    m_clc->compute_gates ();
}

void CLCxGLS3::put (uint new_value)
{
    uint diff = new_value ^ value.get ();

    value.put (new_value);
    if (!diff) return;
    
    if (m_clc->CLCenabled ())
        m_clc->config_inputs (true);
    m_clc->compute_gates ();
}

// CLCx calls to set it's LCx_OUT bit, result shared with
// all CLCx instances.
void CLCDATA::set_bit (bool bit_val, uint pos)
{
    Dprintf (("set_bit LC%u_OUT %d\n", pos + 1, bit_val));

    if (bit_val) value.put (value.get () | (1 << pos));
    else         value.put (value.get () & ~(1 << pos));

    for (int i = 0; i < 4; i++)
        if (m_clc[i]) m_clc[i]->lcxupdate (bit_val, pos);;
}


CLC::CLC (Processor * cpu, uint _index, CLCDATA * _clcdata):
    index (_index),
    clcxcon (this, cpu, "clcxcon", "Configurable Logic Cell Control Register"),
    clcxpol (this, cpu, "clcxpol", "Configurable Logic Cell Signal Polarity"),
    clcxsel0 (this, cpu, "clcxsel0", "Multiplexer Data 1 and 2 Select Register"),
    clcxsel1 (this, cpu, "clcxsel1", "Multiplexer Data 3 and 4 Select Register"),
    clcxgls0 (this, cpu, "clcxgls0", "Gate 1 Logic Select Register"),
    clcxgls1 (this, cpu, "clcxgls1", "Gate 2 Logic Select Register"),
    clcxgls2 (this, cpu, "clcxgls2", "Gate 3 Logic Select Register"),
    clcxgls3 (this, cpu, "clcxgls3", "Gate 4 Logic Select Register"),
    clcdata (_clcdata),
    p_nco (0), pinCLCx (0), CLCxsrc (0), srcCLCxactive (false),
    frc_level (false), NCO_level (false), m_Interrupt (0), Doutput (false),
    Dclock (false), FRCactive (false), LFINTOSCactive (false),
    HFINTOSCactive (false)
{
    for (int i = 0; i < 2; i++)
      {
          INxsink[i] = 0;
          INxactive[i] = 0;
          INxstate[i] = false;
      }
    for (int i = 0; i < 4; i++)
      {
          CMxOUT_level[i] = false;
          pwmx_level[i] = false;
          lcxdT[i] = false;
          lcxg[i] = false;
      }
}

CLC::~CLC ()
{
    if (CLCxsrc)
    {
        delete CLCxsrc;
        CLCxsrc = 0;
    }
}

void CLC::setIOpin(int data, PinModule *pin)
{
    if (data == CLCout_PIN) setCLCxPin(pin);
    else fprintf(stderr, "CLC::setIOpin data=%d not supported\n", data);
}

// Handle output pin multiplexing
void CLC::setCLCxPin (PinModule * alt_pin)
{
    if (alt_pin != pinCLCx)
    {
        oeCLCx (false);
        pinCLCx = alt_pin;
        oeCLCx (true);
    }
}

void CLC::D1S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[0] = CLCxIN0;
          break;

      case 1:
          DxS_data[0] = CLCxIN1;
          break;

      case 2:
          DxS_data[0] = C1OUT;
          break;

      case 3:
          DxS_data[0] = C2OUT;
          break;

      case 4:
          DxS_data[0] = FOSCLK;
          break;

      case 5:
          DxS_data[0] = T0_OVER;
          break;

      case 6:
          DxS_data[0] = T1_OVER;
          break;

      case 7:
          DxS_data[0] = T2_MATCH;
          break;
      }
}

void CLC::D2S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[1] = FOSCLK;
          break;

      case 1:
          DxS_data[1] = T0_OVER;
          break;

      case 2:
          DxS_data[1] = T1_OVER;
          break;

      case 3:
          DxS_data[1] = T2_MATCH;
          break;

      case 4:
          DxS_data[1] = LC1;
          break;

      case 5:
          DxS_data[1] = LC2;
          break;

      case 6:
          DxS_data[1] = UNUSED;
          break;

      case 7:
          DxS_data[1] = UNUSED;
          break;

      }
}

void CLC::D3S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[2] = LC1;
          break;

      case 1:
          DxS_data[2] = LC2;
          break;

      case 2:
          DxS_data[2] = UNUSED;
          break;

      case 3:
          DxS_data[2] = UNUSED;
          break;

      case 4:
          DxS_data[2] = (index) ? LFINTOSC : NCOx;
          break;

      case 5:
          DxS_data[2] = (index) ? FRC_IN : HFINTOSC;
          break;

      case 6:
          DxS_data[2] = (index) ? PWM1 : PWM3;
          break;

      case 7:
          DxS_data[2] = (index) ? PWM2 : PWM4;
          break;
      }
}

void CLC::D4S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[3] = (index) ? LFINTOSC : NCOx;
          break;

      case 1:
          DxS_data[3] = (index) ? FRC_IN : HFINTOSC;
          break;

      case 2:
          DxS_data[3] = (index) ? PWM1 : PWM3;
          break;

      case 3:
          DxS_data[3] = (index) ? PWM2 : PWM4;
          break;

      case 4:
          DxS_data[3] = CLCxIN0;
          break;

      case 5:
          DxS_data[3] = CLCxIN1;
          break;

      case 6:
          DxS_data[3] = C1OUT;
          break;

      case 7:
          DxS_data[3] = C2OUT;
          break;

      }
}

// Handle T0 overflow notification
void CLC::t0_overflow ()
{
    bool gate_change = false;
    for (int i = 0; i < 4; i++)
      {
          if (DxS_data[i] == T0_OVER)
            {
                lcxdT[i] = true;
                gate_change = true;
            }
      }
    if (gate_change)
      {
          Dprintf (("CLC%u t0_overflow() enable=%d\n", index + 1,
                    CLCenabled ()));
          compute_gates ();
          for (int i = 0; i < 4; i++)
            {
                if (DxS_data[i] == T0_OVER)
                    lcxdT[i] = false;
            }
          compute_gates ();
      }
}

// Handle T1 overflow notification
void CLC::t1_overflow ()
{
    bool gate_change = false;
    for (int i = 0; i < 4; i++)
      {
          if (DxS_data[i] == T1_OVER)
            {
                lcxdT[i] = true;
                gate_change = true;
            }
      }
    if (gate_change)
      {
          Dprintf (("CLC%u t1_overflow() enable=%d\n", index + 1,
                    CLCenabled ()));
          compute_gates ();
          for (int i = 0; i < 4; i++)
            {
                if (DxS_data[i] == T1_OVER)
                    lcxdT[i] = false;
            }
          compute_gates ();
      }
}

// Handle T2 match notification
void CLC::t2_match ()
{
    bool gate_change = false;
    for (int i = 0; i < 4; i++)
      {
          if (DxS_data[i] == T2_MATCH)
            {
                lcxdT[i] = true;
                gate_change = true;
            }
      }
    if (gate_change)
      {
          Dprintf (("CLC%u t2_match() enable=%d\n", index + 1, CLCenabled ()));
          compute_gates ();
          for (int i = 0; i < 4; i++)
            {
                if (DxS_data[i] == T2_MATCH)
                    lcxdT[i] = false;
            }
          compute_gates ();
      }
}

// Handle updates for frc or lfintosc
void CLC::osc_out (bool level, int kind)
{
    bool gate_change = false;

    for (int i = 0; i < 4; i++)
      {
          if (DxS_data[i] == kind && lcxdT[i] != level)
            {
                lcxdT[i] = level;
                gate_change = true;
            }
      }
    if (gate_change)
      {
          Dprintf (("CLC%u osc_out() kind=%d level=%d enable=%d\n", index + 1,
                    kind, level, CLCenabled ()));
          compute_gates ();
      }
}

// Handle updates for NCO module
void CLC::NCO_out (bool level)
{
    if (NCO_level != level)
      {
          bool gate_change = false;
          NCO_level = level;

          for (int i = 0; i < 4; i++)
            {
                if (DxS_data[i] == NCOx)
                  {
                      lcxdT[i] = level;
                      gate_change = true;
                  }
            }
          if (gate_change)
            {
                Dprintf (("CLC%u NCO_out() level=%d enable=%d\n", index + 1,
                          level, CLCenabled ()));
                compute_gates ();
            }
      }
}

// Handle updates from comparator module
void CLC::CxOUT_sync (bool level, int cm)
{
    if (CMxOUT_level[cm] != level)
      {
          bool gate_change = false;
          CMxOUT_level[cm] = level;
          for (int i = 0; i < 4; i++)
            {
                if ((DxS_data[i] == C1OUT && cm == 0) ||
                    (DxS_data[i] == C2OUT && cm == 1))
                  {
                      lcxdT[i] = level;
                      gate_change = true;
                  }
            }

          if (gate_change)
            {
                Dprintf (("CLC%u C%dOUT_sync() level=%d enable=%d\n", index + 1,
                          cm + 1, level, CLCenabled ()));
                compute_gates ();
            }
      }
}

// Handle updates from pwm module
void CLC::out_pwm (bool level, int id)
{
    Dprintf (("CLC%u out_pwm() pwm%d level=%d enable=%d\n", index + 1, id + 1,
              level, CLCenabled ()));
    if (pwmx_level[id] != level)
      {
          bool gate_change = false;
          pwmx_level[id] = level;
          for (int i = 0; i < 4; i++)
            {
                if ((DxS_data[i] == PWM1 && id == 0) ||
                    (DxS_data[i] == PWM2 && id == 1) ||
                    (DxS_data[i] == PWM3 && id == 2) ||
                    (DxS_data[i] == PWM4 && id == 3))
                  {
                      lcxdT[i] = level;
                      gate_change = true;
                  }
            }
          if (gate_change)
            {
                Dprintf (("CLC%u out_pwm() pwm%d level=%d enable=%d\n",
                          index + 1, id + 1, level, CLCenabled ()));
                compute_gates ();
            }
      }
}

// notification on CLCxIN[12]
void CLC::setState (char new3State, int id)
{
    bool state = (new3State == '1' || new3State == 'W');
    if (state != INxstate[id])
      {
          bool gate_change = false;
          INxstate[id] = state;
          for (int i = 0; i < 4; i++)
            {
                if ((DxS_data[i] == CLCxIN0 && id == 0) ||
                    (DxS_data[i] == CLCxIN1 && id == 1))
                  {
                      lcxdT[i] = state;
                      gate_change = true;
                  }
            }

          if (gate_change)
            {
                Dprintf (("CLC%u setState() IN%d level=%d enable=%d\n",
                          index + 1, id, state, CLCenabled ()));
                compute_gates ();
            }
      }
}

// Enable/Disable input pin i
void CLC::enableINxpin (int i, bool on)
{
    if (on)
    {
        if (!INxactive[i])
        {
            if( !INxsink[i]) INxsink[i] = new INxSignalSink (this, i);
            
            pinCLCxIN[i]->addSink (INxsink[i]);
            setState (pinCLCxIN[i]->getPin ().getState ()? '1' : '0', i);
        }
        INxactive[i]++;
    }
    else if (!--INxactive[i])
    {
        if (INxsink[i]) pinCLCxIN[i]->removeSink (INxsink[i]);
    }
}

// Enable/disable output pin
void CLC::oeCLCx (bool on)
{
    if (on)
      {
          if (!srcCLCxactive)
            {
                if (!CLCxsrc) CLCxsrc = new CLCSigSource (this, pinCLCx);
                pinCLCx->setSource (CLCxsrc);
                srcCLCxactive = true;
                CLCxsrc->setState ((clcxcon.value.get () & LCxOE) ? '1' : '0');
                pinCLCx->updatePinModule ();
            }
      }
    else if (srcCLCxactive)
    {
        pinCLCx->setSource (0);
        if (CLCxsrc)
        {
            delete CLCxsrc;
            CLCxsrc = 0;
        }
        srcCLCxactive = false;
        pinCLCx->updatePinModule ();
    }
}

// Update the output value of each of the 4 Data Gates
// taking into account both input and output polarity
void CLC::compute_gates ()
{
    bool gate_out;
    uint glsx[] =
        { clcxgls0.value.get (), clcxgls1.value.get (),
          clcxgls2.value.get (), clcxgls3.value.get () };
    int mask;
    uint pol = clcxpol.value.get ();

    for (int j = 0; j < 4; j++)
      {
          gate_out = false;
          mask = 1;
          for (int i = 0; i < 4; i++)
            {
                if (glsx[j] & mask)
                  {
                      gate_out = !lcxdT[i];
                  }
                mask <<= 1;
                if (glsx[j] & mask)
                  {
                      gate_out = lcxdT[i];
                  }
                mask <<= 1;
            }
          gate_out = (pol & (1 << j)) ? !gate_out : gate_out;
          lcxg[j] = gate_out;
      }
    if (CLCenabled ())
        Dprintf (("CLC::compute_gates CLC%u lcxdT = {%d %d %d %d} lcxg={%d %d %d %d}\n", index + 1, lcxdT[0], lcxdT[1], lcxdT[2], lcxdT[3], lcxg[0], lcxg[1], lcxg[2], lcxg[3]));
    cell_function ();
}

// Select and execute cell functions
void CLC::cell_function ()
{
    bool out = false;
    uint con = clcxcon.value.get ();
    uint pol = clcxpol.value.get ();
    switch (con & 0x7)
      {
      case 0:                        // AND-OR
          out = (lcxg[0] && lcxg[1]) || (lcxg[2] && lcxg[3]);
          break;

      case 1:                        // OR-XOR
          out = (lcxg[0] || lcxg[1]) ^ (lcxg[2] || lcxg[3]);
          break;

      case 2:                        // 4 input AND
          out = lcxg[0] && lcxg[1] && lcxg[2] && lcxg[3];
          break;

      case 3:
          out = cell_sr_latch ();
          break;

      case 4:
          out = cell_1_in_flipflop ();
          break;

      case 5:
          out = cell_2_in_flipflop ();
          break;

      case 6:
          out = JKflipflop ();
          break;

      case 7:
          out = transparent_D_latch ();
          break;

      }
    if (pol & LCxPOL)
        out = !out;

    if (CLCenabled ())
        outputCLC (out);

}

// Send output to required consumers
void CLC::outputCLC (bool out)
{
    uint con = clcxcon.value.get ();
    bool old_out = con & LCxOUT;

    Dprintf (("outputCLC CLC%u out=%d old_out=%d clcdata=0x%x\n", index, out,
              old_out, clcdata->value.get ()));

    if (out)
        con |= LCxOUT;
    else
        con &= ~LCxOUT;

    clcxcon.value.put (con);

    assert (m_Interrupt);
    Dprintf (("CLC::outputCLC CLC%u old_out %d out %d int 0x%x \n", index + 1,
              old_out, out, con & LCxINTP));
    if (!old_out && out && (con & LCxINTP))        //Positive edge interrupt
        m_Interrupt->Trigger ();
    if (old_out && !out && (con & LCxINTN))        //Negative edge interrupt
        m_Interrupt->Trigger ();

    assert (clcdata);
    clcdata->set_bit (out, index);
    if (p_nco)
        p_nco->link_nco (out, index);

    if (CLCenabled ())
      {
          CLCxsrc->setState (out ? '1' : '0');
          pinCLCx->updatePinModule ();
      }
}



bool CLC::cell_sr_latch ()
{
    bool set = lcxg[0] || lcxg[1];
    bool reset = lcxg[2] || lcxg[3];

    if (set)
        Doutput = true;
    else if (reset)
        Doutput = false;

    return Doutput;
}

bool CLC::cell_1_in_flipflop ()
{
    bool set = lcxg[3];
    bool reset = lcxg[2];
    bool clock = lcxg[0];
    bool D = lcxg[1];

    if (set)
      {
          Doutput = true;
      }
    else if (reset)
      {
          Doutput = false;
      }
    else if (!Dclock && clock)
      {
          Doutput = D;
      }
    Dclock = clock;
    return Doutput;
}

bool CLC::cell_2_in_flipflop ()
{
    bool reset = lcxg[2];
    bool clock = lcxg[0];
    bool D = lcxg[1] || lcxg[3];

    if (reset)
      {
          Doutput = false;
      }
    else if (!Dclock && clock)
      {
          Doutput = D;
      }
    Dclock = clock;
    return Doutput;
}

bool CLC::JKflipflop ()
{
    bool J = lcxg[1];
    bool K = lcxg[3];
    bool reset = lcxg[2];
    bool clock = lcxg[0];

    if (reset)
      {
          Doutput = false;
      }
    else if (!Dclock && clock)        // Clock + edge
      {
          if (J && K)                // Toggle output
              Doutput = !Doutput;
          else if (J && !K)        // Set output
              Doutput = true;
          else if (!J && K)        // clear output
              Doutput = false;
          /*else if (!J && !K)        // no change
              ;*/
      }
    Dclock = clock;
    return Doutput;
}

bool CLC::transparent_D_latch ()
{
    bool reset = lcxg[0];
    bool D = lcxg[1];
    bool LE = lcxg[2];
    bool set = lcxg[3];

    if (set)
        Doutput = true;
    else if (reset)
        Doutput = false;
    else if (!LE)
        Doutput = D;

    return Doutput;
}



void CLC::releasePinSource (PinModule * pin)
{
    if (pin == pinCLCx)
        srcCLCxactive = false;
}

// Called from clcdata, process LCx_OUT updates where x = pos
void CLC::lcxupdate (bool bit_val, uint pos)
{
    bool update = false;
    for (int i = 0; i < 4; i++)
      {
          if ((lcxdT[i] != bit_val) &&
              ((DxS_data[i] == LC1 && pos == 0) ||
               (DxS_data[i] == LC2 && pos == 1) ||
               (DxS_data[i] == LC3 && pos == 2) ||
               (DxS_data[i] == LC4 && pos == 3)))
            {
                update = true;
                lcxdT[i] = bit_val;
            }
      }

    if (update)
      {
          if (CLCenabled ())
              Dprintf (("CLC%u lcxupdate LC%u_OUT=%d\n", index + 1, pos + 1,
                        bit_val));
          compute_gates ();
      }
}

// CLCCON register has changed
void CLC::update_clccon (uint diff)
{
    uint val = clcxcon.value.get ();
    if (diff & LCxOE)
      {
          if ((val & (LCxOE | LCxEN)) == (LCxOE | LCxEN))
              oeCLCx (true);
          if ((val & (LCxOE | LCxEN)) == (LCxEN))
              oeCLCx (false);
      }
    if (diff & LCxEN)                // clc off or on
      {
          if (val & LCxEN)        // CLC on
            {
                config_inputs (true);
            }
          else                        // CLC off
            {
                config_inputs (false);
                oeCLCx (false);
            }
      }

}

// Initialize inputs as required, called when CLC is enabled or disabled
// or changes in clcxselx clcxglsx while clc enabled
void CLC::config_inputs (bool on)
{
    uint active_gates = clcxgls0.value.get () |
        clcxgls1.value.get () | clcxgls2.value.get () | clcxgls3.value.get ();
    Dprintf (("config_inputs CLC%u on=%d active_gates=0x%x\n", index + 1, on,
              active_gates));

    bool haveIN0 = false;
    bool haveIN1 = false;
    bool haveFRC = false;
    bool haveLFINTOSC = false;
    bool haveHFINTOSC = false;
    int mask = 3;
    for (int i = 0; i < 4; i++)
      {
          if (active_gates & mask)        // data input used
            {
                if (DxS_data[i] == CLCxIN0)
                    haveIN0 = true;
                else if (DxS_data[i] == CLCxIN1)
                    haveIN1 = true;
                else if (DxS_data[i] == FRC_IN)
                    haveFRC = true;
                else if (DxS_data[i] == LFINTOSC)
                    haveLFINTOSC = true;
                else if (DxS_data[i] == HFINTOSC)
                    haveHFINTOSC = true;
            }
          mask <<= 2;
      }

    // If on==true and inactive, turn on
    // if on==false and active. turn off
    if (haveIN0 && (INxactive[0] ^ on))
      {
          Dprintf (("config_inputs CLC%u IN0 on=%d\n", index + 1, on));
          enableINxpin (0, on);
      }
    else if (!haveIN0 && INxactive[0])
      {
          Dprintf (("config_inputs CLC%u IN0 OFF on=%d\n", index + 1, on));
          enableINxpin (0, false);
      }
    if (haveIN1 && (INxactive[1] ^ on))
      {
          Dprintf (("config_inputs CLC%u IN1 on=%d\n", index + 1, on));
          enableINxpin (1, on);
      }
    else if (!haveIN1 && INxactive[1])
      {
          Dprintf (("config_inputs CLC%u IN1 OFF on=%d\n", index + 1, on));
          enableINxpin (0, false);
      }

    if (haveFRC && (FRCactive ^ on))
      {
          Dprintf (("config_inputs CLC%u FRC FRCactive=%d on=%d\n", index + 1,
                    FRCactive, on));
          FRCactive = on;
          frc->start_osc_sim (on);
      }
    else if (!haveFRC && FRCactive)
      {
          Dprintf (("config_inputs CLC%u FRC OFF  on=%d\n", index + 1, on));
          FRCactive = false;
          frc->start_osc_sim (false);
      }
    if (haveLFINTOSC && (LFINTOSCactive ^ on))
      {
          Dprintf (("config_inputs CLC%u LFINTOSC LFINTOSCactive=%d on=%d\n",
                    index + 1, LFINTOSCactive, on));
          LFINTOSCactive = on;
          lfintosc->start_osc_sim (on);
      }
    else if (!haveLFINTOSC && LFINTOSCactive)
      {
          Dprintf (("config_inputs CLC%u LFINTOSC OFF  on=%d\n", index + 1,
                    on));
          LFINTOSCactive = false;
          lfintosc->start_osc_sim (false);
      }
    if (haveHFINTOSC && (HFINTOSCactive ^ on))
      {
          Dprintf (("config_inputs CLC%u HFINTOSC HFINTOSCactive=%d on=%d\n",
                    index + 1, HFINTOSCactive, on));
          HFINTOSCactive = on;
          hfintosc->start_osc_sim (on);
      }
    else if (!haveHFINTOSC && HFINTOSCactive)
      {
          Dprintf (("config_inputs CLC%u HFINTOSC OFF  on=%d\n", index + 1,
                    on));
          HFINTOSCactive = false;
          hfintosc->start_osc_sim (false);
      }
    if (on)
        compute_gates ();
}


// Inputs for p10f32x
CLC1::CLC1 (Processor * cpu, uint _index, CLCDATA * _clcdata):
            CLC (cpu, _index, _clcdata)
{
}

void CLC1::D1S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[0] = LC1;
          break;

      case 1:
          DxS_data[0] = CLCxIN0;
          break;

      case 2:
          DxS_data[0] = CLCxIN1;
          break;

      case 3:
          DxS_data[0] = PWM1;
          break;

      case 4:
          DxS_data[0] = PWM2;
          break;

      case 5:
          DxS_data[0] = NCOx;
          break;

      case 6:
          DxS_data[0] = FOSCLK;
          break;

      case 7:
          DxS_data[0] = LFINTOSC;
          break;
      }
}

void CLC1::D2S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[1] = LC1;
          break;

      case 1:
          DxS_data[1] = CLCxIN0;
          break;

      case 2:
          DxS_data[1] = CLCxIN1;
          break;

      case 3:
          DxS_data[1] = PWM1;
          break;

      case 4:
          DxS_data[1] = PWM2;
          break;

      case 5:
          DxS_data[1] = NCOx;
          break;

      case 6:
          DxS_data[1] = FOSCLK;
          break;

      case 7:
          DxS_data[1] = LFINTOSC;
          break;
      }
}

void CLC1::D3S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[2] = LC1;
          break;

      case 1:
          DxS_data[2] = CLCxIN0;
          break;

      case 2:
          DxS_data[2] = CLCxIN1;
          break;

      case 3:
          DxS_data[2] = PWM1;
          break;

      case 4:
          DxS_data[2] = PWM2;
          break;

      case 5:
          DxS_data[2] = NCOx;
          break;

      case 6:
          DxS_data[2] = FOSCLK;
          break;

      case 7:
          DxS_data[2] = LFINTOSC;
          break;
      }
}

void CLC1::D4S (int select)
{
    switch (select)
      {
      case 0:
          DxS_data[3] = LC1;
          break;

      case 1:
          DxS_data[3] = CLCxIN0;
          break;

      case 2:
          DxS_data[3] = CLCxIN1;
          break;

      case 3:
          DxS_data[3] = PWM1;
          break;

      case 4:
          DxS_data[3] = PWM2;
          break;

      case 5:
          DxS_data[3] = NCOx;
          break;

      case 6:
          DxS_data[3] = FOSCLK;
          break;

      case 7:
          DxS_data[3] = LFINTOSC;
          break;
      }
}

// OSC_SIM simulates clock inputs for CLC.
// If the requested frequency > FOSC/4, OSC_SIM will generate pulses at
// a frequency of FOSC/4.
// If the requested frequency is not a whole fraction of FOSC/4, the
// simulated clock will have jitter to approximate the requested frequency.
// The duty cycle of the simulated frequency will only be 50% when
// the requested frequency is a whole fraction of FOSC/8.
OSC_SIM::OSC_SIM (double _freq, int _data_in):
        frequency (_freq), data_in (_data_in), active (0), future_cycle (0)
{
    for (int i = 0; i < 4; i++)
      {
          m_clc[i] = 0;
      }
}

void OSC_SIM::start_osc_sim (bool on)
{
    if (on)
      {
          Dprintf (("OSC_SIM::start_osc_sim freq=%.0f kHz active %d\n",
                    frequency / 1000., active));
          if (!active)
            {
                int cycles = get_cycles ().instruction_cps () / frequency + 0.5;

                if (cycles < 2)
                  {
                      fprintf (stderr,
                               "OSC_SIM  %.1f kHz not simulated at current CPU frequency\n",
                               frequency / 1000.);
                      fprintf (stderr, "Using pulses at %.1f kHz\n",
                               get_cycles ().instruction_cps () / 1000.);
                      cycles = 1;
                  }
                adjust_cycles =
                    frequency - get_cycles ().instruction_cps () / cycles;
                next_cycle = cycles / 2;
                level = true;
                for (int i = 0; i < 4; i++)
                  {
                      if (m_clc[i])
                          m_clc[i]->osc_out (level, data_in);
                  }
                if (future_cycle)
                    get_cycles ().clear_break (this);
                future_cycle = get_cycles ().get () + cycles - next_cycle;
                get_cycles ().set_break (future_cycle, this);
                Dprintf (("OSC_SIM::start_osc_sim cycles=%d adj_cycles=%ld freq=%.1f kHz inst_cps=%e\n", cycles, adjust_cycles, frequency / 1000., get_cycles ().instruction_cps ()));
            }
          active++;
      }
    else if (--active == 0)
      {
          Dprintf (("OSC_SIM::start_osc_sim stop freq=%.0f\n",
                    frequency / 1000.));
          if (future_cycle)
            {
                get_cycles ().clear_break (this);
                future_cycle = 0;
            }
      }
}

void OSC_SIM::callback ()
{
    for (int i = 0; i < 4; i++)
      {
          if (m_clc[i])
              m_clc[i]->osc_out (!level, data_in);
      }
    if (!next_cycle && level)        // Sending a pulse
      {
          for (int i = 0; i < 4; i++)
            {
                if (m_clc[i])
                    m_clc[i]->osc_out (level, data_in);
            }
      }
    if (next_cycle)
      {
          future_cycle = get_cycles ().get () + next_cycle;
          next_cycle = 0;
          level = false;
      }
    else
      {
          adjust_cycles += frequency;
          int cycles = get_cycles ().instruction_cps () / adjust_cycles + 0.5;
          if (cycles < 2)
            {
                cycles = 1;
                adjust_cycles = 0;
            }
          else
              adjust_cycles -= get_cycles ().instruction_cps () / cycles;
          next_cycle = cycles / 2;
          level = true;
          future_cycle = get_cycles ().get () + cycles - next_cycle;
      }
    get_cycles ().set_break (future_cycle, this);
}

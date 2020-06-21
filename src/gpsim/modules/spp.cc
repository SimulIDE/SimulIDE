/*
   Copyright (C) 2014 Roy R Rankin

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
#include <iostream>

#include "config.h"
#include "stimuli.h"
#include "spp.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

/*
 *         Streaming Parallel Port
 */
//--------------------------------------------------
//
//--------------------------------------------------
class SppSignalSource : public SignalControl
{
public:
  SppSignalSource()
  {
      state = '?';
  }
  ~SppSignalSource() { }
  virtual char getState()
  {
    return state;
  }
  void setState(char _state){ state = _state;}
  virtual void release()
  {
    delete this;
  }
private:
  char state;
};
//--------------------------------------------------
//
//--------------------------------------------------


SPPCON::SPPCON(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
}
void SPPCON::put(uint new_value)
{
  uint mask = (SPP::SPPOWN | SPP::SPPEN );
  uint old = value.data;

  value.data = (new_value & mask);
  if ((old ^ value.data) && value.data == mask)
        cout << "Warning USB functionality of SPP not supported\n";
  else
          spp->enabled(value.data & SPP::SPPEN);
}

void SPPCON::put_value(uint new_value)
{
  value.data = new_value;
}

SPPCFG::SPPCFG(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
}
void SPPCFG::put(uint new_value)
{
  value.data = new_value;
  if (spp) spp->cfg_write(value.data);
}

void SPPCFG::put_value(uint new_value)
{
  value.data = new_value;
}
SPPEPS::SPPEPS(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
}
void SPPEPS::put(uint new_value)
{
  uint mask = 0x0f;
  uint fixed = value.data & 0xd0;  // Read only part of register

  value.data = ((new_value & mask) | fixed);

  if (spp) spp->eps_write(value.data);
}

void SPPEPS::put_value(uint new_value)
{
  value.data = new_value;
  if (spp) spp->eps_write(new_value);
}
SPPDATA::SPPDATA(Processor *pCpu, const char *pName, const char *pDesc)
  : sfr_register(pCpu, pName, pDesc)
{
    spp = 0;
}
void SPPDATA::put(uint new_value)
{
  value.data = new_value;
  if (spp) spp->data_write(new_value);
}

void SPPDATA::put_value(uint new_value)
{
  value.data = new_value;
}
uint SPPDATA::get()
{
    value.data = spp->data_read();
    return(value.data);
}
void SPP::initialize( PIR_SET *_pir_set, PicPSP_PortRegister *_port_set,
        PicTrisRegister *_port_tris,
        SPPCON *_sppcon, SPPCFG *_sppcfg, SPPEPS *_sppeps, 
       SPPDATA *_sppdata, PinModule   *_clk1spp, PinModule  *_clk2spp, 
        PinModule  *_oespp, PinModule  *_csspp )
{
    pir_set = _pir_set;
    parallel_port = _port_set;
    parallel_tris = _port_tris;

    sppcon = _sppcon;
    sppcfg = _sppcfg;
    sppeps = _sppeps;
    sppdata= _sppdata;
    sppdata->set_spp(this);
    sppeps->set_spp(this);
    sppcfg->set_spp(this);
    sppcon->set_spp(this);
    pin_clk1spp = _clk1spp;
    pin_clk2spp = _clk2spp;
    pin_oespp = _oespp;
    pin_csspp = _csspp;
}

SPP::~SPP()
{
    if (active_sig_oe)   pin_oespp->setSource(0);
    if (active_sig_cs)   pin_csspp->setSource(0);
    if (active_sig_clk2) pin_clk2spp->setSource(0);
    if (active_sig_clk1) pin_clk1spp->setSource(0);

    delete sig_oespp;
    delete sig_csspp;
    delete sig_clk2spp;
    delete sig_clk1spp;
}

// SSPDATA register has been written to
void SPP::data_write(uint data)
{
    if((sppcon->get_value() & SPPEN) == 0) return;

    parallel_tris->put(0);                // set port for write
    data_value = data;
    parallel_port->put_value(data);
    eps_value |= SPPBUSY;
    sppeps->put_value(eps_value);
    cycle_state = ST_CYCLE1;
    io_operation = DATA_WRITE;
    sig_oespp->setState('0');
    pin_oespp->updatePinModule();
    
    if (cfg_value & CSEN)
    {
        sig_csspp->setState('1');
        pin_csspp->updatePinModule();
    }
    get_cycles().set_break(get_cycles().get() + (cfg_value & 0x0f) + 1 , this);
}

// SPPEPS register has been written to
void SPP::eps_write(uint data)
{
    uint old = eps_value;
    eps_value = data;
    if((sppcon->get_value() & SPPEN) == 0 || !(old ^ eps_value))
        return;
    
    parallel_tris->put(0);                // set port for write
    parallel_port->put_value(data & 0x0f);
    eps_value |= SPPBUSY;
    sppeps->put_value(eps_value);
    cycle_state = ST_CYCLE1;
    io_operation = ADDR_WRITE;
    sig_oespp->setState('0');
    pin_oespp->updatePinModule();
    
    if (cfg_value & CSEN)
    {
        sig_csspp->setState('1');
        pin_csspp->updatePinModule();
    }
    get_cycles().set_break(get_cycles().get() + (cfg_value & 0x0f) + 1 , this);
}

// SPPCFG register has been written to
void SPP::cfg_write(uint data)
{
    uint diff = cfg_value ^ data;
    cfg_value = data;
    
    if((sppcon->get_value() & SPPEN) == 0) return;

    if (diff & CLK1EN)        // CLK1EN state change
    {
        if (cfg_value & CLK1EN)
        {
            if (!sig_clk1spp) sig_clk1spp = new SppSignalSource();
            pin_clk1spp->setSource(sig_clk1spp);
            active_sig_clk1 = true;
            sig_clk1spp->setState('0');
            pin_clk1spp->updatePinModule();
        }
        else
        {
            pin_clk1spp->setSource(0);
            active_sig_clk1 = false;
        }
    }
    if (diff & CSEN)        // CSEN state change
    {
        if (cfg_value & CSEN)
        {
            if (!sig_csspp) sig_csspp = new SppSignalSource();
            pin_csspp->setSource(sig_csspp);
            active_sig_cs = true;
            sig_csspp->setState('0');
            pin_csspp->updatePinModule();
        }
        else
        {
            active_sig_cs = false;
            pin_csspp->setSource(0);
        }
    }
}

// SPPDATA register has been read from
uint SPP::data_read()
{
    if((sppcon->get_value() & SPPEN) == 0) return(0);

    parallel_tris->put(0xff);                // set port for read
    eps_value |= SPPBUSY;
    sppeps->put_value(eps_value);
    cycle_state = ST_CYCLE1;
    io_operation = DATA_READ;
    sig_oespp->setState('1');
    pin_oespp->updatePinModule();
    if (cfg_value & CSEN)
    {
        sig_csspp->setState('1');
        pin_csspp->updatePinModule();
    }
    get_cycles().set_break(get_cycles().get() + (cfg_value & 0x0f) + 1 , this);
    return data_value;
}
void SPP::enabled(bool _enabled)
{
    if (state_enabled ^ _enabled)
    {
        state_enabled = _enabled;
        if (state_enabled)
        {
            if (!sig_oespp) sig_oespp = new SppSignalSource();
            pin_oespp->setSource(sig_oespp);
            active_sig_oe = true;
            sig_oespp->setState('1');
            pin_oespp->updatePinModule();

            if (!sig_clk2spp) sig_clk2spp = new SppSignalSource();
            pin_clk2spp->setSource(sig_clk2spp);
            active_sig_clk2 = true;
            sig_clk2spp->setState('0');
            pin_clk2spp->updatePinModule();

            if (cfg_value & CLK1EN)
            {
                if (!sig_clk1spp) sig_clk1spp = new SppSignalSource();
                pin_clk1spp->setSource(sig_clk1spp);
                active_sig_clk1 = true;
                sig_clk1spp->setState('0');
                pin_clk1spp->updatePinModule();
            }
            if (cfg_value & CSEN)
            {

                if (!sig_csspp) sig_csspp = new SppSignalSource();
                pin_csspp->setSource(sig_csspp);
                active_sig_cs = true;
                sig_csspp->setState('0');
                pin_csspp->updatePinModule();
            }
            cycle_state = ST_IDLE;
        }
        else
        {
            if (active_sig_oe)
            {
                pin_oespp->setSource(0);
                active_sig_oe = false;
            }
            if (active_sig_clk2) 
            {
                pin_clk2spp->setSource(0);
                active_sig_clk2 = false;
            }
            if (active_sig_clk1) 
            {
                pin_clk1spp->setSource(0);
                active_sig_clk1 = false;
            }

            if (active_sig_cs) 
            {
                pin_csspp->setSource(0);
                active_sig_cs = false;
            }
        }
    }
} 
void SPP::callback()
{
    switch(cycle_state)
    {
    case ST_CYCLE1:
        cycle_state = ST_CYCLE2;
        if(io_operation == DATA_READ)
                data_value = parallel_port->get();
        switch ((cfg_value & (CLKCFG1|CLKCFG0)) >> 6)
        {
        case 3:
        case 2:
            if (eps_value & ADDR0)        // odd address
            {
                if (cfg_value & CLK1EN)
                {
                        sig_clk1spp->setState('1');
                        pin_clk1spp->updatePinModule();
                }
            }
            else
            {
                    sig_clk2spp->setState('1');
                    pin_clk2spp->updatePinModule();
            } 
            break;

        case 1:
            if (io_operation == ADDR_WRITE || io_operation == DATA_WRITE)
            {
                if (cfg_value & CLK1EN)
                {
                        sig_clk1spp->setState('1');
                        pin_clk1spp->updatePinModule();
                }
            }
            else if (io_operation == DATA_READ)
            {
                    sig_clk2spp->setState('1');
                    pin_clk2spp->updatePinModule();
            } 

            break;

        case 0:
            if ((cfg_value & CLK1EN) && io_operation == ADDR_WRITE)
            {
                    sig_clk1spp->setState('1');
                    pin_clk1spp->updatePinModule();
            }
            if (io_operation == DATA_WRITE || io_operation == DATA_READ)
            {
                    sig_clk2spp->setState('1');
                    pin_clk2spp->updatePinModule();
            }
            break;
        }
        get_cycles().set_break(get_cycles().get() + (cfg_value & 0x0f) + 1 , this);
        break;

    case ST_CYCLE2:
        cycle_state = ST_IDLE;
        eps_value &= ~SPPBUSY;
        sppeps->put_value(eps_value);
            sig_oespp->setState('1');
            pin_oespp->updatePinModule();
            sig_clk2spp->setState('0');
            pin_clk2spp->updatePinModule();
        if (cfg_value & CSEN)
        {
                sig_csspp->setState('0');
                pin_csspp->updatePinModule();
        }
        if (cfg_value & CLK1EN)
        {
                sig_clk1spp->setState('0');
                pin_clk1spp->updatePinModule();
        }
        if (!(sppcon->get_value() & SPPOWN))
            pir_set->set_sppif();
        break;

    case ST_IDLE:
    default:
        printf("SPP::callback unexpected callback state=%d\n", cycle_state);
        break;
    }
}

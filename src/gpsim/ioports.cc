/*
   Copyright (C) 1998 Scott Dattalo

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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <assert.h>
#include <string.h>

#include "config.h"
#include "ioports.h"
#include "modules.h"
#include "stimuli.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif
//#define D2
#ifdef D2
#define D2printf(arg) {fprintf(stderr, "%s:%d-%s ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define D2printf(arg) {}
#endif

//--------------------------------------------------
//
//--------------------------------------------------
SignalControl::~SignalControl()
{
}

PeripheralSignalSource::PeripheralSignalSource(PinModule *_pin)
  : m_pin(_pin), m_cState('?')
{
  assert(m_pin);
}

PeripheralSignalSource::~PeripheralSignalSource()
{
}

void PeripheralSignalSource::release()
{
}

// getState is called when the PinModule is attempting to
// update the output state for the I/O Pin.

char PeripheralSignalSource::getState()
{
  return m_cState;
}

/// putState is called when the peripheral output source
/// wants to change the output state.
void PeripheralSignalSource::putState(const char new3State)
{
  if (new3State != m_cState)
  {
    m_cState = new3State;
    m_pin->updatePinModule();
  }
}

void PeripheralSignalSource::toggle()
{
  switch (m_cState)
  {
      case '1':
      case 'W':
        putState('0');
        break;
      case '0':
      case 'w':
        putState('1');
        break;
  }
}

//-------------------------------------------------------------------
//
//                 ioports.cc
//
// The ioport infrastructure for gpsim is provided here. The class
// taxonomy for the IOPORT class is:
//
//  file_register
//     |-> sfr_register
//            |-> IOPORT
//                  |-> PORTA
//                  |-> PORTB
//                  |-> PORTC
//                  |-> PORTD
//                  |-> PORTE
//                  |-> PORTF
//
// Each I/O port has an associated array of I/O pins which provide an
// interface to the virtual external world of the stimuli.
//
//-------------------------------------------------------------------

class SignalSource : public SignalControl
{
    public:
      SignalSource(PortRegister *_reg, uint bitPosition)
        : m_register(_reg), m_bitMask(1<<bitPosition)
      {
      }

      ~SignalSource()
      {
      }

      virtual void release()
      {
      }

      char getState()
      {
        // return m_register ?
        //  (((m_register->getDriving()&m_bitMask)!=0)?'1':'0') : 'Z';
        char r = m_register ? (((m_register->getDriving()&m_bitMask)!=0)?'1':'0') : 'Z';
        /**/
        Dprintf(("SignalSource::getState() %s  bitmask:0x%x state:%c\n",
                 (m_register?m_register->name().c_str():"NULL"),
                 m_bitMask,r));
        /**/
        return r;
      }

    private:
      PortRegister *m_register;
      uint  m_bitMask;
};


//------------------------------------------------------------------------
PortSink::PortSink(PortRegister *portReg, uint iobit)
  : m_PortRegister(portReg), m_iobit(iobit)
{
  assert (m_PortRegister);
}

void PortSink::setSinkState(char cNewSinkState)
{
  Dprintf((" PortSink::setSinkState:bit=%u,val=%c\n", m_iobit, cNewSinkState));

  m_PortRegister->setbit(m_iobit,cNewSinkState);
}

void PortSink::release()
{
  //cout << "PortSink::release() ;" << this << endl;
  delete this;
}
//------------------------------------------------------------------------
PortRegister::PortRegister(Module *pCpu, const char *pName, const char *pDesc, uint numIopins, uint _mask)
  : sfr_register(pCpu, pName, pDesc),
    PortModule(numIopins),
    mEnableMask(_mask),
    drivingValue(0), rvDrivenValue(0,0)
{
    mValidBits = (1<<numIopins)-1; // ugh.
}

IOPIN * PortRegister::addPin(Module *mod, IOPIN *iopin, uint iPinNumber)
{
//    mod->addSymbol(iopin);
    return PortModule::addPin(iopin, iPinNumber);
}

IOPIN * PortRegister::addPin(IOPIN *iopin, uint iPinNumber)
{
//    cpu->addSymbol(iopin);
    return PortModule::addPin(iopin, iPinNumber);
}

void PortRegister::setEnableMask(uint newEnableMask)
{
  Dprintf (( "PortRegister::setEnableMask for %s to %02X\n",
             name_str.c_str(), newEnableMask ));
  mOutputMask = newEnableMask;
  //uint maskDiff = getEnableMask() ^ newEnableMask;
  uint oldEnableMask = getEnableMask();

    for (uint i=0, m=1; i<mNumIopins; i++, m<<= 1)
    if ((newEnableMask & m) && ! (oldEnableMask & m ))
    {
      PinModule *pmP = PortModule::getIOpins(i);
      if (!pmP)
      {
          pmP = new PinModule(this,i);
          PortModule::addPinModule(pmP,i);
          pmP->setDefaultSource(new SignalSource(this, i));
          pmP->addSink(new PortSink(this, i));
      }
      else
      {
        if (pmP->getSourceState() == '?')
        {
           pmP->setDefaultSource(new SignalSource(this, i));
           pmP->addSink(new PortSink(this, i));
        }
      }
    }
   mEnableMask = newEnableMask;
}

void PortRegister::put(uint new_value)
{
  put_value(new_value);
}

void PortRegister::put_value(uint new_value)
{
  Dprintf(("PortRegister::put_value old=0x%x:new=0x%x\n",value.data,new_value));

  uint diff = mEnableMask & (new_value ^ value.data);
  drivingValue = new_value & mEnableMask;
  value.data = drivingValue;

  if(diff) {
    // If no stimuli are connected to the Port pins, then the driving
    // value and the driven value are the same. If there are external
    // stimuli (or perhaps internal peripherals) overdriving or overriding
    // this port, then the call to updatePort() will update 'drivenValue'
    // to its proper value. In either case, calling updatePort ensures
    // the drivenValue is updated properly

    updatePort();
  }
}
//------------------------------------------------------------------------
// PortRegister::updateUI()  UI really means GUI.
// We just pass control to the update method, which is defined in gpsimValue.

void PortRegister::updateUI()
{
  update();
}
//------------------------------------------------------------------------
// PortRegister::setbit
//
// This method is called whenever a stimulus changes the state of
// an I/O pin associated with the port register. 3-state logic is
// used.
// FIXME -  rvDrivenValue and value are always the same, so why have
// FIXME -  both?

void PortRegister::setbit(uint bit_number, char new3State)
{
    int set_mask = (1<<bit_number);
  if(set_mask & mValidBits)
  {
    Dprintf(("PortRegister::setbit() %s bit=%u,val=%c\n",name().c_str(), bit_number,new3State));

    if (new3State=='1' || new3State=='W')
    {
      rvDrivenValue.data |= (1<<bit_number);
      rvDrivenValue.init &= ~(1<<bit_number);
    }
    else if (new3State=='0' || new3State=='w')
    {
      rvDrivenValue.data &= ~(1<<bit_number);
      rvDrivenValue.init &= ~(1<<bit_number);
    }
    else
      // Not a 0 or 1, so it must be unknown.
      rvDrivenValue.init |= (1<<bit_number);

    value = rvDrivenValue;
  }
  else
  {
    Dprintf(("PortRegister::::setbit() %s INVALID BIT bit=%u mask=0x%x\n",
             name().c_str(), bit_number, mValidBits));
  }
}

uint PortRegister::get()
{
  return mOutputMask & rvDrivenValue.data;
}

uint PortRegister::get_value()
{
  Dprintf (( "PortRegister::get_value of %s mask=%02X, data=%02X\n",
              name_str.c_str(), mOutputMask, rvDrivenValue.data ));
  return mOutputMask & rvDrivenValue.data;
}

void PortRegister::putDrive(uint new_value)
{
  put(new_value);
}

uint PortRegister::getDriving()
{
  return drivingValue;
}

//========================================================================
//========================================================================
static PinModule AnInvalidPinModule;

PortModule::PortModule(uint numIopins)
  : mNumIopins(numIopins)
{
  iopins = new PinModule *[mNumIopins];
  for (uint i=0; i<mNumIopins; i++)
    iopins[i] = &AnInvalidPinModule;
}

PortModule::~PortModule()
{
  for (uint i=0; i<mNumIopins; i++)
    if (iopins[i] != &AnInvalidPinModule)
    {
      //cout << __FUNCTION__ << " deleting pin:"<<i<< ':' <<iopins[i] <<endl;
      delete iopins[i];
    }
  delete [] iopins;
}

PinModule &PortModule::operator [] (uint iPinNumber)
{
  if (iPinNumber < mNumIopins) return *iopins[iPinNumber];

  return AnInvalidPinModule;    // error...
}

PinModule * PortModule::getIOpins(uint iPinNumber)
{
  if (iPinNumber < mNumIopins && iopins[iPinNumber] != &AnInvalidPinModule)
    return iopins[iPinNumber];

  return (PinModule *)0;  // error...
}

void PortModule::updatePort()
{
  for (uint i=0; i<mNumIopins; i++)
  {
    if (iopins[i] != &AnInvalidPinModule)
      iopins[i]->updatePinModule();
  }
}

void PortModule::updateUI()
{
  // hmmm nothing
}

void PortModule::updatePin(uint iPinNumber)
{
  if (iPinNumber < mNumIopins)
    iopins[iPinNumber]->updatePinModule();
}

void PortModule::updatePins(uint iPinBitMask)
{
  for (uint i=0,j=1; i<mNumIopins; i++, j<<=1)
    if (j&iPinBitMask && iopins[i])
      iopins[i]->updatePinModule();
}

SignalSink *PortModule::addSink(SignalSink *new_sink, uint iPinNumber)
{
  if (iPinNumber < mNumIopins)
    iopins[iPinNumber]->addSink(new_sink);
  return new_sink;
}

IOPIN *PortModule::addPin(IOPIN *new_pin, uint iPinNumber)
{
  if (iPinNumber < mNumIopins)
  {
    // If there is not a PinModule for this pin, then add one.
    if (iopins[iPinNumber] == &AnInvalidPinModule)
      iopins[iPinNumber] = new PinModule(this,iPinNumber);
    iopins[iPinNumber]->setPin(new_pin);
  }
  else
  {
        printf("PortModule::addPin ERROR pin %u > %u\n", iPinNumber, mNumIopins);
  }
  return new_pin;
}

void PortModule::addPinModule(PinModule *newModule, uint iPinNumber)
{
  if (iPinNumber < mNumIopins  && iopins[iPinNumber] == &AnInvalidPinModule)
    iopins[iPinNumber] = newModule;
}

IOPIN *PortModule::getPin(uint iPinNumber)
{
  if (iPinNumber < mNumIopins) return &iopins[iPinNumber]->getPin();

  return 0;
}

//------------------------------------------------------------------------
// PinModule

PinModule::PinModule()
  : PinMonitor(),
    m_cLastControlState('?'), m_cLastSinkState('?'),
    m_cLastSourceState('?'), m_cLastPullupControlState('?'),
    m_defaultSource(0), m_activeSource(0),
    m_defaultControl(0), m_activeControl(0),
    m_defaultPullupControl(0), m_activePullupControl(0),
    m_pin(0), m_port(0), m_pinNumber(0)
{
    for(int i = 0; i < ANALOG_TABLE_SIZE; i++)
        m_analog_reg[i] = NULL;
}

PinModule::PinModule(PortModule *_port, uint _pinNumber, IOPIN *_pin)
  : PinMonitor(),
    m_cLastControlState('?'), m_cLastSinkState('?'),
    m_cLastSourceState('?'), m_cLastPullupControlState('?'),
    m_defaultSource(0), m_activeSource(0),
    m_defaultControl(0), m_activeControl(0),
    m_defaultPullupControl(0), m_activePullupControl(0),
    m_pin(_pin), m_port(_port), m_pinNumber(_pinNumber),
    m_bForcedUpdate(false)
{
  setPin(m_pin);
  for(int i = 0; i < 3; i++) m_analog_reg[i] = NULL;
}

PinModule::~PinModule()
{
  if (m_pin && (m_activeSource != m_defaultSource))
    D2printf(("Pin %s sources active %p default %p\n", m_pin->name().c_str(), m_activeSource, m_defaultSource));

  if (m_activeSource && (m_activeSource != m_defaultSource))
  {
    //cout << __FUNCTION__ << " deleting active source:"<<m_activeSource<<endl;
    //cout << "    state:" <<m_activeSource->getState() <<endl;
    m_activeSource->release();
    m_activeSource = m_defaultSource;
  }
  if (m_defaultSource)
  {
    m_defaultSource->release();
    delete m_defaultSource;
    m_defaultSource = 0;
  }
  if (m_activeControl && (m_activeControl != m_defaultControl))
  {
    m_activeControl->release();
    m_activeControl = m_defaultControl;
  }
  if (m_defaultControl)
  {
    m_defaultControl->release();
    m_defaultControl = 0;
  }

  if (m_activePullupControl && (m_activePullupControl != m_defaultPullupControl))
    m_activePullupControl->release();

  if (m_defaultPullupControl) m_defaultPullupControl->release();

  if (m_pin) m_pin->setMonitor(0);
}

void PinModule::setPin(IOPIN *new_pin)
{
  // Replace our pin only if this one is valid and we don't have one already.
  if (!m_pin && new_pin)
  {
    m_pin = new_pin;
    m_pin->setMonitor(this);
    m_cLastControlState = getControlState();
    m_cLastSourceState = getSourceState();
  }

}
void PinModule::refreshPinOnUpdate(bool bForcedUpdate)
{
  m_bForcedUpdate = bForcedUpdate;
}

void PinModule::updatePinModule()
{
  if (!m_pin) return;

  bool bStateChange=m_bForcedUpdate;

  Dprintf(("PinModule::updatePinModule():%s enter cont=%c,source=%c,pullup%c\n",
           (m_pin ? m_pin->name().c_str() : "NOPIN"),
            m_cLastControlState,m_cLastSourceState,m_cLastPullupControlState));

  char cCurrentControlState = getControlState();
  uint old_dir = m_pin->get_direction();
  uint new_dir = ( cCurrentControlState=='1' ) ? IOPIN::DIR_INPUT : IOPIN::DIR_OUTPUT;

  if( new_dir != old_dir )
  {
    m_cLastControlState = cCurrentControlState;
    m_pin->update_direction( new_dir, false );
    bStateChange = true;
  }
  char cCurrentSourceState = getSourceState();

  if( cCurrentSourceState != m_cLastSourceState )
  {
    m_cLastSourceState = cCurrentSourceState;
    m_pin->setDrivingState( cCurrentSourceState );
    bStateChange = true;
  }
  char cCurrentPullupControlState = getPullupControlState();

  if( cCurrentPullupControlState != m_cLastPullupControlState )
  {
    m_cLastPullupControlState = cCurrentPullupControlState;
    m_pin->update_pullup(m_cLastPullupControlState,false);
    bStateChange = true;
  }
  if (bStateChange)
  {
    Dprintf(("PinModule::updatePinModule() exit cont=%c,source=%c,pullup%c\n",
             m_cLastControlState,m_cLastSourceState,
             m_cLastPullupControlState));

    if( m_pin->snode ) m_pin->snode->update();
    else               setDrivenState( cCurrentSourceState );
  }
}

void PinModule::setDefaultControl(SignalControl *newDefaultControl)
{
  if(!m_defaultControl && newDefaultControl)
  {
    m_defaultControl = newDefaultControl;
    setControl(m_defaultControl);
  }
  else delete newDefaultControl;   //// YIKES!!! -- wouldn't it be better to return an error code?
}
void PinModule::setControl(SignalControl *newControl)
{
  m_activeControl = newControl ? newControl : m_defaultControl;
}

void PinModule::setDefaultSource(SignalControl *newDefaultSource)
{
  if(!m_defaultSource && newDefaultSource)
  {
    m_defaultSource = newDefaultSource;
    setSource(m_defaultSource);
  }
}

void PinModule::setSource(SignalControl *newSource)
{
  D2printf(("setSource new %p old %p default %p\n", newSource, m_activeSource, m_defaultSource));
  if (m_activeSource && newSource != m_activeSource)
        m_activeSource->release();
  m_activeSource = newSource ? newSource : m_defaultSource;
}

void PinModule::setDefaultPullupControl(SignalControl *newDefaultPullupControl)
{
  if(!m_defaultPullupControl && newDefaultPullupControl)
  {
    m_defaultPullupControl = newDefaultPullupControl;
    setPullupControl(m_defaultPullupControl);
  }
}

void PinModule::setPullupControl(SignalControl *newPullupControl)
{
  m_activePullupControl = newPullupControl ? newPullupControl : m_defaultPullupControl;
}

char PinModule::getControlState()
{
  return m_activeControl ? m_activeControl->getState() : '?';
}
char PinModule::getSourceState()
{
  return m_activeSource ? m_activeSource->getState() : '?';
}

char PinModule::getPullupControlState()
{
  return m_activePullupControl ? m_activePullupControl->getState() : '?';
}

void PinModule::setDrivenState( char new3State )
{
  m_cLastSinkState = new3State;

  list<SignalSink*> :: iterator ssi;
  for (ssi = sinks.begin(); ssi != sinks.end(); ++ssi)
    (*ssi)->setSinkState( new3State );
}

void PinModule::setDrivingState(char new3State)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::set_nodeVoltage(double)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::putState(char)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::setDirection()
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::updateUI()
{
  m_port->updateUI();
}

//        AnalogReq is called by modules such as ADC and Comparator
//        to set or release a pin to/from analog mode. When a pin is in
//        analog mode the TRIS register is still active and output pins
//        are still driven high or low, but reads of the port register
//        return 0 for the pin. When a pin mode is changes the breadboard
//        name of the pin is changed to newname.
//
//        A table of each calling module is kept as a module may
//        request analog mode after another has. The pin is put in
//        analog mode when the first module requests it (up=true)
//        and is taken out of analog mode when all modules have
//        requested analog mode to be released (up=false);
//
void PinModule::AnalogReq( Register* reg, bool analog, const char* newname )
{
    if (!m_port) return;
    
    int i, index;
    uint total_cnt = 0;

    // is the calling register in the table and what is the current
    // count of modules requesting analog mode

    for(i=0, index=-1; i < ANALOG_TABLE_SIZE && m_analog_reg[i]; i++)
    {
        if (m_analog_reg[i] == reg) index = i;
        if (m_analog_active[i])     total_cnt++;
    }

    // Register is not in table so add it.
    //
    if (index < 0)
    {
        assert(i < ANALOG_TABLE_SIZE); // table not large enough
        index = i;
        m_analog_reg[index] = reg;
        m_analog_active[index] = false;
    }
    if( analog )        // Set pin to analog mode request
    {
        m_analog_active[index] = true;
        if (total_cnt == 0)
        {
            uint mask = m_port->getOutputMask();
            mask &= ~(1 << getPinNumber());
            m_port->setOutputMask(mask);
            Dprintf(("PinModule::UpAnalogCnt up %s  newname=%s mask=%x\n", getPin().name().c_str(), newname, mask));

            getPin().set_is_analog(true);
            getPin().set_Cth(5e-12);                // add analog pin input capacitance
         }
    }
    else if (!analog && m_analog_active[index])  // release register request for analog pin
    {
        m_analog_active[index] = false;
        if (total_cnt == 1)
        {
            uint mask = m_port->getOutputMask();
            mask |= (1 << getPinNumber());
            Dprintf(("PinModule::UpAnalogCnt down %s  newname=%s mask=%x\n", getPin().name().c_str(), newname, mask));
            m_port->setOutputMask(mask);

            getPin().set_is_analog(false);
            getPin().set_Cth(0.);
        }
    }
}

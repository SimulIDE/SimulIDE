/*
   Copyright (C) 1998,1999,2000,2001,2002 Scott Dattalo
                 2006,2011 Roy R Rankin

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

#include "config.h"
#include "ssp.h"
#include "pic-ioports.h"
#include "stimuli.h"
#include "14bit-processors.h"
#include "14bit-tmrs.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() %s ",__FILE__,__LINE__,__FUNCTION__,cpu->name().c_str()); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//#define I2C_PROTO
#if defined(I2C_PROTO)
#define I2Cproto(arg) {printf("I2C %s %d ",cpu->name().c_str(), __LINE__); printf arg;}
#else
#define I2Cproto(arg) {}
#endif

//#define SPI_PROTO
#if defined(SPI_PROTO)
#define SPIproto(arg) {printf("SPI %s %d ", cpu->name().c_str(), __LINE__); printf arg;}
#else
#define SPIproto(arg) {}
#endif

//#warning only supports SPI mode.
//-----------------------------------------------------------
// SSPSTAT - Synchronous Serial Port Status register.


_SSPSTAT::_SSPSTAT(Processor *pCpu, SSP_MODULE *pSSP)
  : sfr_register(pCpu, "sspstat","Synchronous Serial Port Status"),
    m_sspmod(pSSP)
{
}

/*
        only CKE and SMP is writable
*/
void _SSPSTAT::put(uint new_value)
{
  uint old6 = value.get() & ~(CKE|SMP);

  // For BSSP register is read only otherwise
  // only CKE and SMP are writable

  if (!m_sspmod || m_sspmod->ssp_type() == SSP_TYPE_BSSP)
        return;

  put_value(old6 | (new_value & (CKE|SMP)));
}

void _SSPSTAT::put_value(uint new_value)
{
  value.put(new_value);
}

class SCK_SignalSource : public SignalControl
{
    public:
      SCK_SignalSource(SSP_MODULE *_ssp_mod, PinModule *_pin)
            : m_pin(_pin), m_ssp_mod(_ssp_mod), m_cState('?')
      {}
      ~SCK_SignalSource(){}
      virtual void release(){m_ssp_mod->releaseSCKpin();}

      virtual char getState() { return m_cState;}

      virtual void putState(const char new3State)
      {
        if (new3State != m_cState) {
          m_cState = new3State;
          m_pin->updatePinModule();
        }
      }

      virtual void toggle()
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

    private:
      PinModule *m_pin;
      SSP_MODULE *m_ssp_mod;
      char m_cState;
};

class SDO_SignalSource : public SignalControl
{
    public:
      SDO_SignalSource(SSP_MODULE *_ssp_mod, PinModule *_pin)
            : m_pin(_pin), m_ssp_mod(_ssp_mod), m_cState('?')
      {}
      virtual void release(){m_ssp_mod->releaseSDOpin();}

      virtual char getState() { return m_cState;}

      virtual void putState(const char new3State)
      {
        if (new3State != m_cState)
        {
          m_cState = new3State;
          m_pin->updatePinModule();
        }
      }

    private:
      PinModule *m_pin;
      SSP_MODULE *m_ssp_mod;
      char m_cState;
};

class SDI_SignalSource : public SignalControl
{
    public:
      SDI_SignalSource(SSP_MODULE *_ssp_mod, PinModule *_pin)
            : m_pin(_pin), m_ssp_mod(_ssp_mod), m_cState('?')
      {}
      virtual void release(){m_ssp_mod->releaseSDIpin();}

      virtual char getState() { return m_cState;}

      virtual void putState(const char new3State)
      {
        if (new3State != m_cState)
        {
          m_cState = new3State;
          m_pin->updatePinModule();
        }
      }

    private:
      PinModule *m_pin;
      SSP_MODULE *m_ssp_mod;
      char m_cState;
};

class SDI_SignalSink : public SignalSink
{
    public:
      SDI_SignalSink(SSP_MODULE *_ssp_mod)
        : m_ssp_mod(_ssp_mod)
      {
        assert(_ssp_mod);
      }
      virtual ~SDI_SignalSink(){}
      virtual void release(){delete this;}

      void setSinkState(char new3State)
      {
        m_ssp_mod->SDI_SinkState(new3State);
      }
    private:
      SSP_MODULE *m_ssp_mod;
};

class SCL_SignalSink : public SignalSink
{
    public:
      SCL_SignalSink(SSP_MODULE *_ssp_mod)
        : m_ssp_mod(_ssp_mod)
      {
        assert(_ssp_mod);
      }
      virtual ~SCL_SignalSink(){}
      virtual void release(){delete this; }

      void setSinkState(char new3State)
      {
        m_ssp_mod->SCL_SinkState(new3State);
      }
    private:
      SSP_MODULE *m_ssp_mod;
};

class SS_SignalSink : public SignalSink
{
    public:
      SS_SignalSink(SSP_MODULE *_ssp_mod)
        : m_ssp_mod(_ssp_mod)
      {
        assert(_ssp_mod);
      }
      virtual ~SS_SignalSink(){}
      virtual void release(){delete this; }

      void setSinkState(char new3State)
      {
        m_ssp_mod->SS_SinkState(new3State);
      }
    private:
      SSP_MODULE *m_ssp_mod;
};


//-----------------------------------------------------------
// SSPCON - Synchronous Serial Port Control register.
//-----------------------------------------------------------

_SSPCON::_SSPCON(Processor *pCpu, SSP_MODULE *pSSP)
  : sfr_register(pCpu, "sspcon","Synchronous Serial Port Control"),
    m_sspmod(pSSP)
{
}
bool _SSPCON::isSPIActive(uint value)
{
    if (value & SSPEN)
    {
      switch(value & SSPM_mask)
      {
        case SSPM_SPImaster4:
        case SSPM_SPImaster16:
        case SSPM_SPImaster64:
        case SSPM_SPImasterTMR2:
        case SSPM_SPIslaveSS:
        case SSPM_SPIslave:
            return(true);

        case SSPM_SPImasterAdd:
            if ((m_sspmod->ssp_type() == SSP_TYPE_MSSP1))
                return(true);
      }
    }
    return(false);
}
bool _SSPCON::isSPIMaster()
{
    uint reg_value = value.get();
    if (reg_value & SSPEN)
    {
      switch(reg_value & SSPM_mask)
      {
        case SSPM_SPImaster4:
        case SSPM_SPImaster16:
        case SSPM_SPImaster64:
        case SSPM_SPImasterTMR2:
            return(true);

        case SSPM_SPImasterAdd:
            if ((m_sspmod->ssp_type() == SSP_TYPE_MSSP1))
                return(true);
            break;
      }
    }
    return(false);
}
/*
        process write to SSPCON
*/
void _SSPCON::put(uint new_value)
{
  uint old_value = value.get();

  put_value(new_value);

  Dprintf(("SSPCON new %x old %x\n", new_value, old_value));
  if ((new_value & SSPEN) && ! (old_value & SSPEN)) // Turn on SSP
       m_sspmod->startSSP(new_value);
  else if (!(new_value & SSPEN) &&  (old_value & SSPEN)) // Turn off SSP
       m_sspmod->stopSSP(old_value);
  else if (new_value != old_value )                 // change while active
      m_sspmod->changeSSP(new_value, old_value);
}

/*
        update SSPCON without action
*/
void _SSPCON::put_value(uint new_value)
{
  value.put(new_value & 0xff);
}

/*
        Set WCOL bit of SSPCON
*/
void _SSPCON::setWCOL()
{
  if (value.get() & WCOL) return;
  put_value(value.get() | WCOL);
}
/*
        return true if a I2C mode is enabled in SSPCON
*/

bool _SSPCON::isI2CActive(uint val)
{
    if ( (val & SSPEN) != SSPEN) return(false);
    switch(val & SSPM_mask)
    {
      case SSPM_I2Cslave_7bitaddr:
      case SSPM_I2Cslave_10bitaddr:
      case SSPM_MSSPI2Cmaster:
      case SSPM_I2Cfirmwaremaster:
      case SSPM_I2Cslave_7bitaddr_ints:
      case SSPM_I2Cslave_10bitaddr_ints:
        return(true);
        break;
    }
    return(false);
}
bool _SSPCON::isI2CMaster(uint val)
{
    if ( (val & SSPEN) != SSPEN) return(false);

    switch(val & SSPM_mask)
    {
      case SSPM_MSSPI2Cmaster:
      case SSPM_I2Cfirmwaremaster:
        return(true);
        break;
    }
    return(false);
}
/*
        return true if an I2C slave mode is enabled in SSPCON
*/
bool _SSPCON::isI2CSlave(uint val)
{
    if ( (val & SSPEN) != SSPEN) return(false);

    switch(val & SSPM_mask)
    {
      case SSPM_I2Cslave_7bitaddr:
      case SSPM_I2Cslave_10bitaddr:
      case SSPM_I2Cslave_7bitaddr_ints:
      case SSPM_I2Cslave_10bitaddr_ints:
        return(true);
        break;
    }
    return(false);
}

//-----------------------------------------------------------
// SSPBUF - Synchronous Serial Port Control register.
//-----------------------------------------------------------

_SSPBUF::_SSPBUF(Processor *pCpu, SSP_MODULE *pSSP)
  : sfr_register(pCpu, "sspbuf","Synchronous Serial Port Buffer"),
    m_sspmod(pSSP), m_bIsFull(false)
{
}

/*
        process write to SSPBUF
*/
void _SSPBUF::put(uint new_value)
{
  put_value(new_value);
  m_sspmod->newSSPBUF(value.get());
  m_bIsFull = false;
}

/*
        update SSPBUF without processing data
*/
void _SSPBUF::put_value(uint new_value)
{
  value.put(new_value & 0xff);
}

uint _SSPBUF::get()
{
  if( m_sspmod ) m_sspmod->rdSSPBUF();

  m_bIsFull = false;

  return value.get();
}

uint _SSPBUF::get_value()
{
  return value.get();
}


//-----------------------------------------------------------
// SSPMSK - Synchronous Serial Port Address mask(for I2C)
//-----------------------------------------------------------
_SSPMSK::_SSPMSK(Processor *pCpu, const char *_name)
  : sfr_register(pCpu, _name,"Synchronous I2C Address mask")
{
   put_value(0xff);
}
void _SSPMSK::put(uint new_value)
{
  put_value(new_value);
}
//-----------------------------------------------------------
// SSPADD - Synchronous Serial Port Address (for I2C)
//-----------------------------------------------------------
_SSPADD::_SSPADD(Processor *pCpu, SSP_MODULE *pSSP)
  : sfr_register(pCpu, "sspadd","Synchronous Serial Port Address (I2C)"),
    m_sspmod(pSSP)
{
}

/*
   On some processors SSPMSK is accessed through SSPADD
   lt is assummed that SSPM_LoadMaskFunction bits of SSPCON
   are reserved on those processors which have seperate addresses
   for SSPMSK and SSPADD

*/
void _SSPADD::put(uint new_value)
{

  if (m_sspmod && m_sspmod->sspmsk &&
        ((m_sspmod->sspcon.value.get() & _SSPCON::SSPM_mask) ==
                _SSPCON::SSPM_LoadMaskFunction))
  {
        m_sspmod->sspmsk->put(new_value);
        return;
  }

  put_value(new_value);

  if( m_sspmod )
  {
     if (m_sspmod->sspmsk)
     {
        m_sspmod->newSSPADD(m_sspmod->sspmsk->value.get() &new_value);
     }
     else
        m_sspmod->newSSPADD(new_value);
  }
}

void _SSPADD::put_value(uint new_value)
{
  value.put(new_value & 0xff);
}
uint  _SSPADD::get()
{
    uint val = value.get();
    if (m_sspmod->sspmsk)
    {
        uint con_val = m_sspmod->sspcon.value.get() & _SSPCON::SSPM_mask;
        if (con_val == _SSPCON::SSPM_LoadMaskFunction)
            return m_sspmod->sspmsk->value.get();
    }

    return val;
}



SPI::SPI(SSP_MODULE *_ssp_mod, _SSPCON *_sspcon, _SSPSTAT *_sspstat, _SSPBUF *_sspbuf)
  : m_state(eIDLE)
{
    m_sspmod = _ssp_mod;
    m_sspcon = _sspcon;
    m_sspstat = _sspstat;
    m_sspbuf = _sspbuf;
    cpu = m_sspmod->cpu;
}

void SPI::clock( bool ClockState )
{
  // A clock has happened. Either we sent one or we recieved one.
  bool onbeat;
  bool allDone = false;
  if( !m_sspstat || ! m_sspcon)
    return;
  uint sspstat_val = m_sspstat->value.get();
  uint sspcon_val = m_sspcon->value.get();

  //cout << "SPi clock " << ClockState << " m_state=" << m_state << endl;

  if( ClockState ) // rising edge
  {
    if( ( (sspcon_val & _SSPCON::CKP) && !(sspstat_val & _SSPSTAT::CKE) )
        || ( !(sspcon_val & _SSPCON::CKP) && (sspstat_val & _SSPSTAT::CKE) ) )
      onbeat = true;
    else
      onbeat = false;
  }
  else // falling edge
  {
    if( ( !(sspcon_val & _SSPCON::CKP) && !(sspstat_val & _SSPSTAT::CKE) )
        || ( (sspcon_val & _SSPCON::CKP) && (sspstat_val & _SSPSTAT::CKE)))
      onbeat = true;
    else
      onbeat = false;
  }

  if( m_state == eIDLE ){
    SPIproto(("Idle clock %d CKE %d CKP %d onbeat %d\n", ClockState, (sspstat_val & _SSPSTAT::CKE), (sspcon_val & _SSPCON::CKP), onbeat));
    if( sspstat_val & _SSPSTAT::CKE )
    {
      // FIX: I have NOT verified that PICs actually behave like this.
      cout << "SSP: I can't handle a non-started transfer with CKE = 1." << endl;
      return;
    }
    else if( onbeat )
    {
      // FIX: I have NOT verified that PICs actually behave like this.
      cout << "SSP: " << cpu->name() << " Ignoring clock transition to neutral in state IDLE." << endl;
      return;
    }
    else
    {
        // RP: This is only relevant in slave mode? I think clock is never called
        // while idle in master mode.

      SPIproto(("Clock called start transfer\n"));
      start_transfer();
    }
  }
  if (!m_sspmod) return;

  if( onbeat ) {
    // on beat: data is read in if SMP = 0
    if( !(sspstat_val & _SSPSTAT::SMP) )
    {
      m_SSPsr <<= 1;
      if (m_sspmod->get_SDI_State())
        m_SSPsr |= 1;

      SPIproto(("In Bit %u byte count=%d onbeat m_SSPsr=0x%02x\n", (m_SSPsr & 1), bits_transfered+1, m_SSPsr));
    }
  }
  else
  {
    // off beat: data is shifted out, data is read in if SMP = 1

    if( sspstat_val & _SSPSTAT::SMP ) {
      m_SSPsr <<= 1;
      if (m_sspmod->get_SDI_State())
        m_SSPsr |= 1;

      SPIproto(("In Bit %u byte count=%d offbeat\n", (m_SSPsr & 1), bits_transfered+1));
    }

    char nextSDO = (m_SSPsr&(1<<7)) ? '1' : '0';
    m_sspmod->putStateSDO(nextSDO);

    SPIproto(("\tSent bit %c m_SSPsr 0x%x\n", nextSDO, m_SSPsr));
  }

  bool bSSPCONValue = (sspcon_val & _SSPCON::CKP) ? true : false;
  if(bSSPCONValue == ClockState) {
    bits_transfered++;
    if( bits_transfered == 8 )
    {
      if( (sspstat_val & _SSPSTAT::SMP) && !(sspstat_val & _SSPSTAT::CKE) )
      {
        m_state = eWAITING_FOR_LAST_SMP;
      }
      else
      {
        stop_transfer();
        allDone = true;
      }
    }
  }
  if ( !allDone && m_sspcon->isSPIMaster() )
    set_halfclock_break();
}

void SPI::set_halfclock_break()
{
  int clock_in_cycles = 1;

  if( !m_sspstat || ! m_sspcon)
    return;

  uint sspcon_val = m_sspcon->value.get();

  switch( sspcon_val & _SSPCON::SSPM_mask ) {
  // Simulation requires Fosc/4 to be run at Fosc/8
  case _SSPCON::SSPM_SPImaster4:
    clock_in_cycles = 1;
    break;
  case _SSPCON::SSPM_SPImaster16:
    clock_in_cycles = 2;
    break;
  case _SSPCON::SSPM_SPImaster64:
    clock_in_cycles = 8;
    break;
  case _SSPCON::SSPM_SPImasterTMR2:
    break;
  }

  get_cycles().set_break(get_cycles().get() + clock_in_cycles, this);
}
void SPI::callback()
{
  if (!m_sspmod)
    return;

  SPIproto(("callback m_state=%d\n", m_state));

  switch( m_state )
  {
  case eIDLE:
    break;
  case eACTIVE:
    m_sspmod->Sck_toggle();
    clock( m_sspmod->get_SCL_State() );
    break;
  case eWAITING_FOR_LAST_SMP:
    if( m_sspstat && m_sspstat->value.get() & _SSPSTAT::SMP )
    {
      m_SSPsr <<= 1;
      if (m_sspmod->get_SDI_State())
        m_SSPsr |= 1;
    }
    m_state = eACTIVE;
    stop_transfer();
    break;
  }
}
//-----------------------------------------------------------

void SPI::startSPI()
{
    m_state = eIDLE;
    bits_transfered = 0;
}

SPI_1::SPI_1(SSP1_MODULE *_ssp_mod, _SSPCON *_sspcon, _SSPSTAT *_sspstat,
        _SSPBUF *_sspbuf, _SSP1CON3 *_ssp1con3, _SSPADD *_sspadd) :
        SPI(_ssp_mod, _sspcon, _sspstat, _sspbuf)
{
    m_ssp1con3 = _ssp1con3;
    m_sspadd = _sspadd;
}
void SPI_1::set_halfclock_break()
{
  int clock_in_cycles = 1;

  if( !m_sspstat || ! m_sspcon)
    return;

  uint sspcon_val = m_sspcon->value.get();

  switch( sspcon_val & _SSPCON::SSPM_mask ) {
  // Simulation requires Fosc/4 to be run at Fosc/8
  case _SSPCON::SSPM_SPImaster4:
    clock_in_cycles = 1;
    break;
  case _SSPCON::SSPM_SPImaster16:
    clock_in_cycles = 2;
    break;
  case _SSPCON::SSPM_SPImaster64:
    clock_in_cycles = 8;
    break;
  case _SSPCON::SSPM_SPImasterAdd:
    // Note, this will be low by 1 cycle/clock when sspadd is even
    clock_in_cycles = (m_sspadd->get() + 1)>>1;
    if (clock_in_cycles < 2)
    {
        cout << "WARNING for SPI sspadd must be >= 3\n";
        clock_in_cycles = 2;
    }
    break;
  case _SSPCON::SSPM_SPImasterTMR2:
    break;
  }

  get_cycles().set_break(get_cycles().get() + clock_in_cycles, this);
}
void SPI_1::stop_transfer()
{

    Dprintf(("stop_transfer SPI_1\n"));
  if (!m_sspcon  || !m_sspstat || !m_sspbuf || !m_sspmod || !m_ssp1con3)
        return;

  if( m_state == eACTIVE ) {
    if (bits_transfered == 8 ) Dprintf(("BOEN %x\n", m_ssp1con3->value.get() & _SSP1CON3::BOEN));
    if( bits_transfered == 8 && m_ssp1con3->value.get() & _SSP1CON3::BOEN)
    {
        m_sspbuf->put_value(m_SSPsr & 0xff);
        m_sspmod->set_sspif();
    }
    else if( bits_transfered == 8 && !m_sspbuf->isFull() )
    {
        m_sspbuf->put_value(m_SSPsr & 0xff);
        m_sspbuf->setFullFlag(true);
        m_sspmod->set_sspif();
        m_sspstat->put_value(m_sspstat->value.get() | _SSPSTAT::BF);
    } else if( bits_transfered == 8 && m_sspbuf->isFull() ) {
        m_sspcon->setSSPOV();
        m_sspmod->set_sspif();      // The real PIC sets sspif even with overflow
    } else {
      cout << "SPI: Stopping transfer. Cancel finish." << endl;
      // The transfer was canceled in some way
    }
  }
  m_state = eIDLE;
}

SSP_MODULE::SSP_MODULE(Processor *pCpu)
  : sspbuf(pCpu,this),
    sspcon(pCpu,this),
    sspstat(pCpu,this),
    sspcon2(pCpu,this),
    sspadd(pCpu,this),
    sspmsk(0),
    cpu(pCpu),
    m_ssp_if(0),
    m_bcl_if(0),
    m_pirset(0),
    m_spi(0),
    m_i2c(0),
    m_sck(0),
    m_ss(0),
    m_sdo(0),
    m_sdi(0),
    m_i2c_tris(0),
    m_SDI_State(false),
    m_SCL_State(false),
    m_SS_State(false),
    m_SckSource(0),
    m_SdoSource(0),
    m_SdiSource(0),
    m_SDI_Sink(0),
    m_SCL_Sink(0),
    m_SS_Sink(0),
    m_sink_set(false),
    m_sdo_active(false),
    m_sdi_active(false),
    m_sck_active(false)
{
}

SSP_MODULE::~SSP_MODULE()
{
    if (!m_sink_set)
    {
        delete m_SDI_Sink;
        delete m_SCL_Sink;
        delete m_SS_Sink;
    }

    if (m_sdi_active && m_sdi)
        m_sdi->setSource(0);
    if (m_SdiSource)
        delete m_SdiSource;

    if (m_sdo_active && m_sdo) m_sdo->setSource(0);
    if (m_SdoSource) delete m_SdoSource;


    if (m_sck_active && m_sck)
        m_sck->setSource(0);
    if (m_SckSource) delete m_SckSource;
    if (m_spi)
    {
        delete m_spi;
        delete m_i2c;
    }
    if (m_ssp_if) delete m_ssp_if;
    if (m_bcl_if) delete m_bcl_if;
}

/*
    SSP1_MODULE adds SSPCON3 and SSPMSK to SSP_MODULE
*/
SSP1_MODULE::SSP1_MODULE(Processor *pCpu)
           : SSP_MODULE(pCpu)
           , ssp1con3(pCpu, this)
{
    sspmsk = new _SSPMSK(pCpu, "ssp1msk");
}

SSP1_MODULE::~SSP1_MODULE()
{
   delete sspmsk;
}

void SSP1_MODULE::setIOpin( int data, PinModule* pin )
{
    switch(data)
    {
        case SCK_PIN:
            set_sckPin(pin);
            break;
        
        case SDI_PIN:
            set_sdiPin(pin);
            break;
        
        case SDO_PIN:
            set_sdoPin(pin);
            break;
        
        case SS_PIN:
            set_ssPin(pin);
            break;
    };
}

void SSP1_MODULE::set_sckPin( PinModule *_sckPin )
{
   if (m_sck == _sckPin) return;        // No change, do nothing
   m_sck = _sckPin;
   if(m_SckSource) delete m_SckSource;
   m_SckSource = new SCK_SignalSource(this, m_sck);
}

void SSP1_MODULE::set_sdoPin(PinModule *_sdoPin)
{
   if (m_sdo == _sdoPin) return;        // No change, do nothing
   m_sdo = _sdoPin;

   if(m_SdoSource) delete m_SdoSource;
   m_SdoSource = new SDO_SignalSource(this, m_sdo);
}

void SSP1_MODULE::set_sdiPin(PinModule *_sdiPin)
{
   if (m_sdi == _sdiPin) return;        // No change, do nothing
   m_sdi = _sdiPin;
   if(m_SdiSource) delete m_SdiSource;
   m_SdiSource = new SDI_SignalSource(this, m_sdi);
}

void SSP1_MODULE::set_ssPin(PinModule *_ssPin)
{
   if (m_ss == _ssPin) return;        // No change, do nothing
   m_ss = _ssPin;

}

void SSP1_MODULE::initialize(
                        PIR_SET *ps,
                        PinModule *SckPin,
                        PinModule *SsPin,
                        PinModule *SdoPin,
                        PinModule *SdiPin,
                         PicTrisRegister *_i2ctris,
                        SSP_TYPE _ssptype
                )
{
  m_pirset = ps;
  m_sck = SckPin;
  m_ss = SsPin;
  m_sdo = SdoPin;
  m_sdi = SdiPin;
  m_i2c_tris = _i2ctris;
  m_ssptype = _ssptype;
  m_SckSource = new SCK_SignalSource(this, m_sck);
  m_SdoSource = new SDO_SignalSource(this, m_sdo);
  m_SdiSource = new SDI_SignalSource(this, m_sdi);
  if (! m_spi)
  {
    m_spi = new SPI_1(this, &sspcon, &sspstat, &sspbuf, &ssp1con3, &sspadd);
    m_i2c = new I2C_1(this, &sspcon, &sspstat, &sspbuf, &sspcon2, &sspadd, &ssp1con3);
    m_SDI_Sink = new SDI_SignalSink(this);
    m_SCL_Sink = new SCL_SignalSink(this);
    m_SS_Sink = new SS_SignalSink(this);
  }


}
void SPI::newSSPBUF(uint newTxByte)
{
  Dprintf(("enabled %d state %d\n", m_sspcon->isSSPEnabled(), m_state));
  if (m_sspcon->isSSPEnabled()) {
    if (m_state == eIDLE || bits_transfered == 0) {
        m_SSPsr = newTxByte;
        SPIproto(("newSSPBUF send 0x%02x\n", m_SSPsr));
        start_transfer();
    } else {
      // Collision
      SPIproto(("newSSPBUF 0x%02x collision\n", m_SSPsr));
      m_sspcon->setWCOL();
    }
  }
  else
  {
        SPIproto(("newSSPBUF !SSPenabled m_SSPsr 0x%02x\n", m_SSPsr));
  }
}
void SPI::start_transfer()
{
  if (!m_sspcon || !m_sspstat)
    return;

  // load the shift register
  m_state = eACTIVE;
  bits_transfered = 0;
  uint sspcon_val = m_sspcon->value.get();
  uint sspstat_val = m_sspstat->value.get();

  switch( sspcon_val & _SSPCON::SSPM_mask ) {
  case _SSPCON::SSPM_SPImaster4:
  case _SSPCON::SSPM_SPImaster16:
  case _SSPCON::SSPM_SPImaster64:
  case _SSPCON::SSPM_SPImasterAdd:
    // In master mode, the SDO line is always set at the start of the transfer
    m_sspmod->putStateSDO((m_SSPsr &(1<<7)) ? '1' : '0');
    // Setup callbacks for clocks
    set_halfclock_break();
    break;
  case _SSPCON::SSPM_SPImasterTMR2:
      m_sspmod->putStateSDO((m_SSPsr &(1<<7)) ? '1' : '0');
        break;
  case _SSPCON::SSPM_SPIslaveSS:
    // The SS pin was pulled low
    if( sspstat_val & _SSPSTAT::CKE )
      m_sspmod->putStateSDO((m_SSPsr &(1<<7)) ? '1' : '0');
    break;
  case _SSPCON::SSPM_SPIslave:
    // I don't do any thing until first clock edge
    SPIproto(("SSPM_SPIslave start_transfer 0x%02x\n", m_SSPsr));
    if( sspstat_val & _SSPSTAT::CKE )
      m_sspmod->putStateSDO((m_SSPsr &(1<<7)) ? '1' : '0');
    break;
  default:
    cout << "start_transfer: The selected SPI mode is unimplemented. mode=" << hex
        <<(sspcon_val & _SSPCON::SSPM_mask) << endl;
  }

}
void SPI::stop_transfer()
{
  if (!m_sspcon  || !m_sspstat || !m_sspbuf || !m_sspmod)
        return;

  if( m_state == eACTIVE ) {
    if( bits_transfered == 8 && !m_sspbuf->isFull() )
    {
        m_SSPsr &= 0xff;
        SPIproto(("Stoping transfer. Normal finish. Setting sspif and BF got=0x%02x\n" , (m_SSPsr & 0xff)));
        m_sspbuf->put_value(m_SSPsr & 0xff);
        m_sspbuf->setFullFlag(true);
        m_sspmod->set_sspif();
        m_sspstat->put_value(m_sspstat->value.get() | _SSPSTAT::BF);
    } else if( bits_transfered == 8 && m_sspbuf->isFull() ) {
        SPIproto(("Stopping transfer. SSPBUF Overflow setting SSPOV.\n"));
        m_sspcon->setSSPOV();
        m_sspmod->set_sspif();      // The real PIC sets sspif even with overflow
    } else {
      cout << "SPI: Stopping transfer. Cancel finish." << endl;
      // The transfer was canceled in some way
    }
  } else {
    SPIproto(("Stopping transfer. State != ACTIVE."));
  }

  m_state = eIDLE;

}

I2C::I2C(SSP_MODULE *_ssp_mod, _SSPCON *_sspcon, _SSPSTAT *_sspstat,
        _SSPBUF *_sspbuf, _SSPCON2 *_sspcon2, _SSPADD *_sspadd)
  : i2c_state(eIDLE)
{
    m_sspmod = _ssp_mod;
    m_sspcon = _sspcon;
    m_sspstat = _sspstat;
    m_sspbuf = _sspbuf;
    m_sspcon2 = _sspcon2;
    m_sspadd = _sspadd;
    future_cycle = 0;

    cpu = m_sspmod->cpu;
}

I2C_1::I2C_1(SSP_MODULE *_ssp_mod, _SSPCON *_sspcon, _SSPSTAT *_sspstat,
        _SSPBUF *_sspbuf, _SSPCON2 *_sspcon2, _SSPADD *_sspadd,
        _SSP1CON3 *_ssp1con3) :
           I2C(_ssp_mod, _sspcon, _sspstat, _sspbuf, _sspcon2, _sspadd)
{
    m_sspmod = _ssp_mod;
    m_sspcon3 = _ssp1con3;
}

void I2C::set_idle()
{
    i2c_state = eIDLE;
    I2Cproto(("%s i2c_state = eIDLE\n", __FUNCTION__));
}
bool I2C::isIdle()
{
    if (i2c_state == eIDLE) return true;
    if(
        (m_sspstat->value.get() & _SSPSTAT::RW) == 0 &&
        (m_sspcon2->value.get() &
           (
             _SSPCON2::ACKEN |
             _SSPCON2::RCEN |
             _SSPCON2::PEN |
             _SSPCON2::RSEN |
             _SSPCON2::SEN
           )) == 0
        )
    {
        set_idle();
    }
    return(i2c_state == eIDLE);
}
bool I2C::rx_byte()
{
    m_SSPsr = ( m_SSPsr << 1 ) |  (m_sspmod->get_SDI_State()?1:0);
    bits_transfered++;
    if (bits_transfered == 8)
    {
        m_sspcon2->put_value(m_sspcon2->value.get() & ~_SSPCON2::RCEN);

        m_sspmod->SaveSSPsr(m_SSPsr & 0xff);
        m_sspmod->set_sspif();
        set_idle();
        return(true);
    }
    return(false);
}
/*
    Called during phase 0
    SCL goes high
*/
bool I2C::scl_pos_tran()
{
        
    return true;
}
/*
    Called during phase 1
    SCL is high, data can be read and STOP, START transitions
*/
bool I2C::scl_clock_high()
{
    if (i2c_state == CLK_STOP)
    {
        setBRG();
        m_sspmod->setSDA(true);
        return false;
    }
    else if (i2c_state == CLK_RSTART)
    {
        m_sspmod->setSDA(true);
    }
    else if (i2c_state == CLK_START)
    {
        m_sspmod->setSDA(false);
    }
    else if (i2c_state == CLK_RX_ACK)
    {
            bool n_ack = m_sspmod->get_SDI_State();

            if (n_ack)
                m_sspcon2->put_value(m_sspcon2->value.get() | _SSPCON2::ACKSTAT);
            else
                m_sspcon2->put_value(m_sspcon2->value.get() & ~_SSPCON2::ACKSTAT);
    }
    else if (i2c_state == CLK_RX_BYTE)
    {
        if (bits_transfered < 8)
        {
            m_SSPsr = (m_SSPsr << 1) | (m_sspmod->get_SDI_State()?1:0);
            bits_transfered++;
        }
    }
    return true;
}
/*
    Called during phase 2
    SCL is going low
*/
        
bool I2C::scl_neg_tran()
{
    if (i2c_state == CLK_STOP)
    {
        // Check SCL and SDI still high
        if (m_sspmod->get_SCL_State() && m_sspmod->get_SDI_State())
        {
            uint sspstat = m_sspstat->value.get() & (_SSPSTAT::SMP | _SSPSTAT::CKE);
            sspstat |= _SSPSTAT::P;
            m_sspstat->value.put(sspstat);
            m_sspmod->set_sspif();
        }
        else
        {
             m_sspmod->set_bclif();
        }
        set_idle();
        m_sspcon2->value.put(m_sspcon2->value.get() & ~_SSPCON2::PEN );
        return false;
    }
    else if (i2c_state == CLK_START)
    {
        m_sspcon2->value.put(m_sspcon2->value.get() &
                ~(_SSPCON2::SEN | _SSPCON2::RSEN));
        if (m_sspmod->get_SCL_State() && !m_sspmod->get_SDI_State())
        {
            m_sspmod->setSCL(false);
            m_sspmod->set_sspif();
        }
        else
        {
            m_sspmod->setSDA(true);
            m_sspmod->set_bclif();
        }
        set_idle();
        return false;
    }
    return true;
}
/*
    Called during phase 3
    SCL is low, data should be put on the bus in this phase
*/
bool I2C::scl_clock_low()
{
    if (i2c_state == CLK_RSTART)
    {
        i2c_state = CLK_START;
    }
    else if (i2c_state == CLK_ACKEN)
    {
               m_sspcon2->value.put( m_sspcon2->value.get() & ~(_SSPCON2::ACKEN ));
        m_sspmod->set_sspif();
        set_idle();
        return false;
    }
    else if (i2c_state == CLK_RX_ACK)
    {
        m_sspstat->put_value(m_sspstat->value.get() & ~_SSPSTAT::RW);
        m_sspmod->set_sspif();
        set_idle();
        return false;
    }
    else if (i2c_state == CLK_TX_BYTE)
    {
        bits_transfered++;
        if (bits_transfered < 8)
        {
            m_SSPsr <<= 1;
            m_sspmod->setSDA((m_SSPsr & 0x80) == 0x80);
        }
        else if(bits_transfered == 8)
        {
            m_sspstat->put_value(m_sspstat->value.get() & ~_SSPSTAT::BF);
            i2c_state = CLK_RX_ACK;
        }
    }
    else if (i2c_state == CLK_RX_BYTE)
    {
        if(bits_transfered == 8)
        {
            m_sspstat->put_value(m_sspstat->value.get() & ~_SSPSTAT::RW);
            m_sspcon2->put_value(m_sspcon2->value.get() & ~_SSPCON2::RCEN);
             m_sspmod->SaveSSPsr(m_SSPsr & 0xff);

            m_sspmod->set_sspif();
            set_idle();
            return false;
        }
    }

    return true;
}
void I2C::callback()
{
    if( future_cycle != get_cycles().get() )
    {
        cout << "I2C callback - program error future_cycle=" << future_cycle << " now="
                << get_cycles().get() << " i2c_state=" << i2c_state << endl;
    }

    future_cycle = 0;
    if( i2c_state == eIDLE ) return;
    switch( phase )
    {
    case 0:                // SCL goes high
        if( scl_pos_tran() )
        {
            setBRG();
            m_sspmod->setSCL(true);
        }
        break;

    case 1:                // SCL low get data (or STOP, START) from SDA bus
        if( scl_clock_high() ) setBRG();
        break;

    case 2:                // SCL goes low
        if( scl_neg_tran() )
        {
            setBRG();
            m_sspmod->setSCL(false);
        }
        break;

    case 3:                // put data on SDA bus
        if( scl_clock_low() )
            setBRG();
        break;
    }
    phase = (phase + 1) % 4;
    return;

    switch( i2c_state )
    {
    case CLK_ACKEN:
        if( phase == 1 )
        {
            m_sspmod->setSCL(true);
        }
        else if (phase == 2)
        {
            m_sspmod->setSCL(false);
            m_sspcon2->value.put( m_sspcon2->value.get() & ~(_SSPCON2::ACKEN ));
            m_sspmod->set_sspif();
        }
        else
        {
            cout << "CLK_ACKEN unexpected phase " << phase << endl;
        }
        break;

    case CLK_START:
        if (phase == 0)
        {
            phase++;
            m_sspmod->setSDA(false);
            setBRG();
        }
        else
        {
            m_sspcon2->value.put(m_sspcon2->value.get() &
                ~(_SSPCON2::SEN | _SSPCON2::RSEN));
            m_sspmod->setSCL(false);
            m_sspmod->set_sspif();
            set_idle();
        }
        break;

    case CLK_RSTART:
        if (phase == 0)
          {
            m_sspmod->setSCL(true);
        }
        break;

    case CLK_STOP:
        if (phase == 0)
        {
            phase++;
            if (m_sspmod->get_SCL_State())
            {
                setBRG();
            }
            m_sspmod->setSCL(true);
        }
        else if (phase == 1)
        {
            phase++;
            setBRG();
            m_sspmod->setSDA(true);
        }
        else
        {
            if (m_sspstat->value.get() &  _SSPSTAT::P)
            {
                m_sspmod->set_sspif();
            }
            else
            {
                m_sspmod->set_bclif();
            }
            set_idle();
            m_sspcon2->value.put(m_sspcon2->value.get() & ~_SSPCON2::PEN );
        }
        break;


    case CLK_TX_BYTE:
        if( m_sspmod->get_SCL_State() )
        {
            bool n_ack = m_sspmod->get_SDI_State();
            bits_transfered++;
            if( bits_transfered < 8 )
            {
                m_SSPsr <<= 1;
                m_sspmod->setSCL( false );
                m_sspmod->setSDA(( m_SSPsr & 0x80 ) == 0x80);
            }
            else if(bits_transfered == 8)
            {
                m_sspmod->setSCL(false);
                m_sspmod->setSDA(true);
                m_sspstat->put_value(m_sspstat->value.get() & ~_SSPSTAT::BF);
            }
            else
            {
                if (n_ack)
                {
                    m_sspcon2->put_value(m_sspcon2->value.get() | _SSPCON2::ACKSTAT);
                    I2Cproto(("NACK\n"));
                }
                else
                {
                    m_sspcon2->put_value(m_sspcon2->value.get() & ~_SSPCON2::ACKSTAT);
                    I2Cproto(("ACK\n"));
                }
                m_sspstat->put_value(m_sspstat->value.get() & ~_SSPSTAT::RW);
                m_sspmod->set_sspif();
                set_idle();
                m_sspmod->setSCL(false);
            }
        }
        else    m_sspmod->setSCL(true);
        break;

    case CLK_RX_BYTE:
        if(m_sspmod->get_SCL_State())
        {
            rx_byte();
            m_sspmod->setSCL(false);
        }
        else
                m_sspmod->setSCL(true);
        break;


    default:
        cout << "I2C::callback unxpected i2c_state=" << dec << i2c_state << endl;
        break;

    }
}

void I2C::clock( bool clock_state )
{
    uint sspcon_val = m_sspcon->value.get();
    uint sspstat_val = m_sspstat->value.get();

    if ((sspcon_val & _SSPCON::SSPM_mask) == _SSPCON::SSPM_MSSPI2Cmaster)
        return;

    if (clock_state)        // Do read on clock high transition
    {
        switch(i2c_state)
        {
          case CLK_STOP:
            if (phase == 1)
                setBRG();
            break;

          case CLK_ACKEN:
            if (phase == 1)
            {
                phase++;
                setBRG();
            }
            break;

/*RRR
        case CLK_RSTART:
            if (phase == 0)
            {
                    if (!m_sspmod->get_SDI_State())
                    {
                    if (verbose)
                        cout << "I2C::clock CLK_RSTART bus collision\n";
                    bus_collide();
                        m_sspmod->setSDA(true);
                }
                else
                {
                    clrBRG();
                    start_bit();
                }
            }
            else if (phase == 1)
            {
                setBRG();
            }
            break;
*/

          case RX_CMD:
          case RX_CMD2:
          case RX_DATA:
                if (bits_transfered < 8)
                {
                      m_SSPsr = (m_SSPsr << 1) | (m_sspmod->get_SDI_State()?1:0);
                    bits_transfered++;
                }
            break;

           case CLK_TX_BYTE:
           case CLK_RX_BYTE:
             setBRG();
             break;

           default:
             break;
        }
    }
    else        // Do writes of clock low transition
    {
        switch(i2c_state)
        {
        case CLK_ACKEN:
            clrBRG();
            if (phase)
            {
                m_sspmod->setSCL(false);
                m_sspcon2->value.put( m_sspcon2->value.get() & ~(_SSPCON2::ACKEN ));
                m_sspmod->set_sspif();
                set_idle();
            }
            break;

        case CLK_START:
        case CLK_RSTART:
            clrBRG();
            if (phase == 0 )
            {
                bus_collide();
            }
            else if (phase == 1)
            {
                m_sspcon2->value.put(m_sspcon2->value.get() & ~(_SSPCON2::SEN | _SSPCON2::RSEN));
            }
            break;

/* RRR
        case CLK_RSTART:
            if (phase == 0)
                m_sspmod->setSDA(true);
            break;
*/

        case RX_CMD:
        case RX_CMD2:
            if (bits_transfered == 8)
            {
                 if( ( m_SSPsr == 0 && (m_sspcon2->value.get() & _SSPCON2::GCEN) )
                      || match_address(m_SSPsr))
                {
                    I2Cproto(("got address sspsr=0x%2x\n", m_SSPsr));
                }
                else
                {
                    I2Cproto(("address not a match sspsr=0x%02x\n",m_SSPsr));
                    set_idle();
                    return;
                }
            }
            else if (bits_transfered == 9)
            {
                I2Cproto(("9 bits RXCMD\n"));
                if(end_ack())
                {
                    m_sspstat->put_value(sspstat_val & ~_SSPSTAT::DA);
                    slave_command();
                }
                return;
            }
            // Fall Through
        case RX_DATA:
            if (bits_transfered == 8)
            {
                I2Cproto(("RX_DATA 0x%02x\n", m_SSPsr&0xff));

                if (m_sspmod->SaveSSPsr(m_SSPsr & 0xff) ) // ACK ?
                {
                    m_sspmod->setSDA(false);
                }
                else
                {
                    m_sspmod->setSDA(true);
                }
                bits_transfered++;
            }
            else if (bits_transfered == 9)
            {
                end_ack();
                m_sspstat->put_value(sspstat_val | _SSPSTAT::DA);
            }
            break;

        case CLK_TX_BYTE:
        case CLK_RX_BYTE:
            setBRG();
            break;

        case TX_DATA:
            bits_transfered++;
            if (bits_transfered < 8)
            {
                m_SSPsr <<= 1;
                m_sspmod->setSDA((m_SSPsr & 0x80) == 0x80);
            }
            else if(bits_transfered == 8)
            {
                m_sspmod->setSDA(true);
                m_sspstat->put_value(sspstat_val & ~_SSPSTAT::BF);
            }
            else if(bits_transfered == 9)
            {
                m_sspmod->set_sspif();
                if (m_sspmod->get_SDI_State())        // NACK
                {
                    m_sspstat->put_value(sspstat_val & _SSPSTAT::BF);
                    set_idle();
                    return;
                }
                m_sspstat->put_value(sspstat_val | _SSPSTAT::DA);
                if (sspstat_val & _SSPSTAT::RW)
                {
                    sspcon_val &= ~ _SSPCON::CKP;
                    m_sspcon->put_value(sspcon_val);
                    m_sspmod->setSCL(false);
                }
            }
            break;

        default:
            break;
        }
    }
}

bool I2C::match_address(uint sspsr)
{
    uint mask = 0xfe;
    uint sspm = m_sspcon->value.get() & _SSPCON::SSPM_mask;
    bool slave_10 = (sspm == _SSPCON::SSPM_I2Cslave_10bitaddr) ||
                        (sspm == _SSPCON::SSPM_I2Cslave_10bitaddr_ints);

    if (slave_10)
    {
        uint ret = (sspsr & 0xff) ^ m_sspadd->get();
        if ((sspsr & 0xf9) == 0xf0)        // 1st byte 10 bit address
        {
            mask = 0x6;
        }
        else
        {
            mask =  (m_sspmod->sspmsk) ? m_sspmod->sspmsk->value.get() : 0xff;
        }

        ret &= mask;
        return !(bool)ret;
    }
    if (m_sspmod->sspmsk) mask &= m_sspmod->sspmsk->value.get();

    return !((sspsr ^ m_sspadd->get()) & mask);
}

void I2C_1::clock(bool clock_state)
{
  uint sspcon_val = m_sspcon->value.get();
  uint sspstat_val = m_sspstat->value.get();

  if ((sspcon_val & _SSPCON::SSPM_mask) == _SSPCON::SSPM_MSSPI2Cmaster)
        return;

    if (clock_state)        // Do read on clock high transition
    {
        switch(i2c_state)
        {
          case CLK_STOP:
            if (phase == 1) setBRG();
            break;

          case CLK_ACKEN:
            if (phase == 1)
            {
                phase++;
                setBRG();
            }
            break;

/* RRR
        case CLK_RSTART:
            if (phase == 0)
            {
                    if (!m_sspmod->get_SDI_State())
                    {
                    if (verbose)
                        cout << "I2c_1::clock CLK_RSTART bus collision\n";
                    bus_collide();
                        m_sspmod->setSDA(true);
                }
                else
                {
                    clrBRG();
                    start_bit();
                }
            }
            else if (phase == 1)
            {
                setBRG();
            }
            break;
*/


          case RX_CMD:
          case RX_CMD2:
          case RX_DATA:
                if (bits_transfered < 8)
                {
                      m_SSPsr = (m_SSPsr << 1) | (m_sspmod->get_SDI_State()?1:0);
                    bits_transfered++;
                }
                else if (bits_transfered == 9 &&
                    m_sspcon3->value.get() & (_SSP1CON3::AHEN | _SSP1CON3::DHEN))
                {
                    m_sspcon3->put(m_sspcon3->value.get() & ~_SSP1CON3::ACKTIM);
                }

            break;

            case TX_DATA:
                if (bits_transfered == 9 &&
                    m_sspcon3->value.get() & (_SSP1CON3::AHEN | _SSP1CON3::DHEN))
                {
                    m_sspcon3->put(m_sspcon3->value.get() & ~_SSP1CON3::ACKTIM);
                }
                break;

           case CLK_TX_BYTE:
           case CLK_RX_BYTE:
                setBRG();
                break;

           default:
                break;
        }
    }
    else        // Do writes of clock low transition
    {
        switch(i2c_state)
        {
        case CLK_ACKEN:
            clrBRG();
            if (phase)
            {
                m_sspmod->setSCL(false);
                m_sspcon2->value.put( m_sspcon2->value.get() & ~(_SSPCON2::ACKEN ));
                m_sspmod->set_sspif();
                set_idle();
            }
            break;

        case CLK_START:
        case CLK_RSTART:
            clrBRG();
            if (phase == 0 )
            {
                bus_collide();
            }
            else if (phase == 1)
            {
                m_sspcon2->value.put(m_sspcon2->value.get() & ~(_SSPCON2::SEN | _SSPCON2::RSEN));
            }
            break;

/* RRR
        case CLK_RSTART:
            if (phase == 0)
                m_sspmod->setSDA(true);
            break;
*/

        case RX_CMD:
        case RX_CMD2:
            if (bits_transfered == 8)
            {
                if ( ( m_SSPsr == 0 && (m_sspcon2->value.get() & _SSPCON2::GCEN) )
                  || match_address(m_SSPsr))
                {
                }
                else
                {
                    cout << "READ_CMD address missmatch " << hex << m_SSPsr << " != " << m_sspadd->get() << endl;
                    set_idle();
                    return;
                }
            }
            else if (bits_transfered == 9)
            {
                if(end_ack())
                {
                    m_sspstat->put_value(sspstat_val & ~_SSPSTAT::DA);
                    slave_command();
                }
                return;
            }
            // Fall Through
        case RX_DATA:
            if (bits_transfered == 8)
            {
                if (m_sspcon->isI2CSlave(m_sspcon->value.get()) &&
                  (
                    (m_sspcon3->value.get() & _SSP1CON3::DHEN && i2c_state == RX_DATA)
                    ||
                    (m_sspcon3->value.get() & _SSP1CON3::AHEN &&
                        (i2c_state == RX_CMD || i2c_state == RX_CMD2))
                  ))
                {
                    uint sspcon3_val = m_sspcon3->value.get();

                    m_sspmod->SaveSSPsr(m_SSPsr & 0xff);
                    m_sspcon->value.put(m_sspcon->value.get() & ~_SSPCON::CKP);
                    m_sspcon3->value.put(sspcon3_val | _SSP1CON3::ACKTIM);
                    m_sspmod->setSCL(false);  // clock low
                    m_sspmod->set_sspif();
                }
                else if (m_sspmod->SaveSSPsr(m_SSPsr & 0xff) ) // ACK ?
                {
                    m_sspmod->setSDA(false);
                }
                else
                {
                    m_sspmod->setSDA(true);
                }
                bits_transfered++;
            }
            else if (bits_transfered == 9)
            {
                m_sspstat->put_value(sspstat_val | _SSPSTAT::DA);
                if(end_ack()
                   && m_sspmod->isI2CSlave()
                   && (m_sspcon2->value.get() & _SSPCON2::SEN))
                {
                        m_sspcon->put(m_sspcon->value.get() & ~_SSPCON::CKP);
                }
            }
            break;


        case CLK_TX_BYTE:
        case CLK_RX_BYTE:
            setBRG();
            break;

        case TX_DATA:
            bits_transfered++;
            if (bits_transfered < 8)
            {
                m_SSPsr <<= 1;
                m_sspmod->setSDA((m_SSPsr & 0x80) == 0x80);
            }
            else if(bits_transfered == 8)
            {
                m_sspmod->setSDA(true);
                    m_sspstat->put_value(sspstat_val & ~_SSPSTAT::BF);

                if(m_sspcon3->value.get() & _SSP1CON3::AHEN)
                {
                    m_sspcon3->value.put(m_sspcon3->value.get() | _SSP1CON3::ACKTIM);
                }
            }
            else if(bits_transfered == 9)
            {
                m_sspmod->set_sspif();
                    if (m_sspmod->get_SDI_State())        // NACK
                    {
                    m_sspcon2->put(m_sspcon2->value.get() | _SSPCON2::ACKSTAT);
                    m_sspstat->put_value(sspstat_val & _SSPSTAT::BF);
                    set_idle();
                    return;
                    }
                m_sspstat->put_value(sspstat_val | _SSPSTAT::DA);
                      if (sspstat_val & _SSPSTAT::RW)
                    {
                    m_sspcon2->put(m_sspcon2->value.get() & ~_SSPCON2::ACKSTAT);
                    sspcon_val &= ~ _SSPCON::CKP;
                    m_sspcon->put_value(sspcon_val);
                    m_sspmod->setSCL(false);
                    }
            }
            break;

        default:
            break;
        }
    }
}

void I2C::slave_command()
{
    uint sspcon_val = m_sspcon->value.get();
    uint sspstat_val = m_sspstat->value.get();

    if ( m_SSPsr == 0 && (m_sspcon2->value.get() & _SSPCON2::GCEN))
    {
        i2c_state = RX_DATA;
        I2Cproto(("slave_command i2c_state = RX_DATA\n"));
        return;
    }

    switch( sspcon_val & _SSPCON::SSPM_mask )
    {
        case _SSPCON::SSPM_I2Cslave_10bitaddr_ints:

        case _SSPCON::SSPM_I2Cslave_10bitaddr:
            if (i2c_state == RX_CMD && (m_SSPsr & 1))
            {
                sspstat_val |= _SSPSTAT::RW;
                i2c_state = TX_DATA;
                I2Cproto(("slave_command i2c_state = TX_DATA\n"));
                m_sspmod->setSCL(false);  // clock low
                sspcon_val &= ~ _SSPCON::CKP;
                m_sspcon->put_value(sspcon_val);
            }
            else
            {
                sspstat_val |= _SSPSTAT::UA;
                i2c_state = (i2c_state == RX_CMD2) ?
                RX_DATA : RX_CMD2;
                I2Cproto(("slave_command i2c_state = %s\n", i2c_state ==RX_DATA?"RX_DATA":"RX_CMD2"));
            }
            break;

        case _SSPCON::SSPM_I2Cslave_7bitaddr:

        case _SSPCON::SSPM_I2Cslave_7bitaddr_ints:
            if (i2c_state == RX_CMD && (m_SSPsr & 1))
            {
                sspstat_val |= _SSPSTAT::RW;
                sspstat_val &= ~_SSPSTAT::BF;
                i2c_state = TX_DATA;
                I2Cproto(("slave_command i2c_state = TX_DATA\n"));
                sspcon_val &= ~ _SSPCON::CKP;
                m_sspcon->put_value(sspcon_val);
                m_sspmod->setSCL(false);  // clock low
            }
            else
            {
                i2c_state = RX_DATA;
                I2Cproto(("slave_command i2c_state = RX_DATA\n"));
            }
            break;
    }
    m_sspstat->put_value(sspstat_val);
}

bool I2C::end_ack()
{
    m_sspmod->set_sspif();
    bits_transfered = 0;
    if (m_sspmod->get_SDI_State())      // NACK
    {
        I2Cproto(("end_ack NACK\n"));
        set_idle();
        return(false);
    }
    else
    {
        m_sspmod->setSDA(true);
        I2Cproto(("end_ack ACK\n"));
        return(true);
    }
}

void I2C::bus_collide()
{
   m_sspcon2->value.put(m_sspcon2->value.get() &
       ~ (_SSPCON2::SEN | _SSPCON2::RSEN | _SSPCON2::PEN |
          _SSPCON2::RCEN | _SSPCON2::ACKEN));
   m_sspmod->set_bclif();
   set_idle();

}
void I2C_1::bus_collide()
{
   if (m_sspmod->isI2CMaster())
   {
       m_sspcon2->value.put(m_sspcon2->value.get() &
           ~ (_SSPCON2::SEN | _SSPCON2::RSEN | _SSPCON2::PEN |
              _SSPCON2::RCEN | _SSPCON2::ACKEN));
       m_sspmod->set_bclif();
   }
   else if (m_sspmod->isI2CSlave() && (m_sspcon3->value.get() & _SSP1CON3::SBCDE))
       m_sspmod->set_bclif();

   set_idle();
}

void I2C::newSSPADD(uint newTxByte)
{
    uint sspstat_val = m_sspstat->value.get();

    if (sspstat_val & _SSPSTAT::UA)
    {
        m_sspstat->put_value(sspstat_val & ~_SSPSTAT::UA);
        m_sspmod->setSCL(true);                // turn off clock stretch
    }
}

void I2C::setBRG()
{
    if (future_cycle) cout << "ERROR I2C::setBRG called with future_cycle=" << future_cycle << endl;

      future_cycle = get_cycles().get() + ((m_sspadd->get() &0x7f)/ 4) + 1;
      get_cycles().set_break(future_cycle, this);
}

void I2C::clrBRG()
{
    if (future_cycle)
    {
        get_cycles().clear_break(this);
        future_cycle = 0;
    }
}
void I2C::newSSPBUF(uint newTxByte)
{
    if (!m_sspstat || !m_sspcon) return;

    uint sspstat_val = m_sspstat->value.get();
    uint sspcon_val = m_sspcon->value.get();

    if (m_sspcon2 && (sspcon_val &  _SSPCON::SSPM_mask) ==  _SSPCON::SSPM_MSSPI2Cmaster)
    {
        if (isIdle())
        {
            m_sspstat->put_value(sspstat_val | _SSPSTAT::BF | _SSPSTAT::RW);
            m_SSPsr = newTxByte;
            m_sspmod->setSDA((m_SSPsr & 0x80) == 0x80);
            bits_transfered = 0;
            phase = 0;
            i2c_state = CLK_TX_BYTE;
            I2Cproto(("%s i2c_state = CLK_TX_BYTE data %x\n", __FUNCTION__, newTxByte));
            setBRG();
        }
        else // Collision
        {
            cout << "I2C::newSSPBUF I2C not idle on write data=" << hex << newTxByte << endl;

            m_sspcon->setWCOL();
        }
    }
    else
    {
        if (sspstat_val & _SSPSTAT::RW)
        {
            if (!(sspstat_val & _SSPSTAT::BF))
            {
                m_SSPsr = newTxByte;
                m_sspstat->put_value(sspstat_val | _SSPSTAT::BF);
                m_sspmod->setSDA((m_SSPsr & 0x80) == 0x80);
                bits_transfered = 0;
                I2Cproto(("%s TX 0x%x\n", __FUNCTION__, newTxByte));
            }
            else // Collision
            {
                cout << "I2C::newSSPBUF I2C not idle on write data=" << hex << newTxByte << endl;
                m_sspcon->setWCOL();
            }
        }
        else cout << "I2C::newSSPBUF write SSPSTAT::RW not set\n";
    }
}
void I2C::sda(bool data_val)
{
    if ((i2c_state == CLK_TX_BYTE) || (i2c_state == CLK_RX_BYTE) ||
        (i2c_state == CLK_ACKEN) || (i2c_state == CLK_RSTART))
    {
        return;
    }
    if (m_sspmod->get_SCL_State())        // Clock is high
    {
            uint stat_val = m_sspstat->value.get();
        uint sspm = (m_sspcon->value.get() & _SSPCON::SSPM_mask);
        if (data_val)        // Data going high - STOP
        {
            if (do_stop_sspif()) m_sspmod->set_sspif();
                stat_val = (stat_val & _SSPSTAT::BF) | _SSPSTAT::P;

            if (!future_cycle && i2c_state != eIDLE) set_idle();

            I2Cproto(("I2C::sda got STOP i2c_state=%d\n", i2c_state));
         }
        else                // Data going low - START
        {
            I2Cproto(("I2C::sda got START i2c_state=%d\n", i2c_state));
            switch (i2c_state)
            {
            case CLK_STOP:
                break;

            case CLK_START:
                if (phase == 0)
                {
                    uint64_t fc = get_cycles().get() + ((m_sspadd->get() &0x7f)/ 2) + 1;

                    if (future_cycle)
                    {
                        phase++;
                        get_cycles().reassign_break(future_cycle, fc, this);
                        future_cycle = fc;
                    }
                    else
                    {
                        get_cycles().set_break(fc, this);
                        future_cycle = fc;
                    }
                }
                break;

            default:
                I2Cproto(("%s i2c_state was %d now RX_CMD\n", __FUNCTION__, i2c_state));
                i2c_state = RX_CMD;
                break;
            }
            stat_val = (stat_val & _SSPSTAT::BF) | _SSPSTAT::S;
            bits_transfered = 0;
            m_SSPsr = 0;
        }
        m_sspstat->put_value(stat_val);

        // interrupt ?
        if (sspm == _SSPCON::SSPM_I2Cslave_7bitaddr_ints ||
            sspm == _SSPCON::SSPM_I2Cslave_10bitaddr_ints)
        {
            m_sspmod->set_sspif();
        }
    }
    else        // clock low
    {
        if (i2c_state == CLK_STOP)
        {
//            setBRG();
        }
    }
}

bool I2C::do_stop_sspif()
{
    return (false);
}

bool I2C_1::do_stop_sspif()
{
    return ( (m_sspcon3->value.get() &_SSP1CON3::PCIE)); 
}
/*
        master mode, begin reading a byte
*/
void I2C::master_rx()
{
//    m_sspmod->setSCL(false);
    m_sspmod->setSDA(true);        // SDA controlled by slave
    bits_transfered = 0;
    m_SSPsr = 0;
    i2c_state = CLK_RX_BYTE;
    setBRG();
}
/*
        master, begin start sequence
                SCL and SDA must be high, then force SDA low
*/
void I2C::start_bit()
{
    if (m_sspmod->get_SCL_State() && m_sspmod->get_SDI_State())
    {

        i2c_state = CLK_START;
        I2Cproto(("%s i2c_state = CLK_START\n", __FUNCTION__));
        phase = 0;
        setBRG();
    }
    else
    {
        I2Cproto(("%s i2c_state = CLK_START bus_collide\n", __FUNCTION__));
        bus_collide();
    }
}
/*
        Master mode, begin rstart sequence
                bring SDA and SCL high, then SDA low with SCL high (start)
*/
void I2C::rstart_bit()
{
    i2c_state = CLK_RSTART;
    I2Cproto(("%s i2c_state = CLK_RSTART\n", __FUNCTION__));
    phase = 0;
    setBRG();
    m_sspmod->setSDA(false);
/*RRR
    m_sspmod->setSCL(false);

    if (!m_sspmod->get_SCL_State())
    {
        setBRG();
        m_sspmod->setSDA(true);
    }
    else
        bus_collide();
*/

}
/*
        master, begin stop sequence
                drop SDA (might cause start if SCL high)
                when SCL high, raise SDA (stop condition)

*/
void I2C::stop_bit()
{
    i2c_state = CLK_STOP;
    I2Cproto(("%s i2c_state = CLK_STOP\n", __FUNCTION__));
    phase = 0;

    // Make sure SDA is low
    m_sspmod->setSDA(false);

    if (!m_sspmod->get_SDI_State())
    {
        setBRG();
    }
    else bus_collide();
}

/*
        master, begin ack sequence
                clock SCL low, set SDA as per ACKDT,
                clock SCL high
*/
void I2C::ack_bit()
{
    i2c_state = CLK_ACKEN;
    I2Cproto(("%s i2c_state = CLK_ACKEN\n", __FUNCTION__));
    phase = 0;
    m_sspmod->setSCL(false);
    if (!m_sspmod->get_SCL_State())
    {
        setBRG();
        m_sspmod->setSDA((m_sspcon2->value.get() & _SSPCON2::ACKDT) ? true : false);
    }
    else bus_collide();
}

void SSP_MODULE::initialize(
                        PIR_SET *ps,
                        PinModule *SckPin,
                        PinModule *SsPin,
                        PinModule *SdoPin,
                        PinModule *SdiPin,
                         PicTrisRegister *_i2ctris,
                        SSP_TYPE _ssptype
                )
{
  m_pirset = ps;
  m_sck = SckPin;
  m_ss = SsPin;
  m_sdo = SdoPin;
  m_sdi = SdiPin;
  m_i2c_tris = _i2ctris;
  m_ssptype = _ssptype;
  if (! m_spi)
  {
    m_spi = new SPI(this, &sspcon, &sspstat, &sspbuf);
    m_i2c = new I2C(this, &sspcon, &sspstat, &sspbuf, &sspcon2, &sspadd);
    m_SDI_Sink = new SDI_SignalSink(this);
    m_SCL_Sink = new SCL_SignalSink(this);
    m_SS_Sink = new SS_SignalSink(this);
    m_SckSource = new SCK_SignalSource(this, m_sck);
    m_SdoSource = new SDO_SignalSource(this, m_sdo);
    m_SdiSource = new SDI_SignalSource(this, m_sdi);
  }
}
// this allows backward compatibility
void SSP_MODULE::set_sspif()
{
    if (m_ssp_if) m_ssp_if->Trigger();
    else m_pirset->set_sspif();
}
// this allows backward compatibility
void SSP_MODULE::set_bclif()
{
    if (m_bcl_if) m_bcl_if->Trigger();
    else m_pirset->set_bclif();
}
void SSP_MODULE::ckpSPI(uint value)
{
    if(m_spi && !m_spi->isIdle())
      cout << "SPI: You just changed CKP in the middle of a transfer." << endl;

    switch( value & _SSPCON::SSPM_mask ) {
    case _SSPCON::SSPM_SPImaster4:
    case _SSPCON::SSPM_SPImaster16:
    case _SSPCON::SSPM_SPImaster64:
    case _SSPCON::SSPM_SPImasterAdd:
      if (m_SckSource) m_SckSource->putState( (value & _SSPCON::CKP) ? '1' : '0' );
      break;
    case _SSPCON::SSPM_SPImasterTMR2:
      break;
    }
}

/*
        drive SCL by changing pin direction (with data low)
*/
void SSP_MODULE::setSCL(bool direction)
{
    if (!m_sck || !m_i2c_tris)
        return;

    uint pin = m_sck->getPinNumber();
    uint tris_val = m_i2c_tris->get_value();
    if (!direction)
        tris_val &= ~(1<<pin);
    else
        tris_val |= (1<<pin);

    m_i2c_tris->put(tris_val);
}
/*
        drive SDA by changing pin direction (with data low)
*/
void SSP_MODULE::setSDA( bool direction )
{
    uint pin = m_sdi->getPinNumber();
    uint tris_val = m_i2c_tris->get_value();

    if (!direction) tris_val &= ~(1<<pin);
    else            tris_val |= (1<<pin);

    m_i2c_tris->put(tris_val);
}
/*
        deactivate SPI and I2C mode
*/
void SSP_MODULE::stopSSP(uint old_value)
{
    if (sspcon.isSPIActive(old_value))
    {
        m_spi->stop_transfer();
        m_sck->setSource(0);
        m_sdo->setSource(0);
    }
    else if (sspcon.isI2CActive(old_value))
    {
        m_i2c->set_idle();
        m_sck->setSource(0);
        m_sdi->setSource(0);
        m_sck_active = false;
        m_sdi_active = false;
    }
}
void SSP_MODULE::putStateSDO(char _state)
{
    m_SdoSource->putState(_state);
}
void SSP_MODULE::putStateSCK(char _state)
{
    m_SckSource->putState(_state);
}
/*
        activate SPI module
*/
void SSP_MODULE::startSSP(uint value)
{
    Dprintf(("SSP_MODULE cmd %x\n", value &  _SSPCON::SSPM_mask ));
    SPIproto(("SSP_MODULE cmd %x\n", value &  _SSPCON::SSPM_mask ));
    sspbuf.setFullFlag(false);
    if (! m_sink_set)
    {
        if (m_sdi)
        {
           m_sdi->addSink(m_SDI_Sink);
           m_SDI_State = m_sdi->getPin().getState();
        }
        if (m_sck)
        {
           m_sck->addSink(m_SCL_Sink);
           m_SCL_State = m_sck->getPin().getState();
        }
        if (m_ss)
        {
           m_ss->addSink(m_SS_Sink);
           m_SS_State = m_ss->getPin().getState();
        }
        m_sink_set = true;
    }
    switch( value & _SSPCON::SSPM_mask ) 
    {
    case _SSPCON::SSPM_SPImasterTMR2:
    case _SSPCON::SSPM_SPImaster4:
    case _SSPCON::SSPM_SPImaster16:
    case _SSPCON::SSPM_SPImaster64:
    case _SSPCON::SSPM_SPImasterAdd:
    Dprintf(("SSP_MODULE case cmd %x\n", value &  _SSPCON::SSPM_mask ));
          if (m_sck)
        {
            m_sck->setSource(m_SckSource);
            m_sck_active = true;
        }
          if (m_sdo)
        {
             m_sdo->setSource(m_SdoSource);
             m_sdo_active = true;
        }
        if (m_SckSource) m_SckSource->putState( (value & _SSPCON::CKP) ? '1' : '0' );
        if (m_SdoSource) m_SdoSource->putState('0'); // BUG, required to put SDO in know state
        break;

    case _SSPCON::SSPM_SPIslave:
    case _SSPCON::SSPM_SPIslaveSS:
          if (m_sdo)
        {
             m_sdo->setSource(m_SdoSource);
             m_sdo_active = true;
        }
        if (m_SdoSource) m_SdoSource->putState('0'); // BUG, required to put SDO in know state
        newSSPBUF(sspbuf.get_value());
        break;

      case _SSPCON::SSPM_I2Cslave_7bitaddr:
      case _SSPCON::SSPM_I2Cslave_10bitaddr:
      case _SSPCON::SSPM_MSSPI2Cmaster:
      case _SSPCON::SSPM_I2Cfirmwaremaster:
      case _SSPCON::SSPM_I2Cslave_7bitaddr_ints:
      case _SSPCON::SSPM_I2Cslave_10bitaddr_ints:

        m_i2c->set_idle();
        m_sck->setSource(m_SckSource);
        m_sdi->setSource(m_SdiSource);
        m_sck_active = true;
        m_sdi_active = true;
        m_sck->refreshPinOnUpdate(true);
        m_sdi->refreshPinOnUpdate(true);
        m_SdiSource->putState('0');
        m_SckSource->putState('0');
        m_sck->refreshPinOnUpdate(false);
        m_sdi->refreshPinOnUpdate(false);
        break;

      case _SSPCON::SSPM_LoadMaskFunction:
        break;

    default:
        cout << "SSP: start, unexpected SSPM select bits SSPM="
                << hex << (value & _SSPCON::SSPM_mask) << endl;;
        break;
    }
}
/*
        process mode change or clock edge due to write to SSPCON
*/
void SSP_MODULE::changeSSP(uint new_value, uint old_value)
{
    uint diff = new_value ^ old_value;

    if (diff & _SSPCON::SSPM_mask)        // mode changed
    {
        Dprintf(("SSP_MODULE stop %x start %x\n", old_value, new_value));
        stopSSP(old_value);
        startSSP(new_value);
    }
    else if (diff & _SSPCON::CKP)
    {
        if (sspcon.isSPIActive(new_value))
            ckpSPI(new_value);
        else if (sspcon.isI2CActive(new_value) && new_value & _SSPCON::CKP)
        {
                setSCL(true);
        }
    }
}

void SSP_MODULE::releaseSDIpin()
{
    m_sdi_active = false;
}

void SSP_MODULE::releaseSDOpin()
{
    m_sdo_active = false;
}

void SSP_MODULE::releaseSCKpin()
{
    m_sck_active = false;
}

void SSP_MODULE::releaseSCLpin()
{
    if (m_sck)
    {
        m_sck->setSource(0);
        m_sck_active = false;
    }
    m_sck = 0;
}

void SSP_MODULE::releaseSSpin()
{
    if (m_SS_Sink)
    {
        delete m_SS_Sink;
        m_SS_Sink = 0;
    }
    m_ss = 0;
}

void SSP_MODULE::Sck_toggle() { m_SckSource->toggle();}
/*
        process mode change or clock edge due to write to SSPCON
*/
void SSP1_MODULE::changeSSP(uint new_value, uint old_value)
{
    uint diff = new_value ^ old_value;

    if (diff & _SSPCON::SSPM_mask)        // mode changed
    {
        Dprintf(("SSP_MODULE stop %x start %x\n", old_value, new_value));
        stopSSP(old_value);
        startSSP(new_value);
    }
    else if (diff & _SSPCON::CKP)
    {
        if (sspcon.isSPIActive(new_value))
            ckpSPI(new_value);
        else if (sspcon.isI2CActive(new_value) && (new_value & _SSPCON::CKP))
        {
            if(ssp1con3.value.get() & (_SSP1CON3::AHEN | _SSP1CON3::DHEN))
            {
                // set ack(yes = 0) if writing and release clock
                if ((sspstat.value.get() & _SSPSTAT::RW) == 0)
                    setSDA(sspcon2.value.get() & _SSPCON2::ACKDT);
                setSCL(true);
            }
            else // RRRif(sspcon2.value.get() & _SSPCON2::SEN)
            {
                // release clock
                setSCL(true);
            }
        }
    }
}

//------------------------------------------------------------
// Called whenever the SDI/SDA input changes states.
//
void SSP_MODULE::SDI_SinkState(char new3State)
{
  bool new_SDI_State = (new3State == '1' || new3State == 'W');

  if (new_SDI_State == m_SDI_State)
        return;

  m_SDI_State = new_SDI_State;

  if(sspcon.isI2CActive(sspcon.value.get()))
  {
      if(m_i2c) m_i2c->sda(m_SDI_State);
   }
}

// Called when the SCK/SDI input changes state
void SSP_MODULE::SCL_SinkState(char new3State)
{
  bool new_SCL_State = (new3State == '1' || new3State == 'W');

  SPIproto(("SCL_SinkState new %d old %d enabled %d m_SS_State %d\n", new_SCL_State, m_SCL_State, sspcon.isSSPEnabled(), m_SS_State));

  if (new_SCL_State == m_SCL_State) return;

  m_SCL_State = new_SCL_State;

  if (!sspcon.isSSPEnabled() ) return;

  switch( sspcon.value.get() & _SSPCON::SSPM_mask )
  {
      case _SSPCON::SSPM_SPIslaveSS:
      /*
             SS high during transfer for BSSP, suspends transfers which
        continues when SS goes low.

        None BSSP interfaces handled when SS goes high
      */
        if (m_SS_State)
            return;        // suspend transfer
                        // Fall through
      case _SSPCON::SSPM_SPIslave:
        if (m_spi) m_spi->clock(m_SCL_State);
        break;

      case _SSPCON::SSPM_I2Cslave_7bitaddr:
      case _SSPCON::SSPM_I2Cslave_10bitaddr:
      case _SSPCON::SSPM_MSSPI2Cmaster:
      case _SSPCON::SSPM_I2Cfirmwaremaster:
      case _SSPCON::SSPM_I2Cslave_7bitaddr_ints:
      case _SSPCON::SSPM_I2Cslave_10bitaddr_ints:
        m_i2c->clock(m_SCL_State);
  }
}
/*
        on write to SSPBUF, pass on to either SPI or I2C if active
*/
void SSP_MODULE::newSSPBUF(uint value)
{
  if (!m_spi)
  {
        cout << "Warning bug, SPI initialization error " << __FILE__ << ":" << dec << __LINE__<<endl;
        return;
  }
  if (!m_i2c)
  {
        cout << "Warning bug, I2C initialization error " << __FILE__ << ":" << dec << __LINE__<<endl;
        return;
  }
  Dprintf(("SSP_MODULE sspcon %x SPIActive %d\n", sspcon.value.get(), sspcon.isSPIActive(sspcon.value.get())));
  if(sspcon.isSPIActive(sspcon.value.get()))
       m_spi->newSSPBUF(value);
  else if(sspcon.isI2CActive(sspcon.value.get()))
       m_i2c->newSSPBUF(value);
}

/*
        on write to SSPADD, pass onto I2C if active
*/
void SSP_MODULE::newSSPADD(uint value)
{
  if(sspcon.isI2CActive(sspcon.value.get()))
       m_i2c->newSSPADD(value);
}
// clear BF flag

void SSP_MODULE::rdSSPBUF()
{
    sspstat.put_value(sspstat.value.get() & ~_SSPSTAT::BF);
}

void SSP_MODULE::SS_SinkState(char new3State)
{
  m_SS_State = (new3State == '1' || new3State == 'W');

  // If SS goes high in the middle of an SPI transfer while in slave_SS mode,
  // transfer is aborted unless BSSP which streches the clocking

#ifdef SPI_PROTO

  if (sspcon.isSSPEnabled() &&
        ((sspcon.value.get() & _SSPCON::SSPM_mask) == _SSPCON::SSPM_SPIslaveSS)
     )
  {
        SPIproto(("SS State change to %d\n", m_SS_State));
  }
#endif
  if (!sspcon.isSSPEnabled() ||
        ! m_SS_State ||
        (sspcon.value.get() & _SSPCON::SSPM_mask) != _SSPCON::SSPM_SPIslaveSS ||
        ! m_spi->isIdle() ||
        ssp_type() == SSP_TYPE_BSSP)
     return;

  m_spi->stop_transfer();
}

void SSP_MODULE::tmr2_clock()
{
  uint sspcon_val = sspcon.value.get();
  if (! (sspcon_val & _SSPCON::SSPEN) ||
     ((sspcon_val & _SSPCON::SSPM_mask) != _SSPCON::SSPM_SPImasterTMR2) ||
     (m_spi && m_spi->isIdle()))
        return;

    Sck_toggle();
    if (m_spi) m_spi->clock( get_SCL_State() );
}
/*
        on write to SSPCON2 select master operation to initiate
*/
void SSP_MODULE::newSSPCON2(uint value)
{
    if (!m_i2c) return;

    if     ( value & _SSPCON2::SEN )   m_i2c->start_bit();
    else if( value & _SSPCON2::RSEN )  m_i2c->rstart_bit();
    else if( value & _SSPCON2::PEN )   m_i2c->stop_bit();
    else if( value & _SSPCON2::RCEN )  m_i2c->master_rx();
    else if( value & _SSPCON2::ACKEN ) m_i2c->ack_bit();
}
/*
        Process a received data byte
            if BF == 0 and SSPOV == 0 return true otherwise false
            if BF == 0 transfer data to SSPBUF and set BF
            if BF == 1 set SSPOV
            set SSPIF
*/
bool SSP_MODULE::SaveSSPsr(uint value)
{
    bool ret = false;
    uint stat_val = sspstat.value.get();
    uint con_val = sspcon.value.get();

    if ((stat_val & _SSPSTAT::BF) == 0)
    {
        sspbuf.put_value(value);
        sspstat.put_value(stat_val | _SSPSTAT::BF);
        if ((con_val & _SSPCON::SSPOV) == 0)
            ret = true;
    }
    else
    {
        sspcon.put_value(con_val | _SSPCON::SSPOV);
        cout << "SSP receive overflow\n";
    }

    return(ret);
}
/*
        Process a received data byte
            if BF == 0 and SSPOV == 0 return true otherwise false
            if BF == 0 transfer data to SSPBUF and set BF
            if BF == 1 set SSPOV
            set SSPIF
*/
bool SSP1_MODULE::SaveSSPsr(uint value)
{
        bool ret = false;
        uint stat_val = sspstat.value.get();
        uint con_val = sspcon.value.get();

        if ((stat_val & _SSPSTAT::BF) == 0)
        {
            if ((con_val & _SSPCON::SSPOV) == 0 ||
                (isI2CSlave() && ssp1con3.value.get() & _SSP1CON3::BOEN))
            {
                sspstat.put_value(stat_val | _SSPSTAT::BF);
                sspbuf.put_value(value);
                ret = true;
            }
        }
        else
        {
            sspcon.put_value(con_val | _SSPCON::SSPOV);
            cout << "SSP receive overflow\n";
        }

        return(ret);
}
//-----------------------------------------------------------
//-------------------------------------------------------------------
_SSPCON2::_SSPCON2(Processor *pCpu, SSP_MODULE *pSSP)
  : sfr_register(pCpu, "sspcon2","Synchronous Serial Port Control"),
    m_sspmod(pSSP)
{
}

/*
        write to SSPCON2 without processing data
*/
void  _SSPCON2::put_value(uint new_value)
{
  value.put(new_value);
}
/*
    If a command is currently active,
        lower 5 bits of register cannot be changed
    if no command is currently active,
        activate command and write data
*/
void _SSPCON2::put(uint new_value)
{
    uint old_value = value.get();
    uint diff = old_value ^ new_value;
    uint mask = (ACKEN|RCEN|PEN|RSEN|SEN);
    uint old_active;

    //Allow SEN to change unless I2CMaster (required for slave)
    if (!m_sspmod->isI2CMaster()) mask &= ~SEN;

    old_active = old_value & mask;

    if (!diff) return;        // nothing to do

    // if I2C not idle, do not change bits in mask
    if (!m_sspmod->isI2CIdle() && (diff & mask))
    {
        cout << "Warrning SSPCON::put I2C not idle and new value "
                << hex << new_value << " changes one of following bits "
                << mask << endl;

        put_value((new_value & ~mask) | old_active);
    }
    // Master and only a new command bit to process
    else if (!old_active && m_sspmod->isI2CMaster())
    {
        switch (new_value & (ACKEN|RCEN|PEN|RSEN|SEN))
        {
        case ACKEN:
        case RCEN:
        case PEN:
        case RSEN:
        case SEN:
            put_value(new_value);
            m_sspmod->newSSPCON2(new_value);
            break;

        case 0:        // just write value
            put_value(new_value);
            break;

        default:
            cout << "SSPCON2 cannot select more than one function at a time\n";
            break;
         }
    }
    else put_value(new_value);
}

//-----------------------------------------------------------
//-------------------------------------------------------------------
_SSP1CON3::_SSP1CON3(Processor *pCpu, SSP1_MODULE *pSSP)
  : sfr_register(pCpu, "ssp1con3","Synchronous Serial Port Control 3"),
    m_sspmod(pSSP)
{
}

/*
        write to SSP1CON3 without processing data
*/
void  _SSP1CON3::put_value( uint new_value )
{
    value.put(new_value);
}

void _SSP1CON3::put( uint new_value )
{
    put_value( new_value & ~ACKTIM ); // ACKTIM not writable by user
}

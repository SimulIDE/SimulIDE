/*
   Copyright (C) 1998,1999 Scott Dattalo
   Copyright (C) 2014        Roy R. Rankin

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
#include "stimuli.h"
#include "uart.h"
#include "14bit-processors.h"
#include "14bit-tmrs.h"
#include "baseprocessor.h"

#define p_cpu ((Processor *)cpu)

// defining EUSART_PIN  causes TX pin direction to be set for EUSART devices
//#define EUSART_PIN
//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//--------------------------------------------------
//
//--------------------------------------------------
// Drive output for TX pin
class TXSignalSource : public SignalControl
{
    public:
      TXSignalSource(_TXSTA *_txsta) : m_txsta(_txsta)
      {
        assert(m_txsta);
      }
      ~TXSignalSource() { }
      
      virtual char getState() { return m_txsta->getState(); }
      virtual void release() { m_txsta->releasePin(); }
      
    private:
      _TXSTA *m_txsta;
};

// Set TX pin to output
class TXSignalControl : public SignalControl
{
    public:
      TXSignalControl(_TXSTA *_txsta) : m_txsta(_txsta) { }
      ~TXSignalControl() { }
      
      virtual char getState() { return '0'; }
      virtual void release() { m_txsta->releasePin(); }
      
    private:
      _TXSTA *m_txsta;
};

// set Synchronous DT pin direction
class RCSignalControl : public SignalControl
{
    public:
      RCSignalControl(_RCSTA *_rcsta) : m_rcsta(_rcsta) { }
      ~RCSignalControl() { }
      
      virtual char getState() { return '0'; }
      //virtual char getState() { return m_rcsta->getDir(); }
      virtual void release()
      {
            m_rcsta->releasePin();
      }
      
    private:
      _RCSTA *m_rcsta;
};

// Drive date of DT  pin when transmitting
class RCSignalSource : public SignalControl
{
    public:
      RCSignalSource(_RCSTA *_rcsta) : m_rcsta(_rcsta)
      {
        assert(m_rcsta);
      }
      ~RCSignalSource() { }
      virtual char getState()
      {
        return m_rcsta->getState();
      }
      virtual void release()
      {
        m_rcsta->releasePin();
      }
      
    private:
      _RCSTA *m_rcsta;
};

//--------------------------------------------------
//
//--------------------------------------------------

// Report state changes on incoming RX pin
class RXSignalSink : public SignalSink
{
    public:
      RXSignalSink(_RCSTA *_rcsta) : m_rcsta(_rcsta)
      {
        assert(_rcsta);
      }

      virtual void setSinkState(char new3State) { m_rcsta->setState(new3State); }
      virtual void release() {delete this; }
      
    private:
      _RCSTA *m_rcsta;
};

//--------------------------------------------------
//
//--------------------------------------------------

// Report state changes on incoming Clock pin for Synchronous slave mode
class CLKSignalSink : public SignalSink
{
    public:
      CLKSignalSink(_RCSTA *_rcsta) : m_rcsta(_rcsta)
      { assert(_rcsta); }

      virtual void setSinkState(char new3State) { m_rcsta->clock_edge(new3State); }
    //  virtual void release() { delete this; }
      virtual void release() {delete this; }
      
    private:
      _RCSTA *m_rcsta;
};

//-----------------------------------------------------------
_RCSTA::_RCSTA(Processor *pCpu, const char *pName, const char *pDesc, USART_MODULE *pUSART)
  : sfr_register(pCpu, pName, pDesc),
    rcreg(0), spbrg(0),
    txsta(0), txreg(nullptr), sync_next_clock_edge_high(false),
    rsr(0), bit_count(0), rx_bit(0), sample(0),
    state(_RCSTA::RCSTA_DISABLED), sample_state(0),
    future_cycle(0), last_cycle(0),
    mUSART(pUSART),
    m_PinModule(0), m_sink(0), m_cRxState('?'),
    SourceActive(false), m_control(0), m_source(0), m_cTxState('\0'),
    m_DTdirection('0'), bInvertPin(false),
    old_clock_state(true)
{
  assert(mUSART);
  dataInQueue = false;
}

_RCSTA::~_RCSTA()
{
    if( SourceActive && m_PinModule )
    {
        m_PinModule->setSource(0);
        m_PinModule->setControl(0);
    }
    delete m_source;
    delete m_control;
}

//-----------------------------------------------------------
_TXSTA::_TXSTA(Processor *pCpu, const char *pName, const char *pDesc, USART_MODULE *pUSART)
  : sfr_register(pCpu, pName, pDesc), txreg(0), spbrg(0),
    mUSART(pUSART),
    m_PinModule(0),
    m_source(0),
    m_control(0),
    m_clkSink(0),
    SourceActive(false),
    m_cTxState('?'),
    bInvertPin(0)
{
  assert(mUSART);
}

_TXSTA::~_TXSTA()
{
    if (SourceActive && m_PinModule)
    {
        m_PinModule->setSource(0);
        m_PinModule->setControl(0);
    }

    if (m_control)
    {
        delete m_source;
        delete m_control;
    }
}

//-----------------------------------------------------------
_RCREG::_RCREG( Processor *pCpu, const char *pName, const char *pDesc, USART_MODULE *pUSART )
      : sfr_register(pCpu, pName, pDesc), fifo_sp(0),  mUSART(pUSART), m_rcsta(0)
{
  assert(mUSART);
}

_TXREG::_TXREG( Processor *pCpu, const char *pName, const char *pDesc, USART_MODULE *pUSART )
      : sfr_register( pCpu, pName, pDesc ), m_txsta(0), mUSART(pUSART)
{
    assert(mUSART);

    m_uartN = QString::fromStdString( name() ).remove("txreg").toInt();
    if( m_uartN > 0 ) m_uartN--;
    //qDebug() << "TXREG " << QString::fromStdString( name() )<<m_uartN;
}

_BAUDCON::_BAUDCON(Processor *pCpu, const char *pName, const char *pDesc)
        : sfr_register(pCpu, pName, pDesc)
{
}

_SPBRG::_SPBRG(Processor *pCpu, const char *pName, const char *pDesc)
      : sfr_register(pCpu, pName, pDesc),
        txsta(0), rcsta(0), brgh(0), baudcon(0), start_cycle(0), last_cycle(0),
        future_cycle(0), running(false), skip(0)
{
}

_SPBRGH::_SPBRGH(Processor *pCpu, const char *pName, const char *pDesc)
       : sfr_register(pCpu, pName, pDesc), m_spbrg(0)
{
}

//-----------------------------------------------------------
// TXREG - USART Transmit Register
//
// writing to this register causes the PIR1::TXIF bit to clear.
// my reading of the spec is this happens at the end of the next pic
// instruction. If the shift register is empty the bit will then be set
// one pic instruction later. Otherwise the bit is set when the shift
// register empties.  RRR 10/2014


void _TXREG::put( uint new_value )
{
    value.put(new_value & 0xff);
    if( ((Processor*)cpu)->isHalted() ) return; // Resetting: new_value=0 coming from reset registers.

    assert(m_txsta);
    assert(m_rcsta);

    // The transmit register has data, so clear the TXIF flag

    full = true;
    get_cycles().set_break(get_cycles().get() + 1, this);

    if(m_txsta->bTRMT() && m_txsta->bTXEN())// If the transmit buffer is empty and the transmitter is enabled
    {                                       // then transmit this new data now...
        get_cycles().set_break(get_cycles().get() + 2, this);

        if (m_txsta->bSYNC()) m_rcsta->sync_start_transmitting();
        else                  m_txsta->start_transmitting();
    }
    else if(m_txsta->bTRMT() && m_txsta->bSYNC())
    {
        m_txsta->value.put(m_txsta->value.get() & ~ _TXSTA::TRMT);
    }
    BaseProcessor::self()->uartOut( m_uartN, new_value );
}

void _TXREG::put_value( uint new_value )
{
  put(new_value);
  update();
}

void _TXREG::callback()
{
  Dprintf(("TXREG callback - time:%" PRINTF_GINT64_MODIFIER "x full %d\n",get_cycles().get(), full));
  if (full)
  {
    mUSART->full();
    full = false;
  }
  else mUSART->emptyTX();

}
void _TXREG::callback_print()
{
  cout << "TXREG " << name() << " CallBack ID " << CallBackID << '\n';
}

//-----------------------------------------------------------
// TXSTA - setIOpin - assign the I/O pin associated with the
// the transmitter.

void _TXSTA::setIOpin(PinModule *newPinModule)
{
  if (!m_source)
  {
      m_source = new TXSignalSource(this);
      m_control = new TXSignalControl(this);
  }
  else if (m_PinModule)        // disconnect old pin
  {
      disableTXPin();
  }
  m_PinModule = newPinModule;
  if(bTXEN() && rcsta->bSPEN())
  {
      enableTXPin();
  }
}

void _TXSTA::disableTXPin()
{
    if (m_PinModule)
    {
        m_PinModule->setSource(0);
        m_PinModule->setControl(0);

        if (m_clkSink)
        {
            m_PinModule->removeSink(m_clkSink);
            m_clkSink->release();
            m_clkSink = 0;
        }
    }
}
void _TXSTA::enableTXPin()
{
  char out;
  assert(m_PinModule);

  if (m_PinModule && !SourceActive)
  {
        if (bSYNC())
        {
            out = '0';
            if (!bCSRC())        // slave clock
            {
                if (!m_clkSink)
                {
                    m_clkSink = new CLKSignalSink(rcsta);
                    m_PinModule->addSink(m_clkSink);
                    rcsta->set_old_clock_state(m_PinModule->getPin().getState());
                }
                mUSART->emptyTX();
                return;
            }
        }
        else out = '1';

        m_PinModule->setSource(m_source);
#ifdef EUSART_PIN
        if(mUSART->IsEUSART())  m_PinModule->setControl(m_control);
#else
        m_PinModule->setControl(m_control);
#endif
        putTXState(out);
        SourceActive = true;
  }
  mUSART->emptyTX();
}

void _TXSTA::releasePin()
{
    if (m_PinModule && SourceActive)
    {
        m_PinModule->setControl(0);
        SourceActive = false;
    }
}
//-----------------------------------------------------------
// TXSTA - putTXState - update the state of the TX output pin
//

void _TXSTA::putTXState(char newTXState)
{
  m_cTxState = bInvertPin ? newTXState ^ 1 : newTXState;

  if (m_PinModule) m_PinModule->updatePinModule();
}

//-----------------------------------------------------------
// TXSTA - Transmit Register Status and Control

void _TXSTA::put_value(uint new_value)
{
  put(new_value);
  update();
}

void _TXSTA::put(uint new_value)
{
  uint old_value = value.get();

  if ( ! mUSART->IsEUSART() )  new_value &= ~SENDB;      // send break only supported on EUSART

  // The TRMT bit is controlled entirely by hardware.
  // It is high if the TSR has any data.

  //RRRvalue.put((new_value & ~TRMT) | ( (bit_count) ? 0 : TRMT));
  value.put((new_value & ~TRMT) | (old_value & TRMT));

  if( (old_value ^ value.get()) & TXEN) 
  {
    // The TXEN bit has changed states.
    //
    // If transmitter is being enabled and the transmit register
    // has some data that needs to be sent, then start a
    // transmission.
    // If the transmitter is being disabled, then abort any
    // transmission.

    if(value.get() & TXEN) 
    {
      if (rcsta->bSPEN()) 
      {
        if (bSYNC() && ! bTRMT() && !rcsta->bSREN() && !rcsta->bCREN())
        {
            // need to check bTRMT before calling enableTXPin
            enableTXPin();
            rcsta->sync_start_transmitting();
        }
        else enableTXPin();
      }
    } 
    else 
    {
      stop_transmitting();
      mUSART->full();         // Turn off TXIF
      disableTXPin();

    }
  }
}
//------------------------------------------------------------
//
char _TXSTA::getState()
{
  return m_cTxState;
}

// _TXSTA::stop_transmitting()
//
void _TXSTA::stop_transmitting()
{
  Dprintf(("stopping a USART transmission\n"));

  bit_count = 0;
  value.put(value.get() | TRMT);

  // It's not clear from the documentation as to what happens
  // to the TXIF when we are aborting a transmission. According
  // to the documentation, the TXIF is set when the TXEN bit
  // is set. In other words, when the Transmitter is enabled
  // the txreg is emptied (and hence TXIF set). But what happens
  // when TXEN is cleared? Should we clear TXIF too?
  //
  // There is one sentence that says when the transmitter is
  // disabled that the whole transmitter is reset. So I interpret
  // this to mean that the TXIF gets cleared. I could be wrong
  // (and I don't have a way to test it on a real device).
  //
  // Another interpretation is that TXIF retains it state
  // through changing TXEN. However, when SPEN (serial port) is
  // set then the whole usart is reinitialized and TXIF will
  // get set.
  //
  //  txreg->full();   // Clear TXIF
}

void _TXSTA::start_transmitting()
{
  Dprintf(("starting a USART transmission\n"));

  // Build the serial byte that's about to be transmitted.
  // I doubt the pic really does this way, but gpsim builds
  // the entire bit stream including the start bit, 8 data
  // bits, optional 9th data bit and the stop, and places
  // this into the tsr register. But since the contents of
  // the tsr are inaccessible, I guess we'll never know.
  //
  // (BTW, if you look carefully you may puzzle over why
  //  there appear to be 2 stop bits in the packet built
  //  below. Well, it's because the way gpsim implements
  //  the transmit logic. The second stop bit doesn't
  //  actually get transmitted - it merely causes the first
  //  stop bit to get transmitted before the TRMT bit is set.
  //
  //  RRR I believe the above paragraph is a mis-understanding
  //  The tsr register becomes empty, and the TRMT flag goes high,
  //  when we start to transmit the stop bit. Note that transmision
  //  is synchronous with the baud clock, so the start of transmision
  //  of a new character waits for the next callback. This delay maybe,
  //  in fact, the stop bit of the previous transmision,
  //
  //  [Recall that the TRMT bit indicates when the tsr
  //  {transmit shift register} is empty. It's not tied to
  //  an interrupt pin, so the pic application software
  //  most poll this bit.
  //
  //  RRR Also The following is wrong:
  //  This bit is set after the STOP
  //  bit is transmitted.] This is a cheap trick that saves
  //  one comparison in the callback code.)

  // The start bit, which is always low, occupies bit position
  // zero. The next 8 bits come from the txreg.
  assert(txreg);
  if(!txreg) return;

  if (value.get() & SENDB)
  {
      transmit_break();
      return;
  }
  tsr = txreg->value.get() << 1;

  // Is this a 9-bit data transmission?
  if(value.get() & TX9)
    {
      // Copy the stop bit and the 9th data bit to the tsr.
      // (See note above for the reason why two stop bits
      // are appended to the packet.)

      tsr |= ( (value.get() & TX9D) ? (3<<9) : (2<<9));
      bit_count = 11;  // 1 start, 9 data, 1 stop
    }
  else
    {
      // The stop bit is always high. (See note above
      // for the reason why two stop bits are appended to
      // the packet.)
      tsr |= (1<<9);
      bit_count = 10;  // 1 start, 8 data, 1 stop
    }

  // Set a callback breakpoint at the next SPBRG edge
  if(cpu) get_cycles().set_break(spbrg->get_cpu_cycle(1), this);

  // The TSR now has data, so clear the Transmit Shift
  // Register Status bit.
  value.put(value.get() & ~TRMT);
}

void _TXSTA::transmit_break()
{
  // A sync-break is 13 consecutive low bits and one stop bit. Use the
  // standard transmit logic to achieve this

  if(!txreg) return;

  tsr = 1<<13;

  bit_count = 14;  // 13 break, 1 stop

  // The TSR now has data, so clear the Transmit Shift
  // Register Status bit.
  value.put(value.get() & ~TRMT);

  callback();        // sent start bit
}

void _TXSTA::transmit_a_bit()
{
    if(bit_count) 
    {
        putTXState((tsr & 1) ? '1' : '0');
        tsr >>= 1;
        --bit_count;
    }
}

void _TXSTA::callback()
{

    transmit_a_bit();

  if(!bit_count) 
  {
    value.put(value.get() & ~SENDB);

    // tsr is empty.
    // If there is any more data in the TXREG, then move it to
    // the tsr and continue transmitting other wise set the TRMT bit

    // (See the note above about the 'extra' stop bit that was stuffed
    // into the tsr register.

    if(mUSART->bIsTXempty()) value.put(value.get() | TRMT);
    else
    {
      start_transmitting();
      mUSART->emptyTX();
    }
  } 
  else  
  {
    // bit_count is non zero which means there is still
    // data in the tsr that needs to be sent.

    if(cpu) get_cycles().set_break(spbrg->get_cpu_cycle(1),this);
  }
}

void _TXSTA::callback_print()
{
  cout << "TXSTA " << name() << " CallBack ID " << CallBackID << '\n';
}

//-----------------------------------------------------------
// Receiver portion of the USART
//-----------------------------------------------------------
//
// First RCSTA -- Receiver Control and Status
// The RCSTA class controls the usart reception. The PIC usarts have
// two modes: synchronous and asynchronous.
// Asynchronous reception:
//   Asynchronous reception means that there is no external clock
// available for telling the usart when to sample the data. Sampling
// timing is all based upon the PIC's oscillator. The SPBRG divides
// this clock down to a frequency that's appropriate to the data
// being received. (e.g. 9600 baud defines the rate at which data
// will be sent to the pic - 9600 bits per second.) The start bit,
// which is a high to low transition on the receive line, defines
// when the usart should start sampling the incoming data.
//   The pic usarts sample asynchronous data three times in "approximately
// the middle" of each bit. The data sheet is not exactly specific
// on what's the middle. Consequently, gpsim takes a stab/ educated
// guess on when these three samples are to be taken. Once the
// three samples are taken, then simple majority summing determines
// the sample e.g. if two out of three of the samples are high, then
// then the data bit is considered high.
//
//-----------------------------------------------------------
// RCSTA::put
//
void _RCSTA::put(uint new_value)
{
  uint diff;
  uint readonly = value.get() & (RX9D | OERR | FERR);

  diff = new_value ^ value.get();

  assert(txsta);
  assert(txsta->txreg);
  assert(rcreg);
  // If SPEN being turned off, clear all readonly bits
  if (diff & SPEN && !(new_value & SPEN))
  {
      readonly = 0;
      // clear receive stack (and rxif)
      rcreg->pop();
      rcreg->pop();
  }
  // if CREN is being cleared, make sure OERR is clear
  else if (diff & CREN && !(new_value & CREN))
      readonly &= (RX9D | FERR);
  value.put( readonly   |  (new_value & ~(RX9D | OERR | FERR)));

  if (!txsta->bSYNC()) // Asynchronous case
  {
      if (diff & (SPEN | CREN)) // RX status change
      {
          if ((value.get() & (SPEN | CREN)) == SPEN )
          {
              if (txsta->bTXEN()) txsta->enableTXPin();
              spbrg->start();
          }
          else if ((value.get() & (SPEN | CREN)) == (SPEN | CREN))
          {
              enableRCPin();
              if (txsta->bTXEN()) txsta->enableTXPin();
              spbrg->start();
              start_receiving();
              // If the rx line is low, then go ahead and start receiving now.
              if (m_cRxState == '0' || m_cRxState == 'w')
                  receive_start_bit();
              // Clear overrun error when turning on RX
              value.put( value.get() & (~OERR) );
          }
          else                 // RX off, check TX
          {
              stop_receiving();
              state = RCSTA_DISABLED;

              if (bSPEN())        // RX off but TX may still be active
              {
                  if (txsta->bTXEN())                 //TX output active
                      txsta->enableTXPin();
                  else                                // TX off
                      txsta->disableTXPin();
              }
              return;
          }
      }
  }
  else                                    // synchronous case
  {
      if (diff & RX9)
      {
          if (bRX9())
              bit_count = 9;
          else
              bit_count = 8;
      }
      if (diff & (SPEN | CREN | SREN )) // RX status change
      {
          // Synchronous transmit (SREN & CREN == 0)
          if ((value.get() & (SPEN | SREN | CREN)) == SPEN)
          {
              enableRCPin(DIR_OUT);
              if (txsta->bTXEN()) txsta->enableTXPin();
              return;
          }
          // Synchronous receive (SREN | CREN != 0)
          else if (value.get() & (SPEN))
          {
              enableRCPin(DIR_IN);
              txsta->enableTXPin();
              rsr = 0;
              if (bRX9())
                  bit_count = 9;
              else
                  bit_count = 8;
              if (txsta->bCSRC()) // Master mode
              {
                sync_next_clock_edge_high = true;
                txsta->putTXState('0');  // clock low
                callback();
              }

              return;
          }
          else        // turn off UART
          {
              if (m_PinModule)
              {
                  if (m_sink)
                  {
                      m_PinModule->removeSink(m_sink);
                      m_sink->release();
                      m_sink = 0;
                  }
              }
              txsta->disableTXPin();
          }
      }
  }
}

void _RCSTA::enableRCPin(char direction)
{
  if (m_PinModule)
  {
      if (txsta->bSYNC()) // Synchronous case
      {
          if (!m_source)
          {
              m_source = new RCSignalSource(this);
              m_control = new RCSignalControl(this);
          }
          if (direction == DIR_OUT)
          {
              m_DTdirection = '0';
              if (SourceActive == false)
              {
                  m_PinModule->setSource(m_source);
                  m_PinModule->setControl(m_control);
                  SourceActive = true;
              }
              putRCState('0');
          }
          else
          {
              m_DTdirection = '1';
              if (SourceActive == true)
              {
                  m_PinModule->setSource(0);
                  m_PinModule->setControl(0);
                  m_PinModule->updatePinModule();
              }
          }
      }
  }
}
void _RCSTA::disableRCPin()
{
}

void _RCSTA::releasePin()
{
    if (m_PinModule && SourceActive)
    {
        m_PinModule->setControl(0);
        SourceActive = false;
    }
}
void _RCSTA::put_value(uint new_value)
{
  put(new_value);
  update();
}

//-----------------------------------------------------------
// RCSTA - putRCState - update the state of the DTx output pin
//                      only used for Synchronous mode
//
void _RCSTA::putRCState(char newRCState)
{
  bInvertPin = mUSART->baudcon.rxdtp();
  m_cTxState = bInvertPin ? newRCState ^ 1 : newRCState;

  if (m_PinModule) m_PinModule->updatePinModule();
}

//-----------------------------------------------------------
// RCSTA - setIOpin - assign the I/O pin associated with the
// the receiver.

void _RCSTA::setIOpin(PinModule *newPinModule)
{
  if (m_sink) 
  {
    if (m_PinModule) m_PinModule->removeSink(m_sink);
  }
  else m_sink = new RXSignalSink(this);

  m_PinModule = newPinModule;
  if (m_PinModule)
  {
    m_PinModule->addSink(m_sink);
    old_clock_state = m_PinModule->getPin().getState();
  }
}

//-----------------------------------------------------------
// RCSTA - setState
// This gets called whenever there's a change detected on the RX pin.
// The usart is only interested in those changes when it is waiting
// for the start bit. Otherwise, the rcsta callback function will sample
// the rx pin (if we're receiving).


void _RCSTA::setState(char new_RxState)
{
  Dprintf((" %s setState:%c\n",name().c_str(), new_RxState));

  m_cRxState = new_RxState;

  if( (state == RCSTA_WAITING_FOR_START) && (m_cRxState =='0' || m_cRxState=='w'))
    receive_start_bit();

}
//  Transmit in synchronous mode
//
void _RCSTA::sync_start_transmitting()
{
    assert(txreg);

    rsr = txreg->value.get();
    if (txsta->bTX9())
    {
        rsr |= (txsta->bTX9D() << 8);
        bit_count = 9;
    }
    else
         bit_count = 8;
    txsta->value.put(txsta->value.get() & ~ _TXSTA::TRMT);
    if (txsta->bCSRC())
    {
        sync_next_clock_edge_high = true;
        txsta->putTXState('0');                // clock low
        callback();
    }
}

void _RCSTA::set_old_clock_state(char new3State)
{
    bool state = (new3State == '1' || new3State == 'W');
    state = mUSART->baudcon.txckp() ? !state : state;
    old_clock_state = state;
}

void _RCSTA::clock_edge(char new3State)
{
    bool state = (new3State == '1' || new3State == 'W');

    // invert clock, if requested
    state = mUSART->baudcon.txckp() ? !state : state;
    if (old_clock_state == state) return;
    old_clock_state = state;
    if (value.get() & SPEN)
    {
        // Transmitting ?
        if ((value.get() & ( CREN | SREN)) == 0)
        {
            if (state)        // clock high, output data
            {
                if (bit_count)
                {
                    putRCState((rsr & 1) ? '1' : '0');
                    rsr >>= 1;
                    bit_count--;
                }
            }
            else
            {
              if(mUSART->bIsTXempty())
              {
                   txsta->value.put(txsta->value.get() | _TXSTA::TRMT);
              }
              else
              {
                  sync_start_transmitting();
                  mUSART->emptyTX();
              }
            }
        }
        else                // receiving
        {
            if (!state) // read data as clock goes low
            {
                bool data = m_PinModule->getPin().getState();
                data = mUSART->baudcon.rxdtp() ? !data : data;

                if (bRX9()) rsr |= data << 9;
                else        rsr |= data << 8;

                rsr >>= 1;
                if (--bit_count == 0)
                {
                    rcreg->push(rsr);
                    if (bRX9()) bit_count = 9;
                    else        bit_count = 8;
                    rsr = 0;
                }
            }
        }
    }
}
//-----------------------------------------------------------
// RCSTA::receive_a_bit(uint bit)
//
// A new bit needs to be copied to the the Receive Shift Register.
// If the receiver is receiving data, then this routine will copy
// the incoming bit to the rsr. If this is the last bit, then a
// check will be made to see if we need to set up for the next
// serial byte.
// If this is not the last bit, then the receive state machine.

void _RCSTA::receive_a_bit(uint bit)
{
  // If we're waiting for the start bit and this isn't it then
  // we don't need to look any further
  Dprintf(("%s receive_a_bit state:%u bit:%u time:0x%" PRINTF_GINT64_MODIFIER "x\n",
    name().c_str(), state, bit, get_cycles().get()));

  if( state == RCSTA_MAYBE_START) 
  {
    if (bit) state = RCSTA_WAITING_FOR_START;
    else     state = RCSTA_RECEIVING;
    return;
  }
  if (bit_count == 0) 
  {
    // we should now have the stop bit
    if (bit) 
    {
      // got the stop bit
      // If the rxreg has data from a previous reception then
      // we have a receiver overrun error.
      // cout << "rcsta.rsr is full\n";

      if((value.get() & RX9) == 0) rsr >>= 1;
      
      value.put(value.get() & (~FERR) );     // Clear any framing error
      
      if(rcreg) rcreg->push( rsr & 0x1ff);   // copy the rsr to the fifo

      Dprintf(("%s _RCSTA::receive_a_bit received 0x%02X\n",name().c_str(), rsr & 0x1ff));
    } 
    else 
    {
      value.put(value.get() | FERR);       //no stop bit; framing error
      
      if(rcreg) rcreg->push( rsr & 0x1ff);   // copy the rsr to the fifo
    }
    // If we're continuously receiving, then set up for the next byte.
    // FIXME -- may want to set a half bit delay before re-starting...
    if(value.get() & CREN) start_receiving();
    else                   state = RCSTA_DISABLED;
    return;
  }
  
  if(bit) rsr |= 1<<9;   // Copy the bit into the Receive Shift Register

  //cout << "Receive bit #" << bit_count << ": " << (rsr&(1<<9)) << '\n';

  rsr >>= 1;
  bit_count--;
}

void _RCSTA::stop_receiving()
{
  rsr = 0;
  bit_count = 0;
  state = RCSTA_DISABLED;
}

void _RCSTA::start_receiving()
{
  Dprintf(("%s The USART is starting to receive data\n", name().c_str()));

  rsr = 0;
  sample = 0;

  // Is this a 9-bit data reception?
  bit_count = (value.get() & RX9) ? 9 : 8;

  state = RCSTA_WAITING_FOR_START;

}
void _RCSTA::overrun()
{
  value.put(value.get() | _RCSTA::OERR);
}

void _RCSTA::set_callback_break(uint spbrg_edge)
{
  uint time_to_event;

  if(cpu && spbrg)
  {
    time_to_event = ( spbrg->get_cycles_per_tick() * spbrg_edge ) / TOTAL_SAMPLE_STATES;
    get_cycles().set_break(get_cycles().get() + time_to_event, this);
  }
}
void _RCSTA::receive_start_bit()
{
  Dprintf(("%s USART received a start bit\n", name().c_str()));

  if((value.get() & (CREN | SREN)) == 0) {
    Dprintf(("  but not enabled\n"));
    return;
  }

  if(txsta && (txsta->value.get() & _TXSTA::BRGH))
    set_callback_break(BRGH_FIRST_MID_SAMPLE);
  else
    set_callback_break(BRGL_FIRST_MID_SAMPLE);

  sample = 0;
  sample_state = RCSTA_WAITING_MID1;
  state = RCSTA_MAYBE_START;
}

void _RCSTA::queueData( uint32_t value ) // Used by Simulide
{
    //qDebug()<< "_RCSTA::queueData"<<value;
    m_dataQueue.append( value );
    if( !dataInQueue ) set_callback_break( spbrg->get_cycles_per_tick()*9 );
    dataInQueue = true;
}

//------------------------------------------------------------
void _RCSTA::callback()
{
  Dprintf(("RCSTA callback. %s time:0x%" PRINTF_GINT64_MODIFIER "x\n", name().c_str(), get_cycles().get()));
  
    if( !m_dataQueue.isEmpty() )
    {
        rcreg->push( m_dataQueue.takeFirst() );
        
        if( !m_dataQueue.isEmpty() )  set_callback_break( spbrg->get_cycles_per_tick()*9 );
        else dataInQueue = false;

        return;
    }
    
  if (txsta->bSYNC())        // Synchronous mode RX/DT is data, TX/CK is clock
  {
      if (sync_next_clock_edge_high)        // + edge of clock
      {
          sync_next_clock_edge_high = false;
          txsta->putTXState('1');        // Clock high
          // Transmit
          if ((value.get() & (SPEN | SREN | CREN)) == SPEN)
          {
              if (bit_count)
              {
                  putRCState((rsr & 1) ? '1' : '0');
                  rsr >>= 1;
                  bit_count--;
              }
          }
      }
      else        // - clock edge
      {
          sync_next_clock_edge_high = true;
          txsta->putTXState('0');        //clock low
          // Receive Master mode
          if ((value.get() & (SPEN | SREN | CREN)) != SPEN)
          {

                if (value.get() & OERR)
                    return;
                bool data = m_PinModule->getPin().getState();
                data = mUSART->baudcon.rxdtp() ? !data : data;
                if (bRX9())
                    rsr |= data << 9;
                else
                    rsr |= data << 8;
                rsr >>= 1;
                if (--bit_count == 0)
                {
                    rcreg->push(rsr);
                    if (bRX9()) bit_count = 9;
                    else        bit_count = 8;
                    rsr = 0;
                    value.put(value.get() & ~SREN);
                    if ((value.get() & (SPEN | SREN | CREN)) == SPEN )
                    {
                         enableRCPin(DIR_OUT);
                        return;
                    }
                }
          }
          else                // Transmit, clock low
          {
              if (bit_count == 0 && !mUSART->bIsTXempty())
              {
                  sync_start_transmitting();
                  mUSART->emptyTX();
                  return;
              }
              else if(bit_count == 0 && mUSART->bIsTXempty())
              {
                   txsta->value.put(txsta->value.get() | _TXSTA::TRMT);
                   putRCState('0');
                   return;
              }
          }
      }
      if (cpu && (value.get() & SPEN))
      {
          future_cycle = get_cycles().get() + spbrg->get_cycles_per_tick();
          get_cycles().set_break(future_cycle, this);
      }
  }
  else
  {
      // A bit is sampled 3 times.
      switch(sample_state) 
      {
      case RCSTA_WAITING_MID1:
        if (m_cRxState == '1' || m_cRxState == 'W')
          sample++;

        if(txsta && (txsta->value.get() & _TXSTA::BRGH))
          set_callback_break(BRGH_SECOND_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE);
        else
          set_callback_break(BRGL_SECOND_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE);

        sample_state = RCSTA_WAITING_MID2;

        break;

      case RCSTA_WAITING_MID2:
        if (m_cRxState == '1' || m_cRxState == 'W')
          sample++;

        if(txsta && (txsta->value.get() & _TXSTA::BRGH))
          set_callback_break(BRGH_THIRD_MID_SAMPLE - BRGH_SECOND_MID_SAMPLE);
        else
          set_callback_break(BRGL_THIRD_MID_SAMPLE - BRGL_SECOND_MID_SAMPLE);

        sample_state = RCSTA_WAITING_MID3;

        break;

      case RCSTA_WAITING_MID3:
        if (m_cRxState == '1' || m_cRxState == 'W')
          sample++;

        receive_a_bit( (sample>=2));
        sample = 0;

        // If this wasn't the last bit then go ahead and set a break for the next bit.
        if(state==RCSTA_RECEIVING) {
          if(txsta && (txsta->value.get() & _TXSTA::BRGH))
            set_callback_break(TOTAL_SAMPLE_STATES -(BRGH_THIRD_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE));
          else
            set_callback_break(TOTAL_SAMPLE_STATES -(BRGL_THIRD_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE));

          sample_state = RCSTA_WAITING_MID1;
        }

        break;

      default:
        //cout << "Error RCSTA callback with bad state\n";
        // The receiver was probably disabled in the middle of a reception.
        ;
      }
  }
}

//-----------------------------------------------------------
void _RCSTA::callback_print()
{
  cout << "RCSTA " << name() << " CallBack ID " << CallBackID << '\n';
}

//-----------------------------------------------------------
// RCREG
//
void _RCREG::push(uint new_value)
{
    //qDebug()<<"RCREG::push"<<new_value<<fifo_sp;
  if(fifo_sp >= 2) 
  {
    if (m_rcsta) m_rcsta->overrun();
  } 
  else 
  {
    fifo_sp++;
    oldest_value = value.get();
    value.put(new_value & 0xff);
    if (m_rcsta)
    {
        uint rcsta = m_rcsta->value.get();
        
        if (new_value & 0x100) rcsta |= _RCSTA::RX9D;
        else                   rcsta &= ~ _RCSTA::RX9D;
        m_rcsta->value.put(rcsta);
    }
  }
  mUSART->set_rcif();
}

void _RCREG::pop()
{
  if(fifo_sp == 0) return;

  if(--fifo_sp == 1)
  {
    value.put(oldest_value & 0xff);
    if (m_rcsta)
    {
        uint rcsta = m_rcsta->value.get();
        if (oldest_value & 0x100)
            rcsta |= _RCSTA::RX9D;
        else
            rcsta &= ~ _RCSTA::RX9D;
        m_rcsta->value.put(rcsta);
    }
  }
  if(fifo_sp == 0) mUSART->clear_rcif();
}

uint _RCREG::get_value()
{
  return value.get();
}

uint _RCREG::get()
{
  pop();
  return value.get();
}

//-----------------------------------------------------------
// SPBRG - Serial Port Baud Rate Generator
//
// The SPBRG is essentially a continuously running programmable
// clock. (Note that this will slow the simulation down if the
// serial port is not used. Perhaps gpsim needs some kind of
// pragma type thing to disable cpu intensive peripherals...)

void _SPBRG::get_next_cycle_break()
{
  future_cycle = last_cycle + get_cycles_per_tick();

  if(cpu)
  {
    if (future_cycle <= get_cycles().get())
    {
        last_cycle = get_cycles().get();
        future_cycle = last_cycle + get_cycles_per_tick();
    }
    get_cycles().set_break(future_cycle, this);
  }
}

uint _SPBRG::get_cycles_per_tick()
{
    uint cpi = (cpu) ? p_cpu->get_ClockCycles_per_Instruction() : 4;
    uint brgval, cpt, ret;

    if ( baudcon && baudcon->brg16() )
    {
        brgval =  ( brgh ? brgh->value.get() * 256 : 0 ) + value.get();
        cpt = 4;    // hi-speed divisor in 16-bit mode is 4
    }
    else
    {
        brgval = value.get();
        cpt = 16;   // hi-speed divisor in 8-bit mode is 16
    }

    if ( txsta && (txsta->value.get() & _TXSTA::SYNC) )
    {
      // Synchronous mode - divisor is always 4
      // However, code wants two transitions per bit
      // to generate clock for master mode, so use 2
      cpt = 2;
    }
    else // Asynchronous mode
    {
        if(txsta && !(txsta->value.get() & _TXSTA::BRGH))
        {
            cpt *= 4;   // lo-speed divisor is 4 times hi-speed
        }
    }
    ret = ((brgval+1)*cpt)/cpi;
    ret = ret ? ret : 1;
    return ret;
}

void _SPBRG::start()
{
  if (running) return;
  
  if( !skip  || get_cycles().get() >= skip )
  {
    if(cpu) last_cycle = get_cycles().get();
    skip = 0;
  }
  running = true;

  start_cycle = last_cycle;

  get_next_cycle_break();

  Dprintf((" SPBRG::start   last_cycle:0x%" PRINTF_GINT64_MODIFIER "x: future_cycle:0x%" PRINTF_GINT64_MODIFIER "x\n",last_cycle,future_cycle));
}

void _SPBRG::put(uint new_value)
{
  value.put(new_value);

  //Prevent updating last_cycle until all current breakpoints have expired
  //Otherwise we see that rx/tx periods get screwed up from now until future_cycle
  future_cycle = last_cycle + get_cycles_per_tick();
  skip = future_cycle;
}

void _SPBRG::put_value(uint new_value)
{
  put(new_value);
  update();
}

void _SPBRGH::put(uint new_value)
{
  value.put(new_value);
  if(m_spbrg) m_spbrg->set_start_cycle();
}

void _SPBRG::set_start_cycle()
{
  //Prevent updating last_cycle until all current breakpoints have expired
  //Otherwise we see that rx/tx persiods get screwed up from now until future_cycle
  future_cycle = last_cycle + get_cycles_per_tick();
  skip = future_cycle;
}

void _SPBRGH::put_value(uint new_value)
{
  put(new_value);
  update();
}

//
// Get the cpu cycle corresponding to the last edge of the SPBRG
//
uint64_t _SPBRG::get_last_cycle()
{
  // There's a chance that a SPBRG break point exists on the current
  // cpu cycle, but has not yet been serviced.
  if(cpu) return( (get_cycles().get() == future_cycle) ? future_cycle : last_cycle);
  else    return 0;
}
//--------------------------
//uint64_t _SPBRG::get_cpu_cycle(uint edges_from_now)
//
//  When the SPBRG is enabled, it becomes a free running counter
// that's synchronous with the cpu clock. The frequency of the
// counter depends on the mode of the usart:
//
//  Synchronous mode:
//    baud = cpu frequency / 4 / (spbrg.value + 1)
//
//  Asynchronous mode:
//   high frequency:
//     baud = cpu frequency / 16 / (spbrg.value + 1)
//   low frequency:
//     baud = cpu frequency / 64 / (spbrg.value + 1)
//
// What this routine will do is return the cpu cycle corresponding
// to a (rising) edge of the spbrg clock.

uint64_t _SPBRG::get_cpu_cycle(uint edges_from_now)
{
  // There's a chance that a SPBRG break point exists on the current
  // cpu cycle, but has not yet been serviced.
  uint64_t cycle = (get_cycles().get() == future_cycle) ? future_cycle : last_cycle;

  return ( edges_from_now * get_cycles_per_tick() + cycle );
}

void _SPBRG::callback()
{
  if (skip)
  {
    Dprintf((" SPBRG skip=0x%" PRINTF_GINT64_MODIFIER "x, cycle=0x%" PRINTF_GINT64_MODIFIER "x\n", skip, get_cycles().get()));
  }
  if(! skip  || get_cycles().get() >= skip) 
  {
    last_cycle = get_cycles().get();
    skip = 0;
  }
  //Dprintf(("SPBRG rollover at cycle:0x%" PRINTF_GINT64_MODIFIER "x\n",last_cycle));

  if((rcsta && rcsta->bSPEN()) || (txsta && txsta->bTXEN()))
  {
      // If the serial port is enabled, then set another
      // break point for the next clock edge.
      get_next_cycle_break();
  }
  else running = false;
}

//-----------------------------------------------------------
// TXSTA - Transmit Register Status and Control

void _BAUDCON::put_value(uint new_value)
{
  put(new_value);
  update();
}

void _BAUDCON::put(uint new_value)
{
  uint old_value = value.get();

  // The RCIDL bit is controlled entirely by hardware.
  new_value &= ~RCIDL;
  if ( rcsta->rc_is_idle() ) new_value |= RCIDL;

  value.put(new_value);

  if( (old_value ^ value.get()) & TXCKP) // The TXCKP bit has changed states.
  {
    txsta->set_pin_pol ((value.get() & TXCKP) ? true : false);
  }
}

//--------------------------------------------------
// member functions for the USART
//--------------------------------------------------
void USART_MODULE::initialize(PIR *_pir,
                              PinModule *tx_pin, PinModule *rx_pin,
                              _TXREG *_txreg, _RCREG *_rcreg)
{
  assert(_txreg && _rcreg);
  
  pir = _pir;

  spbrg.txsta = &txsta;
  spbrg.rcsta = &rcsta;

  txreg = _txreg;

  txreg->assign_rcsta(&rcsta);
  txreg->assign_txsta(&txsta);

  rcreg = _rcreg;
  rcreg->assign_rcsta(&rcsta);

  txsta.txreg = txreg;
  txsta.rcsta = &rcsta;
  txsta.spbrg = &spbrg;
  txsta.bit_count = 0;
  txsta.setIOpin(tx_pin);

  rcsta.rcreg = rcreg;
  rcsta.spbrg = &spbrg;
  rcsta.txsta = &txsta;
  rcsta.txreg = txreg;
  rcsta.setIOpin(rx_pin);
}

void USART_MODULE::setIOpin( int data, PinModule* pin )
{
    if (data == TX_PIN) set_TXpin( pin );
    else                set_RXpin( pin );
}

void USART_MODULE::set_TXpin(PinModule *tx_pin)
{
  txsta.setIOpin(tx_pin);
}

void USART_MODULE::set_RXpin(PinModule *rx_pin)
{
  rcsta.setIOpin(rx_pin);
}

bool USART_MODULE::bIsTXempty()
{
  if (m_txif) return m_txif->Get();
  return pir ? pir->get_txif() : true;
}

void USART_MODULE::emptyTX()
{
  Dprintf(("usart::empty - setting TXIF %s\n", txsta.name().c_str()));

  if (txsta.bTXEN())
  {
    if (m_txif)   m_txif->Trigger();
    else if (pir) pir->set_txif();
    else          assert(pir);
  }

}

void USART_MODULE::full()
{
  Dprintf((" txreg::full - clearing TXIF\n"));
  if (m_txif)  m_txif->Clear();
  else if(pir) pir->clear_txif();
  else         assert(pir);
}

void USART_MODULE::set_rcif()
{
  Dprintf((" - setting RCIF\n"));
  if (m_rcif)  m_rcif->Trigger();
  else if(pir) pir->set_rcif();
}

void USART_MODULE::clear_rcif()
{
  Dprintf((" - clearing RCIF\n"));
  if (m_rcif) m_rcif->Clear();
  else if(pir) pir->clear_rcif();
}

//--------------------------------------------------
USART_MODULE::USART_MODULE(Processor *pCpu)
  : txsta(pCpu,"","USART Transmit Status",this),    // Don't set names incase there are two UARTS
    rcsta(pCpu,"","USART Receive Status",this),
    spbrg(pCpu,"","Serial Port Baud Rate Generator"),
    spbrgh(pCpu,"spbrgh","Serial Port Baud Rate high byte"),
    baudcon(pCpu,"baudcon","Serial Port Baud Rate Control"),
    is_eusart(false), m_rcif(0), m_txif(0)
{
    baudcon.txsta = &txsta;
    baudcon.rcsta = &rcsta;
}

USART_MODULE::~USART_MODULE()
{
    if (m_rcif) delete m_rcif;
    if (m_txif) delete m_txif;
}

//--------------------------------------------------
void USART_MODULE::set_eusart ( bool is_it )
{
    if ( is_it )
    {
        spbrgh.assign_spbrg ( &spbrg );
        spbrg.baudcon = &baudcon;
        spbrg.brgh = &spbrgh;
        is_eusart = true;
    }
    else
    {
        spbrgh.assign_spbrg ( 0 );
        spbrg.baudcon = 0;
        spbrg.brgh = 0;
        is_eusart = false;
    }
}


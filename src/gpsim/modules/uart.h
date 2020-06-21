/*
   Copyright (C) 1998,1999 T. Scott Dattalo

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

#include <iostream>
#include <stdio.h>
#include <QList>

class InvalidRegister;   // Forward reference


#ifndef __USART_H__
#define __USART_H__

#include "pic-processor.h"
#include "14bit-registers.h"
#include "pir.h"
#include "apfcon.h"

class _TXSTA;   // Forward references
class _SPBRG;
class _RCSTA;
class _SPBRGH;
class _BAUDCON;
class  _14bit_processor;

class RXSignalSink;
class CLKSignalSink;
class TXSignalSource;
class TXSignalControl;
class RCSignalSource;
class RCSignalControl;
class USART_MODULE;


// USART Data Transmit Register
class _TXREG : public sfr_register, public TriggerObject
{
    public:

        _TXREG(Processor *pCpu, const char *pName, const char *pDesc,USART_MODULE *);
        virtual void put(uint);
        virtual void put_value(uint);
        virtual void assign_txsta(_TXSTA *new_txsta) { m_txsta = new_txsta; }
        virtual void assign_rcsta(_RCSTA *new_rcsta) { m_rcsta = new_rcsta; }
        virtual void callback();
        virtual void callback_print();

    private:
        _TXSTA  *m_txsta;
        _RCSTA  *m_rcsta;
        USART_MODULE *mUSART;
        bool full;
        int  m_uartN;  // Uart number
};

// Transmit Status and Control register
class _TXSTA : public sfr_register, public TriggerObject
{
    public:
        _TXREG  *txreg;
        _SPBRG  *spbrg;
        _RCSTA  *rcsta;

        uint tsr;
        uint bit_count;

        enum {
        TX9D  = 1<<0,
        TRMT  = 1<<1,
        BRGH  = 1<<2,
        SENDB = 1<<3,
        SYNC  = 1<<4,
        TXEN  = 1<<5,
        TX9   = 1<<6,
        CSRC  = 1<<7
        };

        _TXSTA(Processor *pCpu, const char *pName, const char *pDesc,USART_MODULE *);
        ~_TXSTA();

        virtual void put(uint new_value);
        virtual void put_value(uint new_value);

        virtual void transmit_a_bit();
        virtual void start_transmitting();
        virtual void stop_transmitting();
        virtual void transmit_break();
        virtual void callback();
        virtual void callback_print();
        virtual char getState();

        virtual void enableTXPin();
        virtual void disableTXPin();
        virtual void setIOpin(PinModule *);
        virtual PinModule *getIOpin() { return m_PinModule; }
        virtual void putTXState(char newTXState);

        bool bTXEN() { return (value.get() & TXEN) != 0; }
        bool bSYNC() { return (value.get() & SYNC) != 0; }
        bool bTRMT() { return (value.get() & TRMT) != 0; }
        bool bCSRC() { return (value.get() & CSRC) != 0; }
        bool bTX9() { return (value.get() & TX9) != 0; }
        int  bTX9D() { return (value.get() & TX9D) ? 1 : 0; }

        void set_pin_pol ( bool invert ) { bInvertPin = invert; }
        void releasePin();

    protected:
        USART_MODULE *mUSART;
        PinModule *m_PinModule;
        TXSignalSource *m_source;
        TXSignalControl *m_control;
        CLKSignalSink   *m_clkSink;
        bool          SourceActive;
        char m_cTxState;
        bool bInvertPin;
};

// USART Data Receive Register
class _RCREG : public sfr_register
{
    public:
        uint oldest_value;  /* rcreg has a 2-deep fifo. The oldest received
                          * value is stored here, while the most recent
                          * is stored in sfr_register.value . */

        uint fifo_sp;       /* fifo stack pointer */

        _RCREG(Processor *pCpu, const char *pName, const char *pDesc,USART_MODULE *);
        virtual uint get();
        virtual uint get_value();
        virtual void push(uint);
        virtual void pop();

    virtual void assign_rcsta(_RCSTA *new_rcsta) { m_rcsta = new_rcsta; }

    private:
        USART_MODULE *mUSART;
        _RCSTA  *m_rcsta;
};

// Receive Status and Control Register
class _RCSTA : public sfr_register, public TriggerObject
{
     public:
      enum {
        RX9D = 1<<0,
        OERR = 1<<1,
        FERR = 1<<2,
        ADDEN = 1<<3,
        CREN = 1<<4,
        SREN = 1<<5,
        RX9  = 1<<6,
        SPEN = 1<<7
      };

      enum {
        RCSTA_DISABLED,
        RCSTA_WAITING_FOR_START,
        RCSTA_MAYBE_START,
        RCSTA_WAITING_MID1,
        RCSTA_WAITING_MID2,
        RCSTA_WAITING_MID3,
        RCSTA_RECEIVING
      };

      // The usart samples the middle of the bit three times and
      // produces a sample based on majority averaging. 
      // 

    #define TOTAL_SAMPLE_STATES    16

    #define BRGH_FIRST_MID_SAMPLE  4
    #define BRGH_SECOND_MID_SAMPLE 8
    #define BRGH_THIRD_MID_SAMPLE  12

    #define BRGL_FIRST_MID_SAMPLE  7
    #define BRGL_SECOND_MID_SAMPLE 8
    #define BRGL_THIRD_MID_SAMPLE  9

      _RCREG  *rcreg;
      _SPBRG  *spbrg;
      _TXSTA  *txsta;
      _TXREG  *txreg;

      bool sync_next_clock_edge_high;
      uint rsr;
      uint bit_count;
      uint rx_bit;
      uint sample,state, sample_state;
      uint64_t future_cycle, last_cycle;

      _RCSTA(Processor *pCpu, const char *pName, const char *pDesc, USART_MODULE *);
      ~_RCSTA();

      virtual void put(uint new_value);
      virtual void put_value(uint new_value);
      void receive_a_bit(unsigned);
      void receive_start_bit();
      virtual void start_receiving();
      virtual void stop_receiving();
      virtual void overrun();
      virtual void callback();
      virtual void callback_print();
      void setState(char new_RxState);
    //RRR  char getDir() { return m_DTdirection;}
      bool bSPEN() { return (value.get() & SPEN); }
      bool bSREN() { return (value.get() & SREN); }
      bool bCREN() { return (value.get() & CREN); }
      bool bRX9()  { return (value.get() & RX9); }
      virtual void setIOpin(PinModule *);
      bool rc_is_idle(void) { return ( state <= RCSTA_WAITING_FOR_START ); }
      virtual void enableRCPin(char direction = DIR_OUT);
      virtual void disableRCPin();
      void releasePin();
      virtual char getState() { return m_cTxState;}
      virtual void putRCState(char newRCState);
      virtual void sync_start_transmitting();
      virtual void clock_edge(char new3State);
      void set_old_clock_state(char new3State);
      
      void queueData( uint32_t value );

    protected:
      enum {
         DIR_OUT,
         DIR_IN
      };
      void set_callback_break(uint spbrg_edge);

      USART_MODULE  *mUSART;
      PinModule     *m_PinModule;
      RXSignalSink  *m_sink;
      char          m_cRxState;
      bool          SourceActive;
      RCSignalControl *m_control;
      RCSignalSource  *m_source;
      char          m_cTxState;
      char          m_DTdirection;
      
      QList<uint32_t> m_dataQueue;
      
      bool bInvertPin;
      bool old_clock_state;
      bool dataInQueue;
};


// BAUD Rate Control Register
class _BAUDCON : public sfr_register
{
     public:
      enum {
        ABDEN  = 1<<0,
        WUE    = 1<<1,
        BRG16  = 1<<3,
        TXCKP  = 1<<4,
        SCKP   = 1<<4,     // synchronous clock polarity Select bit (16f88x)
        RXDTP  = 1<<5,
        RCIDL  = 1<<6,
        ABDOVF = 1<<7
      };

      _TXSTA *txsta;
      _RCSTA *rcsta;

      _BAUDCON(Processor *pCpu, const char *pName, const char *pDesc);

      virtual void put(uint);
      virtual void put_value(uint);
      bool brg16(void) { return ( value.get() & BRG16 ) != 0; }
      bool txckp() { return ( value.get() & TXCKP) != 0; }
      bool rxdtp() { return ( value.get() & RXDTP) != 0; }

    // private:
};

// USART Baud Rate Generator, High Byte
class _SPBRGH : public sfr_register
{
    public:
        _SPBRGH(Processor *pCpu, const char *pName, const char *pDesc);
        virtual void assign_spbrg(_SPBRG *new_spbrg) { m_spbrg = new_spbrg; }
        virtual void put(uint);
        virtual void put_value(uint);
        
    private:
        _SPBRG   *m_spbrg;
};

// USART Baud Rate Generator, Low Byte
class _SPBRG : public sfr_register, public TriggerObject
{
    public:
        _TXSTA *txsta;
        _RCSTA *rcsta;
        _SPBRGH *brgh;
        _BAUDCON *baudcon;

        uint64_t 
        start_cycle,   // The cycle the SPBRG was started
        last_cycle,    // The cycle when the spbrg clock last changed
        future_cycle;  // The next cycle spbrg is predicted to change

        bool running;

        _SPBRG(Processor *pCpu, const char *pName, const char *pDesc);

        virtual void callback();
        virtual void callback_print() {
        cout << "_SPBRG " << name() << " CallBack ID " << CallBackID << '\n';
        }
        virtual void start();
        virtual void get_next_cycle_break();
        virtual uint64_t get_cpu_cycle(uint edges_from_now);
        virtual uint64_t get_last_cycle();

        virtual void put(uint);
        virtual void put_value(uint);
        void set_start_cycle();
        // protected:
        virtual uint get_cycles_per_tick();
        
    private:
        uint64_t skip;
};

//---------------------------------------------------------------
//---------------------------------------------------------------
class USART_MODULE: public apfpin
{
    public:
        enum
        {
        TX_PIN = 0,
        RX_PIN = 1,
        };
        
        _TXSTA       txsta;
        _RCSTA       rcsta;
        _SPBRG       spbrg;

        _TXREG      *txreg;
        _RCREG      *rcreg;
        PIR         *pir;

        // Extra registers for when it's an EUSART
        _SPBRGH  spbrgh;
        _BAUDCON baudcon;

        USART_MODULE( Processor *pCpu );
        ~USART_MODULE();

        void initialize( PIR *,
                PinModule *tx_pin, PinModule *rx_pin,
                _TXREG *, _RCREG *);

        virtual void setIOpin(int data, PinModule *pin);
        void set_TXpin( PinModule *tx_pin);
        void set_RXpin( PinModule *rx_pin);
        bool bIsTXempty();
        void emptyTX();
        void full();
        void set_rcif();
        void clear_rcif();
        void mk_rcif_int( PIR *reg, uint bit)
              { m_rcif = new InterruptSource( reg, bit);}
        void mk_txif_int(PIR *reg, uint bit)
              { m_txif = new InterruptSource( reg, bit);}
        bool IsEUSART ( void ) { return is_eusart; }
        void set_eusart ( bool is_it );

    private:
        bool is_eusart;
        InterruptSource *m_rcif;
        InterruptSource *m_txif;
};

#endif

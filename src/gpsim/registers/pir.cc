/*
   Copyright (C) 1998-2003 Scott Dattalo


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

#include "pir.h"
#include "intcon.h"
#include "processor.h"

PIR::PIR(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie, int _valid_bits)
  : sfr_register(pCpu,pName,pDesc),
    intcon(_intcon),pie(_pie),ipr(0),valid_bits(_valid_bits),writable_bits(0)
{
}

void PIR::put(uint new_value)
{
  // Only the "writable bits" can be written with put.
  // The "read-only" ones (such as TXIF) are written
  // through the set_/clear_ member functions.

  value.put((new_value & writable_bits) | (value.get() & ~writable_bits));

  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR::set_intcon(INTCON *_intcon)
{
  intcon = _intcon;
}

void PIR::set_pie(PIE *_pie)
{
  pie = _pie;
}

void PIR::set_ipr(sfr_register *_ipr)
{
  ipr = _ipr;
}

void PIR::setInterrupt(uint bitMask)
{
  value.put(value.get() | bitMask);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}


void PIR::setPeripheralInterrupt()
{
  if (intcon)
    intcon->peripheral_interrupt ( ipr && (value.get() & valid_bits & ipr->value.get() & pie->value.get()) );
}

/*
   return 0 : no interrupt pending
          1 : low priority interrupt pending
	  2 : high priority unterrupt pending
*/
int PIR::interrupt_status()
{
  assert(pie);
  if ( ipr )
  {
    int result = 0;
    if ( value.get() & valid_bits & pie->value.get() & ~(ipr->value.get()) )
        result |= 1;
    if ( value.get() & valid_bits & pie->value.get() & ipr->value.get() )
        result |= 2;
    return result;
  }
  else
    return ( value.get() & valid_bits & pie->value.get() ) ? 1 : 0;
}

//========================================================================
InterruptSource::InterruptSource(PIR *_pir, uint bitMask)
  : m_pir(_pir), m_bitMask(bitMask)
{
  assert(m_pir);
  // Only one bit in the bit mask should be set.
  assert(m_bitMask && ((m_bitMask & (m_bitMask-1)) == 0));
}

void InterruptSource::Trigger()
{
  m_pir->setInterrupt(m_bitMask);
}

void InterruptSource::Clear()
{
  m_pir->value.put(m_pir->value.get() & ~m_bitMask);
}


void InterruptSource::release()
{
  delete this;
}



//------------------------------------------------------------------------

PIR1v1::PIR1v1(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  // Even though TXIF is a valid bit, it can't be written by the PIC
  // source code.  Its state reflects whether the usart txreg is full
  // or not. Similarly for RCIF
  valid_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | TXIF | RCIF | CMIF | EEIF;
  writable_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | CMIF | EEIF;
}


void PIR1v1::clear_sspif(void)
{
  value.put(value.get() & ~SSPIF);
}

void PIR1v1::set_txif(void)
{
  value.put(value.get() | TXIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v1::clear_txif(void)
{
  value.put(value.get() & ~TXIF);
}

void PIR1v1::set_rcif(void)
{
  value.put(value.get() | RCIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v1::clear_rcif(void)
{
  value.put(value.get() & ~RCIF);
}

void PIR1v1::set_cmif(void)
{
  value.put(value.get() | CMIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v1::set_eeif(void)
{
  value.put(value.get() | EEIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
//------------------------------------------------------------------------
//
PIR1v2::PIR1v2(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  // Even though TXIF is a valid bit, it can't be written by the PIC
  // source code.  Its state reflects whether the usart txreg is full
  // or not. Similarly for RCIF
  valid_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | TXIF | RCIF | ADIF | PSPIF;
  writable_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF |  ADIF | PSPIF;
}

void PIR1v2::clear_sspif(void)
{
  value.put(value.get() & ~SSPIF);
}

void PIR1v2::set_txif(void)
{
  value.put(value.get() | TXIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v2::set_pspif(void)
{
  value.put(value.get() | PSPIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v2::set_sppif(void)
{
  value.put(value.get() | SPPIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v2::set_sspif(void)
{
  value.put(value.get() | SSPIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v2::clear_txif(void)
{
  value.put(value.get() & ~TXIF);
}

void PIR1v2::set_rcif(void)
{
  value.put(value.get() | RCIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v2::clear_rcif(void)
{
  value.put(value.get() & ~RCIF);
}

//------------------------------------------------------------------------

PIR1v3::PIR1v3(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = TMR1IF | ADIF | CMIF | EEIF;
  writable_bits = TMR1IF | ADIF | CMIF | EEIF;
}

void PIR1v3::set_tmr1if(void)
{
  value.put(value.get() | TMR1IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v3::set_tmr2if(void)
{
  value.put(value.get() | TMR2IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v3::set_cmif(void)
{
  value.put(value.get() | CMIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR1v3::set_eeif(void)
{
  value.put(value.get() | EEIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v3::set_adif(void)
{
  value.put(value.get() | ADIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v3::set_c1if(void)
{
  value.put(value.get() | C1IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v3::set_c2if(void)
{
  value.put(value.get() | C2IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

//------------------------------------------------------------------------

PIR1v4::PIR1v4(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  writable_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | ADIF | EEIF;
  valid_bits = 0xff;
}
void PIR1v4::set_txif(void)
{
  value.put(value.get() | TXIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v4::clear_txif(void)
{
  value.put(value.get() & ~TXIF);
}

void PIR1v4::set_rcif(void)
{
  value.put(value.get() | RCIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR1v4::clear_rcif(void)
{
  value.put(value.get() & ~RCIF);
}
//------------------------------------------------------------------------
PIR2v1::PIR2v1(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = CCP2IF;
  writable_bits = valid_bits;
}
//------------------------------------------------------------------------
PIR2v2::PIR2v2(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = ECCP1IF | TMR3IF | LVDIF | BCLIF | EEIF | CMIF;
  writable_bits = valid_bits;
}

void PIR2v2::set_cmif(void)
{
  value.put(value.get() | CMIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v2::set_eeif(void)
{
  value.put(value.get() | EEIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v2::set_bclif(void)
{
  value.put(value.get() | BCLIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
//------------------------------------------------------------------------
PIR2v3::PIR2v3(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = CCP2IF | ULPWUIF | BCLIF | EEIF | C1IF | C2IF | OSFIF;
  writable_bits = valid_bits;
}

void PIR2v3::set_c1if(void)
{
  value.put(value.get() | C1IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v3::set_c2if(void)
{
  value.put(value.get() | C2IF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v3::set_eeif(void)
{
  value.put(value.get() | EEIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v3::set_bclif(void)
{
  value.put(value.get() | BCLIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

PIR2v4::PIR2v4(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = OSCFIF | CMIF | USBIF | EEIF | BCLIF | HLVDIF | TMR3IF | CCP2IF;
  writable_bits = valid_bits;
}
void PIR2v4::set_usbif(void)
{
  value.put(value.get() | USBIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v4::set_cmif(void)
{
  value.put(value.get() | CMIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v4::set_eeif(void)
{
  value.put(value.get() | EEIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
void PIR2v4::set_bclif(void)
{
  value.put(value.get() | BCLIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}
PIR2v5::PIR2v5(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = OSFIF | LVDIF | LCDIF | C1IF | C2IF |CCP2IF;
  writable_bits = valid_bits;
}
//------------------------------------------------------------------------

PIR3v1::PIR3v1(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  // Even though TXIF is a valid bit, it can't be written by the PIC
  // source code.  Its state reflects whether the usart txreg is full
  // or not. Similarly for RCIF
  valid_bits = CCP3IF | CCP4IF | CCP5IF | TMR4IF | TXIF | RCIF;
  writable_bits = CCP3IF | CCP4IF | CCP5IF | TMR4IF;
}


void PIR3v1::set_txif(void)
{
  value.put(value.get() | TXIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR3v1::clear_txif(void)
{
  value.put(value.get() & ~TXIF);
}

void PIR3v1::set_rcif(void)
{
  value.put(value.get() | RCIF);
  if( value.get() & pie->value.get() ) setPeripheralInterrupt();
}

void PIR3v1::clear_rcif(void)
{
  value.put(value.get() & ~RCIF);
}

//------------------------------------------------------------------------
PIR3v2::PIR3v2(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = RXB0IF | RXB1IF | TXB0IF | TXB1IF | TXB2IF | ERRIF |
    WAKIF | IRXIF;
  writable_bits = valid_bits;
}

PIR3v3::PIR3v3(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = TMR3GIF | TMR5GIF | TMR1GIF | CTMUIF | TX2IF | RC2IF 
	| BCL2IF | SSP2IF;
  writable_bits = valid_bits;
}

PIR4v1::PIR4v1(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = CCP3IF | CCP4IF | CCP5IF ;
  writable_bits = CCP3IF | CCP4IF | CCP5IF;
}
PIR5v1::PIR5v1(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = TMR4IF | TMR5IF | TMR6IF ;
  writable_bits = TMR4IF | TMR5IF | TMR6IF ;
}

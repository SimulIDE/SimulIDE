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

#include <stdio.h>
#include <iostream>
#include <sstream>

#include "config.h"
#include "processor.h"
#include "registers.h"

uint count_bits(uint ui)
{
  uint bits=0;

  while (ui) {
    ui &= (ui-1);
    bits++;
  }
  return bits;
}
//========================================================================
// toString
//
// Convert a RegisterValue type to a string.
//
// A RegisterValue type allows the bits of a register to take on three
// values: High, Low, or undefined. If all of the bits are defined,
// then this routine will convert register value to a hexadecimal string.
// Any undefined bits within a nibble will cause the associated nibble to
// be undefined and will get converted to a question mark.
//

char * RegisterValue::toString(char *str, int len, int regsize) const
{
  if(str && len) {
    RegisterValue rv = *this;

    char hex2ascii[] = "0123456789ABCDEF";
    char undefNibble = '?';
    int i;

    int m = regsize * 2 + 1;
    if(len < m)
      m = len;

    m--;

    for(i=0; i < m; i++) {
      if(rv.init & 0x0f)
        str[m-i-1] = undefNibble;
      else
        str[m-i-1] = hex2ascii[rv.data & 0x0f];
      rv.init >>= 4;
      rv.data >>= 4;
    }
    str[m] = 0;

  }
  return str;
}

//========================================================================
// SplitBitString
//
// The purpose of this routine is to convert a string of bitnames into
// an array of names. The string is formatted like:
//
//  b1.b2.b3
//
// In other words, a period is the delimeter between the names.
// This gets converted to:
//
//  b1
//  b2
//  b2
//
// INPUTS
//  n - number of names
//  in - input string formatted as described
//  in2 - if 'in' is NULL, then all 'n' names will be 'in2'
//
// OUTPUTS
//  out - an array to hold the strings.
//
// Note, the input string 'in' will be modified such that all of the '.'s will
// get turned into string terminating 0's.
//

static void SplitBitString(int n, const char **out, char *in, const char *in2)
{

  if(!in) {
    for(int i=0; i<n; i++)
      out[i] = in2;
  } else {

    char *str = in;
    for(int i =0; i<n; i++ ) {

      out[i] = in;

      str = strchr(in, '.');
      if(str) {
	*str = 0;
	in = ++str;
      }
      //cout << "split :" << i << " ==> " << out[i] << endl;
    }
  }
}

//========================================================================
// toBitStr
//
// Convert a RegisterValue type to a bit string
//
// Given a pointer to a string, this function will convert a register
// value into a string of ASCII characters. If no names are given
// for the bits, then the default values of 'H', 'L', and '?' are
// used for high, low and undefined.
//
// The input 'BitPos' is a bit mask that has a bit set for each bit that
// the user wishes to display.
//

char * RegisterValue::toBitStr(char *s, int len, uint BitPos,
			       const char *cByteSeparator,
			       const char *HiBitNames,
			       const char *LoBitNames,
			       const char *UndefBitNames) const
{
  uint i,mask,max;

  if(!s || len<=0)
    return 0;

  max = 32;

  uint nBits = count_bits(BitPos);

  if(nBits >= max)
    nBits = max;

  const char *HiNames[32];
  const char *LoNames[32];
  const char *UndefNames[32];


  char *cHi = HiBitNames ? strdup(HiBitNames) : 0;
  char *cLo = LoBitNames ? strdup(LoBitNames) : 0;
  char *cUn = UndefBitNames ? strdup(UndefBitNames) : 0;

  SplitBitString(nBits, HiNames, cHi, "1");
  SplitBitString(nBits, LoNames, cLo, "0");
  SplitBitString(nBits, UndefNames, cUn, "?");

  char *dest = s;

  int bitNumber=31;
  for(i=0,mask=1<<31; mask; mask>>=1,bitNumber--) {

    if(BitPos & mask) {

      const char *H = HiNames[i];
      const char *L = LoNames[i];
      const char *U = UndefNames[i];

      const char *c = (init & mask) ?  U :
	((data & mask) ? H : L);

      strncpy(dest, c, len);
      int l = strlen(c);
      len -= l;
      dest += l;
      *dest = 0;

      if(i++>nBits || len < 0)
	break;

      if(cByteSeparator && bitNumber && ((bitNumber%8)==0)) {
	strncpy(dest, cByteSeparator, len);
	int l = strlen(cByteSeparator);
	len -= l;
	dest += l;
	*dest = 0;
	if(len < 0)
	  break;
      }

    }

  }

  free(cHi);
  free(cLo);
  free(cUn);

  return s;
}


//--------------------------------------------------
// Member functions for the file_register base class
//--------------------------------------------------
//
// For now, initialize the register with valid data and set that data equal to 0.
// Eventually, the initial value will be marked as 'uninitialized.

Register::Register(Module *_cpu, const char *pName, const char *pDesc)
  : Value(pName,pDesc,_cpu), value(RegisterValue(0, 0)),
    address(AN_INVALID_ADDRESS),
    alias_mask(0), por_value(RegisterValue(0, 0)), m_replaced(0)
{
  //set_xref(new XrefObject(this));
  read_access_count=0;
  write_access_count=0;
  mValidBits = 0xFF;

}
Register::~Register()
{
/*  if (cpu) {
    //cout << "Removing register from ST:" << name_str <<  " addr "<< this << endl;
    cpu->removeSymbol(this);
  }*/
}

int Register::clear_break()
{
  return -1;
}

//------------------------------------------------------------
// get()
//
//  Return the contents of the file register.
// (note - breakpoints on file register reads
//  are not checked here. Instead, a breakpoint
//  object replaces those instances of file
//  registers for which we wish to monitor.
//  So a file_register::get call will invoke
//  the breakpoint::get member function. Depending
//  on the type of break point, this get() may
//  or may not get called).

uint Register::get()
{
  return(value.get());
}

//------------------------------------------------------------
// put()
//
//  Update the contents of the register.
//  See the comment above in file_register::get()
//  with respect to break points
//

void Register::put(uint new_value)
{
  value.put(new_value);
}

bool Register::get_bit(uint bit_number)
{
  return  (value.get() & (1<<bit_number) ) ? true : false;
}

double Register::get_bit_voltage(uint bit_number)
{
  if(get_bit(bit_number)) return 5.0;
  else                    return 0.0;
}
//--------------------------------------------------
// set_bit
//
//  set a single bit in a register. Note that this
// is really not intended to be used on the file_register
// class. Instead, setbit is a place holder for high level
// classes that overide this function
void Register::setbit(uint bit_number, bool new_value)
{
    int set_mask = (1<<bit_number);

  if ( set_mask & mValidBits ) {
    value.put((value.get() & ~set_mask) | (new_value ? set_mask : 0));
  }
}

//-----------------------------------------------------------
//  void Register::put_value(uint new_value)
//
//  put_value is used by the gui to change the contents of
// file registers. We could've let the gui use the normal
// 'put' member function to change the contents, however
// there are instances where 'put' has a cascading affect.
// For example, changing the value of an i/o port's tris
// could cause i/o pins to change states. In these cases,
// we'd like the gui to be notified of all of the cascaded
// changes. So rather than burden the real-time simulation
// with notifying the gui, I decided to create the 'put_value'
// function instead.
//   Since this is a virtual function, derived classes have
// the option to override the default behavior.
//
// inputs:
//   uint new_value - The new value that's to be
//                            written to this register
// returns:
//   nothing
//
//-----------------------------------------------------------

void Register::put_value(uint new_value)
{

  // go ahead and use the regular put to write the data.
  // note that this is a 'virtual' function. Consequently,
  // all objects derived from a file_register should
  // automagically be correctly updated.

  value.put(new_value);

  // Even though we just wrote a value to this register,
  // it's possible that the register did not get fully
  // updated (e.g. porta on many pics has only 5 valid
  // pins, so the upper three bits of a write are meaningless)
  // So we should explicitly tell the gui (if it's
  // present) to update its display.

  update();

}

///
/// New accessor functions
//////////////////////////////////////////////////////////////

uint Register::register_size () const
{
  Processor *pProc = Value::get_cpu();
  return pProc == 0 ? 1 : pProc->register_size();
}

//------------------------------------------------------------

char * Register::toString(char *str, int len)
{
  return getRV_notrace().toString(str, len, register_size()*2);
}
char * Register::toBitStr(char *s, int len)
{
  uint bit_length = register_size() * 8;
  uint bits = (1<<bit_length) - 1;

  return getRV_notrace().toBitStr(s,len,bits);
}

//-----------------------------------------------------------

void Register::new_name(const char *s)
{

  if(s) {
    string str(s);
    new_name(str);
  }
}

void Register::new_name(string &new_name)
{
  if (name_str != new_name) {
    if (name_str.empty()) {
      name_str = new_name;
      return;
    }

    name_str = new_name;
    if (cpu) {
      addName(new_name);
//      cpu->addSymbol(this, &new_name);
    }

  }
}
//------------------------------------------------------------------------
// set -- assgin the value of some other object to this Register
//
// This is used (primarily) during Register stimuli processing. If
// a register stimulus is attached to this register, then it will
// call ::set() and supply a Value pointer.

void Register::set(Value * pVal)
{
  Register *pReg = dynamic_cast<Register *>(pVal);
  if (pReg) {
    putRV(pReg->getRV());
    return;
  }

  if (pVal) {
    put_value( (uint)*pVal);
  }
}

//------------------------------------------------------------------------
// copy - create a new Value object that's a 'copy' of this object
//
// We really don't perform a true copy. Instead, an Integer object
// is created containing the same numeric value of this object.
// This code is called during expression parsing. *NOTE* this copied
// object can be assigned a new value, however that value will not
// propagate to the Register!

Value *Register::copy()
{
  Value *val = new ValueWrapper(this);
  return val;
}
void Register::get(int64_t &i)
{
  i = get_value();
}
//--------------------------------------------------
//--------------------------------------------------
//--------------------------------------------------
sfr_register::sfr_register(Module *pCpu, const char *pName, const char *pDesc)
  : Register(pCpu,pName,pDesc), wdtr_value(0,0xff)
{}

void sfr_register::reset(RESET_TYPE r)
{
  switch (r) {

  case POR_RESET:
    putRV(por_value);
    break;

  default:
    // Most registers simply retain their value across WDT resets.
    if (wdtr_value.initialized())
    {
        putRV(wdtr_value);
    }
    break;
  }
}

//--------------------------------------------------
//--------------------------------------------------

//--------------------------------------------------
// member functions for the InvalidRegister class
//--------------------------------------------------
void InvalidRegister::put(uint new_value)
{
  cout << "attempt write to invalid file register\n";

  if (address != AN_INVALID_ADDRESS)
    cout << "    address 0x" << hex << address << ',';
  cout << "   value 0x" << hex << new_value << endl;

  if(((Processor*)cpu)->getBreakOnInvalidRegisterWrite()) {
    bp.halt();
  }
  return;
}

uint InvalidRegister::get()
{
  cout << "attempt read from invalid file register\n";
  if (address != AN_INVALID_ADDRESS)
    cout << "    address 0x" << hex << address << endl;

  if(((Processor*)cpu)->getBreakOnInvalidRegisterRead()) {
    bp.halt();
  }
  return(0);
}

InvalidRegister::InvalidRegister(Processor *pCpu, const char *pName, const char *pDesc)
  : Register(pCpu,pName,pDesc)
{}



/*
   Copyright (C) 2013,2014,2017 Roy R. Rankin

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

#ifndef __APFCON_H__
#define __APFCON_H__

#include "ioports.h"


// Base class to allow APFCON to change IO pins
class apfpin
{
 public:
     virtual void setIOpin(int data, PinModule *pin) { fprintf(stderr, "unexpected call afpin::setIOpin data=%d\n", data);}
};

// ALTERNATE PIN LOCATIONS register
// set_pins is used to configure operation
class APFCON : public  sfr_register
{
 public:
  APFCON(Processor *pCpu, const char *pName, const char *pDesc, unsigned int _mask);
  virtual void put(unsigned int new_value);
  void set_pins(unsigned int bit, class apfpin *pt_apfpin, int arg, 
		PinModule *pin_default, PinModule *pin_alt);
  void set_ValidBits(unsigned int _mask){mValidBits = _mask;}

 private:
   unsigned int mValidBits;
   struct dispatch
   {
	class apfpin *pt_apfpin;	// pointer to pin setting function
	int          arg;	        // argument for pin setting function
	PinModule    *pin_default; // pin when bit=0
	PinModule    *pin_alt;	// pin when bit=1
   } dispatch[8];
};

#endif

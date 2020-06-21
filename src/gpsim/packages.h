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


#ifndef __PACKAGES_H__
#define __PACKAGES_H__

typedef unsigned int uint;

#include <string>
using namespace std;

#include "gpsim_classes.h"

class IOPIN; // forward reference


enum PACKAGE_PIN_ERRORS
{
  E_NO_PIN,
  E_NO_PACKAGE,
  E_PIN_OUT_OF_RANGE,
  E_PIN_EXISTS
};

class Package
{
    public:
      uint number_of_pins;

      Package();
      explicit Package(uint number_of_pins);
      virtual ~Package();

      void assign_pin(uint pin_number, IOPIN *pin, bool warn=true);
      void destroy_pin(uint pin_number, IOPIN *pin=0);
      void create_pkg(uint _number_of_pins);

      uint isa(void){ return 0; };
      virtual void create_iopin_map(void);

      virtual int get_pin_count(void) {return number_of_pins;};

      int pin_existance(uint pin_number);
      IOPIN *get_pin(uint pin_number);

    protected:

      IOPIN **pins; 
};

#endif // __PACKAGES_H__

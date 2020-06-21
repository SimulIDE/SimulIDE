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


#include <stdio.h>

#include "config.h"
#include "pic-processor.h"
#include "stimuli.h"
#include "packages.h"
#include <math.h>

Package::Package(void)
{

  pins = 0;
  number_of_pins = 0;
}

Package::Package(uint _number_of_pins)
{
  number_of_pins = 0;
  create_pkg(_number_of_pins);
}

Package::~Package()
{
  if (pins)
    destroy_pin(0); // delete all of the pins
  delete [] pins;
}
void Package::create_pkg(uint _number_of_pins)
{
  if(number_of_pins)
    {
      cout << "error: Package::create_pkg. Package appears to already exist.\n";
      return;
    }
  number_of_pins = _number_of_pins;

  pins = new IOPIN *[number_of_pins];

  for(uint i=0; i<number_of_pins; i++)
  {
    uint pins_per_side;
    pins[i] = 0;

    pins_per_side = number_of_pins/2;
    if(number_of_pins&1) // If odd number of pins
        pins_per_side++;
  }
}


int Package::pin_existance(uint pin_number)
{
  if(!number_of_pins)
    {
      cout << "error: Package::assign_pin. No package.\n";
      return E_NO_PACKAGE;
    }

  if((pin_number > number_of_pins) || (pin_number == 0))
    {
      cout << "error: Package::assign_pin. Pin number is out of range.\n";
      cout << "Max pins " << number_of_pins << ". Trying to add " << pin_number <<".\n";
      return E_PIN_OUT_OF_RANGE;
    }

  if(pins[pin_number-1]) return E_PIN_EXISTS;
  
  return E_NO_PIN;
}

IOPIN *Package::get_pin(uint pin_number)
{
  if(E_PIN_EXISTS == pin_existance(pin_number))
    return pins[pin_number-1];
  else
    return 0;

}

void Package::assign_pin(uint pin_number, IOPIN *pin, bool warn)
{
  switch(pin_existance(pin_number)) {

  case E_PIN_EXISTS:
    if(pins[pin_number-1] && warn)
      cout << "warning: Package::assign_pin. Pin number " << pin_number << " already exists.\n";

  case E_NO_PIN:
    pins[pin_number-1] = pin;
    break;
  }
}

void Package::destroy_pin(uint pin_number, IOPIN *pin)
{
  if (pin_number) 
  {
    if(pin_number <= number_of_pins) 
    {
      IOPIN *pPin = pins[pin_number-1];
      if (pPin)
          delete pPin;
      pins[pin_number-1] = 0;
    }

  } 
  else 
  {
    // Delete all pins
    for (pin_number=1; pin_number <= number_of_pins; pin_number++)
      destroy_pin(pin_number);
    number_of_pins = 0;
  }
}

void Package::create_iopin_map(void)
{
}


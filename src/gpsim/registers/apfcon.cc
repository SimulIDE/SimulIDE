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

#include "apfcon.h"
#include "processor.h"

// Alternate Pin function
APFCON::APFCON(Processor *pCpu, const char *pName, const char *pDesc, unsigned int _mask)
      : sfr_register(pCpu,pName,pDesc)
      , mValidBits(_mask)
{
    for(int j=0; j<8; j++)
    {
        dispatch[j].pt_apfpin = 0;
    }
}

void APFCON::set_pins(unsigned int bit, class apfpin* pt_apfpin, int arg,
                      PinModule* pin_default, PinModule* pin_alt)
{
    dispatch[bit].pt_apfpin = pt_apfpin;
    dispatch[bit].arg = arg;
    dispatch[bit].pin_default = pin_default;
    dispatch[bit].pin_alt = pin_alt;
}

void APFCON::put( unsigned int new_value )
{
    unsigned int old_value = value.get();
    unsigned int diff = (new_value ^ old_value) & mValidBits;

    new_value &= mValidBits;
    value.put(new_value);

    for( int i=0; i<8; i++ )
    {
        unsigned int bit = 1<<i;
        if(diff & bit)
        {
            assert(dispatch[i].pt_apfpin);
            dispatch[i].pt_apfpin->setIOpin(dispatch[i].arg, (bit & new_value)? dispatch[i].pin_alt: dispatch[i].pin_default);
        }
    }
}

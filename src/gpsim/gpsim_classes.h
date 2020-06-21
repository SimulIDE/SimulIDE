/*
   Copyright (C) 1998 T. Scott Dattalo

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

/*
 * gpsim_classes.h
 *
 * This include file contains most of the class names defined in gpsim
 * It's used to define forward references to classes and help alleviate
 * include file dependencies.
 */

#ifndef __GPSIM_CLASSES_H__
#define __GPSIM_CLASSES_H__

/*==================================================================
 *
 * Here are a few enum definitions 
 */


/*
 * Define all of the different types of reset conditions:
 */

enum RESET_TYPE
{
  POR_RESET,          // Power-on reset
  WDT_RESET,          // Watch Dog timer timeout reset
  IO_RESET,           // I/O pin  reset
  MCLR_RESET,         // MCLR (Master Clear) reset
  SOFT_RESET,         // Software initiated reset
  BOD_RESET,          // Brown out detection reset
  SIM_RESET,          // Simulation Reset
  EXIT_RESET,         // Leaving Reset, resuming normal execution.
  OTHER_RESET,         //
  STKUNF_RESET,		// Stack undeflow reset
  STKOVF_RESET		// Statck overflow reset

};

enum IOPIN_TYPE
{
  INPUT_ONLY,          // e.g. MCLR
  BI_DIRECTIONAL,      // most iopins
  BI_DIRECTIONAL_PU,   // same as bi_directional, but with pullup resistor. e.g. portb
  OPEN_COLLECTOR       // bit4 in porta on the 18 pin midrange devices.
};
#endif //  __GPSIM_CLASSES_H__

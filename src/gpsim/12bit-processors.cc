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

#include <stdio.h>
#include <iostream>
#include <iomanip>

#include "config.h"
#include "12bit-processors.h"

#include <string>
#include "stimuli.h"

extern uint config_word;

//-------------------------------------------------------------------
_12bit_processor::_12bit_processor(const char *_name, const char *desc)
  : pic_processor(_name, desc)
{
  pc         = new Program_Counter("pc", "Program Counter", this);
  option_reg = new OPTION_REG(this, "option_reg");
  stack      = new Stack(this);
}

_12bit_processor::~_12bit_processor()
{
  delete pc;

  delete_sfr_register(fsr);
  delete_sfr_register(option_reg);
}

void _12bit_processor::reset(RESET_TYPE r)
{
  option_reg->reset(r);
  pic_processor::reset(r);
}

void _12bit_processor::save_state()
{
  pic_processor::save_state();
}

bool _12bit_processor::set_config_word(uint address,uint cfg_word)
{

  // Clear all of the configuration bits in config_modes and then
  // reset each of them based on the config bits in cfg_word:
  //config_modes &= ~(CM_WDTE);
  //config_modes |= ( (cfg_word & WDTE) ? CM_WDTE : 0);
  //cout << " setting cfg_word and cfg_modes " << hex << config_word << "  " << config_modes << '\n';

  if((address == config_word_address()) && config_modes) {
    config_word = cfg_word;

    if (m_configMemory && m_configMemory->getConfigWord(0))
      m_configMemory->getConfigWord(0)->set((int)cfg_word);

    /*
    config_modes->config_mode = (config_modes->config_mode & ~7) | (cfg_word & 7);

    config_word = cfg_word;

    if((bool)verbose && config_modes)
      config_modes->print();
    */
    return true;
  }
  return false;
}

void _12bit_processor::create()
{
  pa_bits = 0;                 // Assume only one code page (page select bits in status)

  pic_processor::create();

  fsr = new FSR_12(this,"fsr",fsr_register_page_bits(), fsr_valid_bits());

  // Sigh. Hack, hack,... manually assign indf bits
  indf->fsr_mask = 0x1f;
  indf->base_address_mask1 = 0x0;
  indf->base_address_mask2 = 0x1f;

  stack->stack_mask = 1;        // The 12bit core only has 2 stack positions

  //1 tmr0.set_cpu(this);
  //1 tmr0.start(0);
}

void _12bit_processor::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new ConfigWord("CONFIG", 0xfff,"Configuration Word",this,0xfff));
}

void _12bit_processor::option_new_bits_6_7(uint bits)
{
  cout << "12bit, option bits 6 and/or 7 changed\n";
}

void _12bit_processor::put_option_reg(uint val)
{
  option_reg->put(val);
}

void _12bit_processor::dump_registers ()
{
}

void _12bit_processor::enter_sleep()
{
    tmr0.sleep();
    pic_processor::enter_sleep();
}

void _12bit_processor::exit_sleep()
{
  if (m_ActivityState == ePASleeping)
  {
    tmr0.wake();
    pic_processor::exit_sleep();
  }
}

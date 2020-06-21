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

#include "pic-processor.h"

#ifndef __12_BIT_PROCESSORS_H__
#define __12_BIT_PROCESSORS_H__

class _12bit_processor;
class IOPIN;
class OptionTraceType;

extern instruction *disasm12 (pic_processor *cpu,uint address,uint inst);

class _12bit_processor : public pic_processor
{
    public:

    #define WDTE                     4

    enum _12BIT_DEFINITIONS
    {
      PA0 = 1<<5,     /* Program page preselect bits (in status) */
      PA1 = 1<<6,
      PA2 = 1<<7,

      RP0 = 1<<5,     /* Register page select bits (in fsr) */
      RP1 = 1<<6

    };

      uint pa_bits;   /* a CPU dependent bit-mask defining which of the program
                               * page bits in the status register are significant. */
      OPTION_REG   *option_reg;


      virtual void reset(RESET_TYPE r);
      virtual void save_state();

      #define FILE_REGISTERS  0x100
      virtual uint register_memory_size () const { return FILE_REGISTERS;}
      virtual void dump_registers();
      virtual void tris_instruction(uint tris_register){return;}
      virtual void create();
      virtual void create_config_memory();
      virtual PROCESSOR_TYPE isa(){return _12BIT_PROCESSOR_;}
      virtual PROCESSOR_TYPE base_isa(){return _12BIT_PROCESSOR_;}
      virtual instruction * disasm (uint address, uint inst)
        {
          return disasm12(this, address, inst);
        }
      void interrupt() { return; }

      // Declare a set of functions that will allow the base class to
      // get information about the derived classes. NOTE, the values returned here
      // will cause errors if they are used -- the derived classes must define their
      // parameters appropriately.
      virtual uint program_memory_size(){ return 3; }; // A bogus value that will cause errors if used
      // The size of a program memory bank is 2^11 bytes for the 12-bit core
      virtual void create_sfr_map() { return;};

      // Return the portion of pclath that is used during branching instructions
      // Actually, the 12bit core has no pclath. However, the program counter class doesn't need
      // to know that. Thus this virtual function really just returns the code page for the
      // 12bit cores.

      virtual uint get_pclath_branching_jump()
        {
          return ((status->value.get() & pa_bits) << 4);
        }

      // The modify pcl type instructions execute exactly like call instructions
      virtual uint get_pclath_branching_modpcl()
        { return ((status->value.get() & pa_bits) << 4);}

      // The valid bits in the FSR register vary across the various 12-bit devices
      virtual uint fsr_valid_bits() { return 0x1f; } // Assume only 32 register addresses 

      virtual uint fsr_register_page_bits() { return 0; }// Assume only one register page.
        
      virtual void put_option_reg(uint);
      virtual void option_new_bits_6_7(uint);

      virtual uint config_word_address() const {return 0xfff;};
      virtual bool set_config_word(uint address, uint cfg_word);
      virtual void enter_sleep();
      virtual void exit_sleep();

      _12bit_processor(const char *_name=0, const char *desc=0);
      virtual ~_12bit_processor();

    protected:
      OptionTraceType *mOptionTT;
};

#define cpu12 ( (_12bit_processor *)cpu)

#endif

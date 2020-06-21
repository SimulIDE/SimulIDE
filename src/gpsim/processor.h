/*
   Copyright (C) 1998-2003 T. Scott Dattalo

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

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <cstdio>
#include <vector>
#include <list>
#include <map>

#include "gpsim_classes.h"
#include "modules.h"
#include "registers.h"
#include "gpsim_time.h"

extern void init_attributes();
extern void destroy_attributes();

class Processor;
class ProcessorConstructor;
class ProgramFileType;
class FileContext;
class FileContextList;
class CPU_Freq;
class CPU_Vdd;
class Stimulus_Node;

//---------------------------------------------------------
/// MemoryAccess - A base class designed to support
/// access to memory. For the PIC, this class is extended by
/// the ProgramMemoryAccess and RegisterMemoryAccess classes.

class MemoryAccess :  public TriggerObject, public gpsimObject
{
    public:

      explicit MemoryAccess(Processor *new_cpu);
      ~MemoryAccess();

      virtual Processor *get_cpu(void);

      list<Register *> SpecialRegisters;

    protected:

      Processor *cpu;     /// The processor to which this object belongs
};


//---------------------------------------------------------
/// The ProgramMemoryAccess class is the interface used
/// by objects other than the simulator to manipulate the
/// pic's program memory. For example, the breakpoint class
/// modifies program memory when break points are set or
/// cleared. The modification goes through here.

class ProgramMemoryAccess :  public MemoryAccess
{
     public:

      explicit ProgramMemoryAccess(Processor *new_cpu);
      ~ProgramMemoryAccess();

      virtual void putToAddress(uint addr, instruction *new_instruction);
      virtual void putToIndex(uint uIndex, instruction *new_instruction);
      
      instruction *getFromAddress(uint addr);
      instruction *getFromIndex(uint uIndex);
      instruction *get_base_instruction(uint addr);
      
      uint get_rom(uint addr);
      void put_rom(uint addr,uint value);
      
      virtual uint get_PC(void);
      virtual void set_PC(uint);
      virtual Program_Counter *GetProgramCounter(void);

      void remove(uint address, instruction *bp_instruction);

      uint get_opcode(uint addr);
      void put_opcode(uint addr, uint new_opcode);
      char *get_opcode_name(uint addr, char *buffer, uint size);
      bool hasValid_opcode_at_address(uint address);
      bool hasValid_opcode_at_index(uint uIndex);
      
      // When a pic is replacing one of it's own instructions, this routine is called.
      void put_opcode_start(uint addr, uint new_opcode);

      virtual void callback(void);
      void init( Processor* );

      // isModified -- returns true if the program at the address has been modified
      // (this is only valid for those processor capable of writing to their own program memory)
      bool isModified( uint address );

    private:
      uint
        _address,
        _opcode,
        _state;
};


//---------------------------------------------------------
/// The RegisterMemoryAccess class is the interface used
/// by objects other than the simulator to manipulate the
/// cpu's register memory.

class RegisterMemoryAccess : public MemoryAccess
{
     public:

      explicit RegisterMemoryAccess(Processor *pCpu);
      virtual ~RegisterMemoryAccess();
      
      virtual Register *get_register(uint address);
      uint get_size(void) { return nRegisters; }
      void set_Registers(Register **_registers, int _nRegisters);

      // The insertRegister and removeRegister methods are used primarily
      // to set and clear breakpoints.
      bool insertRegister(uint address, Register *);
      bool removeRegister(uint address, Register *);

      void reset(RESET_TYPE r);

      Register &operator [] (uint address);

     private:
      uint nRegisters;
      bool initialized;
      Register **registers;       // Pointer to the array of registers.
};


class CPU_Vdd : public Float
{
    public:
      CPU_Vdd(Processor * _cpu, double freq); //const char *_name, double newValue, const char *desc);

      virtual void set( double d );

    private:
      Processor * cpu;
};

//------------------------------------------------------------------------
//
/// Processor - a generic base class for processors supported by gpsim

class Processor : public Module
{
    public:
      Processor(const char *_name=0, const char *desc=0);
      virtual ~Processor();

      uint clocks_per_inst; /// Oscillator cycles for 1 instruction

      /// Stimulus nodes for CVREF and V06REF
      Stimulus_Node         *CVREF;
      Stimulus_Node         *V06REF;

      /// Processor capabilities
      unsigned long m_Capabilities;
      enum {
        eSTACK                  = 0x00000001,
        eWATCHDOGTIMER          = 0x00000002,
        eBREAKONSTACKOVER       = 0x00000004,
        eBREAKONSTACKUNDER      = 0x00000009,
        eBREAKONWATCHDOGTIMER   = 0x00000010,
      };
      unsigned long GetCapabilities();

      /// Processor RAM
      Register    **registers;
      Register    **register_bank;/// Currently selected RAM bank
      
      instruction **program_memory;/// Program memory - where instructions are stored.

      /// Program memory interface
      ProgramMemoryAccess  *pma;
      virtual ProgramMemoryAccess * createProgramMemoryAccess(Processor *processor);
      virtual void                  destroyProgramMemoryAccess(ProgramMemoryAccess *pma);
      virtual instruction *         ConstructInvalidInstruction(Processor *processor,
        uint address, uint new_opcode) { return new invalid_instruction(processor,address,new_opcode); }
        
      /// register memory interface
      RegisterMemoryAccess rma;

      /// eeprom memory interface (if present).
      RegisterMemoryAccess ema;
      uint m_uPageMask;
      uint m_uAddrMask;

      /// Program Counter
      Program_Counter *pc;

      invalid_instruction bad_instruction; // Processor's 'bad_instruction' object

      // Creation and manipulation of registers
      void create_invalid_registers ();
      void delete_invalid_registers ();
      
      void add_file_registers(uint start_address, uint end_address, uint alias_offset);
      void delete_file_registers(uint start_address, uint end_address, bool bRemoveWithoutDelete=false);
      void alias_file_registers(uint start_address, uint end_address, uint alias_offset);

      virtual void init_register_memory(uint memory_size);
      virtual uint register_memory_size () const = 0;
      
      virtual uint CalcJumpAbsoluteAddress(uint uInstAddr, uint uDestAddr) { return uDestAddr; }
      virtual uint CalcCallAbsoluteAddress(uint uInstAddr, uint uDestAddr) { return uDestAddr; }

      // Creation and manipulation of Program Memory
      virtual void init_program_memory(uint memory_size);
      virtual void init_program_memory(uint address, uint value);
      virtual void init_program_memory_at_index(uint address, uint value);
      virtual void init_program_memory_at_index(uint address, const unsigned char *, int nBytes);
      
      virtual uint program_memory_size(void) const {return 0;};
      virtual uint program_address_limit(void) const { return map_pm_index2address(program_memory_size());};
      virtual uint get_program_memory_at_address( uint address );
      void build_program_memory( uint *memory, uint minaddr, uint maxaddr);
      virtual void erase_program_memory( uint address );

      virtual int  map_rm_address2index(int address) {return address;};
      virtual int  map_rm_index2address(int index) {return index;};
      virtual int  map_pm_address2index(int address) const {return address;};
      virtual int  map_pm_index2address(int index) const {return index;};
  
      virtual void set_out_of_range_pm(uint address, uint value);
      
      uint64_t cycles_used(uint address);
      
      virtual bool IsAddressInRange(uint address) { return address < program_address_limit(); }

      virtual int opcode_size() { return 2;}// opcode_size - number of bytes for an opcode.

      // Symbolic debugging
      virtual void dump_registers(void);
      virtual instruction * disasm( uint address,uint inst)=0;
      
      // Processor State
      virtual void save_state(FILE *); // copy the entire processor state to a file
      virtual void save_state();// take an internal snap shot of the current state.
      virtual void load_state(FILE *);// restore the processor state

      // Execution control
      virtual void finish(void) = 0;

      virtual void sleep(void) {};
      virtual void exit_sleep() {fputs("RRR exit_sleep\n", stderr);}
      virtual void step(uint steps,bool refresh=true) = 0;
      virtual void step_over(bool refresh=true);
      virtual void step_one(bool refresh=true) = 0;
      virtual void step_cycle() = 0;
      virtual void interrupt(void) = 0 ;

      bool getBreakOnInvalidRegisterRead() { return *m_pbBreakOnInvalidRegisterRead; }
      bool getBreakOnInvalidRegisterWrite() { return *m_pbBreakOnInvalidRegisterWrite; }

      // Processor Clock control
      //
      void set_frequency(double f);
      void set_frequency_rc(double f);
      void set_RCfreq_active(bool);
      virtual double get_frequency();

      void set_ClockCycles_per_Instruction(uint cpi) { clocks_per_inst = cpi; }
      uint get_ClockCycles_per_Instruction(void)     { return clocks_per_inst; }

      void update_cps(void);

      virtual double get_OSCperiod();
      virtual double get_InstPeriod() { return get_OSCperiod()* get_ClockCycles_per_Instruction(); }

      // Configuration control
      virtual bool set_config_word(uint address, uint cfg_word) {return false;} // fixme - make this a pure virtual function...
      virtual uint get_config_word(uint address) = 0;
      virtual uint config_word_address(void) {return 0;}
      virtual int get_config_index(uint address){return -1;};

      virtual void reset(RESET_TYPE r) = 0;    // Processor reset

      virtual double get_Vdd() { return m_vdd->getVal(); }
      virtual void set_Vdd(double v) { m_vdd->set(v); }
      virtual void update_vdd();

      virtual void Debug(); // Debugging - used to view the state of the processor (or whatever).

      virtual void create(void);
      static Processor *construct(void);
      ProcessorConstructor  *m_pConstructorObject;

      CPU_Vdd   *m_vdd;
      
      ClockPhase            *mCurrentPhase;
      phaseExecute1Cycle    *mExecute1Cycle;
      phaseExecute2ndHalf   *mExecute2ndHalf;     // misnomer - should be 2-cycle
      phaseCaptureInterrupt *mCaptureInterrupt;
      phaseIdle             *mIdle;
      
    private:
      CPU_Freq *mFrequency;
      uint  m_ProgramMemoryAllocationSize;

      Boolean *m_pbBreakOnInvalidRegisterRead;
      Boolean *m_pbBreakOnInvalidRegisterWrite;
};

//-------------------------------------------------------------------
//
// ProcessorConstructor -- a class to handle all of gpsim's supported
// processors
//
class ProcessorConstructor
{
    public:
        typedef Processor* (*tCpuContructor) (const char *_name);

        ProcessorConstructor( tCpuContructor    _cpu_constructor,
                             const char *name1,
                             const char *name2,
                             const char *name3=0,
                             const char *name4=0);

        virtual ~ProcessorConstructor(){}
      
        virtual Processor* ConstructProcessor( const char *opt_name=0 );
        
 static list <ProcessorConstructor*>* GetList();
 
 static Processor* CreatePic( const char *type );
 
 static string     dump(void);
 
        #define nProcessorNames 4 // The processor name (plus upto three aliases).
      const char *names[nProcessorNames];
 
    protected:
        tCpuContructor cpu_constructor; // A pointer to a function that when called will construct a processor
        
 static list <ProcessorConstructor*>*  processor_list;
};

#endif

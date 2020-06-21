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

/*
  stuff that needs to be fixed:

  Register aliasing
  The "invalid instruction" in program memory.
*/

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <cstdio>
#include <typeinfo>

#include "config.h"
#include "gpsim_classes.h"
#include "processor.h"
#include "pic-processor.h"
#include "clock_phase.h"
#include "errors.h"


//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

static char pkg_version[] = PACKAGE_VERSION;

class CPU_Freq : public Float
{
    public:
      CPU_Freq(Processor * _cpu, double freq); //const char *_name, double newValue, const char *desc);

      virtual void set(double d);
      void set_rc_freq(double d);
      virtual void get(double &);
      void set_rc_active(bool _use_rc_freq) { use_rc_freq = _use_rc_freq;}
      
    private:
      Processor * cpu;
      double         RCfreq;
      bool                use_rc_freq;
};

CPU_Freq::CPU_Freq(Processor * _cpu, double freq)
  : Float("frequency",freq, " oscillator frequency."),
    cpu(_cpu), RCfreq(0.), use_rc_freq(false)
{
}

void CPU_Freq::set_rc_freq(double d)
{
   RCfreq = d;
}

void CPU_Freq::get(double &d)
{
    if (use_rc_freq) d = RCfreq;
    else
    {
        double x;
        Float::get(x);
        d = x;
    }
}
       
void CPU_Freq::set(double d)
{
  pic_processor *pCpu = dynamic_cast<pic_processor *>(cpu);

  Float::set ( d );
  if ( cpu )  cpu->update_cps();
  if ( pCpu ) pCpu->wdt.update();
}

CPU_Vdd::CPU_Vdd(Processor * _cpu, double vdd)
  : Float("Vdd", vdd , "Processor supply voltage"),
    cpu(_cpu)
{
}

void CPU_Vdd::set(double d)
{
  Float::set ( d );
  if ( cpu ) cpu->update_vdd();
}

//------------------------------------------------------------------------
//
// Processor - Constructor
//
Processor::Processor(const char *_name, const char *_desc)
  : Module(_name, _desc),
    pma(0),
    rma(this),
    ema(this),
    pc(0),
    bad_instruction(0, 0x3fff, 0),
    mFrequency(0)
{
  registers = 0;

  m_pConstructorObject = 0;
  m_Capabilities = 0;
  m_ProgramMemoryAllocationSize = 0;

  set_ClockCycles_per_Instruction(4);
  update_cps();

  // derived classes need to override these values
  m_uPageMask    = 0x00;
  m_uAddrMask    = 0xff;

  // let the processor version number simply be gpsim's version number.
  version = &pkg_version[0];

  m_vdd = new CPU_Vdd(this, 5.0);

  m_pbBreakOnInvalidRegisterRead = new Boolean("BreakOnInvalidRegisterRead",
    true, "Halt simulation when an invalid register is read from.");

  m_pbBreakOnInvalidRegisterWrite = new Boolean("BreakOnInvalidRegisterWrite",
    true, "Halt simulation when an invalid register is written to.");

  set_Vdd(5.0);
}

Processor::~Processor()
{
  delete_invalid_registers();

  delete []registers;

  destroyProgramMemoryAccess(pma);

  for (uint i = 0; i < m_ProgramMemoryAllocationSize; i++)
  {
    if (program_memory[i] != &bad_instruction) delete program_memory[i];
  }
  delete []program_memory;
}

unsigned long Processor::GetCapabilities() 
{
  return m_Capabilities;
}

//------------------------------------------------------------------------
// Attributes

void Processor::set_RCfreq_active(bool state)
{
    if (mFrequency) mFrequency->set_rc_active(state);
    update_cps();
}
void Processor::set_frequency(double f)
{
  if(mFrequency) mFrequency->set(f);
  update_cps();
}
void Processor::set_frequency_rc(double f)
{
  if(mFrequency) mFrequency->set_rc_freq(f);
  update_cps();
}
double Processor::get_frequency()
{
  double d=0.0;

  if(mFrequency) mFrequency->get(d);
  return d;
}

void Processor::update_cps(void)
{
  get_cycles().set_instruction_cps((uint64_t)(get_frequency()/clocks_per_inst));
}

double  Processor::get_OSCperiod()
{
  double f = get_frequency();

  if(f>0.0) return 1/f;
  else      return 0.0;
}

//-------------------------------------------------------------------
//
// init_register_memory (uint memory_size)
//
// Allocate an array for holding register objects.
//
void Processor::init_register_memory (uint memory_size)
{
  registers = new Register *[memory_size];

    if (registers  == 0)
    {
      throw new FatalError("Out of memory - PIC register space");
    }

  // For processors with banked memory, the register_bank corresponds to the
  // active bank. Let this point to the beginning of the register array for
  // now.

  register_bank = registers;

  rma.set_Registers(registers, memory_size);

  // Make all of the file registers 'undefined' (each processor derived from this base
  // class defines its own register mapping).

  for (uint i = 0; i < memory_size; i++) registers[i] = 0;
}

//-------------------------------------------------------------------
// create_invalid_registers
//
//   The purpose of this function is to complete the initialization
// of the file register memory by placing an instance of an 'invalid
// file register' at each 'invalid' memory location. Most of PIC's
// do not use the entire address space available, so this routine
// fills the voids.

void Processor::create_invalid_registers ()
{
  uint addr;

  // Now, initialize any undefined register as an 'invalid register'
  // Note, each invalid register is given its own object. This enables
  // the simulation code to efficiently capture any invalid register
  // access. Furthermore, it's possible to set break points on
  // individual invalid file registers. By default, gpsim halts whenever
  // there is an invalid file register access.

  for (addr = 0; addr < register_memory_size(); addr+=map_rm_index2address(1)) 
  {
    uint index = map_rm_address2index(addr);

    if (!registers[index]) 
    {
      char nameBuff[100];
      snprintf(nameBuff,sizeof(nameBuff), "INVREG_%X",addr);

      registers[index] = new InvalidRegister(this, nameBuff);
      registers[index]->setAddress(addr);
    }
  }
}

void Processor::delete_invalid_registers ()
{
  for( uint i=0; i<rma.get_size(); i++ ) 
  {
   // cout << __FUNCTION__ << "  reg: 0x"<<hex << i << " ptr:" << registers[i] << endl;

    InvalidRegister *pReg = dynamic_cast<InvalidRegister *> (registers[i]);
    if( pReg ) 
    {
      delete registers[i];
      registers[i]= 0;
    }
    else if (registers[i])
    {
      char reg_name[11];
      cout << __FUNCTION__ << "  reg: 0x"<<hex << i << " ptr:" << registers[i] ;
      
      cout.flush();
      strncpy(reg_name, registers[i]->name().c_str(), 10);
      reg_name[10] = 0;
      cout << " " << reg_name <<endl;
    }
  }
}

//-------------------------------------------------------------------
//    add_file_registers
//
//  The purpose of this member function is to allocate memory for the
// general purpose registers.
//
void Processor::add_file_registers(uint start_address, uint end_address, uint alias_offset)
{
  // Initialize the General Purpose Registers:
  Dprintf((" from 0x%x to 0x%x alias 0x%x\n", start_address, end_address, alias_offset));

  char str[100];
  for( uint j = start_address; j <= end_address; j++ ) 
  {
#ifdef DEBUG
    if (j == 0x11)
    {
        printf("Processor::add_file_registers j 0x%x\n", j);
    }
#endif
    if (registers[j] && (registers[j]->isa() == Register::INVALID_REGISTER))
        delete registers[j];
        
    else if (registers[j])
        cout << __FUNCTION__ << " Already register " << registers[j]->name() << " at 0x" << hex << j <<endl;

    //The default register name is simply its address
    snprintf (str, sizeof(str), "REG%03X", j);
    registers[j] = new Register(this, str);

    if (alias_offset) 
    {
      registers[j + alias_offset] = registers[j];
      registers[j]->alias_mask = alias_offset;
    } 
    else registers[j]->alias_mask = 0;

    registers[j]->setAddress(j);
  }
}

//-------------------------------------------------------------------
//    delete_file_registers
//
//  The purpose of this member function is to delete file registers
//
void Processor::delete_file_registers(uint start_address,
                                      uint end_address,
                                      bool bRemoveWithoutDelete)
{
#define DFR_DEBUG 0
  if (DFR_DEBUG)
    cout << __FUNCTION__
         << "  start:" << hex << start_address
         << "  end:" << hex << end_address
         << endl;

  //  FIXME - this function is bogus.
  // The aliased registers do not need to be searched for - the alias mask
  // can tell at what addresses a register is aliased.

#define SMALLEST_ALIAS_DISTANCE  32
#define ALIAS_MASK (SMALLEST_ALIAS_DISTANCE-1)

  if (start_address != end_address) Dprintf(("from 0x%x to 0x%x\n", start_address, end_address));

  for( uint j = start_address; j <= end_address; j++) 
  {
    if(registers[j]) 
    {
      Register *thisReg = registers[j];
      Register *replaced = thisReg->getReplaced();

      if(thisReg->alias_mask) 
      {
        // This register appears in more than one place. Let's find all
        // of its aliases.
        for( uint i=j&ALIAS_MASK; i<rma.get_size(); i+=SMALLEST_ALIAS_DISTANCE)
          if(thisReg == registers[i]) 
          {
            if(DFR_DEBUG) cout << "   removing at address:" << hex << i << endl;
            registers[i] = 0;
          }
      }
      if(DFR_DEBUG) cout << " deleting: " << hex << j << endl;
      registers[j] = 0;
      
      if (!bRemoveWithoutDelete)
      {
        if (replaced) delete replaced;
        delete thisReg;
      }
    }
    else printf("%s register 0x%x already deleted\n", __FUNCTION__, j);
  }
}

//-------------------------------------------------------------------
//
//    alias_file_registers
//
//  The purpose of this member function is to alias the
// general purpose registers.
//
void Processor::alias_file_registers(uint start_address, uint end_address, uint alias_offset)
{
  // FIXME -- it'd probably make better sense to keep a list of register addresses at
  // which a particular register appears.
#ifdef DEBUG
  if (start_address == 0x20)
        printf("DEBUG trace %x\n", start_address);
  if (start_address != end_address)
        Dprintf((" from 0x%x to 0x%x alias_offset 0x%x\n", start_address, end_address, alias_offset));
#endif

    for( uint j = start_address; j <= end_address; j++)
    {
        if (alias_offset && (j+alias_offset < rma.get_size()))
        {
          if (registers[j + alias_offset])
          {
                if (registers[j + alias_offset] == registers[j])
                    printf("alias_file_register Duplicate alias %s from 0x%x to 0x%x \n",registers[j + alias_offset]->name().c_str(), j, j+alias_offset);
                else
                    delete registers[j + alias_offset];

           }
          registers[j + alias_offset] = registers[j];
          
          if (registers[j]) registers[j]->alias_mask = alias_offset;
        }
    }
}

//-------------------------------------------------------------------
//
// init_program_memory(uint memory_size)
//
// The purpose of this member function is to allocate memory for the
// pic's code space. The 'memory_size' parameter tells how much memory
// is to be allocated
//
//  The following is not correct for 18f2455 and 18f4455 processors
//  so test has been disabled (RRR)
//
//  AND it should be an integer of the form of 2^n.
// If the memory size is not of the form of 2^n, then this routine will
// round up to the next integer that is of the form 2^n.
//
//   Once the memory has been allocated, this routine will initialize
// it with the 'bad_instruction'. The bad_instruction is an instantiation
// of the instruction class that chokes gpsim if it is executed. Note that
// each processor owns its own 'bad_instruction' object.

void Processor::init_program_memory (uint memory_size)
{
#ifdef RRR
  if ((memory_size-1) & memory_size)
    {
      cout << "*** WARNING *** memory_size should be of the form 2^N\n";
      memory_size = (memory_size + ~memory_size) & MAX_PROGRAM_MEMORY;
      cout << "gpsim is rounding up to memory_size = " << memory_size << '\n';
    }
#endif
  // Initialize 'program_memory'. 'program_memory' is a pointer to an array of
  // pointers of type 'instruction'. This is where the simulated instructions
  // are stored.
  program_memory = new instruction *[memory_size];
  if (program_memory == 0) {
    throw new FatalError("Out of memory for program space");
  }

  m_ProgramMemoryAllocationSize = memory_size;

  bad_instruction.set_cpu(this);
  for (uint i = 0; i < memory_size; i++) program_memory[i] = &bad_instruction;

  pma = createProgramMemoryAccess(this);
  pma->name();
}

ProgramMemoryAccess * Processor::createProgramMemoryAccess(Processor *processor) {
  return new ProgramMemoryAccess(processor);
}

void Processor::destroyProgramMemoryAccess(ProgramMemoryAccess *pma) {
  delete pma;
}

//-------------------------------------------------------------------
// init_program_memory(int address, int value)
//
// The purpose of this member fucntion is to instantiate an Instruction
// object in the program memory. If the opcode is invalid, then a 'bad_instruction'
// is inserted into the program memory instead. If the address is beyond
// the program memory address space, then it may be that the 'opcode' is
// is in fact a configuration word.
//
void Processor::init_program_memory(uint address, uint value)
{
  uint uIndex = map_pm_address2index(address);

  if (!program_memory) 
  {
    std::stringstream buf;
    buf << "ERROR: internal bug " << __FILE__ << ":" << __LINE__;
    throw new FatalError(buf.str());
  }

  if(uIndex < program_memory_size()) 
  {
    if(program_memory[uIndex] != 0 && program_memory[uIndex]->isa() != instruction::INVALID_INSTRUCTION) 
    {
      delete program_memory[uIndex];           // this should not happen
    }
    program_memory[uIndex] = disasm(address,value);
    if(program_memory[uIndex] == 0)
      program_memory[uIndex] = &bad_instruction;
  }
  else if (set_config_word(address, value))
  { }
  else set_out_of_range_pm(address,value);  // could be e2prom
}
//-------------------------------------------------------------------
//erase_program_memory(uint address)
//
//        Checks if a program memory location contains an instruction
//        and deletes it if it does.
//
void Processor::erase_program_memory(uint address)
{
  uint uIndex = map_pm_address2index(address);

  if (!program_memory) 
  {
    std::stringstream buf;
    buf << "ERROR: internal bug " << __FILE__ << ":" << __LINE__;
    throw new FatalError(buf.str());
  }
  if(uIndex < program_memory_size())
  {
    if(program_memory[uIndex] != 0 && program_memory[uIndex]->isa() != instruction::INVALID_INSTRUCTION) 
    {
      delete program_memory[uIndex];
      program_memory[uIndex] = &bad_instruction;
    }
  }
  else
  {
    cout << "Erase Program memory\n";
    cout << "Warning::Out of range address " << hex << address << endl;
    cout << "Max allowed address is 0x" << hex << (program_address_limit()-1) << '\n';
  }
}

void Processor::init_program_memory_at_index(uint uIndex, uint value)
{
  init_program_memory(map_pm_index2address(uIndex), value);
}

void Processor::init_program_memory_at_index(uint uIndex,
                                             const unsigned char *bytes, int nBytes)
{
  for (int i =0; i<nBytes/2; i++)
    init_program_memory_at_index(uIndex+i, (((uint)bytes[2*i+1])<<8)  | bytes[2*i]);
}

//------------------------------------------------------------------
// Fetch the rom contents at a particular address.
uint Processor::get_program_memory_at_address(uint address)
{
  uint uIndex = map_pm_address2index(address);

  return (uIndex < program_memory_size() && program_memory[uIndex])
    ? program_memory[uIndex]->get_opcode()
    : 0xffffffff;
}

//-------------------------------------------------------------------
// build_program_memory - given an array of opcodes this function
// will convert them into instructions and insert them into the
// simulated program memory.
//
void Processor::build_program_memory(uint *memory,
                                     uint minaddr,
                                     uint maxaddr)
{
  for (uint i = minaddr; i <= maxaddr; i++)
    if(memory[i] != 0xffffffff)
      init_program_memory(i, memory[i]);
}

//-------------------------------------------------------------------
/** @brief Write a word of data into memory outside flash
 *
 *  This method is called when loading data from the COD or HEX file
 *  and the address is not in the program ROM or normal config space.
 *  In this base class, there is no such memory. Real processors,
 *  particularly those with EEPROM, will need to override this method.
 *
 *  @param  address Memory address to set. Byte address on 18F
 *  @param  value   Word data to write in.
 */
void Processor::set_out_of_range_pm(uint address, uint value)
{
  cout << "Warning::Out of range address " << address << " value " << value << endl;
  cout << "Max allowed address is 0x" << hex << (program_address_limit()-1) << '\n';
}

void Processor::save_state(FILE *fp)
{
  if(!fp) return;

  fprintf(fp,"PROCESSOR:%s\n",name().c_str());

  for( uint i=1; i<register_memory_size(); i++) 
  {
    Register *reg = rma.get_register(i);

    if(reg && reg->isa() != Register::INVALID_REGISTER) 
    {
      fprintf(fp,"R:%X:%s:(%X,%X)\n",
              reg->address,
              reg->name().c_str(),
              reg->value.get(),
              reg->value.geti());
    }
  }
  if(pc) fprintf(fp,"P:0:PC:%X\n",pc->value);
}

void Processor::save_state(void)
{
}

void Processor::load_state(FILE *fp)
{
  if(!fp) return;
  cout << "Not implemented\n";
}

/* If Vdd is changed, fix up the digital high low thresholds */
void Processor::update_vdd()
{
    IOPIN *pin;
    for(int i=1; i <= get_pin_count(); i++)
    {
        pin = get_pin(i);
        if (pin)  pin->set_digital_threshold(get_Vdd());
    }
}

Processor * Processor::construct(void)
{
  cout << " Can't create a generic processor\n";
  return 0;
}

//-------------------------------------------------------------------
//
// step_over - In most cases, step_over will simulate just one instruction.
// However, if the next instruction is a branching one (e.g. goto, call,
// return, etc.) then a break point will be set after it and gpsim will
// begin 'running'. This is useful for stepping over time-consuming calls.
//
void Processor::step_over (bool refresh)
{
  step(1,refresh); // Try one step
}

//-------------------------------------------------------------------
//    create
//
//  The purpose of this member function is to 'create' a pic processor.
// Since this is a base class member function, only those things that
// are common to all pics are created.

void Processor::create (void)
{
    std::stringstream buf;
    buf << " a generic processor cannot be created " << __FILE__ << ":" << __LINE__;
    throw new FatalError(buf.str());
}

void Processor::dump_registers (void)
{
  //  parse_string("dump");
}

void Processor::Debug()
{
  cout << " === Debug === \n";
  if(pc) cout << "PC=0x"<<hex << pc->value << endl;
}

uint64_t Processor::cycles_used(uint address)
{
    return program_memory[address]->getCyclesUsed();
}

MemoryAccess::MemoryAccess(Processor *new_cpu)
{
  cpu = new_cpu;
}

MemoryAccess::~MemoryAccess()
{
}

Processor *MemoryAccess::get_cpu(void)
{
  return cpu;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//
// ProgramMemoryAccess
//
// The ProgramMemoryAccess class provides an interface to the processor's
// program memory. On Pic processors, this is the memory where instructions
// are stored.
//
ProgramMemoryAccess::ProgramMemoryAccess(Processor *new_cpu) :
  MemoryAccess(new_cpu)
{
  init(new_cpu);
}

ProgramMemoryAccess::~ProgramMemoryAccess()
{
}

void ProgramMemoryAccess::init( Processor *new_cpu )
{
    _address = _opcode = _state = 0;
}

void ProgramMemoryAccess::putToAddress( uint address, instruction *new_instruction )
{
    putToIndex(cpu->map_pm_address2index(address), new_instruction);
}

void ProgramMemoryAccess::putToIndex( uint uIndex, instruction *new_instruction )
{
    if(!new_instruction) return;

    cpu->program_memory[uIndex] = new_instruction;

    new_instruction->update();
}

void ProgramMemoryAccess::remove(uint address, instruction *bp_instruction)
{
}

instruction *ProgramMemoryAccess::getFromAddress(uint address)
{
  if(!cpu || !cpu->IsAddressInRange(address)) return &cpu->bad_instruction;
  
  uint uIndex = cpu->map_pm_address2index(address);
  return getFromIndex(uIndex);
}

instruction *ProgramMemoryAccess::getFromIndex(uint uIndex)
{
  if(uIndex < cpu->program_memory_size()) return cpu->program_memory[uIndex];
  else                                    return 0;
}

// like get, but will ignore instruction break points
instruction *ProgramMemoryAccess::get_base_instruction(uint uIndex)
{
    instruction *p;
    p=getFromIndex(uIndex);
    return p;
}

//----------------------------------------
// get_rom - return the rom contents from program memory
//           If the address is normal program memory, then the opcode
//           of the instruction at that address is returned.
//           If the address is some other special memory (like configuration
//           memory in a PIC) then that data is returned instead.

uint ProgramMemoryAccess::get_rom(uint addr)
{
  return cpu->get_program_memory_at_address(addr);
}

//----------------------------------------
// put_rom - write new data to the program memory.
//           If the address is in normal program memory, then a new instruction
//           will be generated (if possible). If the address is some other
//           special memory (like configuration memory), then that area will
//           be updated.
//
void ProgramMemoryAccess::put_rom(uint addr,uint value)
{
  return cpu->init_program_memory(addr,value);
}

//----------------------------------------
// get_opcode - return an opcode from program memory.
//              If the address is out of range return 0.

uint ProgramMemoryAccess::get_opcode(uint addr)
{
  instruction * pInstr = getFromAddress(addr);
  if(pInstr != 0) return pInstr->get_opcode();
  else            return 0;
}

//----------------------------------------
// get_opcode_name - return an opcode name from program memory.
//                   If the address is out of range return 0;

char *ProgramMemoryAccess::get_opcode_name(uint addr, char *buffer, uint size)
{
  uint uIndex = cpu->map_pm_address2index(addr);
  if(uIndex < cpu->program_memory_size())
    return cpu->program_memory[uIndex]->name(buffer,size);

  *buffer = 0;
  return 0;
}

//----------------------------------------
// Get the current value of the program counter.

uint ProgramMemoryAccess::get_PC(void)
{
  if(cpu && cpu->pc) return cpu->pc->get_value();

  return 0;
}

//----------------------------------------
// Get the current value of the program counter.
void ProgramMemoryAccess::set_PC(uint new_pc)
{
  if(cpu && cpu->pc) return cpu->pc->put_value(new_pc);
}

Program_Counter *ProgramMemoryAccess::GetProgramCounter(void)
{
  if(cpu) return cpu->pc;
  return 0;
}

void ProgramMemoryAccess::put_opcode_start(uint addr, uint new_opcode)
{

  uint uIndex = cpu->map_pm_address2index(addr);
  if( (uIndex < cpu->program_memory_size()) && (_state == 0))
    {
      _state = 1;
      _address = addr;
      _opcode = new_opcode;
      get_cycles().set_break_delta(40000, this);
      bp.set_pm_write();
    }
}

void ProgramMemoryAccess::put_opcode(uint addr, uint new_opcode)
{
  uint uIndex = cpu->map_pm_address2index(addr);
  if(uIndex >= cpu->program_memory_size()) return;

  instruction *old_inst = get_base_instruction(uIndex);
  instruction *new_inst = cpu->disasm(addr,new_opcode);

  if(new_inst==0)
  {
      puts("FIXME, in ProgramMemoryAccess::put_opcode");
      return;
  }
  if(!old_inst) 
  {
    putToIndex(uIndex,new_inst);
    return;
  }
  if(old_inst->isa() == instruction::INVALID_INSTRUCTION) 
  {
    putToIndex(uIndex,new_inst);
    return;
  }
  // Now we need to make sure that the instruction we are replacing is
  // not a multi-word instruction. The 12 and 14 bit cores don't have
  // multi-word instructions, but the 16 bit cores do. If we are replacing
  // the second word of a multiword instruction, then we only need to
  // 'uninitialize' it.

  instruction *prev = get_base_instruction(cpu->map_pm_address2index(addr-1));

  if(prev) prev->initialize(false);

  cpu->program_memory[uIndex] = new_inst;
  cpu->program_memory[uIndex]->setModified(true);
  cpu->program_memory[uIndex]->update();

  delete(old_inst);
}

bool  ProgramMemoryAccess::hasValid_opcode_at_address(uint address)
{
  if(getFromAddress(address)->isa() != instruction::INVALID_INSTRUCTION)
    return true;

  return false;
}

bool  ProgramMemoryAccess::hasValid_opcode_at_index(uint uIndex)
{
  if((getFromIndex(uIndex))->isa() != instruction::INVALID_INSTRUCTION)
    return true;

  return false;
}
//--------------------------------------------------------------------------

bool  ProgramMemoryAccess::isModified(uint address)     // ***FIXME*** - address or index?
{
  uint uIndex = cpu->map_pm_address2index(address);

  if((uIndex < cpu->program_memory_size()) && cpu->program_memory[uIndex]->bIsModified())
    return true;

  return false;
}

//========================================================================
// Register Memory Access

RegisterMemoryAccess::RegisterMemoryAccess(Processor *new_cpu) :
  MemoryAccess(new_cpu)
{
  registers = 0;
  nRegisters = 0;
}

RegisterMemoryAccess::~RegisterMemoryAccess()
{
}

Register *RegisterMemoryAccess::get_register(uint address)
{
  if(!cpu || !registers || nRegisters<=address) return 0;

  Register *reg = registers[address];

  // If there are breakpoints set on the register, then drill down
  // through them until we get to the real register.

  return reg ? reg->getReg() : 0;
}

void RegisterMemoryAccess::set_Registers(Register **_registers, int _nRegisters)
{
  nRegisters = _nRegisters;
  registers = _registers;
}
//------------------------------------------------------------------------
// insertRegister - Each register address may contain a linked list of registers.
// The top most register is the one that is referenced whenever a processor
// accesses the register memory. The primary purpose of the linked list is to
// support register breakpoints. For example, a write breakpoint is implemented
// with a breakpoint class derived from the register class. Setting a write
// breakpoint involves creating the write breakpoint object and placing it
// at the top of the register linked list. Then, when a processor attempts
// to write to this register, the breakpoint object will capture this and
// halt the simulation.

bool RegisterMemoryAccess::insertRegister(uint address, Register *pReg)
{
  if(!cpu || !registers || nRegisters <= address ||!pReg) return false;

  Register *ptop = registers[address];
  pReg->setReplaced(ptop);
  registers[address] = pReg;

  return true;
}

//------------------------------------------------------------------------
// removeRegister - see comment on insertRegister. This method removes
// a register object from the breakpoint linked list.

bool RegisterMemoryAccess::removeRegister(uint address, Register *pReg)
{
  if(!cpu || !registers || nRegisters <= address ||!pReg) return false;

  Register *ptop = registers[address];

  if (ptop == pReg  &&  pReg->getReplaced())
    registers[address] = pReg->getReplaced();
  else
    while (ptop) 
    {
      Register *pNext = ptop->getReplaced();
      if (pNext == pReg) 
      {
        ptop->setReplaced(pNext->getReplaced());
        return true;
      }
      ptop = pNext;
    }
  return false;
}

static InvalidRegister AnInvalidRegister(0,"AnInvalidRegister");

Register &RegisterMemoryAccess::operator [] (uint address)
{
  if(!registers || get_size() <= address) return AnInvalidRegister;

  return *registers[address];
}

void RegisterMemoryAccess::reset (RESET_TYPE r)
{
  for(uint i=0; i<nRegisters; i++)
  {
    // Do not reset aliased registers
    if ( !(operator[](i).alias_mask && (operator[](i).alias_mask & i)))
        operator[](i).reset(r);
  }
}

//========================================================================

list<ProcessorConstructor*>* ProcessorConstructor::processor_list;

ProcessorConstructor::ProcessorConstructor(tCpuContructor _cpu_constructor,
                                           const char *name1,
                                           const char *name2,
                                           const char *name3,
                                           const char *name4)
{
  cpu_constructor = _cpu_constructor;  // Pointer to the processor constructor
  names[0] = name1;                    // First name
  names[1] = name2;                    //  and three aliases...
  names[2] = name3;
  names[3] = name4;
  // Add the processor to the list of supported processors:
  GetList()->push_back(this);
}

Processor* ProcessorConstructor::ConstructProcessor(const char *opt_name)
{
  // Instantiate a specific processor. If a name is provided, then that
  // will be used. Otherwise, the third name in the list of aliases for
  // this processor will be used instead. (Why 3rd?... Before optional
  // processor names were allowed, the default name matched what is now
  // the third alias; this maintains a backward compatibility).

  if (opt_name && strlen(opt_name)) return cpu_constructor( opt_name );
  
  return cpu_constructor( names[2] );
}

list<ProcessorConstructor*>* ProcessorConstructor::GetList()
{
  if(processor_list == NULL) processor_list = new list <ProcessorConstructor*>;

  return processor_list;
}

//------------------------------------------------------------
// Search through the list of supported processors for
// the one matching 'name' and construct Processor
Processor* ProcessorConstructor::CreatePic( const char *name )
{
  list<ProcessorConstructor*>::iterator processor_iterator;
  list<ProcessorConstructor*>* pl = GetList();

  for( processor_iterator = pl->begin(); processor_iterator != pl->end(); ++processor_iterator ) 
  {
    ProcessorConstructor *p = *processor_iterator;
    for(int j=0; j<nProcessorNames; j++)
      if(p->names[j] && strcmp(name,p->names[j]) == 0)
        return p->ConstructProcessor( name );
  }
  return 0;
}

//------------------------------------------------------------
// dump() --  Print out a list of all of the processors
//
string ProcessorConstructor::dump(void)
{
  ostringstream stream;
  list <ProcessorConstructor *>::iterator processor_iterator;

  const int nPerRow = 4;   // Number of names to print per row.

  int i,j,k,longest;

  list<ProcessorConstructor*>* pl = GetList();
  ProcessorConstructor *p;

  // loop through all of the processors and find the
  // one with the longest name
  longest = 0;

  for( processor_iterator = pl->begin(); processor_iterator != pl->end(); ++processor_iterator) 
  {
    p = *processor_iterator;
    k = strlen( p->names[1] );
    if( k>longest ) longest = k;
  }

  for( processor_iterator = pl->begin(); processor_iterator != pl->end(); )   // Print the name of each processor.
  {
    for( i=0; i<nPerRow && processor_iterator != pl->end(); i++ ) 
    {
      p = *processor_iterator++;
      stream << p->names[1];

      if( i<nPerRow-1 ) 
      {
        // if this is not the last processor in the column, then
        // pad a few spaces to align the columns.

        k = longest + 2 - strlen(p->names[1]);
        for(j=0; j<k; j++) stream << ' ';
      }
    }
    stream << endl;
  }
  stream << ends;
  return string(stream.str());
}

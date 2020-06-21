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


#ifndef  __BREAKPOINTS_H__
#define  __BREAKPOINTS_H__

#include <string>
#include <list>

#include "trigger.h"
#include "pic-instructions.h"
#include "registers.h"
#include "gpsim_object.h" // defines ObjectBreakTypes

using namespace std;

extern Integer *verbosity;  // in ../src/init.cc
class InvalidRegister;

class TriggerGroup : public TriggerAction
{
    public:

    protected:
      list<TriggerObject*> triggerList;

      virtual ~TriggerGroup(){}
};

#define MAX_BREAKPOINTS 0x400
#define BREAKPOINT_MASK (MAX_BREAKPOINTS-1)

class Breakpoints;

#if defined(IN_MODULE) && defined(_WIN32)
// we are in a module: don't access the Breakpoints object directly!
LIBGPSIM_EXPORT Breakpoints & get_bp();
#else
// we are in gpsim: use of get_bp() is recommended,
// even if the bp object can be accessed directly.
extern Breakpoints bp;

inline Breakpoints &get_bp()
{
  return bp;
}
#endif

class Breakpoints
{
    public:
      enum BREAKPOINT_TYPES
        {
          BREAK_DUMP_ALL            = 0,
          BREAK_CLEAR               = 0,
          BREAK_ON_EXECUTION        = 1<<24,
          BREAK_ON_REG_READ         = 2<<24,
          BREAK_ON_REG_WRITE        = 3<<24,
          BREAK_ON_REG_READ_VALUE   = 4<<24,
          BREAK_ON_REG_WRITE_VALUE  = 5<<24,
          BREAK_ON_INVALID_FR       = 6<<24,
          BREAK_ON_CYCLE            = 7<<24,
          BREAK_ON_WDT_TIMEOUT      = 8<<24,
          BREAK_ON_STK_OVERFLOW     = 9<<24,
          BREAK_ON_STK_UNDERFLOW    = 10<<24,
          NOTIFY_ON_EXECUTION       = 11<<24,
          PROFILE_START_NOTIFY_ON_EXECUTION = 12<<24,
          PROFILE_STOP_NOTIFY_ON_EXECUTION = 13<<24,
          NOTIFY_ON_REG_READ        = 14<<24,
          NOTIFY_ON_REG_WRITE       = 15<<24,
          NOTIFY_ON_REG_READ_VALUE  = 16<<24,
          NOTIFY_ON_REG_WRITE_VALUE = 17<<24,
          BREAK_ON_ASSERTION        = 18<<24,
          BREAK_MASK                = 0xff<<24
        };

    #define  GLOBAL_CLEAR         0
    #define  GLOBAL_STOP_RUNNING  (1<<0)
    #define  GLOBAL_INTERRUPT     (1<<1)
    #define  GLOBAL_SLEEP         (1<<2)
    #define  GLOBAL_PM_WRITE      (1<<3)
    #define  GLOBAL_SOCKET        (1<<4)
    #define  GLOBAL_LOG	          (1<<5)

      struct BreakStatus
      {
        BREAKPOINT_TYPES type;
        Processor *cpu;
        uint arg1;
        uint arg2;
        TriggerObject *bpo;
      } break_status[MAX_BREAKPOINTS];

      int m_iMaxAllocated;

      class iterator 
      {
          public:
            explicit iterator(int index) : iIndex(index) { }
            int iIndex;
            iterator & operator++(int) 
            {
              iIndex++;
              return *this;
            }
            BreakStatus * operator*() { return &get_bp().break_status[iIndex]; }
            bool operator!=(iterator &it) { return iIndex != it.iIndex; }
      };

      iterator begin() { return iterator(0); }
      iterator end() { return iterator(m_iMaxAllocated); }

      BreakStatus *get(int index)
      {
        return (index>=0 && index<MAX_BREAKPOINTS) ? &break_status[index] : 0;
      }
      int  global_break;
      
      int breakpoint_number,last_breakpoint;

      Breakpoints();
      int set_breakpoint(BREAKPOINT_TYPES,Processor *, uint, uint, TriggerObject *f = 0);

      int set_cycle_break(Processor *cpu, uint64_t cycle, TriggerObject *f = 0);
      int set_wdt_break(Processor *cpu);
      int set_stk_overflow_break(Processor *cpu);
      int set_stk_underflow_break(Processor *cpu);
      int check_cycle_break(uint abp);

      inline void clear_global() {global_break = GLOBAL_CLEAR;};
      inline void set_logging() { global_break |= GLOBAL_LOG; }
      void halt();
      inline bool have_halt()
        {
          return( (global_break & GLOBAL_STOP_RUNNING) != 0 );
        }
      inline void clear_halt()
        {
          global_break &= ~GLOBAL_STOP_RUNNING;
        }
      inline bool have_interrupt()
        {
          return( (global_break & GLOBAL_INTERRUPT) != 0 );
        }
      inline void clear_interrupt()
        {
          global_break &= ~GLOBAL_INTERRUPT;
        }
      inline void set_interrupt()
        {
          global_break |= GLOBAL_INTERRUPT;
        }
      inline bool have_sleep()
        {
          return( (global_break & GLOBAL_SLEEP) != 0 );
        }
      inline void clear_sleep()
        {
          global_break &= ~GLOBAL_SLEEP;
        }
      inline void set_sleep()
        {
          global_break |= GLOBAL_SLEEP;
        }
      inline bool have_pm_write()
        {
          return( (global_break & GLOBAL_PM_WRITE) != 0 );
        }
      inline void clear_pm_write()
        {
          global_break &= ~GLOBAL_PM_WRITE;
        }
      inline void set_pm_write()
        {
          global_break |= GLOBAL_PM_WRITE;
        }
      inline bool have_socket_break()
        {
          return( (global_break & GLOBAL_SOCKET) != 0);
        }
      inline void set_socket_break()
        {
          global_break |= GLOBAL_SOCKET;
        }
      inline void clear_socket_break()
        {
          global_break &= ~GLOBAL_SOCKET;
        }

      bool dump1(uint bp_num, int dump_type = BREAK_DUMP_ALL);
      bool dump(TriggerObject *);
      void dump(int dump_type = BREAK_DUMP_ALL);
      void clear(uint b);
      bool bIsValid(uint b);
      bool bIsClear(uint b);
      void set_message(uint b,string &);
      void clear_all(Processor *c);
      void clear_all_set_by_user(Processor *c);

      void initialize_breakpoints(uint memory_size);
      int find_free();
};

#endif   //  __BREAKPOINTS_H__

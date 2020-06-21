/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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

#ifndef __GPSIM_TIME_H__
#define __GPSIM_TIME_H__

#include "breakpoints.h"

//---------------------------------------------------------
// Cycle Counter
//
// The cycle counter class is used to coordinate the timing 
// between the different peripherals within a processor and
// in some cases, the timing between several simulated processors
// and modules.
//
// The smallest quantum of simulated time is called a 'cycle'.
// The simuluation engine increments a 'Cycle Counter' at quantum
// simulation step. Simulation objects that wished to be notified
// at a specific instance in time can set a cycle counter break
// point that will get invoked whenever the cycle counter reaches
// that instance.

//------------------------------------------------------------
//
// Cycle counter breakpoint list
//
// This is a friend class to the Cycle Counter class. Its purpose
// is to maintain a doubly linked list of cycle counter break
// points.

class Cycle_Counter_breakpoint_list
{

public:
  // This is the value compared to the cycle counter.
  uint64_t break_value;

  // True when this break is active.
  bool bActive;

  // The breakpoint_number is a number uniquely identifying this
  // cycle counter break point. Note, this number is used only 
  // when the break point was assigned by a user

  uint breakpoint_number;

  // If non-null, the TriggerObject will point to an object that will get invoked
  // when the breakpoint is encountered.

  TriggerObject *f;

  // Doubly-linked list mechanics..
  // (these will be made private eventually)
  Cycle_Counter_breakpoint_list *next;
  Cycle_Counter_breakpoint_list *prev;

  Cycle_Counter_breakpoint_list *getNext();
  Cycle_Counter_breakpoint_list *getPrev();
  void clear();
  void invoke();
  Cycle_Counter_breakpoint_list();
};

class Cycle_Counter
{
    public:

      #define BREAK_ARRAY_SIZE  4
      #define BREAK_ARRAY_MASK  (BREAK_ARRAY_SIZE -1)

      static const uint64_t  END_OF_TIME=0xFFFFFFFFFFFFFFFFULL;// Largest cycle counter value

      bool reassigned;        // Set true when a break point is reassigned (or deleted)

      Cycle_Counter_breakpoint_list
      active,     // Head of the active breakpoint linked list
        inactive;   // Head of the inactive one.

      bool bSynchronous; // a flag that's true when the time per counter tick is constant

      Cycle_Counter();
      ~Cycle_Counter();
      void preset(uint64_t new_value);     // not used currently.

      /*
        increment - This inline member function is called once or
        twice for every simulated instruction. Its purpose is to
        increment the cycle counter using roll over arithmetic.
        If there's a breakpoint set on the new value of the cycle
        counter then the simulation is either stopped or a callback
        function is invoked. In either case, the break point is
        cleared.
      */
      void increment();

      /*
        advance the Cycle Counter by more than one instruction quantum.
        This is almost identical to the increment() function except that
        we allow the counter to be advanced by an arbitrary amount.
        They're separated only for efficiency reasons. This one runs slower.
      */
      inline void advance(uint64_t step)
      {
        while (step--)
        {
            if (value == break_on_this) breakpoint();
        }
          value++;
      }

      uint64_t get()  { return value; }// Return the current cycle counter value
      uint64_t get(double future_time_from_now);// Return the cycle counter for some time off in the future:

      bool set_break(uint64_t future_cycle, TriggerObject *f=0, uint abp = MAX_BREAKPOINTS);
      bool set_break_delta(uint64_t future_cycle, TriggerObject *f=0, uint abp = MAX_BREAKPOINTS);
      bool reassign_break(uint64_t old_cycle,uint64_t future_cycle, TriggerObject *f=0);
      void clear_current_break(TriggerObject *f=0);
      void dump_breakpoints();

      void clear_break(uint64_t at_cycle);
      void clear_break(TriggerObject *f);
      void set_instruction_cps(uint64_t cps);
      double instruction_cps() { return m_instruction_cps; }
      double seconds_per_cycle() { return m_seconds_per_cycle; }

    private:

      // The number of instruction cycles that correspond to one second
      double m_instruction_cps;
      double m_seconds_per_cycle;

      uint64_t value;          // Current value of the cycle counter.
      uint64_t break_on_this;  // If there's a pending cycle break point, then it'll be this

      /*
        breakpoint
        when the member function "increment()" encounters a break point,
        breakpoint() is called.
      */
      void breakpoint();
};

#if defined(IN_MODULE) && defined(_WIN32)
    // we are in a module: don't access cycles object directly!
    LIBGPSIM_EXPORT Cycle_Counter &get_cycles();
#else
    // we are in gpsim: use of get_cycles() is recommended,
    // even if cycles object can be accessed directly.
    extern Cycle_Counter cycles;
    inline Cycle_Counter &get_cycles() { return cycles; }
#endif

#endif

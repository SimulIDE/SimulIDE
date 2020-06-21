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


#if !defined(__TRIGGER_H__)
#define __TRIGGER_H__

typedef unsigned int uint;

#include <string>
using namespace std;

class TriggerObject;


//========================================================================
//
// Triggers
// (these comments are not completely implemented in code)
//
// gpsim divides a breakpoint into a TriggerAction and a TriggerObject.
// The TriggerObject is something that gets evaluated. If it evaluates
// to true then a TriggerAction is invoked.
// Most breakpoints are simple and don't need this complexity. For example,
// an execution breakpoint only needs to halt simulation whenever it's
// encountered. But gpsim defines the TriggerObject to be something like
// 'if address is executed' and the TriggerAction to be 'halt simulation'.
// However this design accomodates much more complicated situations. For
// example, the use may wish to break whenever register 42 is cleared during
// a time when interrupts are disabled. In this case, the trigger action
// is still a simple halt. However, the trigger object is more complicated:
//
// break w reg(42) (reg(42) == 0) && (STATUS & GIE == 0)
//
// In this case, the compound expression gets associated with write operations
// to register 42.

class TriggerAction
{
public:
  TriggerAction();
  virtual ~TriggerAction();
  virtual bool evaluate();
  virtual bool getTriggerState();
  virtual void action();
};

class SimpleTriggerAction : public TriggerAction
{
public:
  explicit SimpleTriggerAction(TriggerObject *_to);
  virtual void action();
protected:
  TriggerObject *to;

};

// TriggerObject - a base class for handling all of gpsim's breakpoints.
//
// The TriggerObject class is designed to be part of a multiple inheritance
// class heirarchy. Its main function is to provide an interface to the
// breakpoint functionality.
//
//

class TriggerObject
{
 public:

  uint bpn;

  // Enable the breakpoint and return true if successful
  virtual bool set_break() {return false;}

  // A unique number assigned when the break point is armed.
  int CallBackID;

  // When the breakpoint associated with this object is encountered,
  // then 'callback' is invoked.
  virtual void callback();

  // Invoked to display info about the breakpoint.
  virtual void callback_print();

  // clear_trigger is invoked when the breakpoint associated with
  // this object is cleared.
  virtual void clear_trigger();

  // Will search for a place to store this break point.
  virtual int find_free();

  // This object has no cpu associated with it. However, derived
  // types may and can choose to provide access to it through here:
  //virtual Processor *get_cpu() { return 0; }

  // Display the breakpoint - Probably should tie into a stream...
  virtual void print();

  // Clear the breakpoint
  virtual void clear();

  virtual char const * bpName() { return "Generic"; }

  virtual void set_action(TriggerAction *ta) { m_action = ta; }
  virtual TriggerAction *get_action() { return m_action;}
  virtual void invokeAction();

  // Messages can be associatated with triggers
  string &message() {return m_sMessage;}
  virtual void new_message(const char *);
  virtual void new_message(string &);

  TriggerObject();
  explicit TriggerObject(TriggerAction *);
  // Virtual destructor place holder
  virtual ~TriggerObject();

private:
  string m_sMessage;

  // When the TriggerObject becomes true, then the TriggerAction is
  // evaluated. E.g. If the trigger object is an execution breakpoint,
  // then whenever the PC == break address, the Breakpoint_Instruction
  // class (which is derived from this class) will invoke action->evaluate()
  // which will in turn halt the execution.

  TriggerAction *m_action;
};



#endif // !defined(__TRIGGER_H__)

/*
   Copyright (C) 2004 T. Scott Dattalo

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


#include "trigger.h"
#include "config.h"
#include "value.h"
#include "errors.h"
#include "breakpoints.h"

#include <iostream>
#include <iomanip>

using namespace std;

static TriggerAction DefaultTrigger;

//------------------------------------------------------------------------
// TriggerAction
//
TriggerAction::TriggerAction()
{
}

TriggerAction::~TriggerAction()
{
}

bool TriggerAction::evaluate()
{
  action();
  return true;
}

bool TriggerAction::getTriggerState()
{
  return false;
}

void TriggerAction::action()
{
  bp.halt();
}

//------------------------------------------------------------------------
// SimpleTriggerAction
//
// For most cases... A single trigger action coupled with a single trigger
// object
SimpleTriggerAction::SimpleTriggerAction(TriggerObject *_to)
  : TriggerAction(), to(_to)
{
}

void SimpleTriggerAction::action()
{
  TriggerAction::action();
}

//------------------------------------------------------------------------
TriggerObject::TriggerObject()
{
  set_action(&DefaultTrigger);
}

TriggerObject::TriggerObject(TriggerAction *ta)
{
  if(ta) set_action(ta);
  else   set_action(&DefaultTrigger);
}

TriggerObject::~TriggerObject()
{
  if( m_action != &DefaultTrigger ) delete m_action;
}

void TriggerObject::callback()
{
  cout << "generic callback\n";
}

void TriggerObject::callback_print()
{
  cout << " has callback, ID =  0x" << CallBackID << '\n';
}

void  TriggerObject::clear_trigger()
{
}

int TriggerObject::find_free()
{
  bpn = bp.find_free();

  if(bpn < MAX_BREAKPOINTS) {

    bp.break_status[bpn].type = Breakpoints::BREAK_CLEAR;
    bp.break_status[bpn].cpu  = 0; //get_cpu();
    bp.break_status[bpn].arg1 = 0;
    bp.break_status[bpn].arg2 = 0;
    bp.break_status[bpn].bpo  = this;
  }
  return bpn;
}

void TriggerObject::print()
{
}

void TriggerObject::clear()
{
  cout << "clear Generic breakpoint " << bpn << endl;
}

//------------------------------------------------------------------------
void TriggerObject::invokeAction()
{
  m_action->action();
}

//-------------------------------------------------------------------
void TriggerObject::new_message(const char *s)
{
  m_sMessage = string(s);
}

void TriggerObject::new_message(string &new_message)
{
  m_sMessage = new_message;
}

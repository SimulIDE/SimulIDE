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

#include <typeinfo>
#include <stdio.h>
#ifdef _WIN32
#include "unistd.h"
#else
#include <unistd.h>
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string>

#include "config.h"
#include "pic-processor.h"
#include "pic-registers.h"
#include "stimuli.h"

#include "p16x5x.h"
#include "p16f62x.h"
#include "p16f8x.h"
#include "p16f88x.h"
#include "p16x8x.h"
#include "p16f87x.h"
#include "p16x6x.h"
#include "p16x7x.h"
#include "p16f91x.h"
#include "p12x.h"
#include "p12f6xx.h"
#include "p1xf1xxx.h"
#ifdef P17C7XX  // code no longer works
#include "p17c75x.h"
#endif
#include "p18x.h"
#include "p18fk.h"
#include "clock_phase.h"


//================================================================================
//
// pic_processor
//
// This file contains all (most?) of the code that simulates those features
// common to all pic microcontrollers.
//
//

ProcessorConstructor pP10F200( P10F200::construct , "__10F200",   "pic10f200",    "p10f200",  "10f200");
ProcessorConstructor pP10F202(P10F202::construct , "__10F202",   "pic10f202",    "p10f202",  "10f202");
ProcessorConstructor pP10F204(P10F204::construct , "__10F204",   "pic10f204",    "p10f204",  "10f204");
ProcessorConstructor pP10F220(P10F220::construct , "__10F220",   "pic10f220",    "p10f220",  "10f220");
ProcessorConstructor pP10F222(P10F222::construct ,  "__10F222",   "pic10f222",    "p10f222",  "10f222");
ProcessorConstructor pP10F320(P10F320::construct , "__10F320",   "pic10f320",    "p10f320",  "10f320");
ProcessorConstructor pP10LF320(P10LF320::construct , "__10LF320",   "pic10lf320",    "p10lf320",  "10lf320");
ProcessorConstructor pP10F322(P10F322::construct , "__10F322",   "pic10f322",    "p10f322",  "10f322");
ProcessorConstructor pP10LF322(P10LF322::construct , "__10LF322",   "pic10lf322",    "p10lf322",  "10lf322");
ProcessorConstructor pP12C508(P12C508::construct , "__12C508",   "pic12c508",    "p12c508",  "12c508");
ProcessorConstructor pP12C509(P12C509::construct , "__12C509",   "pic12c509",    "p12c509",  "12c509");
ProcessorConstructor pP12CE518(P12CE518::construct , "__12ce518",  "pic12ce518",   "p12ce518", "12ce518");
ProcessorConstructor pP12CE519(P12CE519::construct , "__12ce519",  "pic12ce519",   "p12ce519", "12ce519");
ProcessorConstructor pP12F508(P12F508::construct , "__12F508",   "pic12f508",    "p12f508",  "12f508");
ProcessorConstructor pP12F509(P12F509::construct , "__12F509",   "pic12f509",    "p12f509",  "12f509");
ProcessorConstructor pP12F510(P12F510::construct , "__12F510",   "pic12f510",    "p12f510",  "12f510");
ProcessorConstructor pP12F629(P12F629::construct , "__12F629",   "pic12f629",    "p12f629",  "12f629");
ProcessorConstructor pP12F675(P12F675::construct , "__12F675",   "pic12f675",    "p12f675",  "12f675");
ProcessorConstructor pP12F683(P12F683::construct , "__12F683",   "pic12f683",    "p12f683",  "12f683");
ProcessorConstructor pP12F1822(P12F1822::construct , "__12F1822", "pic12f1822", "p12f1822", "12f1822");
ProcessorConstructor pP12LF1822(P12LF1822::construct , "__12LF1822", "pic12lf1822", "p12lf1822", "12lf1822");
ProcessorConstructor pP12F1840(P12F1840::construct , "__12F1840", "pic12f1840", "p12f1840", "12f1840");
ProcessorConstructor pP12LF1840(P12LF1840::construct , "__12LF1840", "pic12lf1840", "p12lf1840", "12lf1840");
ProcessorConstructor pP16C54(P16C54::construct , "__16C54",     "pic16c54",     "p16c54",   "16c54");
ProcessorConstructor pP16C55(P16C55::construct , "__16C55",     "pic16c55",     "p16c55",   "16c55");
ProcessorConstructor pP16C56(P16C56::construct , "__16C56",     "pic16c56",     "p16c56",   "16c56");
ProcessorConstructor pP16C61(P16C61::construct , "__16C61",     "pic16c61",     "p16c61",   "16c61");
ProcessorConstructor pP16C62(P16C62::construct , "__16C62",     "pic16c62",     "p16c62",   "16c62");
ProcessorConstructor pP16C62A(P16C62::construct , "__16C62A",   "pic16c62a",    "p16c62a",  "16c62a");
ProcessorConstructor pP16CR62(P16C62::construct , "__16CR62",   "pic16cr62",    "p16cr62",  "16cr62");
ProcessorConstructor pP16C63(P16C63::construct , "__16C63",     "pic16c63",     "p16c63",   "16c63");
ProcessorConstructor pP16C64(P16C64::construct , "__16C64",     "pic16c64",     "p16c64",   "16c64");
ProcessorConstructor pP16C65(P16C65::construct , "__16C65",     "pic16c65",     "p16c65",   "16c65");
ProcessorConstructor pP16C65A(P16C65::construct , "__16C65A",    "pic16c65a",    "p16c65a",  "16c65a");
ProcessorConstructor pP16C71(P16C71::construct , "__16C71",     "pic16c71",     "p16c71",   "16c71");
ProcessorConstructor pP16C712(P16C712::construct , "__16C712",   "pic16c712",    "p16c712",  "16c712");
ProcessorConstructor pP16C716(P16C716::construct , "__16C716",   "pic16c716",    "p16c716",  "16c716");
ProcessorConstructor pP16C72(P16C72::construct ,
                             "__16C72",     "pic16c72",     "p16c72",   "16c72");
ProcessorConstructor pP16C73(P16C73::construct ,
                             "__16C73",     "pic16c73",     "p16c73",   "16c73");
ProcessorConstructor pP16C74(P16C74::construct ,
                             "__16C74",     "pic16c74",     "p16c74",   "16c74");
ProcessorConstructor pP16C84(P16C84::construct ,
                             "__16C84",     "pic16c84",     "p16c84",   "16c84");
ProcessorConstructor pP16CR83(P16CR83::construct ,
                              "__16CR83",   "pic16cr83",    "p16cr83",  "16cr83");
ProcessorConstructor pP16CR84(P16CR84::construct ,
                              "__16CR84",   "pic16cr84",    "p16cr84",  "16cr84");
ProcessorConstructor pP16F505(P16F505::construct ,
                              "__16F505",   "pic16f505",    "p16f505",  "16f505");
ProcessorConstructor pP16F73(P16F73::construct ,
                             "__16F73",     "pic16f73",     "p16f73",   "16f73");
ProcessorConstructor pP16F74(P16F74::construct ,
                             "__16F74",     "pic16f74",     "p16f74",   "16f74");
ProcessorConstructor pP16F716(P16F716::construct ,
                              "__16F716",   "pic16f716",    "p16f716",  "16f716");
ProcessorConstructor pP16F83(P16F83::construct ,
                             "__16F83",     "pic16f83",     "p16f83",   "16f83");
ProcessorConstructor pP16F84(P16F84::construct ,
                             "__16F84",     "pic16f84",     "p16f84",   "16f84");
ProcessorConstructor pP16F87(P16F87::construct ,
                              "__16F87",    "pic16f87",     "p16f87",   "16f87");
ProcessorConstructor pP16F88(P16F88::construct ,
                              "__16F88",    "pic16f88",     "p16f88",   "16f88");
ProcessorConstructor pP16F882(P16F882::construct ,
                     "__16F882",    "pic16f882",     "p16f882",   "16f882");
ProcessorConstructor pP16F883(P16F883::construct ,
                     "__16F883",    "pic16f883",     "p16f883",   "16f883");
ProcessorConstructor pP16F884(P16F884::construct ,
                     "__16F884",    "pic16f884",     "p16f884",   "16f884");
ProcessorConstructor pP16F886(P16F886::construct ,
                     "__16F886",    "pic16f886",     "p16f886",   "16f886");
ProcessorConstructor pP16F887(P16F887::construct ,
                     "__16F887",    "pic16f887",     "p16f887",   "16f887");
ProcessorConstructor pP16F627(P16F627::construct ,
                       "__16F627",   "pic16f627",    "p16f627",  "16f627");
ProcessorConstructor pP16F627A(P16F627::construct ,
                       "__16F627A",  "pic16f627a",   "p16f627a", "16f627a");
ProcessorConstructor pP16F628(P16F628::construct ,
                       "__16F628",   "pic16f628",    "p16f628",  "16f628");
ProcessorConstructor pP16F628A(P16F628::construct ,
                       "__16F628A",  "pic16f628a",   "p16f628a", "16f628a");
ProcessorConstructor pP16F630(P16F630::construct ,
                       "__16F630",   "pic16f630",    "p16f630",  "16f630");
ProcessorConstructor pP16F631(P16F631::construct ,
                       "__16F631",   "pic16f631",    "p16f631",  "16f631");
ProcessorConstructor pP16F648(P16F648::construct ,
                       "__16F648",   "pic16f648",    "p16f648",  "16f648");
ProcessorConstructor pP16F648A(P16F648::construct ,
                       "__16F648A",  "pic16f648a",   "p16f648a", "16f648a");
ProcessorConstructor pP16F676(P16F676::construct ,
                       "__16F676",   "pic16f676",    "p16f676",  "16f676");
ProcessorConstructor pP16F677(P16F677::construct ,
                       "__16F677",   "pic16f677",    "p16f677",  "16f677");
ProcessorConstructor pP16F684(P16F684::construct ,
                       "__16F684",   "pic16f684",    "p16f684",  "16f684");
ProcessorConstructor pP16F685(P16F685::construct ,
                       "__16F685",   "pic16f685",    "p16f685",  "16f685");
ProcessorConstructor pP16F687(P16F687::construct ,
                       "__16F687",   "pic16f687",    "p16f687",  "16f687");
ProcessorConstructor pP16F689(P16F689::construct ,
                       "__16F689",   "pic16f689",    "p16f689",  "16f689");
ProcessorConstructor pP16F690(P16F690::construct ,
                       "__16F690",   "pic16f690",    "p16f690",  "16f690");
ProcessorConstructor pP16F818(P16F818::construct ,
                       "__16F818",   "pic16f818",    "p16f818",  "16f818");
ProcessorConstructor pP16F819(P16F819::construct ,
                       "__16F819",   "pic16f819",    "p16f819",  "16f819");
ProcessorConstructor pP16F871(P16F871::construct ,
                       "__16F871",   "pic16f871",    "p16f871",  "16f871");
ProcessorConstructor pP16F873(P16F873::construct ,
                       "__16F873",   "pic16f873",    "p16f873",  "16f873");
ProcessorConstructor pP16F874(P16F874::construct ,
                       "__16F874",   "pic16f874",    "p16f874",  "16f874");
ProcessorConstructor pP16F876(P16F876::construct ,
                       "__16F876",   "pic16f876",    "p16f876",  "16f876");
ProcessorConstructor pP16F877(P16F877::construct ,
                       "__16F877",   "pic16f877",    "p16f877",  "16f877");
ProcessorConstructor pP16F873A(P16F873A::construct ,
                       "__16F873a",  "pic16f873a",   "p16f873a", "16f873a");
ProcessorConstructor pP16F874A(P16F874A::construct ,
                       "__16F874a",  "pic16f874a",   "p16f874a", "16f874a");
ProcessorConstructor pP16F876A(P16F876A::construct ,
                       "__16F876a",  "pic16f876a",   "p16f876a", "16f876a");
ProcessorConstructor pP16F877A(P16F877A::construct ,
                       "__16F877a",  "pic16f877a",   "p16f877a", "16f877a");
ProcessorConstructor pP16F913(P16F913::construct ,
                       "__16F913",  "pic16f913",   "p16f913", "16f913");
ProcessorConstructor pP16F914(P16F914::construct ,
                       "__16F914",  "pic16f914",   "p16f914", "16f914");
ProcessorConstructor pP16F916(P16F916::construct ,
                       "__16F916",  "pic16f916",   "p16f916", "16f916");
ProcessorConstructor pP16F917(P16F917::construct ,
                       "__16F917",  "pic16f917",   "p16f917", "16f917");
ProcessorConstructor pP16F1503(P16F1503::construct ,
                       "__16F1503", "pic16f1503", "p16f1503", "16f1503");
ProcessorConstructor pP16LF1503(P16LF1503::construct ,
                       "__16LF1503", "pic16lf1503", "p16lf1503", "16lf1503");
ProcessorConstructor pP16F1788(P16F1788::construct ,
                       "__16F1788", "pic16f1788", "p16f1788", "16f1788");
ProcessorConstructor pP16LF1788(P16LF1788::construct ,
                       "__16LF1788", "pic16lf1788", "p16lf1788", "16lf1788");
ProcessorConstructor pP16F1823(P16F1823::construct ,
                       "__16F1823", "pic16f1823", "p16f1823", "16f1823");
ProcessorConstructor pP16LF1823(P16LF1823::construct ,
                       "__16LF1823", "pic16lf1823", "p16lf1823", "16lf1823");
ProcessorConstructor pP16F1825( P16F1825::construct ,
                       "__16F1825", "pic16f1825", "p16f1825", "16f1825");
ProcessorConstructor pP16LF1825( P16F1825::construct , "__16LF1825", "pic16lf1825", "p16lf1825", "16lf1825");
#ifdef P17C7XX  // code no longer works
ProcessorConstructor pP17C7xx(P17C7xx::construct ,
                              "__17C7xx", "pic17c7xx",  "p17c7xx", "17c7xx");
ProcessorConstructor pP17C75x(P17C75x::construct ,
                              "__17C75x", "pic17c75x",  "p17c75x", "17c75x");
ProcessorConstructor pP17C752(P17C752::construct ,
                              "__17C752", "pic17c752",  "p17c752", "17c752");
ProcessorConstructor pP17C756(P17C756::construct ,
                              "__17C756", "pic17c756",  "p17c756", "17c756");
ProcessorConstructor pP17C756A(P17C756A::construct ,
                               "__17C756A", "pic17c756a",   "p17c756a", "17c756a");
ProcessorConstructor pP17C762(P17C762::construct ,
                              "__17C762", "pic17c762",  "p17c762", "17c762");
ProcessorConstructor pP17C766(P17C766::construct ,
                              "__17C766", "pic17c766",  "p17c766", "17c766");
#endif // P17C7XX
ProcessorConstructor pP18C242(P18C242::construct ,
                              "__18C242",   "pic18c242",    "p18c242",  "18c242");
ProcessorConstructor pP18C252(P18C252::construct ,
                              "__18C252",   "pic18c252",    "p18c252",  "18c252");
ProcessorConstructor pP18C442(P18C442::construct ,
                              "__18C442",   "pic18c442",    "p18c442",  "18c442");
ProcessorConstructor pP18C452(P18C452::construct ,
                              "__18C452",   "pic18c452",    "p18c452",  "18c452");
ProcessorConstructor pP18F242(P18F242::construct ,
                              "__18F242",   "pic18f242",    "p18f242",  "18f242");
ProcessorConstructor pP18F248(P18F248::construct ,
                              "__18F248",   "pic18f248",    "p18f248",  "18f248");
ProcessorConstructor pP18F258(P18F258::construct ,
                              "__18F258",   "pic18f258",    "p18f258",  "18f258");
ProcessorConstructor pP18F252(P18F252::construct ,
                              "__18F252",   "pic18f252",    "p18f252",  "18f252");
ProcessorConstructor pP18F442(P18F442::construct ,
                              "__18F442",   "pic18f442",    "p18f442",  "18f442");
ProcessorConstructor pP18F448(P18F448::construct ,
                              "__18F448",   "pic18f448",    "p18f448",  "18f448");
ProcessorConstructor pP18F458(P18F458::construct ,
                              "__18F458",   "pic18f458",    "p18f458",  "18f458");
ProcessorConstructor pP18F452(P18F452::construct,
                              "__18F452",   "pic18f452",    "p18f452",  "18f452");
ProcessorConstructor pP18F1220(P18F1220::construct,
                              "__18F1220",  "pic18f1220",   "p18f1220", "18f1220");
ProcessorConstructor pP18F1320(P18F1320::construct,
                              "__18F1320",  "pic18f1320",   "p18f1320", "18f1320");
ProcessorConstructor pP18F14K22(P18F14K22::construct,
                              "__18F14K22", "pic18f14k22",  "p18f14k22", "18f14k22");
ProcessorConstructor pP18F2221(P18F2221::construct,
                              "__18F2221",  "pic18f2221",   "p18f2221", "18f2221");
ProcessorConstructor pP18F2321(P18F2321::construct,
                              "__18F2321",  "pic18f2321",   "p18f2321", "18f2321");
ProcessorConstructor pP18F2420(P18F2420::construct,
                              "__18F2420",  "pic18f2420",   "p18f2420", "18f2420");
ProcessorConstructor pP18F2455(P18F2455::construct,
                              "__18F2455",  "pic18f2455",   "p18f2455", "18f2455");
ProcessorConstructor pP18F2520(P18F2520::construct,
                              "__18F2520",  "pic18f2520",   "p18f2520", "18f2520");
ProcessorConstructor pP18F2525(P18F2525::construct,
                              "__18F2525",  "pic18f2525",   "p18f2525", "18f2525");
ProcessorConstructor pP18F2550(P18F2550::construct,
                              "__18F2550",  "pic18f2550",   "p18f2550", "18f2550");
ProcessorConstructor pP18F2620(P18F2620::construct,
                              "__18F2620",  "pic18f2620",   "p18f2620", "18f2620");
ProcessorConstructor pP18F26K22(P18F26K22::construct,
                              "__18F26K22", "pic18f26k22",  "p18f26k22", "18f26k22");
ProcessorConstructor pP18F4221(P18F4221::construct,
                              "__18F4221",  "pic18f4221",   "p18f4221", "18f4221");
ProcessorConstructor pP18F4321(P18F4321::construct,
                              "__18F4321",  "pic18f4321",   "p18f4321", "18f4321");
ProcessorConstructor pP18F4420(P18F4420::construct,
                              "__18F4420",  "pic18f4420",   "p18f4420", "18f4420");
ProcessorConstructor pP18F4520(P18F4520::construct,
                              "__18F4520",  "pic18f4520",   "p18f4520", "18f4520");
ProcessorConstructor pP18F4550(P18F4550::construct,
                              "__18F4550",  "pic18f4550",   "p18f4550", "18f4550");
ProcessorConstructor pP18F4455(P18F4455::construct,
                              "__18F4455",  "pic18f4455",   "p18f4455", "18f4455");
ProcessorConstructor pP18F4620(P18F4620::construct,
                              "__18F4620",  "pic18f4620",   "p18f4620", "18f4620");
ProcessorConstructor pP18F6520(P18F6520::construct,
                              "__18F6520",  "pic18f6520",   "p18f6520", "18f6520");


//========================================================================

void pic_processor::set_eeprom( EEPROM *e )
{
  eeprom = e;
  if (e) ema.set_Registers(e->rom, e->rom_size);
}

void pic_processor::BP_set_interrupt()
{
  mCaptureInterrupt->firstHalf();
}

//-------------------------------------------------------------------
//
// sleep - Begin sleeping and stay asleep until something causes a wake
//
void pic_processor::sleep ()
{
}
//-------------------------------------------------------------------
//
// enter_sleep - The processor is about to go to sleep, so update
//  the status register.

void pic_processor::enter_sleep()
{
  status->put_TO(1);
  status->put_PD(0);

  sleep_time = get_cycles ().get ();
  wdt.update();
  pc->increment();
  save_pNextPhase = mCurrentPhase->getNextPhase();
  save_CurrentPhase = mCurrentPhase;
  mCurrentPhase->setNextPhase(mIdle);
  mCurrentPhase = mIdle;
  mCurrentPhase->setNextPhase(mIdle);
  m_ActivityState = ePASleeping;
}

void pic_processor::exit_sleep()
{
  // If enter and exit sleep at same clock cycle, restore execute state
  if (get_cycles ().get () == sleep_time)
  {
      mCurrentPhase = save_CurrentPhase;
      mCurrentPhase->setNextPhase(save_pNextPhase);
  }
  else
  {
      mCurrentPhase = mExecute1Cycle;
      mCurrentPhase->setNextPhase(mExecute1Cycle);
  }
  
  m_ActivityState = ePAActive;
}

bool pic_processor::is_sleeping()
{
  return m_ActivityState == ePASleeping;
}

//-------------------------------------------------------------------
//
// pm_write - program memory write
//
void pic_processor::pm_write ()
{
  m_ActivityState = ePAPMWrite;

  do get_cycles().increment();     // burn cycles until we're through writing
  while(bp.have_pm_write());

  simulation_mode = eSM_RUNNING;
}

void pic_processor::save_state()
{
  Processor::save_state();
  if(eeprom) eeprom->save_state();
}

//-------------------------------------------------------------------
//
// step - Simulate one (or more) instructions. If a breakpoint is set
// at the current PC-> 'step' will go right through it. (That's supposed
// to be a feature.)
//
void pic_processor::step (uint steps, bool refresh)
{
  if(!steps) return;

  if(simulation_mode != eSM_STOPPED) return;

  simulation_mode = eSM_SINGLE_STEPPING;

  mCurrentPhase = mCurrentPhase ? mCurrentPhase : mExecute1Cycle;

  do mCurrentPhase = mCurrentPhase->advance();
  while(!bp.have_halt() && --steps>0);

  // complete the step if this is a multi-cycle instruction.
  if (mCurrentPhase == mExecute2ndHalf)
    while (mCurrentPhase != mExecute1Cycle)
      mCurrentPhase = mCurrentPhase->advance();

  bp.clear_halt();
  simulation_mode = eSM_STOPPED;
}

//-------------------------------------------------------------------
void pic_processor::step_cycle()
{
  mCurrentPhase = mCurrentPhase->advance();
}

//
//-------------------------------------------------------------------
//
// step_over - In most cases, step_over will simulate just one instruction.
// However, if the next instruction is a branching one (e.g. goto, call,
// return, etc.) then a break point will be set after it and gpsim will
// begin 'running'. This is useful for stepping over time-consuming calls.
//
void pic_processor::step_over( bool refresh )
{
}

//-------------------------------------------------------------------
//
// finish
//
// this method really only applies to processors with stacks.

void pic_processor::finish()
{
    if( !stack ) return;
}


//-------------------------------------------------------------------
//
// reset - reset the pic based on the desired reset type.
//
void pic_processor::reset( RESET_TYPE r )
{
    m_halted = true;
    rma.reset(r);
    m_halted = false;
    stack->reset(r);
    wdt.reset(r);
    pc->reset();

    bp.clear_global();

    switch (r) 
    {
      case POR_RESET:
        if( config_modes ) config_modes->print();
        mCurrentPhase = mCurrentPhase ? mCurrentPhase : mExecute1Cycle;
        m_ActivityState = ePAActive;
        break;

      case SOFT_RESET:
        cout << "Reset due to Software reset instruction\n";
        mCurrentPhase = mExecute1Cycle;
        mCurrentPhase->setNextPhase(mExecute1Cycle);
        m_ActivityState = ePAActive;
        break;

      case MCLR_RESET:
        cout << "MCLR reset\n";
        mCurrentPhase = mIdle;
        mCurrentPhase->setNextPhase(mIdle);
        m_ActivityState = ePAIdle;
        break;

      case IO_RESET:
        mCurrentPhase = mExecute1Cycle;
        mCurrentPhase->setNextPhase(mExecute1Cycle);
        m_ActivityState = ePAActive;
        break;

      case WDT_RESET:
        cout << "Reset on Watch Dog Timer expire\n";
        mCurrentPhase = mCurrentPhase ? mCurrentPhase : mExecute1Cycle;
        mCurrentPhase->setNextPhase(mExecute1Cycle);
        m_ActivityState = ePAActive;
        break;

      case EXIT_RESET:        // MCLR reset has cleared
        cout <<"MCLR low, resume execution\n";
        mCurrentPhase = mCurrentPhase ? mCurrentPhase : mExecute1Cycle;
        mCurrentPhase->setNextPhase(mExecute1Cycle);
        m_ActivityState = ePAActive;
        return;
        break;

      case STKOVF_RESET:
        cout << "Reset on Stack overflow\n";
         mCurrentPhase = mCurrentPhase ? mCurrentPhase : mIdle;
        mCurrentPhase->setNextPhase(mIdle);
        m_ActivityState = ePAActive;
      //  mCurrentPhase->setNextPhase(mExecute1Cycle);
      //  m_ActivityState = ePAActive;
        break;

      case STKUNF_RESET:
        cout << "Reset on Stack undeflow\n";
        mCurrentPhase = mCurrentPhase ? mCurrentPhase : mIdle;
        mCurrentPhase->setNextPhase(mIdle);
        m_ActivityState = ePAActive;
        break;

      default:
        printf("pic_processor::reset unknow reset type %d\n", r);
        m_ActivityState = ePAActive;
        break;
    }
}

//-------------------------------------------------------------------
//
// pic_processor -- constructor
//
pic_processor::pic_processor(const char *_name, const char *_desc)
  : Processor(_name,_desc),
    wdt(this, 18.0e-3),indf(0),fsr(0), stack(0), status(0),
    Wreg(0), pcl(0), pclath(0),m_PCHelper(0),
    tmr0(this,"tmr0","Timer 0"),
    m_configMemory(0),
    m_MCLR(0), m_MCLR_Save(0), m_MCLRMonitor(0),
    PPLx4(false), clksource(0), clkcontrol(0)
{
    mExecute1Cycle    = new phaseExecute1Cycle(this);
    mExecute2ndHalf   = new phaseExecute2ndHalf(this);
    mCaptureInterrupt = new phaseCaptureInterrupt(this);
    mIdle             = new phaseIdle(this);
    mCurrentPhase   = mExecute1Cycle;

    m_Capabilities = eSTACK | eWATCHDOGTIMER;

    eeprom = 0;
    config_modes = create_ConfigMode();

    pll_factor = 0;

    Integer::setDefaultBitmask(0xff);

    // Test code for logging to disk:
    for(int i = 0; i < 4; i++) osc_pin_Number[i] = 254;
}

pic_processor::~pic_processor()
{
  if (pma)
  {
    while(!rma.SpecialRegisters.empty())
      rma.SpecialRegisters.pop_back();
    while(!pma->SpecialRegisters.empty())
      pma->SpecialRegisters.pop_back();
  }
  delete_sfr_register(Wreg);
  delete_sfr_register(pcl);

  delete_sfr_register(pclath);
  delete_sfr_register(status);
  delete_sfr_register(indf);
  delete m_PCHelper;
  delete stack;

  delete mExecute1Cycle;
  delete mExecute2ndHalf;
  delete mCaptureInterrupt;
  delete mIdle;

  delete config_modes;
  delete m_configMemory;

  if (m_MCLR)        m_MCLR->setMonitor(0);
  if (m_MCLR_Save)   m_MCLR_Save->setMonitor(0);
  if (m_MCLRMonitor) delete m_MCLRMonitor;

  if (clksource) delete clksource;
  if (clkcontrol) delete clkcontrol;
}

//-------------------------------------------------------------------
//
//    create
//
//  The purpose of this member function is to 'create' a pic processor.
// Since this is a base class member function, only those things that
// are common to all pics are created.

void pic_processor::create ()
{
  init_program_memory (program_memory_size());
  init_register_memory (register_memory_size());

  // Now, initialize the core stuff:
  pc->set_cpu(this);

  Wreg = new WREG(this,"W","Working Register");

  pcl = new PCL(this,"pcl", "Program Counter Low");
  pclath = new PCLATH(this,"pclath", "Program Counter Latch High");
  status = new Status_register(this,"status", "Processor status");
  indf = new INDF(this,"indf","Indirect register");

  register_bank = &registers[0];  // Define the active register bank

  if(pma) 
  {
    m_PCHelper = new PCHelper(this,pma);
    rma.SpecialRegisters.push_back(m_PCHelper);
    rma.SpecialRegisters.push_back(status);
    rma.SpecialRegisters.push_back(Wreg);

    pma->SpecialRegisters.push_back(m_PCHelper);
    pma->SpecialRegisters.push_back(status);
    pma->SpecialRegisters.push_back(Wreg);
  }
  create_config_memory();
}

//-------------------------------------------------------------------
//
// add_sfr_register
//
// The purpose of this routine is to add one special function register
// to the file registers. If the sfr has a physical address (like the
// status or tmr0 registers) then a pointer to that register will be
// placed in the file register map.

// FIXME It doesn't make any sense to initialize the por_value here!
// FIXME The preferred way is to initialize all member data in their
// FIXME parent's constructor.

void pic_processor::add_sfr_register(Register *reg, uint addr,
                                     RegisterValue por_value,
                                     const char *new_name,
                                     bool warn_dup)
{
    reg->set_cpu(this);
    if(addr < register_memory_size())
    {
      if (registers[addr])
      {
        if (registers[addr]->isa() == Register::INVALID_REGISTER)
        {
            delete registers[addr];
            registers[addr] = reg;
        }
      }
      else registers[addr] = reg;

      reg->address = addr;
      reg->alias_mask = 0;

      if(new_name) reg->new_name(new_name);
    }
    reg->value       = por_value;
    reg->por_value   = por_value;  /// FIXME why are we doing this?
    reg->initialize();
}

// Use this function when register is initialized on WDT reset to
// same value as a POR.
void pic_processor::add_sfr_registerR(sfr_register *reg, uint addr,
                                     RegisterValue por_value,
                                     const char *new_name,
                                     bool warn_dup)
{
    add_sfr_register(reg, addr, por_value, new_name, warn_dup);
    reg->wdtr_value = por_value;
}
//-------------------------------------------------------------------
//
// delete_sfr_register
// This both deletes the register from the registers array,
// but also deletes the register class.
//
void pic_processor::delete_sfr_register(Register *pReg)
{
  if (pReg) 
  {
    uint a = pReg->getAddress();

    if (0) cout << __FUNCTION__ << " addr = 0x"<<hex<<a <<" reg " << pReg->name()<<endl;
    
    if (a<rma.get_size() && registers[a] == pReg)
      delete_file_registers(a,a);
    else delete pReg;
  }
}

//-------------------------------------------------------------------
//
// remove_sfr_register
// This is the inverse of add_sfr_register and does not delete the register.
//
void pic_processor::remove_sfr_register(Register *ppReg)
{
  if (ppReg) 
  {
    uint a = ppReg->getAddress();
    if (a == AN_INVALID_ADDRESS) return;
    if (registers[a] == ppReg)   delete_file_registers(a,a,true);
  }
}

//-------------------------------------------------------------------
//
// init_program_memory
//
// The purpose of this member function is to allocate memory for the
// pic's code space. The 'memory_size' parameter tells how much memory
// is to be allocated AND it should be an integer of the form of 2^n.
// If the memory size is not of the form of 2^n, then this routine will
// round up to the next integer that is of the form 2^n.
//   Once the memory has been allocated, this routine will initialize
// it with the 'bad_instruction'. The bad_instruction is an instantiation
// of the instruction class that chokes gpsim if it is executed. Note that
// each processor owns its own 'bad_instruction' object.

void pic_processor::init_program_memory (uint memory_size)
{
  // The memory_size_mask is used by the branching instructions
  pc->memory_size = memory_size;

  Processor::init_program_memory(memory_size);
}

bool pic_processor::set_config_word(uint address,uint cfg_word)
{
  int i = get_config_index(address);

  if( i >= 0)
  {
      m_configMemory->getConfigWord(i)->set((int)cfg_word);
      if (i == 0 && config_modes)
      {
           config_word = cfg_word;
           config_modes->config_mode = (config_modes->config_mode & ~7) | (cfg_word & 7);
      }
      return true;
  }
  return false;
}


uint pic_processor::get_config_word(uint address)
{
  int i= get_config_index(address);

  if( i >= 0 ) return m_configMemory->getConfigWord(i)->getVal();

  return 0xffffffff;
}

int pic_processor::get_config_index(uint address)
{
  if (m_configMemory)
  {
      for(int i = 0; i < m_configMemory->getnConfigWords(); i++)
      {
        if (m_configMemory->getConfigWord(i))
        {
          if (m_configMemory->getConfigWord(i)->ConfigWordAdd() == address)
          {
            return i;
          }
        }
     }
  }
  return -1;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//  ConfigMode
//
void ConfigMode::print()
{
    cout << "\nPic ConfigMode:\n" <<"    ";
    
  if(config_mode & CM_FOSC1x)                  // Internal Oscillator type processor
    switch(config_mode& (CM_FOSC0 | CM_FOSC1)) // Lower two bits are the clock type
    {
        case 0: cout << "LP"; break;
        case CM_FOSC0: cout << "XT"; break;
        case CM_FOSC1: cout << "Internal RC"; break;
        case (CM_FOSC0|CM_FOSC1): cout << "External RC"; break;
    }
  else 
    switch(config_mode& (CM_FOSC0 | CM_FOSC1)) // Lower two bits are the clock type
    {
        case 0: cout << "LP"; break;
        case CM_FOSC0: cout << "XT"; break;
        case CM_FOSC1: cout << "HS"; break;
        case (CM_FOSC0|CM_FOSC1): cout << "RC"; break;
    }
  cout << " oscillator\n";

  if(valid_bits & CM_WDTE)  cout <<"    "<<"WDT is "<<(get_wdt()?"enabled\n":"disabled\n");
  if(valid_bits & CM_MCLRE) cout <<"    "<<"MCLR is "<<(get_mclre()?"enabled\n":"disabled\n");
  if(valid_bits & CM_CP0) 
  {
    if(valid_bits & CM_CP1) 
    {
      cout <<"    " <<"CP0 is "<<(get_cp0()?"high\n" : "low\n");
      cout <<"    " <<"CP1 is "<<(get_cp1()?"high\n" : "low\n");
    } 
    else cout <<"    "<<"code protection is "<<(get_cp0()?"enabled\n":"disabled\n");
  }
  cout << "\n";
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void ProgramMemoryAccess::callback()
{
  if(_state)
    {
      _state = 0;
      //cout << __FUNCTION__ << " address= " << address << ", opcode= " << opcode << '\n';
      //cpu->program_memory[address]->opcode = opcode;
      put_opcode(_address,_opcode);
      // FIXME trace.opcode_write(_address,_opcode);
      bp.clear_pm_write();
    }
}

//--------------------------------------------------
WDT::WDT(pic_processor *p_cpu, double _timeout)
  : gpsimObject("WDT","Watch Dog Timer"),
    cpu(p_cpu), breakpoint(0),prescale(1), postscale(128), future_cycle(0),
    timeout(_timeout), wdte(false), cfgw_enable(false)
{
}

//--------------------------------------------------
void WDT::update()
{
  if( wdte ) 
  {  //cout << "WDT Enabled\n\n";
    // FIXME - the WDT should not be tied to the instruction counter...
    uint64_t delta_cycles;

    if( !use_t0_prescale ) postscale = 1;

    delta_cycles = (uint64_t)(postscale*prescale*timeout/get_cycles().seconds_per_cycle());

    uint64_t fc = get_cycles().get() + delta_cycles ;

    if(future_cycle) get_cycles().reassign_break( future_cycle, fc, this );
    else             get_cycles().set_break( fc, this );

    future_cycle = fc;
  }
}

//--------------------------------------------------
// WDT::put - shouldn't be called?
//
void WDT::put(uint new_value)
{
  cout << "WDT::put should not be called\n";
}

void WDT::set_timeout( double _timeout)
{
    timeout = _timeout;
    update();
}
//  TMR0 prescale is WDT postscale
void WDT::set_postscale(uint newPostscale)
{
    uint value = 1<< newPostscale;

    if (value != postscale) 
    {
    postscale = value;
    update();
    }
}

void WDT::swdten( bool enable )
{
    if (cfgw_enable) return;

    if( wdte != enable )
    {
        wdte = enable;
        warned = 0;
        
        if( wdte ) update();
        else
        {
            if (future_cycle) 
            {
                cout << "Disabling WDT\n";
                get_cycles().clear_break(this);
                future_cycle = 0;
            }
        }
    }
}

// For WDT period select 0-11
void WDT::set_prescale(uint newPrescale)
{
    uint value = 1<< (5 + newPrescale);
    if (value != prescale) 
    {
        prescale = value;
        update();
    }
}

void WDT::initialize( bool enable, bool _use_t0_prescale )
{
  wdte = enable;
  cfgw_enable = enable;
  use_t0_prescale = _use_t0_prescale;
  warned = 0;

  if( wdte ) update();
  else 
  {
    if (future_cycle) 
    {
      cout << "Disabling WDT\n";
      get_cycles().clear_break( this );
      future_cycle = 0;
    }
  }
}

void WDT::reset(RESET_TYPE r)
{
  switch (r) 
  {
      case POR_RESET:
      case EXIT_RESET:
        update();
        break;
      case MCLR_RESET:
        if (future_cycle)
          get_cycles().clear_break(this);
        future_cycle = 0;
        break;
      default:
        ;
  }
}

void WDT::set_breakpoint(uint bpn)
{
  breakpoint = bpn;
}

void WDT::callback()
{
  if( wdte ) 
  {
    if( breakpoint ) bp.halt();
    
    else if (cpu->is_sleeping() && cpu->exit_wdt_sleep())
    {
        cout << "WDT expired during sleep\n";
        update();
        cpu->exit_sleep();
        cpu->status->put_TO(0);
    }
    else      // The TO bit gets cleared when the WDT times out.
    {
        cout << "WDT expired reset\n";
        update();
        cpu->status->put_TO(0);
        cpu->reset(WDT_RESET);
    }
  }
}

void WDT::clear()
{
  if( wdte ) update();
  
  else if(!warned) 
  {
    warned = 1;
    cout << "The WDT is not enabled - clrwdt has no effect!\n";
  }
}

void WDT::callback_print()
{
  cout <<  name() << " has callback, ID = " << CallBackID << '\n';
}


//------------------------------------------------------------------------
// ConfigMemory - Base class
ConfigWord::ConfigWord(const char *_name, uint default_val, const char *desc,
                       pic_processor *pCpu, uint addr, bool EEw)
  : Integer(_name, default_val, desc), m_pCpu(pCpu), m_addr(addr),
    EEWritable(EEw)
{
  /*
  if (m_pCpu)
    m_pCpu->addSymbol(this);
  */
}
// this get controls the display format in the symbols window
void ConfigWord::get(char *buffer, int buf_size)
{
    if(buffer)
    {
        int64_t i;
        get(i);
    }
}
void ConfigWord::get(int64_t &i)
{
    Integer::get(i);
}

//------------------------------------------------------------------------
ConfigMemory::ConfigMemory(pic_processor *pCpu, uint nWords)
  : m_pCpu(pCpu), m_nConfigWords(nWords)
{
  if (nWords > 0 && nWords < 100) 
  {
    m_ConfigWords = new ConfigWord *[nWords];
    
    for (uint i = 0; i < nWords; i++)
      m_ConfigWords[i] = 0;
  }
}

ConfigMemory::~ConfigMemory()
{
  for (uint i = 0; i < m_nConfigWords; i++)
    if (m_ConfigWords[i])
//      m_pCpu->deleteSymbol(m_ConfigWords[i]);

  delete [] m_ConfigWords;
}

int ConfigMemory::addConfigWord(uint addr, ConfigWord *pConfigWord)
{
  if (addr < m_nConfigWords) 
  {
    m_ConfigWords[addr] = pConfigWord;
    return 1;
  }
  delete pConfigWord;
  return 0;
}

ConfigWord *ConfigMemory::getConfigWord(uint addr)
{
  return addr < m_nConfigWords ? m_ConfigWords[addr] : 0;
}
//-------------------------------------------------------------------
class MCLRPinMonitor : public PinMonitor
{
    public:
      MCLRPinMonitor(pic_processor *pCpu);
      ~MCLRPinMonitor() {}

      virtual void setDrivenState(char);
      virtual void setDrivingState(char) {}
      virtual void set_nodeVoltage(double) {}
      virtual void putState(char) {}
      virtual void setDirection() {}
      
    private:
      pic_processor *m_pCpu;
      char m_cLastResetState;
};

MCLRPinMonitor::MCLRPinMonitor(pic_processor *pCpu)
  : m_pCpu(pCpu),
    m_cLastResetState('I')  // I is not a valid state. It's used here for 'I'nitialization
{
}

void MCLRPinMonitor::setDrivenState(char newState)
{
  if (newState =='0' || newState =='w') 
  {
    m_cLastResetState = '0';
    m_pCpu->reset(MCLR_RESET);
  }

  if (newState =='1' || newState =='W') 
  {
    if (m_cLastResetState == '0') m_pCpu->reset(EXIT_RESET);

    m_cLastResetState = '1';
  }
}

//-------------------------------------------------------------------
void pic_processor::createMCLRPin(int pkgPinNumber)
{
  if (m_MCLR) cout << "BUG?: assigning multiple MCLR pins: " << __FILE__ << dec << " " << __LINE__ << endl;

  if(package) 
  {
    m_MCLR = new IO_open_collector("MCLR");
    package->assign_pin(pkgPinNumber,m_MCLR);

    m_MCLRMonitor = new MCLRPinMonitor(this);
    m_MCLR->setMonitor(m_MCLRMonitor);
  }
}

//-------------------------------------------------------------------
// This function is called instead of createMCLRPin where the pin
// is already defined, but the configuration word has set the function
// to MCLR

void pic_processor::assignMCLRPin(int pkgPinNumber)
{
  if (package)
  {
    if (m_MCLR == NULL)
    {
        m_MCLR_pin = pkgPinNumber;
        m_MCLR = new IO_open_collector("MCLR");

        m_MCLR_Save = package->get_pin(pkgPinNumber);
        package->assign_pin(pkgPinNumber,m_MCLR, false);

        m_MCLRMonitor = new MCLRPinMonitor(this);
        m_MCLR->setMonitor(m_MCLRMonitor);
    }
    else if (m_MCLR != package->get_pin(pkgPinNumber))
    {
        cout << "BUG?: assigning multiple MCLR pins: "
        << dec << pkgPinNumber << " " << __FILE__ <<  " "
        << __LINE__ << endl;
    }
  }
}
//-------------------------------------------------------------------
// This function sets the pin currently set as MCLR back to its original function
void pic_processor::unassignMCLRPin()
{
    if (package && m_MCLR_Save)
    {
        package->assign_pin(m_MCLR_pin,m_MCLR_Save, false);

        if (m_MCLR)
        {
            m_MCLR->setMonitor(0);
            m_MCLR = NULL;
            
            if (m_MCLRMonitor)
            {
                delete m_MCLRMonitor;
                m_MCLRMonitor = NULL;
            }
        }
    }
}
//--------------------------------------------------
//
class IO_SignalControl : public SignalControl
{
    public:
      IO_SignalControl(char _dir){ direction = _dir; }
      ~IO_SignalControl(){}
      virtual char getState() { return direction; }
      virtual void release() {}
      void setState(char _dir) { direction = _dir; }
      
    private:
      char direction;
};

// This function sets a label on a pin and if PinMod is defined
// removes its control from it's port register
//
void pic_processor::set_clk_pin(uint pkg_Pin_Number,
                PinModule *PinMod,
                const char * name,
                bool in,
                PicPortRegister *m_port,
                PicTrisRegister *m_tris,
                PicLatchRegister *m_lat)
{
    if (PinMod)
    {
        if (m_port)
        {
            uint mask = m_port->getEnableMask();
            mask &= ~(1<< PinMod->getPinNumber());
            m_port->setEnableMask(mask);
            
            if (m_tris) m_tris->setEnableMask(mask);
            if (m_lat)  m_lat->setEnableMask(mask);
        }
        if (!clksource)
        {
            clksource = new  PeripheralSignalSource(PinMod);
            clkcontrol = new IO_SignalControl(in ? '1' : '0');
        }
        PinMod->setSource(clksource);
        PinMod->setControl(clkcontrol);
        PinMod->updatePinModule();
    }
}

// This function reverses the effects of the previous function
void pic_processor::clr_clk_pin(uint pkg_Pin_Number,
                PinModule *PinMod,
                PicPortRegister *m_port,
                PicTrisRegister *m_tris,
                PicLatchRegister *m_lat)
{
    if (PinMod)
    {
        if (m_port)
        {
            uint mask = m_port->getEnableMask();
            mask |= (1<< PinMod->getPinNumber());
            m_port->setEnableMask(mask);
            
            if (m_tris) m_tris->setEnableMask(mask);
            if (m_lat)  m_lat->setEnableMask(mask);
        }
        PinMod->setSource(0);
        PinMod->setControl(0);
        PinMod->updatePinModule();
    }
}

void pic_processor::osc_mode(uint value)
{
    IOPIN *m_pin;
    uint pin_Number =  get_osc_pin_Number(0);

    if (pin_Number < 253)  m_pin = package->get_pin(pin_Number);

    if ( (pin_Number =  get_osc_pin_Number(1)) < 253 &&
        (m_pin = package->get_pin(pin_Number)))
    {
        pll_factor = 0;
        if (value < 5)
        {
            set_clk_pin(pin_Number, m_osc_Monitor[1], "OSC2", true);
        }
        else if(value == 6 )
        {
            pll_factor = 2;
            set_clk_pin(pin_Number, m_osc_Monitor[1], "CLKO", false);
        }
        else clr_clk_pin(pin_Number, m_osc_Monitor[1]);
    }
}

void pic_processor::Wput(uint value)
{
    Wreg->put(value);
}

uint pic_processor::Wget()
{
    return Wreg->get();
}

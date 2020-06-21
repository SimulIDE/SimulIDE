/*
   Copyright (C) 2006 T. Scott Dattalo

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


#include "clock_phase.h"

#include "processor.h"
#include "gpsim_time.h"

//========================================================================
ClockPhase::ClockPhase()
  : m_pNextPhase(this)
{
}

ClockPhase::~ClockPhase()
{
}
//========================================================================
ProcessorPhase::ProcessorPhase(Processor *pcpu)
  : ClockPhase(), 
    m_pcpu(pcpu)
{
}
ProcessorPhase::~ProcessorPhase()
{
}
//========================================================================
phaseExecute1Cycle::phaseExecute1Cycle(Processor *pcpu)
  : ProcessorPhase(pcpu)
{
}
phaseExecute1Cycle::~phaseExecute1Cycle()
{
}
/*
  phaseExecute1Cycle::advance() - advances a processor's time one clock cycle.
*/
ClockPhase *phaseExecute1Cycle::advance()
{
  setNextPhase( this );
  m_pcpu->step_one(false);
  
  if (bp.global_break & GLOBAL_LOG) bp.global_break &= ~GLOBAL_LOG;

  if (!bp.global_break) get_cycles().increment();
  
  return m_pNextPhase;
}

//========================================================================

phaseIdle::phaseIdle(Processor *pcpu)
  : ProcessorPhase(pcpu)
{
}
phaseIdle::~phaseIdle()
{
}

/*
  phaseIdle::advance() - advances a processor's time one clock cycle,
  but does not execute code.
 */

ClockPhase *phaseIdle::advance()
{
  setNextPhase(this);
  get_cycles().increment();
  return m_pNextPhase;
}
#if 0
const char* phaseDesc(ClockPhase *pPhase)
{
  if (pPhase == mIdle)
    return ("mIdle");
  if (pPhase == mExecute1Cycle)
    return ("mExecute1Cycle");
  if (pPhase == mExecute2ndHalf)
    return ("mExecute2ndHalf");
  if (pPhase == mCaptureInterrupt)
    return ("mCaptureInterrupt");
  return "unknown phase";
}
#endif


phaseCaptureInterrupt::phaseCaptureInterrupt(Processor *pcpu)
  :  ProcessorPhase(pcpu), m_pCurrentPhase(0),m_pNextNextPhase(0)
{
}
phaseCaptureInterrupt::~phaseCaptureInterrupt()
{}
#define Rprintf(arg) {printf("0x%06X %s() ",cycles.get(),__FUNCTION__); printf arg; }
ClockPhase *phaseCaptureInterrupt::advance()
{
  //Rprintf (("phaseCaptureInterrupt\n"));
  if (m_pNextPhase == m_pcpu->mExecute2ndHalf)
    m_pNextPhase->advance();

  if (m_pCurrentPhase == m_pcpu->mIdle) { // Interrupted sleep

    // complete sleeping phase
    m_pNextPhase = m_pNextNextPhase->advance();
    
    if (m_pNextPhase == m_pcpu->mIdle)
    {
        m_pNextPhase = m_pcpu->mExecute1Cycle;
	do 
	{
	    m_pNextPhase = m_pcpu->mExecute1Cycle->advance();
	}while (m_pNextPhase != m_pcpu->mExecute1Cycle);
    }
    m_pcpu->mCurrentPhase = this;

    if (bp.global_break) m_pNextNextPhase = m_pNextPhase;
    else                 m_pCurrentPhase = NULL;
    
    m_pcpu->exit_sleep();
    return this;
  }
  m_pcpu->interrupt();

  return m_pNextPhase;
}

void phaseCaptureInterrupt::firstHalf()
{
  m_pCurrentPhase = m_pcpu->mCurrentPhase;

  m_pNextPhase = this;
  m_pNextNextPhase = m_pcpu->mCurrentPhase->getNextPhase();
  m_pcpu->mCurrentPhase->setNextPhase(this);
  m_pcpu->mCurrentPhase = this;
}




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

#if !defined(__CLOCK_PHASE_H__)
#define __CLOCK_PHASE_H__



/*
  Clock Phase

  The Clock Phase base class takes an external clock source as its
  input and uses this to control a module's simulation state. For
  example, the clock input on a microcontroller drives all of the
  digital state machines. On every edge of the clock, there is digital
  logic that can potentially change states. The Clock Phase base class
  can be thought of the "logic" that responds to the clock input and
  redirects control to the state machines inside of a processor.
*/

class Processor;
//RRRclass pic_processor;
class ClockPhase
{
public:
  ClockPhase();
  virtual ~ClockPhase();
  virtual ClockPhase *advance()=0;
  void setNextPhase(ClockPhase *pNextPhase) { m_pNextPhase = pNextPhase; }
  ClockPhase *getNextPhase() { return m_pNextPhase; }
protected:
  ClockPhase *m_pNextPhase;
};


/*
  The Processor Phase base class is a Clock Phase class that contains a
  pointer to a Processor object. It's the base class from which all of
  the processor's various Phase objects are derived.
*/
class ProcessorPhase : public ClockPhase
{
public:
  explicit ProcessorPhase(Processor *pcpu);
  virtual ~ProcessorPhase();
protected:
  Processor *m_pcpu;
};

/*
  The Execute 1 Cycle class is a Processor Phase class designed to
  execute a single instruction.

*/
class phaseExecute1Cycle : public ProcessorPhase
{
public:
  explicit phaseExecute1Cycle(Processor *pcpu);
  virtual ~phaseExecute1Cycle();
  virtual ClockPhase *advance();
};

class phaseExecute2ndHalf : public ProcessorPhase
{
public:
  explicit phaseExecute2ndHalf(Processor *pcpu);
  virtual ~phaseExecute2ndHalf();
  virtual ClockPhase *advance();
  ClockPhase *firstHalf(unsigned int uiPC);
protected:
  unsigned int m_uiPC;
};

class phaseCaptureInterrupt : public ProcessorPhase
{
public:
  explicit phaseCaptureInterrupt(Processor *pcpu);
  ~phaseCaptureInterrupt();
  virtual ClockPhase *advance();
  void firstHalf();
protected:
  ClockPhase *m_pCurrentPhase;
  ClockPhase *m_pNextNextPhase;
};

// phaseIdle - when a processor is idle, the current
// clock source can be handled by this class.

class phaseIdle : public ProcessorPhase
{
public:
  explicit phaseIdle(Processor *pcpu);
  virtual ~phaseIdle();
  virtual ClockPhase *advance();
protected:
};


#endif  //if !defined(__CLOCK_PHASE_H__)

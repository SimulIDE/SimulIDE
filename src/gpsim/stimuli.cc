/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2006,2015 Roy R Rankin

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

#include <QDebug>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>
#include <sstream>
#include <math.h>

#include "config.h"
#include "pic-processor.h"
#include "stimuli.h"
#include "errors.h"

//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

static char num_nodes = 'a';

/*
 * stimulus.cc
 *
 * This file contains some rudimentary infrastructure to support simulating
 * the environment outside of the pic. Simple net lists interconnecting pic
 * I/O pins and various signal generators may be created. 
 *
 * Details:
 * There are two basic concepts behind the stimulus code: nodes and stimuli.
 * The nodes are like wires and the stimuli are like sources and loads. The
 * nodes define the interconnectivity between the stimuli. In most cases there
 * will be only two stimuli connected by one node. For example, you may wish
 * to simulate the effects of a clock input connected to porta.0 . In this case,
 * the stimuli would be the external clock and the pic I/O pin.
 */

Stimulus_Node::Stimulus_Node(const char *n)
  : TriggerObject(0)
{
    warned  = 0;
    voltage = 0;
    Cth = 0.0;
    Zth = 0.0;
    current_time_constant = 0.0;
    delta_voltage = 0.0;
    minThreshold = 0.1; // volts
    cap_start_cycle = 0;
    future_cycle = 0;
    initial_voltage = 0.0;
    DCVoltage = 0.0;
    bSettling = false;  
    stimuli = 0;
    nStimuli = 0;
    settlingTimeStep = 0;

    if(n) gpsimObject::new_name(n);
    else 
    {
        char name_str[100];
        snprintf(name_str,sizeof(name_str),"node%d",num_nodes);
        num_nodes++;    // %%% FIX ME %%%
        gpsimObject::new_name(name_str);
    }
}

Stimulus_Node::~Stimulus_Node()
{
    stimulus *sptr;

    sptr = stimuli;
    while(sptr) 
    {
        sptr->detach(this);
        sptr = sptr->next;
    }

}

Stimulus_Node * Stimulus_Node::construct(const char * psName)
{
    return new Stimulus_Node(psName);
}

void Stimulus_Node::new_name(const char *cPname, bool bClearableSymbol)
{
    cout << " Warning ignoring stimulus node name change from " 
         << name() << " to " << cPname <<endl;
}

void Stimulus_Node::new_name(string &rName, bool bClearableSymbol)
{
    new_name(rName.c_str(), bClearableSymbol);
}

double Stimulus_Node::get_nodeVoltage()
{
    if (future_cycle > cap_start_cycle) // RC calculation in progress, get current value
        callback();
        
    return(voltage);
}

string Stimulus_Node::toString()
{
    string out = name() + " : " + showType();

    for(stimulus *pt = stimuli; pt; pt = pt->next)
    {
        out += "\n\n " + pt->name() + pt->toString();
    }
    return out;
}

//
// Add the stimulus 's' to the stimulus list for this node
//
void Stimulus_Node::attach_stimulus(stimulus *s)
{
    if (!s) return;

    stimulus *sptr;
    warned = 0;

    if(stimuli)
    {
        sptr = stimuli;
        bool searching=1;
        int nTotalStimuliConnected = 1;

        while( searching )
        {
            if(s == sptr) return;    // The stimulus is already attached to this node.

            nTotalStimuliConnected++;
            if(sptr->next == 0)
            {
                sptr->next = s;
                searching=0;
            }
            sptr = sptr->next;
        }
        nStimuli = nTotalStimuliConnected;
    }
    else
    {
        stimuli = s;     // This is the first stimulus attached to this node.
        nStimuli = 1;
    }
    // If we reach this point, then it means that the stimulus that we're
    // trying to attach has just been placed at the end of the the stimulus
    // list for this node. So we need to 0 terminate the singly-linked list.

    s->next = 0;

    // Now tell the stimulus to attach itself to the node too (If it hasn't already.)
    s->attach(this);
}

//
// Search for the stimulus 's' in the stimulus list for this node.
// If it is found, then remove it from the list.
//
void Stimulus_Node::detach_stimulus( stimulus* s )
{
  if(!s) return;        // You can't remove a non-existant stimulus
  
  stimulus *sptr;
    
  if(stimuli) 
  {
    if(s == stimuli) 
    {
      // This was the first stimulus in the list.

      stimuli = s->next;
      s->detach(this);
      nStimuli--;
    } 
    else 
    {
      sptr = stimuli;

      do 
      {
        if(s == sptr->next) 
        {
          sptr->next = s->next;
          s->detach(this);
          nStimuli--;
          //gi.node_configuration_changed(this);
          return;
        }
        sptr = sptr->next;
      } while(sptr);
    } 
  }
}

//------------------------------------------------------------------------
//
// Stimulus_Node::update(uint64_t current_time)
//
// update() is called whenever a stimulus attached to this node changes states.
//  
void Stimulus_Node::update( uint64_t current_time )
{
    update(); // So far, 'update' only applies to the current time. 
}

//------------------------------------------------------------------------
// refresh() - compute the Thevenin voltage and Thevenin impedance
//
void Stimulus_Node::refresh()
{
  if(stimuli) 
  {
    stimulus *sptr = stimuli;

    initial_voltage = get_nodeVoltage();

    switch (nStimuli) 
    {
    case 0:
      // hmm, strange nStimuli is 0, but the stimuli pointer is non null.
      break;

    case 1:
      // Only one stimulus is attached.
      DCVoltage = sptr->get_Vth();   // RP - was just voltage
      Zth =  sptr->get_Zth();
      break;

    case 2:
      // 2 stimuli are attached to the node. This is the typical case
      // and we'll optimize for it.
      {
      stimulus *sptr2 = sptr ? sptr->next : 0;
      if(!sptr2) break;     // error, nStimuli is two, but there aren't two stimuli
      
      double V1,Z1,C1;
      double V2,Z2,C2;
      sptr->getThevenin(V1,Z1,C1);
      sptr2->getThevenin(V2,Z2,C2);
      DCVoltage = (V1*Z2  + V2*Z1) / (Z1+Z2);
      Zth = Z1*Z2/(Z1+Z2);
      Cth = C1+C2;
      }
      break;

    default:
      {
      /*
        There are 3 or more stimuli connected to this node. Recall
        that these are all in parallel. The Thevenin voltage and 
        impedance for this is:

        Thevenin impedance:
        Zt = 1 / sum(1/Zi)

        Thevenin voltage:

        Vt = sum( Vi / ( ((Zi - Zt)/Zt) + 1) )
        = sum( Vi * Zt /Zi)
        = Zt * sum(Vi/Zi)
      */

      double conductance=0.0;        // Thevenin conductance.
      Cth=0;
      DCVoltage=0.0; 

      //cout << "multi-node summing:\n";
      while(sptr) {

      double V1,Z1,C1;
      sptr->getThevenin(V1,Z1,C1);
        /*
        cout << " N: " <<sptr->name() 
        << " V=" << V1
        << " Z=" << Z1
        << " C=" << C1 << endl;
        */

        double Cs = 1 / Z1;
        DCVoltage += V1 * Cs;
        conductance += Cs;
        Cth += C1;
        sptr = sptr->next;
      }
      Zth = 1.0/conductance;
      DCVoltage *= Zth;
      }
    }
    current_time_constant = Cth * Zth;
    Dprintf(("%s DCVoltage %.3f voltage %.3f Cth=%.2e Zth=%2e time_constant %fsec or %" PRINTF_GINT64_MODIFIER "d cycles now=%" PRINTF_GINT64_MODIFIER "d \n",name().c_str(), DCVoltage, voltage, Cth, Zth, current_time_constant, (uint64_t)(current_time_constant*get_cycles().instruction_cps()), get_cycles().get()));
    
    if (((uint64_t)(current_time_constant*get_cycles().instruction_cps()) < 5) ||
      (fabs(DCVoltage - voltage) < minThreshold))
    {
      if (future_cycle)        // callback is active
      {
        get_cycles().clear_break(this);
      }
      voltage = DCVoltage;
      future_cycle = 0;
    } 
    else 
    {
        settlingTimeStep = calc_settlingTimeStep();
        voltage = initial_voltage;
        
        // If future_cycle is not 0 we are in the middle of an RC
        // calculation, but an input condition has changed.
        
      if (future_cycle && (get_cycles().get() > cap_start_cycle)) callback();
      else
      {
        if (future_cycle) get_cycles().clear_break(this);
        cap_start_cycle = get_cycles().get(); 
        future_cycle = cap_start_cycle + settlingTimeStep;
        get_cycles().set_break(future_cycle,this);
      }
    }
  }
}

uint64_t Stimulus_Node::calc_settlingTimeStep()
{
    /* Select a time interval where the voltage does not change more
       than about 0.125 volts in each step(unless timestep < 1).
       First we calculate dt_dv = CR/V with dt in cpu cycles to
       determine settling time step
    */
    uint64_t TimeStep;
    double dv = fabs(DCVoltage - voltage);
    
    // avoid divide by zero
    if (dv < 0.000001) dv = 0.000001;
    
    double dt_dv = get_cycles().instruction_cps()*current_time_constant/dv;
    TimeStep = (uint64_t) (0.125 * dt_dv);
    TimeStep = (TimeStep) ? TimeStep : 1;

    Dprintf(("%s dt_dv = %.2f TimeStep 0x%" PRINTF_GINT64_MODIFIER "x now 0x%" PRINTF_GINT64_MODIFIER "x\n", __FUNCTION__, dt_dv, TimeStep, get_cycles().get()));

    return(TimeStep);
}

//------------------------------------------------------------------------
// updateStimuli
// drive all the stimuli connected to this node.

void Stimulus_Node::updateStimuli()
{
    stimulus *sptr = stimuli;

    while(sptr) 
    {
        sptr->set_nodeVoltage(voltage);
        sptr = sptr->next;
    }
}

void Stimulus_Node::update()
{
    if(stimuli) 
    {
        refresh();
        updateStimuli();
    }
}

void Stimulus_Node::set_nodeVoltage(double v)
{
    voltage = v;
    updateStimuli();
}

void Stimulus_Node::callback()
{
    initial_voltage = voltage;
    double Time_Step;
    double expz;
    //
    // increase time step as capacitor charges more slowly as final
    // voltage is approached.
    //
                                                                        
    //
    // The following is an exact calculation, assuming no circuit
    // changes,  regardless of time step.
    //
    Time_Step = (get_cycles().get() - cap_start_cycle)/
    (get_cycles().instruction_cps()*current_time_constant);
    expz = exp(-Time_Step);
    voltage = DCVoltage - (DCVoltage - voltage)*expz;

    if (fabs(DCVoltage - voltage) < minThreshold)
    {
        voltage = DCVoltage;
        if (future_cycle) get_cycles().clear_break(this);
        future_cycle = 0;

        Dprintf(("%s DC Voltage %.2f reached at 0x%" PRINTF_GINT64_MODIFIER "x cycles\n", name().c_str(), DCVoltage, get_cycles().get()));
    } 
    else if(get_cycles().get() >= future_cycle) // got here via break
    {
        settlingTimeStep = calc_settlingTimeStep();
        cap_start_cycle = get_cycles().get();
        get_cycles().clear_break(this);
        future_cycle = cap_start_cycle + settlingTimeStep;
        get_cycles().set_break(future_cycle, this);

    }
    else        // updating value before break don't increase step size
    {
        cap_start_cycle = get_cycles().get();
        get_cycles().reassign_break(future_cycle, 
        cap_start_cycle + settlingTimeStep, this);
        future_cycle = get_cycles().get() + settlingTimeStep;
    }
    updateStimuli();
}

void Stimulus_Node::callback_print()
{
    cout << "Node: " << name() ;
    TriggerObject::callback_print();
}

//------------------------------------------------------------------------

stimulus::stimulus( const char *cPname, double _Vth, double _Zth)
  : Value(cPname, "", 0),snode(0), next(0),
    bDrivingState(false), bDriving(false),
    Vth(_Vth), Zth(_Zth), 
    Cth(0.0), // Farads
    nodeVoltage(0.0) // volts
{
}

void stimulus::new_name(const char *cPname, bool bClearableSymbol)
{
    gpsimObject::new_name( cPname );
}

void stimulus::new_name(string &rName, bool bClearableSymbol)
{
    new_name(rName.c_str(),bClearableSymbol);
}

stimulus::~stimulus(void)
{
    if(snode) snode->detach_stimulus(this);
}

void stimulus::show()
{
}

string stimulus::toString() 
{
    ostringstream s;

    s << " stimulus ";
    if(snode) s << " attached to " << snode->name();
        s << endl
        << " Vth=" << get_Vth() << "V"
        << " Zth=" << get_Zth() << " ohms"
        << " Cth=" << get_Cth() << "F"
        << " nodeVoltage= " << get_nodeVoltage() << "V"
        << endl 
        << " Driving=" << getDriving()
        << " drivingState=" << getDrivingState()
        << " drivenState=" << getDrivenState()
        << " bitState=" << getBitChar();

    return s.str();
}
void stimulus::attach(Stimulus_Node *s)
{
    detach(snode);
    snode = s;
}
void stimulus::detach( Stimulus_Node *s )
{
    if( snode == s ) snode = 0; 
}

void stimulus::getThevenin(double &v, double &z, double &c)
{
    v = get_Vth();
    z = get_Zth();
    c = get_Cth();
}

//========================================================================
// 
PinMonitor::PinMonitor()
{
}

PinMonitor::~PinMonitor()
{
    // Release all of the sinks:
    list <SignalSink *> :: iterator ssi = sinks.begin();
    while (ssi != sinks.end()) 
    {
        Dprintf(("release sink %p\n", *ssi));
        fflush(stdout);
        (*ssi)->release();
        ++ssi;
    }

    list <AnalogSink *> :: iterator asi = analogSinks.begin();
    while (asi != analogSinks.end()) 
    {
        (*asi)->release();
        ++asi;
    }
}

void PinMonitor::addSink(SignalSink *new_sink)
{
    if(new_sink) sinks.push_back(new_sink);
}

void PinMonitor::removeSink(SignalSink *pSink)
{
    if(pSink) sinks.remove(pSink);
}

void PinMonitor::addSink(AnalogSink *new_sink)
{
    if(new_sink) analogSinks.push_back(new_sink);
}

void PinMonitor::removeSink(AnalogSink *pSink)
{
    if(pSink) analogSinks.remove(pSink);
}

//========================================================================
//
IOPIN::IOPIN(const char *_name,
             double _Vth, 
             double _Zth,
             double _ZthWeak,
             double _ZthFloating
             )
  : stimulus(_name,_Vth, _Zth),
    bDrivenState(false),
    m_monitor(0),
    ZthWeak(_ZthWeak), ZthFloating(_ZthFloating),
    l2h_threshold(2.0),       // PICs are CMOS and use CMOS-like thresholds
    h2l_threshold(1.0),
    Vdrive_high(4.4),
    Vdrive_low(0.6),
    m_type( BI_DIRECTIONAL )
{
    is_analog = false;
    m_picPin = 0l;
}

IOPIN::~IOPIN()
{
    if (m_monitor) ((PinModule *)m_monitor)->clrPin();
}

void IOPIN::set_digital_threshold(double vdd)
{
    set_l2h_threshold(vdd > 4.5 ? 2.0 : 0.25 * vdd + 0.8);
    set_h2l_threshold(vdd > 4.5 ? 0.8 : 0.15 * vdd);
    Vdrive_high = vdd - 0.6;
    Vdrive_low = 0.6;
}

void IOPIN::setMonitor(PinMonitor *new_pinMonitor)
{
    if (m_monitor && new_pinMonitor) cout << "IOPIN already has a monitor!" << endl;
    else  m_monitor = new_pinMonitor;
}

void IOPIN::get(char *return_str, int len)
{
    if (return_str)
    {
        if (get_direction() == DIR_OUTPUT)
            strncpy(return_str, IOPIN::getDrivingState()?"1": "0", len);
        else
            strncpy(return_str, IOPIN::getState()?"1": "0", len);
    }
}

void IOPIN::attach(Stimulus_Node *s)
{
    snode = s;
}

void IOPIN::show()
{
    stimulus::show();
}

void IOPIN::set_nodeVoltage( double new_nodeVoltage )
{
    nodeVoltage = new_nodeVoltage;

    if     ( nodeVoltage < h2l_threshold ) setDrivenState(false); // The voltage is below the low threshold
    else if( nodeVoltage > l2h_threshold ) setDrivenState(true); // The voltage is above the high threshold
}

//------------------------------------------------------------
// putState - called by peripherals when they wish to
// drive an I/O pin to a new state.

void IOPIN::putState(bool new_state)
{
    if(new_state != bDrivingState) 
    {
        bDrivingState = new_state;
        Vth = bDrivingState ? Vdrive_high : Vdrive_low;

        // If this pin is tied to a node, then update the node.
        // Note that when the node is updated, then the I/O port
        // (if there is one) holding this I/O pin will get updated.
        // If this pin is not tied to a node, then try to update
        // the I/O port directly.

        if(snode) snode->update();
    }
    if(m_monitor) m_monitor->putState(new_state?'1':'0');
}

void IOPIN::putState(double new_Vth)
{
    if( new_Vth != Vth ) 
    {
        Vth = new_Vth;

        if (Vth <= 0.3) bDrivingState = false;
        else            bDrivingState = true;

        // If this pin is tied to a node, then update the node.
        if(snode) snode->update();
    }
    if(m_monitor) m_monitor->putState(bDrivingState?'1':'0');
}

bool IOPIN::getState()
{
    return getDriving() ? getDrivingState() : getDrivenState();
}

void IOPIN::setDrivingState(bool new_state)
{ 
    bDrivingState = new_state;
    if(m_monitor) m_monitor->setDrivingState(bDrivingState?'1':'0');
}

void IOPIN::setDrivingState(char new3State)
{
    bDrivingState = (new3State=='1' || new3State=='W');

    if( m_picPin ) m_picPin->setState( bDrivingState ); // SimulIDE Pic pin

    if( m_monitor ) m_monitor->setDrivingState( new3State );
}

bool IOPIN::getDrivingState(void)
{
  return bDrivingState;
}

bool IOPIN::getDrivenState()
{
  return bDrivenState;
}

//------------------------------------------------------------------------
// setDrivenState
//
// An stimulus attached to this pin is driving us to a new state.
// This state will be recorded and propagate up to anything 
// monitoring this pin.

void IOPIN::setDrivenState(bool new_state)
{
  bDrivenState = new_state;

  // Propagate the new state to those things monitoring this pin.
  // (note that the 3-state value is what's propagated).
  if(m_monitor  /*&& !is_analog*/ ) // SimulIDE
  {
    m_monitor->setDrivenState(getBitChar());
  }
}

void IOPIN::toggle()
{
  putState((bool) (getState() ^ true));
}

/*************************************
 *  int IOPIN::get_Vth()
 *
 * If this iopin has a stimulus attached to it then
 * the voltage will be dictated by the stimulus. Otherwise,
 * the voltage is determined by the state of the ioport register
 * that is inside the pic. For an input (like this), the pic code
 * that is being simulated can not change the state of the I/O pin.
 * However, the user has the ability to modify the state of
 * this register either by writing directly to it in the cli,
 * or by clicking in one of many places in the gui.
 */
double IOPIN::get_Vth()
{
  return Vth;
}

char IOPIN::getBitChar()
{
  if(snode )
  {                                              //  was  return 'Z';  // High impedance - unknown state.
      if( snode->get_nodeZth() > ZthFloating ) return 'Z';
      if( snode->get_nodeZth() > ZthWeak )     return getDrivenState() ? 'W' : 'w';
  }
  return getDrivenState() ? '1' : '0';
}

//========================================================================
//
IO_bi_directional::IO_bi_directional(const char *_name,
                                     double _Vth, 
                                     double _Zth,
                                     double _ZthWeak,
                                     double _ZthFloating,
                                     double _VthIn,
                                     double _ZthIn)
  : IOPIN(_name, _Vth, _Zth, _ZthWeak, _ZthFloating),
    ZthIn(_ZthIn), VthIn(_VthIn)
{
}

void IO_bi_directional::set_nodeVoltage( double new_nodeVoltage)
{
  IOPIN::set_nodeVoltage(new_nodeVoltage);
}

double IO_bi_directional::get_Vth()
{
  if( getDriving() ) return getDrivingState() ? Vth : 0;

  return VthIn;
}

double IO_bi_directional::get_Zth()
{
  return getDriving() ? Zth : ZthIn;
}

/* getBitChar() returns bit status as follows
     Input pin
        1> Pin considered floating, 
           return 'Z'
        2> Weak Impedance on pin, 
           return 'W" if high or 'w' if low
        3> Pin being driven externally
           return '1' node voltage high '0' if low
     Output pin
        1> Node voltage opposite driven value
           return 'X' if node voltage high or 'x' if inode voltage low
        2> Node voltage same as driven value
           return '1' node voltage high '0' if low
*/

char IO_bi_directional::getBitChar()
{
  if(snode) 
  {
    if (!getDriving())                // input pin
    {
      if(snode->get_nodeZth() > ZthFloating) return 'Z';
                                                                                
      if(snode->get_nodeZth() > ZthWeak) return getDrivenState() ? 'W' : 'w';
    }
    else if(getDrivenState() != getDrivingState()) return getDrivenState() ? 'X' : 'x';
  }
  return getDrivenState() ? '1' : '0';
}

//---------------
//::update_direction(uint new_direction)
//
//  This is called when a new value is written to the tris register
// with which this bi-direction pin is associated.

void IO_bi_directional::update_direction( uint new_direction, bool refresh )
{
    bool out = new_direction ? true : false;

    setDriving( out );

    // If this pin is not associated with an IO Port, but it's tied
    // to a stimulus, then we need to update the stimulus.

    if( m_picPin ) m_picPin->setDirection( out ); // SimulIDE Pic pin

    if( refresh && snode ) snode->update();
}

void IO_bi_directional::putState(bool new_state)
{
    IOPIN::putState(new_state);
}

void IO_bi_directional::putState(double new_Vth)
{
    VthIn = new_Vth;
    IOPIN::putState(new_Vth);
}

IO_bi_directional_pu::IO_bi_directional_pu(const char *_name,
                                           double _Vth, 
                                           double _Zth,
                                           double _ZthWeak,
                                           double _ZthFloating,
                                           double _VthIn,
                                           double _ZthIn,
                                           double _Zpullup)
  : IO_bi_directional(_name, _Vth, _Zth, _ZthWeak,
                      _ZthFloating, _VthIn, _ZthIn),
    Zpullup(_Zpullup)
{
  Vpullup = Vth;
  bPullUp = false;
}

IO_bi_directional_pu::~IO_bi_directional_pu(void)
{
}

void IO_bi_directional_pu::set_is_analog(bool flag)
{
    //cout << "IO_bi_directional_pu::set_is_analog Ignoring "<<name().c_str()<<" " << flag <<"\n";//SimulIDE
    //return;
    if( is_analog != flag )
    {
        is_analog = flag;

        if (snode) snode->update();
        //else if (!getDriving()) setDrivenState( bPullUp /*&& !is_analog*/ );
    }
}

void IO_bi_directional_pu::update_pullup( char new_state, bool refresh )
{
  bool bNewPullupState = new_state == '1' || new_state == 'W';
  if (bPullUp != bNewPullupState)  
  {
    bPullUp = bNewPullupState;
    if (refresh) 
    {
      // If there is a node attached to the pin, then we already 
      // know the driven state. If there is no node attached and 
      // this pin is configured as an input, then let the drivenState
      // be the same as the pullup state.
      if (snode) snode->update();
      else if (!getDriving()) setDrivenState(bPullUp && !is_analog);
    }
    if( m_picPin ) m_picPin->setPullup( bPullUp ); // SimulIDE Pic pin
  }
}

double IO_bi_directional_pu::get_Zth()
{
  return getDriving() ? Zth : ((bPullUp && ! is_analog)? Zpullup : ZthIn);
}

double IO_bi_directional_pu::get_Vth()
{
  // If the pin is configured as an output, then the driving voltage
  // depends on the pin state. If the pin is an input, and the pullup resistor
  // is enabled, then the pull-up resistor will 'drive' the output. The
  // open circuit voltage in this case will be Vth (the thevenin voltage, 
  // which is assigned to be same as the processor's supply voltage).

  if(getDriving()) return getDrivingState() ? Vth : 0;
  else             return (bPullUp && !is_analog) ? Vpullup : VthIn;
}

/*
   getBitChar() returns bit status as follows
     Input pin
        1> Pin considered floating, 
           return 'Z'
        2> Weak Impedance on pin, 
           return 'W" if high or 'w' if low
        3> Pin being driven externally
           return '1' node voltage high '0' if low
     Output pin
        1> Node voltage opposite driven value
           return 'X' if node voltage high or 'x' if inode voltage low
        2> Node voltage same as driven value
           return '1' node voltage high '0' if low
*/

char IO_bi_directional_pu::getBitChar()
{
  if(snode) 
  {
    if (!getDriving())                // input pin
    {
      if(snode->get_nodeZth() > ZthFloating) return 'Z';

      if(snode->get_nodeZth() > ZthWeak)
        return getDrivenState() ? 'W' : 'w';
     }
    else if(getDrivenState() != getDrivingState())
        return getDrivenState() ? 'X' : 'x';
  }
  return getDrivenState() ? '1' : '0';
}

IO_open_collector::IO_open_collector(const char *_name)
  : IO_bi_directional_pu(_name)
{
    m_type = OPEN_COLLECTOR;
}

double IO_open_collector::get_Vth()
{
  if(getDriving() && !getDrivingState()) return 0.0;

  return bPullUp ? Vpullup : VthIn;
}

double IO_open_collector::get_Zth()
{
  if(getDriving() && !getDrivingState()) return Zth;

  return bPullUp ? Zpullup : ZthIn;
}

char IO_open_collector::getBitChar()
{
  if(snode) 
  {
    if(snode->get_nodeZth() > ZthFloating)
      return bPullUp ? 'W' : 'Z';

    if(getDriving() && getDrivenState() && !getDrivingState())
      return 'X';

    if(snode->get_nodeZth() > ZthWeak)
      return getDrivenState() ? 'W' : 'w';
    else
      return getDrivenState() ? '1' : '0';
  }
  //return getDrivingState() ? 'W' : '0';
  return getDrivenState() ? '1' : '0';   //SimulIDE RA4 not working as input
}


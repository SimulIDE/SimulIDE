/*
   Copyright (C) 2017 Roy R Rankin

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

COMPLEMENTARY WAVEFORM GENERATOR (CWG) MODULE

*/

#ifndef __CWG_h__
#define __CWG_h__
#include "registers.h"

class CWG;
class TristateControl;
class CWGSignalSource;
class FLTSignalSink;

class CWGxCON0 : public sfr_register
{
public:
	CWGxCON0(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc);
	virtual void put(uint new_value);
	void set_con0_mask(uint mask) { con0_mask = mask; }
private:
	CWG *pt_cwg;
	uint con0_mask;
};

class CWGxCON1 : public sfr_register
{
public:
	CWGxCON1(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc);
	virtual void put(uint new_value);
	void set_con1_mask(uint mask) { con1_mask = mask; }
private:
	CWG *pt_cwg;
	uint con1_mask;
};

class CWGxCON2 : public sfr_register
{
public:
	CWGxCON2(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc);
	virtual void put(uint new_value);
private:
	CWG *pt_cwg;
	uint con2_mask;
};

class CWGxDBR : public sfr_register, public TriggerObject

{
public:
	enum {
		CWGxDBR0 = 1<<0,
		CWGxDBR1 = 1<<1,
		CWGxDBR2 = 1<<2,
		CWGxDBR3 = 1<<3,
		CWGxDBR4 = 1<<4,
		CWGxDBR5 = 1<<5
	};
	CWGxDBR(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc);
        virtual void callback();
        virtual void callback_print();
	void new_edge(bool level, double mult);
	void kill_callback();

private:
	CWG *pt_cwg;
        uint64_t  future_cycle;
	bool     next_level;
};
class CWGxDBF : public sfr_register, public TriggerObject
{
public:
	enum {
		CWGxDBF0 = 1<<0,
		CWGxDBF1 = 1<<1,
		CWGxDBF2 = 1<<2,
		CWGxDBF3 = 1<<3,
		CWGxDBF4 = 1<<4,
		CWGxDBF5 = 1<<5
	};
	CWGxDBF(CWG *pt, Processor *pCpu, const char *pName, const char *pDesc);
        virtual void callback();
        virtual void callback_print();
	void new_edge(bool level, double mult);
	void kill_callback();
private:
	CWG *pt_cwg;
        uint64_t  future_cycle;
	bool     next_level;
};

// Complementary waveform generator - 4 clock inputs
class CWG
{
public:
	enum {
		//CWG1CON0
		GxCS0  = 1<<0,
		GxPOLA = 1<<3,
		GxPOLB = 1<<4,
                GxOEA  = 1<<5,
		GxOEB  = 1<<6,
		GxEN   = 1<<7,
		//CWG1CON1
		GxIS0    = 1<<0,
		GxIS1    = 1<<1,
		GxIS2    = 1<<2,
		GxASDLA0 = 1<<4,
		GxASDLA1 = 1<<5,
		GxASDLB0 = 1<<6,
		GxASDLB1 = 1<<7,
		//CWG1CON2
		GxASDFLT = 1<<0,
		GxASDCLC1 = 1<<1,
		GxARSEN  = 1<<6,
		GxASE    = 1<<7
	};

	CWGxCON0 cwg1con0;
	CWGxCON1 cwg1con1;
	CWGxCON2 cwg1con2;
	CWGxDBF  cwg1dbf;
	CWGxDBR  cwg1dbr;
	CWG(Processor *pCpu);
	~CWG();
	void set_IOpins(PinModule *, PinModule *, PinModule *);
	void oeA();
	void oeB();
	void cwg_con0(uint);
	void cwg_con1(uint);
	void cwg_con2(uint);
	void releasePin(PinModule *pin);
	void releasePinSource(PinModule *pin);
	virtual void out_pwm(bool level, char index);
	virtual void out_NCO(bool level);
	virtual void out_CLC(bool level, char index=1);
	void input_source(bool level);
	void set_outA(bool level);
	void set_outB(bool level);
  	void autoShutEvent(bool on);
	void enableAutoShutPin(bool on);
	virtual void setState(char);

protected:
        bool         pwm_state[4];
	bool	     clc_state[4];
	bool	     nco_state;
	uint con0_value;
	uint con1_value;
	uint con2_value;
	bool         shutdown_active;

private:
	Processor       *cpu;
	PinModule       *pinA;
	PinModule       *pinB;
	PinModule       *pinFLT;
	TristateControl *Atri;
	TristateControl *Btri;
	CWGSignalSource *Asrc;
	CWGSignalSource *Bsrc;
	FLTSignalSink   *FLTsink;
	bool         pinAactive;
	bool         pinBactive;
	bool         srcAactive;
	bool         srcBactive;
	bool         cwg_enabled;
	bool         OEA_state;
	bool         OEB_state;
	string       Agui;
	string       Bgui;
	string       FLTgui;
	bool         active_next_edge;
	bool	     FLTstate;
};

class CWG4 : public CWG
{
public:
	CWG4(Processor *pCpu);
	virtual void out_pwm(bool level, char index);
	virtual void out_NCO(bool level);
};
#endif //__CWG_h__


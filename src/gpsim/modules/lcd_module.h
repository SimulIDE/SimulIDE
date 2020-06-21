/*
   Copyright (C) 2017   Roy R Rankin

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
#ifndef __LCD_H__
#define __LCD_H__

class LCD_MODULE;
class pic_processor;


// LCDCON - LIQUID CRYSTAL DISPLAY CONTROL REGISTER

class LCDCON : public sfr_register
{
public:

  enum
  {
      LMUX0	= 1<<0,	//LMUX<1:0> Commons Select bits
      LMUX1	= 1<<1,	
      CS0	= 1<<2,	//CS<1:0> Clock Source Select bits
      CS1	= 1<<3,
      VLCDEN	= 1<<4,	// LCD Bias Voltage Pins Enable bit
      WERR	= 1<<5, // LCD Write Failed Error bit
      SLPEN	= 1<<6,	// LCD Driver Enable in Sleep mode bit
      LCDEN	= 1<<7	// LCD Driver Enable bit
  };

  LCDCON(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *);
  virtual void put(uint new_value);
  virtual void put_value(uint new_value);

  LCD_MODULE *lcd_module;
};

// LCDPS - LCD PRESCALER SELECT REGISTER
class LCDPS : public sfr_register
{
public:

  enum
  {
	LP0	= 1<<0,	//LP<3:0>: LCD Prescaler Select bits
	LP1	= 1<<1,
   	LP2	= 1<<2,
	LP3	= 1<<3,
	WA	= 1<<4,	// LCD Write Allow Status bit
	LCDA	= 1<<5,	// LCD Active Status bit
	BIASMD	= 1<<6,	// Bias Mode Select bit
	WFT	= 1<<7, // Waveform Type Select bit

	LPMASK = (LP0 | LP1 | LP2 | LP3)
  };
  LCDPS(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *, uint);
  virtual void put(uint new_value);
  LCD_MODULE *lcd_module;
  uint mask_writeable;
};

// LCDSEn - LCD SEGMENT REGISTERS

class LCDSEn : public sfr_register
{
public:

  LCDSEn(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *, uint _n);
  virtual void put(uint new_value);
  LCD_MODULE *lcd_module;
  uint n;
};

// LCDDATAx - LCD DATA REGISTERS
class LCDDATAx : public sfr_register
{
public:

  LCDDATAx(Processor *pCpu, const char *pName, const char *pDesc, LCD_MODULE *, uint _n);
  virtual void put(uint new_value);
  LCD_MODULE *lcd_module;
  uint n;
  // bypass put for Power On Reset so WERR flag not set
  virtual void putRV(RegisterValue rv)
  {
    value.init = rv.init;
    value.put(rv.data);
  }

};

class LCD_MODULE: public TriggerObject
{
public:
    LCD_MODULE(Processor *pCpu, bool p16f917);
    void set_Vlcd(PinModule *, PinModule *, PinModule *);
    void set_LCDcom(PinModule *, PinModule *, PinModule *, PinModule *);
    void set_LCDsegn(uint, PinModule *, PinModule *, PinModule *, PinModule *);
    void set_t1con(T1CON *t1c) {t1con = t1c;}
    void lcd_on_off(bool lcdOn);
    void set_bias(uint lmux);
    void lcd_set_com(bool lcdOn, uint lmux);
    void lcd_set_segPins(uint regno, uint old, uint diff);
    void clear_bias();
    void set_lcdcon_werr() { lcdcon->value.put(lcdcon->value.get() | LCDCON::WERR); }
    bool get_lcdps_wa() { return lcdps->value.get() & LCDPS::WA; }
    bool get_lcdcon_lcden() { return lcdcon->value.get() & LCDCON::LCDEN;}
    bool typeB() {return (lcdps->value.get() & LCDPS::WFT) && mux_now;}
    virtual void callback();
    virtual void setIntSrc(InterruptSource *_IntSrc) { IntSrc = _IntSrc;}
    void start_clock();
    void stop_clock();
    void drive_lcd();
    void save_hold_data();
    void start_typeA();
    void start_typeB();
    virtual void sleep();
    virtual void wake();
    


    Processor 		*cpu;
    InterruptSource 	*IntSrc;
    bool		Vlcd1_on, Vlcd2_on, Vlcd3_on;
    bool		is_sleeping;
    PinModule 		*Vlcd1, *Vlcd2, *Vlcd3;
    PinModule		*LCDsegn[24];
    PinModule		*LCDcom[4];
    unsigned char	LCDsegDirection[3];
    unsigned char	LCDcomDirection;
    unsigned char	hold_data[12];
    unsigned char	bias_now;
    unsigned char	mux_now;
    unsigned char	phase;
    unsigned char	num_phases;
    uint 	clock_tick;
    uint64_t 		future_cycle;
    uint64_t		map_com[4];
    uint64_t		map_on;
    uint64_t		map_off;

    LCDCON	*lcdcon;
    LCDPS	*lcdps;
    LCDSEn   	*lcdSEn[3];
    LCDDATAx 	*lcddatax[12];
    T1CON	*t1con;

};    

#endif // __LCD_H__

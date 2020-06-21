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


//
// p12x
//
//  This file supports:
//    PIC12C508 PIC12C509
//    PIC12CE518 PIC12CE519
//    PIC10F200 PIC10F202 PIC10F204
//    PIC10F220 PIC10F222
//

#include <stdio.h>
#include <iostream>
#include <string>

#include "config.h"
#include "packages.h"
#include "stimuli.h"
#include "i2c-ee.h"
#include "p12x.h"

//========================================================================
// Generic Configuration word for the midrange family.

class Generic12bitConfigWord : public ConfigWord
{
    public:
      Generic12bitConfigWord(P12bitBase *pCpu)
        : ConfigWord("CONFIG", 0xfff, "Configuration Word", pCpu, 0xfff),
        m_pCpu(pCpu)
      {
        assert(pCpu);
        pCpu->wdt.initialize(true);
      }

      enum {
        FOSC0  = 1<<0,
        FOSC1  = 1<<1,
        WDTEN  = 1<<2,
        CP     = 1<<3,
        MCLRE  = 1<<4
      };

      virtual void set(int64_t v)
      {
        int64_t oldV = getVal();

        Integer::set(v);
        if (m_pCpu) 
        {
          int64_t diff = oldV ^ v;
          m_pCpu->setConfigWord(v & 0x3ff, diff & 0x3ff);
        }
      }
      virtual string toString()
      {
        int64_t i64;
        get(i64);
        int i = i64 &0xfff;

        char buff[256];

        snprintf(buff,sizeof(buff),
                 "$%3x\n"
                 " FOSC=%d - Clk source = %s\n"
                 " WDTEN=%d - WDT is %s\n"
                 " CP=%d - Code protect is %s\n"
                 " MCLRE=%d - /MCLR is %s",
                 i,
                 i & (FOSC0 | FOSC1),
                 ((i & FOSC0) ? ((i & FOSC1) ? "EXTRC" : "XT") : ((i & FOSC1) ? "INTRC" : "LP")),
                 ((i & WDTEN) ? 1 : 0), ((i & WDTEN) ? "enabled" : "disabled"),
                 ((i & CP) ? 1 : 0), ((i & CP) ? "enabled" : "disabled"),
                 ((i & MCLRE) ? 1 : 0), ((i & MCLRE) ? "enabled" : "disabled"));
        return string(buff);
      }
      
    private:
      P12bitBase *m_pCpu;
};

void  P12_OSCCON::put(uint new_value)
{
  uint old = value.get();

  value.put(new_value);
  if((new_value ^ old) & FOSC4 && m_CPU) m_CPU->updateGP2Source();
  if ((new_value ^ old) & 0xfe && m_CPU) m_CPU->freqCalibration();
}

//========================================================================
// The P12 devices with an EEPROM contain two die. One is the 12C core and
// the other is an I2C EEPROM (Actually, it is not know if there are two
// physical die. However, it is known that there are two functional layouts
// in the same package.) These two devices are connected internally.
class P12_I2C_EE : public I2C_EE
{
    public:
      P12_I2C_EE(pic_processor *, uint _rom_size);
      ~P12_I2C_EE();
};


P12_I2C_EE::P12_I2C_EE(pic_processor *pcpu, uint _rom_size)
  : I2C_EE(pcpu,_rom_size)
{
  if(pcpu) pcpu->ema.set_Registers(rom, rom_size);
}

P12_I2C_EE::~P12_I2C_EE()
{
}

//-------------------------------------------------------------------

P12bitBase::P12bitBase(const char *_name, const char *desc)
  : _12bit_processor(_name,desc),
    m_gpio(0),
    m_tris(0),
    osccal(this,"osccal","Oscillator Calibration")
{

  configWord = 0;
  set_frequency(4e6);
  if(config_modes)
    config_modes->valid_bits = config_modes->CM_FOSC0 | config_modes->CM_FOSC1 |
      config_modes->CM_FOSC1x | config_modes->CM_WDTE | config_modes->CM_MCLRE;
}

P12bitBase::~P12bitBase()
{
 // printf("P12bitBase::~P12bitBase\n");
  if (m_gpio) {
    (&(*m_gpio)[3])->setControl(0);
    (&(*m_gpio)[2])->setControl(0);
  }
  delete m_IN_SignalControl;
  delete_sfr_register(m_gpio);
  delete_sfr_register(m_tris);
  remove_sfr_register(&tmr0);
  remove_sfr_register(&osccal);
}

void P12bitBase::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new Generic12bitConfigWord(this));
}

//========================================================================
void P12bitBase::create_iopin_map()
{
  package = new Package(8);
  if(!package) return;

  package->assign_pin(7, m_gpio->addPin(new IO_bi_directional_pu("gpio0"),0));
  package->assign_pin(6, m_gpio->addPin(new IO_bi_directional_pu("gpio1"),1));
  package->assign_pin(5, m_gpio->addPin(new IO_bi_directional("gpio2"),2));
  package->assign_pin(4, m_gpio->addPin(new IO_bi_directional_pu("gpio3"),3));
  package->assign_pin(3, m_gpio->addPin(new IO_bi_directional("gpio4"),4));
  package->assign_pin(2, m_gpio->addPin(new IO_bi_directional("gpio5"),5));
  package->assign_pin(1, 0);
  package->assign_pin(8, 0);

  // gpio3 is input only, but we want pullup, so use IO_bi_directional_pu
  // but force as input pin disableing TRIS control
  m_IN_SignalControl = new IN_SignalControl;
  (&(*m_gpio)[3])->setControl(m_IN_SignalControl);
}

//--------------------------------------------------------
void P12bitBase::reset(RESET_TYPE r)
{
  m_tris->reset(r);

  switch (r) 
  {
      case IO_RESET:
        // Set GPWUF flag
        status->put(status->value.get() | 0x80);

        // fall through...
       default:
        _12bit_processor::reset(r);
  }
}

//------------------------------------------------------------------------
#define STATUS_GPWUF  0x80

void P12bitBase::enter_sleep()
{
  pic_processor::enter_sleep();

  status->put( status->value.get() & ~STATUS_GPWUF);
  cout << "enter sleep status="<<hex <<status->get()<<endl;
}


void  P12bitBase::updateGP2Source()
{
  PinModule *pmGP2 = &(*m_gpio)[2];

  if(option_reg->value.get() & OPTION_REG::T0CS)
  {
    printf("OPTION_REG::T0CS forcing GPIO2 as input, TRIS disabled\n");
    pmGP2->setControl(m_IN_SignalControl);
  }
  else
  {
    cout << "TRIS now controlling gpio2\n";
    pmGP2->setControl(0);
  }
}
// freqCalibrate modifies the internal RC frequency
// the base varsion is for the 12C508 and 12C509 Processors
// the spec sheet does not indicate the range or step size of corrections
// so this is based on +/- 12.5 % as per 16f88
void  P12bitBase::freqCalibration()
{
    // If internal RC oscilator
    if((configWord & (FOSC0 | FOSC1)) == FOSC1)
    {
        int osccal_val = (osccal.get() >> 4) - 0x07;
        double freq = get_frequency();
        freq *= 1. + 0.125 * osccal_val / 0x08;
        set_frequency(freq);
    }
}
// option_new_bits_6_7 is called from class OPTION_REG when
// bits 5, 6, or 7 of  OPTION_REG change state
//
void  P12bitBase::option_new_bits_6_7(uint bits)
{
  // Weak pullup if NOT_GPPU == 0
  m_gpio->setPullUp ( (bits & OPTION_REG::BIT6) != OPTION_REG::BIT6 , (configWord & MCLRE));
  updateGP2Source();
}

void P12bitBase::create_sfr_map()
{
  RegisterValue porVal(0,0);

  add_sfr_register(indf,   0, porVal);
  add_sfr_register(&tmr0,  1, porVal);
  add_sfr_register(pcl,    2, RegisterValue(0xff,0));
  add_sfr_register(status, 3, porVal);
  add_sfr_register(fsr,    4, porVal);
  add_sfr_register(&osccal,5, RegisterValue(0x70,0));
  add_sfr_register(m_gpio, 6, porVal);
  add_sfr_register(m_tris, 0xffffffff, RegisterValue(0x3f,0));
  add_sfr_register(Wreg, 0xffffffff, porVal);
  option_reg->set_cpu(this);

  osccal.set_cpu(this);
}

void P12bitBase::dump_registers ()
{
  _12bit_processor::dump_registers();

  cout << "tris = 0x" << hex << m_tris->value.get() << '\n';
  cout << "osccal = 0x" << osccal.value.get()  << '\n';
}

void  P12bitBase::setConfigWord(uint val, uint diff)
{
    PinModule *pmGP3 = &(*m_gpio)[3];

    configWord = val;
    if (diff & WDTEN)  wdt.initialize((val & WDTEN) == WDTEN);

    if ((val & MCLRE) == MCLRE) pmGP3->getPin().update_pullup('1', true);
}

void P12bitBase::tris_instruction(uint tris_register)
{
  m_tris->put(Wget());
}

void P12C508::create()
{
  create_iopin_map();

  _12bit_processor::create();

  add_file_registers(0x07, 0x1f, 0);
  P12bitBase::create_sfr_map();
  create_invalid_registers ();

  tmr0.set_cpu(this,m_gpio,2,option_reg);
  tmr0.start(0);

  pc->reset();
}


Processor * P12C508::construct(const char *name)
{
  P12C508 *p = new P12C508(name);

  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}

P12C508::P12C508(const char *_name, const char *desc)
  : P12bitBase(_name,desc)
{
  m_gpio = new GPIO(this,"gpio","I/O port",8,0x3f);
  m_tris = new PicTrisRegister(this,"tris","Port Direction Control", m_gpio, false);
  m_tris->wdtr_value=RegisterValue(0x3f,0);
}

P12C508::~P12C508()
{
  delete_file_registers(0x07, 0x1f);
}

P12F508::P12F508(const char *_name, const char *desc)
  : P12C508(_name,desc)
{
}
P12F508::~P12F508()
{
}

Processor * P12F508::construct(const char *name)
{
  P12F508 *p = new P12F508(name);
  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}
//--------------------------------------------------------

void P12C509::create_sfr_map()
{
}

Processor * P12C509::construct(const char *name)
{
  P12C509 *p = new P12C509(name);

  p->pc->set_reset_address(0x3ff);
  p->create();

  return p;
}

void P12C509::create()
{
  P12C508::create();

  alias_file_registers(0x00,0x0f,0x20);
  add_file_registers(0x30, 0x3f, 0);

  pa_bits = PA0;                 // the 509 has two code pages (i.e. PAO in status is used)
  indf->base_address_mask2 = 0x3F;  // RP - need this or INDF won't work right
}

P12C509::P12C509(const char *_name, const char *desc)
  : P12C508(_name,desc)
{
}

P12C509::~P12C509()
{
  delete_file_registers(0x30, 0x3f);
}


P12F509::P12F509(const char *_name, const char *desc)
  : P12C509(_name,desc)
{
}
P12F509::~P12F509()
{
}
Processor * P12F509::construct(const char *name)
{
  P12F509 *p = new P12F509(name);
  p->pc->set_reset_address(0x3ff);
  p->create();

  return p;
}

//
P12F510::P12F510(const char *_name, const char *desc)
  : P12F509(_name,desc)
{
}
P12F510::~P12F510()
{
}

Processor * P12F510::construct(const char *name)
{
  P12F510 *p = new P12F510(name);
  p->pc->set_reset_address(0x3ff);
  p->create();

  return p;
}

//--------------------------------------------------------

// construct function is identical to 12C508 version ??
Processor * P12CE518::construct(const char *name)
{
  P12CE518 *p = new P12CE518(name);

  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}


void P12CE518::create_iopin_map()
{
  P12C508::create_iopin_map();

  // Define the valid I/O pins.
  //gpio.valid_iopins = 0xff;
}

void P12CE518::create()
{
  P12C508::create();

  m_eeprom = new P12_I2C_EE(this, 0x10);
  m_eeprom->debug();

  // GPIO bits 6 and 7 are not bonded to physical pins, but are tied
  // to the internal I2C device.
  m_gpio->setEnableMask(0xc0 | m_gpio->getEnableMask());
  RegisterValue por_value(0xc0,0x00);
  m_gpio->value       = por_value;
  m_gpio->por_value   = por_value;
  m_gpio->wdtr_value  = por_value;
  m_gpio->put(0xc0);

  osccal.por_value = RegisterValue(0x80,0);

  // Kludge to force top two bits to be outputs
  m_tris->put(0x3f);

  {
    scl = new Stimulus_Node ( "EE_SCL" );
    io_scl = new IO_bi_directional_pu("gpio7");
    io_scl->update_pullup('1',true);
    io_scl->setDrivingState(true);
    io_scl->setDriving(true);
    scl->attach_stimulus( m_gpio->addPin(io_scl,7));
    scl->update();
  }
  {
    sda = new Stimulus_Node ( "EE_SDA" );

    io_sda = new IO_open_collector("gpio6");
    // enable the pullup resistor.
    io_sda->update_pullup('1',true);
    io_sda->setDrivingState(true);
    io_sda->setDriving(true);
    m_gpio->addPin(io_sda,6);
    sda->attach_stimulus (io_sda);
    sda->update();
  }

  m_eeprom->attach ( scl, sda );
  /*
  ema.set_cpu(this);
  ema.set_Registers(m_eeprom->rom, m_eeprom->rom_size);
  */
}

P12CE518::P12CE518(const char *_name, const char *desc)
  : P12C508(_name,desc)
{
  if(config_modes)
    config_modes->valid_bits = config_modes->CM_FOSC0 | config_modes->CM_FOSC1 |
      config_modes->CM_FOSC1x | config_modes->CM_WDTE | config_modes->CM_MCLRE;
}

P12CE518::~P12CE518()
{
    delete m_eeprom;
    delete io_scl;
    delete io_sda;
    delete scl;
    delete sda;
}

void P12CE518::tris_instruction(uint tris_register)
{
    uint w_val = Wget();
    m_tris->put ( w_val & 0x3F );     // top two bits always output
}


// freqCalibrate modifies the internal RC frequency
// this version is for the 12CE518 and 12CE519 Processors but would also
// be correct for 12C508A/C509A/CR509A
// the spec sheet does not indicate the range or step size of corrections
// so this is based on +/- 12.5 % as per 16f88
void  P12CE518::freqCalibration()
{
    // If internal RC oscilator
    if((configWord & (FOSC0 | FOSC1)) == FOSC1)
    {
        int osccal_val = (osccal.get() >> 2) - 0x20;
        double freq = 4e6;
        freq *= 1. + 0.125 * osccal_val / 0x20;
        set_frequency(freq);
    }
}
//--------------------------------------------------------

void P12CE519::create_sfr_map()
{
}

Processor * P12CE519::construct(const char *name)
{
  P12CE519 *p = new P12CE519(name);

  cout << " 12ce519 construct\n";

  p->pc->set_reset_address(0x3ff);
  p->create();

  return p;
}


void P12CE519::create()
{
  P12CE518::create();

  alias_file_registers(0x00,0x0f,0x20);
  add_file_registers(0x30, 0x3f, 0);

  pa_bits = PA0;                 // the 519 has two code pages (i.e. PAO in status is used)
  indf->base_address_mask2 = 0x3F;  // RP - need this or INDF won't work right
}


P12CE519::P12CE519(const char *_name, const char *desc)
  : P12CE518(_name,desc)
{
}

P12CE519::~P12CE519()
{
  delete_file_registers(0x30, 0x3f);
}


//--------------------------------------------------------
//
// GPIO Port

GPIO::GPIO(P12bitBase *pCpu, const char *pName, const char *pDesc,
           uint numIopins,
           uint enableMask,
           uint resetMask,
           uint wakeupMask,
           uint configMaskMCLRE)
  : PicPortRegister (pCpu,pName,pDesc, numIopins, enableMask), m_CPU(pCpu)
  , m_resetMask(resetMask)
  , m_wakeupMask(wakeupMask)
  , m_configMaskMCLRE(configMaskMCLRE)
{
}

void GPIO::setbit(uint bit_number, char new_value)
{
  uint lastDrivenValue = rvDrivenValue.data;

  PortRegister::setbit(bit_number, new_value);

  // If gpio bit 0,1 or 3 changed states AND
  // ~GPWU is low (wake up on change is enabled) AND
  // the processor is sleeping.
  //    Then wake

  uint diff = lastDrivenValue ^ rvDrivenValue.data;
  //if ((diff & (1<<3)) && cpu_pic->config_modes->get_mclre()) { // GP3 is the reset pin
  if ((diff & m_resetMask) && (m_CPU->configWord & m_configMaskMCLRE))
  {
    cpu->reset( (rvDrivenValue.data & m_resetMask) ? EXIT_RESET : MCLR_RESET);
    return;
  }

  if (diff & m_wakeupMask) 
  {
    // If /GPWU is 0 (i.e. enabled) and the processor is currently sleeping
    // then wake up the processor by resetting it.
    if( ((cpu12->option_reg->value.get() & 0x80) == 0) &&
        cpu12->getActivityState() == pic_processor::ePASleeping) 
    {
      cpu->reset(IO_RESET);
    }
  }
}

// if bNewPU == true set weak pullups otherwise clear weak pullups
void GPIO::setPullUp ( bool bNewPU , bool mclr)
{
  m_bPU = bNewPU;

  // In the following do not change pullup state of internal pins
  uint mask = getEnableMask() & 0x3f;

  // If mclr active do not change pullup on gpio3
  if (mclr) mask &= 0x37;

  for (uint i=0, m = 1; mask; i++, m <<= 1)
  {
    if (mask & m)
    {
      mask ^= m;
      getPin(i)->update_pullup ( m_bPU ? '1' : '0', true );
    }
  }
}


//------------------------------------------------------------------------
void P10F200::create_iopin_map()
{
  package = new Package(6);
  if(!package) return;

  package->assign_pin(1, m_gpio->addPin(new IO_bi_directional_pu("gpio0"),0));
  package->assign_pin(3, m_gpio->addPin(new IO_bi_directional_pu("gpio1"),1));
  package->assign_pin(4, m_gpio->addPin(new IO_bi_directional("gpio2"),2));
  package->assign_pin(6, m_gpio->addPin(new IO_bi_directional_pu("gpio3"),3));
  package->assign_pin(2, 0);
  package->assign_pin(5, 0);

  // gpio3 is input only, but we want pullup, so use IO_bi_directional_pu
  // but force as input pin disableing TRIS control
  m_IN_SignalControl = new IN_SignalControl;
  m_OUT_SignalControl = new OUT_SignalControl;
  m_OUT_DriveControl = new OUT_DriveControl;
  (&(*m_gpio)[3])->setControl(m_IN_SignalControl);
}

void P10F200::create()
{
  create_iopin_map();

  _12bit_processor::create();

  add_file_registers(0x10, 0x1f, 0);    // 10F200 only has 16 bytes RAM
  P12bitBase::create_sfr_map();
  create_invalid_registers ();

  tmr0.set_cpu(this,m_gpio,2,option_reg);
  tmr0.start(0);
  osccal.set_cpu(this);
  osccal.por_value = RegisterValue(0xfe,0);

  pc->reset();
}

Processor * P10F200::construct(const char *name)
{
  P10F200 *p = new P10F200(name);

  p->pc->set_reset_address(0x0ff);
  p->create();

  return p;
}

P10F200::P10F200(const char *_name, const char *desc)
  : P12bitBase(_name,desc)
{
  m_gpio = new GPIO(this,"gpio","I/O port",8,0x0f);
  m_tris = new PicTrisRegister(this, "tris", "Port Direction Control",m_gpio, false);
  m_tris->wdtr_value=RegisterValue(0x3f,0);

  if(config_modes)
    config_modes->valid_bits = config_modes->CM_WDTE | config_modes->CM_MCLRE;
}

P10F200::~P10F200()
{
    (&(*m_gpio)[3])->setControl(0);
    (&(*m_gpio)[2])->setControl(0);

    delete m_OUT_SignalControl;
    delete m_OUT_DriveControl;
    delete_file_registers(0x10, 0x1f);
}

void P10F200::updateGP2Source()
{
  PinModule *pmGP2 = &(*m_gpio)[2];

  if (osccal.value.get() & P12_OSCCON::FOSC4 )
  {
    pmGP2->setSource(m_OUT_DriveControl);
    printf("OSCCON::FOSC4 forcing GPIO2 high on output, TODO FOSC4 toggle output\n");
  }
  else if(option_reg->value.get() & OPTION_REG::T0CS)
  {
    printf("OPTION_REG::T0CS forcing GPIO2 as input, TRIS disabled\n");
    pmGP2->setControl(m_IN_SignalControl);
    pmGP2->setSource(0);
  }
  else
  {
    // revert to default control, i.e. let TRIS control the output
    pmGP2->setControl(0);
    pmGP2->setSource(0);
    cout << "TRIS now controlling gpio2\n";
  }
  pmGP2->updatePinModule();
}

// freqCalibrate modifies the internal RC frequency
// this version is for the 10F2xx Processors
// the spec sheet does not indicate the range or step size of corrections
// so this is based on +/- 12.5 % as per 16f88
void  P10F200::freqCalibration()
{
    // If internal RC oscilator
    char osccal_val = (osccal.value.get() & 0xfe);
    double freq = (configWord & 1)? 8e6 : 4e6;

    freq *= 1. + (0.125 * osccal_val) / 0x80;
    set_frequency(freq);
}

//------------------------------------------------------------------------

void P10F202::create()
{
  create_iopin_map();

  _12bit_processor::create();

  add_file_registers(0x08, 0x1f, 0);    // 10F202 has 24 bytes RAM
  P12bitBase::create_sfr_map();
  create_invalid_registers ();

  tmr0.set_cpu(this,m_gpio,2,option_reg);
  tmr0.start(0);

  pc->reset();
}

Processor * P10F202::construct(const char *name)
{
  P10F202 *p = new P10F202(name);

  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}

P10F202::P10F202(const char *_name, const char *desc)
  : P10F200(_name,desc)
{
}

P10F202::~P10F202()
{
    delete_file_registers(0x08, 0x0f); // Rest is deleted by P10F200
}

//========================================================================
// Comparator module for the 10c204 and 10c206
//
class Comparator10C20x
{
    public:
      Comparator10C20x();
      ~Comparator10C20x();
};

class COUT_SignalSource;

//========================================================================
// COUT_SignalControl -- controls GPIO2's direction when the comparator is
// enabled. When the comparator is enabled, GPIO2 is an output.

class COUT_SignalControl : public SignalControl
{
    public:
      COUT_SignalControl(){}
      ~COUT_SignalControl(){ }
      virtual char getState() { return '0'; }
      virtual void release() { delete this; }
};


class CIN_SignalSink;
class CMCON0 : public sfr_register
{
    public:
      enum {
        CWU    = 1<<0,
        CPREF  = 1<<1,
        CNREF  = 1<<2,
        CMPON  = 1<<3,
        CMPTOCS = 1<<4,
        POL    = 1<<5,
        COUTEN = 1<<6,
        CMPOUT = 1<<7
      };

      CMCON0(P10F204 *pCpu, const char *pName, const char *pDesc,
             PinModule *CInP, PinModule *CInM, PinModule *COut);

      ~CMCON0();

      virtual void put(uint new_value);
      virtual void put_value(uint new_value);
      bool isEnabled() { return ((value.get() & COUTEN) == 0); }

      char getState()
      {
        char ret='Z';
        if ( (value.get() & (COUTEN | CMPON)) == CMPON)
          ret = (((value.get() & CMPOUT)==CMPOUT) ^ ((value.get() & POL)==POL)) ? '0' : '1';

        return ret;
      }

      void refresh();

      SignalControl *getSource() { return (SignalControl *)m_source; }
      SignalControl *getGPDirectionControl() { return m_control; }

      void setInputState(char newState, bool bInput);
  
    private:
      P10F204 *p_F204;
      COUT_SignalControl *m_control;
      COUT_SignalSource *m_source;
      bool                        active_control;
      bool                        active_source;
      CIN_SignalSink    *m_PosInput;
      CIN_SignalSink    *m_NegInput;

      PinModule *m_CInP;
      PinModule *m_CInM;
      PinModule *m_COut;

      double m_pV, m_nV;
};


//========================================================================
// COUT_SignalSource
//
// The comparator output is driven on to the GPIO pin if the COUTEN bit in
// CMCON0 is cleared ( and if the FOSC/4 logic is not driving).
// This is implemented via COUT_SignalSource. When COUTEN bit is asserted,
// then COUT_SignalSource overides the default output driver control for
// the GPIO pin.

class COUT_SignalSource : public SignalControl
{
    public:
      COUT_SignalSource(CMCON0 *pcmcon0)
          : m_cmcon0(pcmcon0)
      { }
      ~COUT_SignalSource() { }

      virtual char getState() { return m_cmcon0->getState(); }
      virtual void release() { }
      
    private:
      CMCON0 *m_cmcon0;
};

class CIN_SignalSink : public SignalSink
{
    public:
      CIN_SignalSink(CMCON0 *pcmcon0, bool binput)
        : m_cmcon0(pcmcon0),
          m_binput(binput)  // true==+input
      {}
      virtual void setSinkState(char new3State)
      {
        m_cmcon0->setInputState(new3State, m_binput);
      }
      virtual void release() {delete this; }

    private:
      CMCON0 *m_cmcon0;
      bool m_binput;
};

//-----------------------------------------------------------
CMCON0::CMCON0(P10F204 *pCpu, const char *pName, const char *pDesc,
               PinModule *CInP, PinModule *CInM, PinModule *COut)
  : sfr_register(pCpu, pName, pDesc),
    p_F204(pCpu),
    m_CInP(CInP),
    m_CInM(CInM),
    m_COut(COut)
{
  // assign the I/O pin associated with the
  // the comparator output.

  m_source = new COUT_SignalSource(this);
  m_control = new COUT_SignalControl();
  m_PosInput = new CIN_SignalSink(this,true);
  m_NegInput = new CIN_SignalSink(this,false);

  active_source = false;
  active_control = false;

  CInP->addSink(m_PosInput);
  CInM->addSink(m_NegInput);
  //COut->setSource(m_source);

  m_pV = m_nV = 0.0;
}

CMCON0::~CMCON0()
{
    if (!isEnabled())
    {
        delete m_source;
        delete m_control;
    }
}
void CMCON0::put(uint new_value)
{
  uint old_value = value.get();

  value.put((new_value & 0x7f ) | (old_value & CMPOUT) );

  // If any of the control bits that afffect CMPOUT have changed,
  // then refresh CMPOUT
  if ((old_value ^ new_value) & (CPREF | CNREF | CMPON | CMPTOCS | POL))
    refresh();

  // If the output enable changed states.
  if ((old_value ^ new_value) & COUTEN) p_F204->updateGP2Source();

  // If the comparator output state has changed or the polarity changed:
  if ((old_value ^ value.get()) & (CMPOUT | POL)) m_COut->updatePinModule();
}

void CMCON0::refresh()
{
  if (value.get() & CMPON) 
  {
    if (value.get() & CPREF) m_pV = m_CInP->getPin().get_nodeVoltage();
    else                     m_pV = m_CInM->getPin().get_nodeVoltage();

    if (value.get() & CNREF) m_nV = m_CInM->getPin().get_nodeVoltage();
    else                     m_nV = 0.6;

    value.put( (value.get() & 0x7f) | ((m_pV>m_nV)? CMPOUT : 0));
  }
}

void CMCON0::put_value(uint new_value)
{
}

void CMCON0::setInputState(char newState, bool bInput)
{
  if (bInput) 
  {
    if (value.get() & CPREF) m_pV = m_CInP->getPin().get_nodeVoltage();
  }
  else 
  {
    if ((value.get() & CPREF) == 0) m_pV = m_CInM->getPin().get_nodeVoltage();
    if (value.get() & CNREF)        m_nV = m_CInM->getPin().get_nodeVoltage();
    else                            m_nV = 0.6;
  }
  uint old_value = value.get();

  value.put( (old_value&0x7f) | ((m_pV>m_nV) ? CMPOUT : 0));

  m_COut->updatePinModule();
}

//========================================================================
P10F204::P10F204(const char *_name, const char *desc)
  : P10F200(_name,desc)
{
}

P10F204::~P10F204()
{
  delete_sfr_register(m_cmcon0);
}

void P10F204::create()
{
  P10F200::create();

  m_cmcon0 = new CMCON0(this, "cmcon0", "Comparator Control",
                        &(*m_gpio)[0], &(*m_gpio)[1], &(*m_gpio)[2]);

  RegisterValue porVal = RegisterValue(0xff,0);
  add_sfr_register(m_cmcon0, 7, porVal);
}

void P10F204::updateGP2Source()
{
  //    m_gpio->getIOpins(2)->setSource(m_cmcon0->getSource());
  PinModule *pmGP2 = &(*m_gpio)[2];

  if (osccal.get() & P12_OSCCON::FOSC4 )
  {

    pmGP2->setSource(m_OUT_DriveControl);
    printf("OSCCON::FOSC4 forcing GPIO2 high on output, TODO FOSC4 toggle output\n");
  }
  else if (m_cmcon0->isEnabled()) 
  {
    pmGP2->setControl(m_cmcon0->getGPDirectionControl());
    pmGP2->setSource(m_cmcon0->getSource());
    cout << "comparator is controlling the output of GPIO2\n";
  }
  else if(option_reg->get() & OPTION_REG::T0CS)
  {
    printf("OPTION_REG::T0CS forcing GPIO2 as input, TRIS disabled\n");
    pmGP2->setControl(m_IN_SignalControl);
    pmGP2->setSource(0);
  }
  else 
  {
    pmGP2->setControl(0);
    pmGP2->setSource(0);
  }
  pmGP2->updatePinModule();
}

//========================================================================
Processor * P10F204::construct(const char *name)
{
  P10F204 *p = new P10F204(name);

  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}
//========================================================================
P10F220::P10F220(const char *_name, const char *desc)
  : P10F200(_name,desc),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adres(this,"adres", "A2D Result")
{
}

P10F220::~P10F220()
{
    remove_sfr_register(&adcon0);
    remove_sfr_register(&adcon1);
    remove_sfr_register(&adres);
}

void P10F220::create()
{
  P10F200::create();
  add_sfr_register(&adcon0,  0x07, RegisterValue(0xcc,0));
  add_sfr_register(&adres,  0x08, RegisterValue(0,0));

  adcon1.setValidCfgBits(ADCON1::PCFG0 | ADCON1::PCFG1,0);
  adcon1.setNumberOfChannels(4);
  adcon1.setIOPin(0, &(*m_gpio)[0]);
  adcon1.setIOPin(1, &(*m_gpio)[1]);
  adcon1.setVoltRef(2, 0.6);
  adcon1.setVoltRef(3, 0.6);
  adcon1.setChannelConfiguration(0, 0x03);
  adcon1.setChannelConfiguration(1, 0x03);
  adcon1.setChannelConfiguration(2, 0x00);
  adcon1.setChannelConfiguration(3, 0x00);

  adcon0.setChannel_Mask(3);
  adcon0.setChannel_shift(2);
  adcon0.setAdres(&adres);
  adcon0.setAdresLow(0);
  adcon0.setAdcon1(&adcon1);
  adcon0.setA2DBits(8);
}

//========================================================================
Processor * P10F220::construct(const char *name)
{
  P10F220 *p = new P10F220(name);

  p->pc->set_reset_address(0xff);
  p->create();

  return p;
}
void P10F220::enter_sleep()
{
  uint val;

  _12bit_processor::enter_sleep();

  status->put( status->get() & ~STATUS_GPWUF);
  val = (adcon0.get() & ~(ADCON0_10::ADON|ADCON0_10::GO))
          | ADCON0_10::CHS1 | ADCON0_10::CHS0;
  adcon0.put(val);
}

void P10F220::exit_sleep()
{
  _12bit_processor::exit_sleep();

  adcon0.put(adcon0.get() | ADCON0_10::ANS1 | ADCON0_10::ANS0);
}

void  P10F220::setConfigWord(uint val, uint diff)
{
    PinModule *pmGP3 = &(*m_gpio)[3];

    configWord = val;
    if (diff & WDTEN) wdt.initialize((val & WDTEN) == WDTEN);

    if ((val & MCLRE))
    {
        if (!(val & NOT_MCPU)) pmGP3->getPin().update_pullup('1', true);
    }
    if ((val & IOSCFS)) set_frequency(8e6);
}
//========================================================================
P10F222::P10F222(const char *_name, const char *desc)
  : P10F220(_name,desc)
{
}
P10F222::~P10F222()
{
    delete_file_registers(0x09, 0x0f);
}

void P10F222::create()
{
  P10F220::create();
  add_file_registers(0x09, 0x0f, 0);    // 10F222 has 23 bytes RAM
}


//========================================================================
Processor * P10F222::construct(const char *name)
{
  P10F222 *p = new P10F222(name);

  p->pc->set_reset_address(0x1ff);
  p->create();

  return p;
}

//========================================================================
// P16F505 Config Word

class P16F505ConfigWord : public ConfigWord
{
    public:
      enum {
        FOSC0  = 1<<0,
        FOSC1  = 1<<1,
        FOSC2  = 1<<2,
        WDTEN  = 1<<3,
        CP     = 1<<4,
        MCLRE  = 1<<5
      };

      P16F505ConfigWord(P12bitBase *pCpu)
        : ConfigWord("CONFIG", 0xfff, "Configuration Word", pCpu, 0xfff),
            m_pCpu(pCpu)
      {
        assert(pCpu);
        pCpu->wdt.initialize(true);
      }

      virtual void set(int64_t v)
      {
        int64_t oldV = getVal();

        Integer::set(v);
        if (m_pCpu) {
          int64_t diff = oldV ^ v;
          m_pCpu->setConfigWord(v & 0x3ff, diff & 0x3ff);
        }
      }

      virtual string toString()
      {
        int64_t i64;
        get(i64);
        int i = i64 &0xfff;

        char buff[256];
        const char *src;

        switch(i&(FOSC0|FOSC1|FOSC2)) {
        case 0:
            src = "LP";
            break;
        case 1:
            src = "XT";
            break;
        case 2:
            src = "HS";
            break;
        case 3:
            src = "EC";
            break;
        case 4:
            src = "INTRCRB4";
            break;
        case 5:
            src = "INTRCCLK";
            break;
        case 6:
            src = "EXTRCRB4";
            break;
        case 7:
            src = "EXTRCCLK";
            break;
        }

        snprintf(buff, sizeof(buff),
                 "$%3x\n"
                 " FOSC=%d - Clk source = %s\n"
                 " WDTEN=%d - WDT is %s\n"
                 " CP=%d - Code protect is %s\n"
                 " MCLRE=%d - /MCLR is %s",
                 i,
                 i & (FOSC0 | FOSC1), src,
                 ((i & WDTEN) ? 1 : 0), ((i & WDTEN) ? "enabled" : "disabled"),
                 ((i & CP) ? 1 : 0), ((i & CP) ? "enabled" : "disabled"),
                 ((i & MCLRE) ? 1 : 0), ((i & MCLRE) ? "enabled" : "disabled"));
        return string(buff);
      }
      
    private:
      P12bitBase *m_pCpu;
};


//========================================================================
// P16F505 Implementation
P16F505::P16F505(const char *_name, const char *desc)
  : P12bitBase(_name,desc)
{
  m_portb = new GPIO(this,"portb","I/O port",8,0x3f, 1<<3, 0x1B, 1<<5);
  m_portc = new GPIO(this,"portc","I/O port",8,0x3f, 0, 0);
  m_trisb = new PicTrisRegister(this,"trisb","Port Direction Control", m_portb, false);
  m_trisc = new PicTrisRegister(this,"trisc","Port Direction Control", m_portc, false);
  m_trisb->wdtr_value=RegisterValue(0x3f,0);
  m_trisc->wdtr_value=RegisterValue(0x3f,0);
  if (config_modes)
      config_modes->valid_bits = config_modes->CM_FOSC0 | config_modes->CM_FOSC1 |
      config_modes->CM_FOSC1x | config_modes->CM_WDTE | config_modes->CM_MCLRE;

}

P16F505::~P16F505()
{
  delete_sfr_register(m_portb);
  delete_sfr_register(m_portc);
  delete_sfr_register(m_trisb);
  delete_sfr_register(m_trisc);
  delete_file_registers(0x08, 0x1f);
  delete_file_registers(0x30, 0x3f);
  delete_file_registers(0x50, 0x5f);
  delete_file_registers(0x70, 0x7f);
}

Processor * P16F505::construct(const char *name)
{
  P16F505 *p = new P16F505(name);

  p->pc->set_reset_address(0x3ff);
  p->create();

  return p;
}

void P16F505::create()
{
  create_iopin_map();

  _12bit_processor::create();

  add_file_registers(0x08, 0x1f, 0);
  create_sfr_map();
  create_invalid_registers ();

  alias_file_registers(0x00,0x0f,0x20);
  add_file_registers(0x30, 0x3f, 0);

  alias_file_registers(0x00,0x0f,0x40);
  add_file_registers(0x50, 0x5f, 0);

  alias_file_registers(0x00,0x0f,0x60);
  add_file_registers(0x70, 0x7f, 0);

  pa_bits = PA0;
  indf->base_address_mask2 = 0x7F;

  tmr0.set_cpu(this,m_portc,5,option_reg); // T0CKI pin
  tmr0.start(0);

  pc->reset();
}

void P16F505::create_iopin_map()
{
  package = new Package(14);
  if(!package) return;

  package->assign_pin(1, 0);
  package->assign_pin(2,  m_portb->addPin(new IO_bi_directional("portb5"),5));
  package->assign_pin(3,  m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(4,  m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(5,  m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(6,  m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(7,  m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(8,  m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(9,  m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(10, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional("portb2"),2));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(14, 0);

  // portb3 is input only, but we want pullup, so use IO_bi_directional_pu
  // but force as input pin disableing TRIS control
  m_IN_SignalControl = new IN_SignalControl;
  (&(*m_portb)[3])->setControl(m_IN_SignalControl);
}

void P16F505::create_sfr_map()
{
  RegisterValue porVal(0,0);

  add_sfr_register(indf,   0, porVal);
  add_sfr_register(&tmr0,  1, porVal);
  add_sfr_register(pcl,    2, RegisterValue(0xff,0));
  add_sfr_register(status, 3, porVal);
  add_sfr_register(fsr,    4, porVal);
  add_sfr_register(&osccal,5, RegisterValue(0x70,0));
  add_sfr_register(m_portb,6, porVal);
  add_sfr_register(m_portc,7, porVal);
  add_sfr_register(m_trisb, 0xffffffff, RegisterValue(0x3f,0));
  add_sfr_register(m_trisc, 0xffffffff, RegisterValue(0x3f,0));
  add_sfr_register(Wreg, 0xffffffff, porVal);
  option_reg->set_cpu(this);

  osccal.set_cpu(this);
}

void P16F505::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new P16F505ConfigWord(this));
}

void P16F505::tris_instruction(uint tris_register)
{
        if      ( tris_register == 6 ) m_trisb->put(Wget());
        else if ( tris_register == 7 ) m_trisc->put(Wget());
}

void  P16F505::setConfigWord(uint val, uint diff)
{
    PinModule *pmRB3 = &(*m_portb)[3];

    configWord = val;

    if( diff & WDTEN ) wdt.initialize((val & WDTEN) == WDTEN);

    if( (val & MCLRE) == MCLRE ) pmRB3->getPin().update_pullup('1', true);
}

void  P16F505::updateGP2Source()
{
  PinModule *pmPC5 = &(*m_portc)[5];

  if(option_reg->value.get() & OPTION_REG::T0CS)
  {
    printf("OPTION_REG::T0CS forcing PORTC5 as input, TRIS disabled\n");
    pmPC5->setControl(m_IN_SignalControl);
  }
  else
  {
    cout << "TRIS now controlling PORTC5\n";
    pmPC5->setControl(0);
  }
}

// option_new_bits_6_7 is called from class OPTION_REG when
// bits 5, 6, or 7 of  OPTION_REG change state
//
void  P16F505::option_new_bits_6_7(uint bits)
{
    bool bit6 = (bits & OPTION_REG::BIT6) != OPTION_REG::BIT6;

    // Weak pullup if NOT_GPPU == 0
    m_portb->setPullUp (bit6 , (configWord & MCLRE));
    updateGP2Source();
}

void P16F505::reset(RESET_TYPE r)
{
    m_trisb->reset(r);
    m_trisc->reset(r);

    switch (r) 
    {
      case IO_RESET:
        // Set GPWUF/RBWUF flag
        status->put(status->value.get() | 0x80);

      default:
        _12bit_processor::reset(r);
    }
}

void P16F505::dump_registers ()
{
  _12bit_processor::dump_registers();

  cout << "trisb = 0x" << hex << m_trisb->value.get() << '\n';
  cout << "trisc = 0x" << hex << m_trisc->value.get() << '\n';
  cout << "osccal = 0x" << osccal.value.get()  << '\n';
}

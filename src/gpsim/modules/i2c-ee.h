/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2003 Mike Durian
		 2006 Roy R Rankin

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

#ifndef I2C_EE_H
#define I2C_EE_H

#include "trigger.h"
#include "gpsim_classes.h"
//#include "value.h"

class Register;
class pic_processor;
class I2C_EE;
class I2C_SLAVE_SCL;
class I2C_SLAVE_SDA;
class Stimulus_Node;

//------------------------------------------------------------------------
//------------------------------------------------------------------------

class PromAddress : public Value
{
  public:
    PromAddress(I2C_EE *eeprom, const char *_name, const char * desc);
    void get(I2C_EE  *&eeprom) { eeprom = m_eeprom;}
    void get(char *buffer, int buf_size);

  private:
     I2C_EE *m_eeprom;
};


class i2c_slave : public TriggerObject
{

  public:
    i2c_slave();
    ~i2c_slave();
    void new_sda_edge(bool direction);
    void new_scl_edge(bool direction);
    bool shift_read_bit ( bool x );
    bool shift_write_bit ();
    virtual bool match_address();
    virtual void put_data(uint data){}
    virtual uint get_data(){return 0;}
    virtual void slave_transmit(bool yes){}
    const char * state_name();

    I2C_SLAVE_SCL	*scl;	// I2C clock
    I2C_SLAVE_SDA	*sda;	// I2C data
    uint i2c_slave_address;
    virtual void callback();

  protected:

    bool    scl_high;
    bool    nxtbit;
    bool    r_w;
    uint bit_count;  // Current bit number for either Tx or Rx
    uint xfr_data;  // latched data from I2C.


  enum {
    IDLE=0,
    START,
    RX_I2C_ADD,
    ACK_I2C_ADD,
    RX_DATA,
    ACK_RX,
    ACK_WR,
    WRPEND,
    ACK_RD,
    TX_DATA
  } bus_state;


};
class I2C_EE :  public i2c_slave//RRR, public TriggerObject
{
    public:

      I2C_EE(Processor *pCpu,
             uint _rom_size, uint _write_page_size = 1,
         uint _addr_bytes = 1, uint _CSmask = 0,
         uint _BSmask = 0, uint _BSshift = 0
        );
      virtual ~I2C_EE();
      void reset(RESET_TYPE);
      void debug();

    //  virtual void callback();
      virtual void callback_print();
      virtual void start_write();
      virtual void write_busy();
      virtual void write_is_complete();
      virtual void put_data(uint data);
      virtual uint get_data();
      virtual void slave_transmit(bool);
      virtual bool match_address();

      virtual Register *get_register(uint address);
      inline int register_size() {return rom_data_size; }
      inline void set_register_size(int bytes) { rom_data_size = bytes; }

      virtual void attach ( Stimulus_Node *_scl, Stimulus_Node *_sda );
      virtual void set_chipselect(uint _chipselect); 

      inline virtual uint get_rom_size() { return (rom_size); }
      // XXX might want to make get_rom a friend only to cli_dump
      inline virtual Register **get_rom() { return (rom); }

      void dump();

    protected:
      Register **rom;          //  The data area.

      uint rom_size;
      int	rom_data_size;	   // width of data in bytes
      uint xfr_addr;  // latched adr from I2C.
      uint write_page_off;	// offset into current write page
      uint write_page_size; // max number of writes in one block
      uint bit_count;  // Current bit number for either Tx or Rx
      uint m_command;  // Most recent command received from I2C host
      uint m_chipselect; // Chip select bits, A0 = bit 1, A1 = bit 2, A2 = bit 3
      uint m_CSmask;    // Which chip select bits in command are active
      uint m_BSmask;    // Which block select bits are active in command
      uint m_BSshift;   // right shift for block select bits
      uint m_addr_bytes; // number of address bytes in write command
      uint m_addr_cnt;  // # 0f address bytes yet to get
      bool	m_write_protect;		    // chip is write protected
      bool ee_busy;            // true if a write is in progress.
      bool nxtbit;

      enum {
         RX_EE_ADDR = 1,
         RX_EE_DATA,
         TX_EE_DATA
      } io_state;

    private:
      // Is this even used?
      virtual void change_rom(uint offset, uint val);
};




#endif /* I2C_EE_H */

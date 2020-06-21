/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2004 Rob Pearce
                 2006,2015   Roy R Rankin

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

#include <assert.h>

#include <iostream>
#include <iomanip>
using namespace std;

//#include <glib.h>

#include "config.h"

//#include "trace.h"
#include "pic-processor.h"
#include "stimuli.h"
#include "i2c-ee.h"
#include "registers.h"

//#define DEBUG
#if defined(DEBUG)
#include "config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

// I2C EEPROM Peripheral
//
//  This object emulates the I2C EEPROM peripheral on the 12CE51x
//
//  It's main purpose is to provide a means by which the port pins
//  may communicate.
//


//--------------------------------------------------------------
//
//

PromAddress::PromAddress(I2C_EE *eeprom, const char *_name, const char * desc)
           : Value(_name,desc)
{
     m_eeprom = eeprom;
}
void PromAddress::get(char *buffer, int buf_size)
{
        snprintf(buffer, buf_size, "%p", m_eeprom);
}

class I2C_SLAVE_SDA : public IO_open_collector
{
    public:
        i2c_slave *pEE;

        I2C_SLAVE_SDA(i2c_slave *_pEE, const char *_name) :
            IO_open_collector(_name), pEE(_pEE)
        {
            bDrivingState = true;
            bDrivenState = true;

                // Make the pin an output.
            update_direction( IO_bi_directional::DIR_OUTPUT, true );
        }

        void setDrivenState(bool new_dstate)
        {
            bool diff = new_dstate ^ bDrivenState;

            Dprintf(("i2c_slave sda setDrivenState %d\n", new_dstate));
            if( pEE && diff ) 
            {
              bDrivenState = new_dstate;
              pEE->new_sda_edge(new_dstate);
            }
        }
        void setDrivingState(bool new_state)
        {
            bDrivingState = new_state;
            bDrivenState = new_state;

            if (!new_state) update_direction( IO_bi_directional::DIR_OUTPUT,true );
            else            update_direction( IO_bi_directional::DIR_INPUT,true );

            if(snode) snode->update();
        }
};


class I2C_SLAVE_SCL : public IO_open_collector
{
    public:
        i2c_slave *pEE;

        I2C_SLAVE_SCL(i2c_slave *_pEE, const char *_name) :
            IO_open_collector(_name), pEE(_pEE)
        {
            bDrivingState = true;
            bDrivenState  = true;

                // Make the pin an output.
            update_direction(IO_bi_directional::DIR_INPUT,true);
        }

      void setDrivenState(bool new_state)
      {
        bool diff = new_state ^ bDrivenState;

        Dprintf(("i2c_slave scl setDrivenState %d\n", new_state));
        if( pEE && diff ) 
        {
          bDrivenState = new_state;
          pEE->new_scl_edge(bDrivenState);
        }
      }
        void setDrivingState(bool new_state)
        {
            bDrivingState = new_state;
            bDrivenState = new_state;

            if(snode) snode->update();
        }
};

i2c_slave::i2c_slave()
{
    scl = new I2C_SLAVE_SCL(this, "SCL");
    sda = new I2C_SLAVE_SDA(this, "SDA");

    bus_state = IDLE;
    bit_count = 0;
    xfr_data = 0;
}

i2c_slave::~i2c_slave()
{
    if (sda) delete sda;
    if (scl) delete scl;
}
void i2c_slave::new_scl_edge(bool level)
{
    scl_high = level;
    get_cycles().set_break(get_cycles().get() + 1, this);
    if (!level)        //SCL goes low
    {
        if (bus_state == RX_DATA && bit_count == 0)
        {
            sda->setDrivingState (true);  // Master drives bus
        }
    }
}

void i2c_slave::callback()
{
    if (scl_high)        // read data from master
    {
        switch ( bus_state )
        {
            case RX_I2C_ADD :        // Read address, send ACK to master if us
                if ( shift_read_bit ( sda->getDrivenState() ) )
                {
                    if (match_address())
                    {
                        bus_state = ACK_I2C_ADD;
                        r_w = xfr_data & 1;
                    }
                    else
                    {
                        // not for us
                        bus_state = IDLE;
                    }
                }
                break;

            case RX_DATA :        // read data from master, send ACK when complete
                if ( shift_read_bit ( sda->getDrivenState() ) )
                {
                    //start_write();
                    put_data(xfr_data);
                    bus_state = ACK_RX;
                }
                break;

            case ACK_RD :        // read ACK/NACK from master
                if ( sda->getDrivenState() == false ) // ACK
                {
                    // The master has asserted ACK, so we send another byte
                    bus_state = TX_DATA;
                    bit_count = 8;
                    xfr_data = get_data();
                }
                else                                        // NACK
                {
                    bus_state = IDLE;   // Terminate writes to master
                }
                break;

            case ACK_WR :        // slave sent ACK/NACK
                if (r_w)
                {
                    // master is reading, we transmit
                    bus_state = TX_DATA;
                    bit_count = 8;
                    xfr_data = get_data();
                }
                else
                {
                    // master is writing, we read
                    bus_state = RX_DATA;
                    bit_count = 0;
                    xfr_data = 0;
                }
                break;

            case ACK_RX :        // ACK being read by master
                bus_state = RX_DATA;
                bit_count = 0;
                xfr_data = 0;
                break;

        default:
                break;
                
        }
    }
    else        // SCL low, put data on bus for master (if required)
    {
        switch ( bus_state )
        {
            case ACK_I2C_ADD :                // after address ACK start to send data
                sda->setDrivingState ( false );  // send ACK
                bus_state = ACK_WR;
                // Check the R/W bit of the address byte

                if ( xfr_data & 0x01 ) slave_transmit(true);
                else                   slave_transmit(false);
                break;

            case TX_DATA :        // send data to master
                if ( bit_count == 0 )
                {
                    sda->setDrivingState ( true );     // Release the bus
                    bus_state = ACK_RD;
                }
                else sda->setDrivingState ( shift_write_bit() );

                break;

            case ACK_RX :        // Send ACK read data
                sda->setDrivingState (false);
                break;

            default:
                break;
        }
    }
}

void i2c_slave::new_sda_edge(bool direction)
{
 //     Vprintf(("i2c_slave::new_sda_edge direction:%d\n", direction));
      if (scl->getDrivenState())        // SCL high
      {
        if ( direction )        // SDA high
        {
            /* stop bit */
            if ( bus_state == WRPEND )
            {
                bus_state = IDLE;   // Should be busy
            }
            else bus_state = IDLE;
        }
        else
        {
            /* start bit */
            if (bus_state == IDLE)
                bus_state = RX_I2C_ADD;
            else
                bus_state = START;
            bit_count = 0;
            xfr_data = 0;
        }
     }
}

bool i2c_slave::shift_read_bit ( bool x )
{
    xfr_data = ( xfr_data << 1 ) | ( x != 0 );
    bit_count++;
    if ( bit_count == 8 )
        return true;
    else
        return false;
}

bool i2c_slave::shift_write_bit ()
{
    bool bit;

    bit_count--;
    bit = ( xfr_data >> bit_count ) & 1;
    Dprintf(("I2c_slave : send bit %u = %c\n", bit_count,
      bit ? '1' : '0'));

    return bit;
}

bool i2c_slave::match_address()
{
        if((xfr_data & 0xfe) == i2c_slave_address)
        {
            r_w = xfr_data & 1;
            return true;
        }
        return false;
}

const char* i2c_slave::state_name()
{
  switch (bus_state) {
  case IDLE:
    return "IDLE";
    break;
  case START:
    return "START";
    break;
  case RX_I2C_ADD:
    return "RX_I2C_ADD";
    break;
  case ACK_I2C_ADD:
    return "ACK_I2C_ADD";
    break;
  case RX_DATA:
    return "RX_DATA";
    break;
  case ACK_WR:
    return "ACK_WR";
    break;
  case ACK_RX:
    return "ACK_RX";
    break;
  case WRPEND:
    return "WRPEND";
    break;
  case ACK_RD:
    return "ACK_RD";
    break;
  case TX_DATA:
    return "TX_DATA";
    break;
  }
  return "UNKNOWN";
}
//----------------------------------------------------------
//
// I2C EE PROM
//
// There are many conditions that need to be verified against a real part:
//    1) what happens if
//       > the simulator
//    2) what happens if a RD is initiated while data is being written?
//       > the simulator ignores the read
//    3) what happens if
//       > the simulator

I2C_EE::I2C_EE(Processor *pCpu, uint _rom_size, uint _write_page_size,
        uint _addr_bytes, uint _CSmask,
        uint _BSmask, uint _BSshift)
  : i2c_slave(),
    rom(0),
    rom_size(_rom_size),                // size of eeprom in bytes
    rom_data_size(1),
    xfr_addr(0),
    write_page_off(0),
    write_page_size(_write_page_size),  // Page size for writes
    bit_count(0), m_command(0),
    m_chipselect(0),
    m_CSmask(_CSmask),                  // mask for chip select in command
    m_BSmask(_BSmask),                  // mask for bank select in command
    m_BSshift(_BSshift),                // right shift bank select to bit 0
    m_addr_bytes(_addr_bytes),          // number of address bytes
     m_write_protect(false),
    ee_busy(false)
{

  // Create the rom

  rom = (Register **) new char[sizeof (Register *) * rom_size];
  assert(rom != 0);

  // Initialize the rom

  char str[100];
  for (uint i = 0; i < rom_size; i++) {
    snprintf (str,sizeof(str),"ee0x%02x", i);
    rom[i] = new Register(pCpu,str,"");
    rom[i]->address = i;
    rom[i]->value.put(0);
    rom[i]->alias_mask = 0;
  }
}

I2C_EE::~I2C_EE()
{
  for (uint i = 0; i < rom_size; i++) delete rom[i];
  delete [] rom;
}

void I2C_EE::slave_transmit(bool yes)
{
    if (yes)        // prepare to output eeprom data
    {
        io_state = TX_EE_DATA;
        xfr_addr += write_page_off;
        write_page_off = 0;
    }
    else         // getting eeprom address
    {
        io_state = RX_EE_ADDR;
        xfr_addr = (m_command & m_BSmask) >> m_BSshift;
        m_addr_cnt = m_addr_bytes;
    }
}

// data written by master device
void I2C_EE::put_data(uint data)
{
    switch(io_state)
    {
      case RX_EE_ADDR:
         // convert xfr_data to base and page offset to allow
         // sequencel writes to wrap around page
         xfr_addr = ((xfr_addr << 8) | data ) % rom_size;
         if (--m_addr_cnt == 0)
         {
              write_page_off = xfr_addr % write_page_size;
              xfr_addr -= write_page_off;

              io_state = RX_EE_DATA;
          }
          break;

       case RX_EE_DATA:
          if (! m_write_protect)
          {
             rom[xfr_addr + write_page_off]->value.put( data );
             write_page_off = (write_page_off+1) % write_page_size;
          }
          else
             cout << "I2c_EE start_write- write protect\n";
          break;

       case TX_EE_DATA:
          cout << "I2C_EE put_data in output state\n";
          break;

       default:
           cout << "I2c_EE unexpected state\n";
           break;
    }
}

uint I2C_EE::get_data()
{
    uint data = rom[xfr_addr]->get();

    xfr_addr = (xfr_addr + 1) % rom_size;
    return (data);
}


// Bit 0 is write protect, 1-3 is A0 - A2
void I2C_EE::set_chipselect(uint _cs)
{
    m_write_protect = (_cs & 1) == 1;
    m_chipselect = (_cs & m_CSmask);
}

void I2C_EE::debug()
{
  if (!scl || !sda || !rom) return;

  cout << "I2C EEPROM: current state="<<state_name()<<endl;
  cout << " t=0x"<< hex <<get_cycles().get() << endl;
  cout << "  scl drivenState="  << scl->getDrivenState()
       << " drivingState=" << scl->getDrivingState()
       << " direction=" << ((scl->get_direction()==IOPIN::DIR_INPUT) ?"IN":"OUT")
       << endl;
  cout << "  sda drivenState="  << sda->getDrivenState()
       << " drivingState=" << sda->getDrivingState()
       << " direction=" << ((sda->get_direction()==IOPIN::DIR_INPUT) ?"IN":"OUT")
       << endl;

  cout << "  bit_count:"<<bit_count
       << " ee_busy:"<<ee_busy
       << " xfr_addr:0x"<<hex<<xfr_addr
       << " xfr_data:0x"<<hex<<xfr_data
       << endl;
}

Register * I2C_EE::get_register(uint address)
{
  if ( address < rom_size ) return rom[address];
  return 0;
}

void I2C_EE::change_rom(uint offset, uint val)
{
  assert(offset < rom_size);
  rom[offset]->value.put(val);
}

// write data to eeprom unles write protect is active
void I2C_EE::start_write()
{
    uint addr = xfr_addr + write_page_off;
    if (! m_write_protect) rom[addr]->put ( xfr_data );

    else cout << "I2c_EE start_write- write protect\n";
}

// allow 5 msec after last write
void I2C_EE::write_busy()
{
    uint64_t fc;

    if( ! ee_busy && ! m_write_protect )
    {
        fc = (uint64_t)(get_cycles().instruction_cps() * 0.005);
        get_cycles().set_break(get_cycles().get() + fc, this);
        ee_busy = true;
    }
}

void I2C_EE::write_is_complete()
{
}

void I2C_EE::callback_print()
{
  cout << "Internal I2C-EEPROM\n";
}

bool I2C_EE::match_address()
{
  if( (xfr_data & 0xf0) == 0xa0 && ((xfr_data & m_CSmask) == m_chipselect) )
  {
    m_command = xfr_data;
    return true;
  }
  return false;
}

void I2C_EE::reset(RESET_TYPE by)
{
  switch(by)
    {
        case POR_RESET:
            bus_state = IDLE;
            ee_busy = false;
            break;
        default:
          break;
    }
}

void I2C_EE::attach ( Stimulus_Node* _scl, Stimulus_Node* _sda )
{
  _scl->attach_stimulus ( scl );
  _sda->attach_stimulus ( sda );
}


void I2C_EE::dump()
{
    uint i, j, reg_num,v;

    cout << "     " << hex;

    // Column labels
    for( i = 0; i < 16; i++) cout << setw(2) << setfill('0') <<  i << ' ';

    cout << '\n';

    for (i = 0; i < rom_size/16; i++)
    {
        cout << setw(2) << setfill('0') <<  i << ":  ";

        for (j = 0; j < 16; j++)
        {
            reg_num = i * 16 + j;
            if(reg_num < rom_size)
            {
              v = rom[reg_num]->get_value();
              cout << setw(2) << setfill('0') <<  v << ' ';
            }
            else cout << "-- ";
        }
        cout << "   ";

        for (j = 0; j < 16; j++)
        {
            reg_num = i * 16 + j;
            if(reg_num < rom_size)
            {
              v = rom[reg_num]->get_value();
              if( (v >= ' ') && (v <= 'z')) cout.put(v);
              else                          cout.put('.');
            }
        }
        cout << '\n';
    }
}

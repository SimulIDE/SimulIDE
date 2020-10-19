/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "avrprocessor.h"
#include "simulator.h"
#include "mcucomponent.h"
#include "utils.h"

// simavr includes
#include "sim_elf.h"
#include "sim_hex.h"
#include "sim_core.h"
#include "sim_gdb.h"
#include "avr_uart.h"
#include "avr_eeprom.h"

//AvrProcessor* AvrProcessor::m_pSelf = 0l;

extern "C"
int elf_read_firmware_ext(const char * file, elf_firmware_t * firmware);

AvrProcessor::AvrProcessor( QObject* parent ) 
            : BaseProcessor( parent )
{
    m_pSelf = this;
    m_avrProcessor = 0l;
    m_initGdb = false;
    setSteps( 16 );
}
AvrProcessor::~AvrProcessor() {}

void AvrProcessor::terminate()
{
    BaseProcessor::terminate();
    if( m_avrProcessor )
    {
        avr_deinit_gdb( m_avrProcessor );
        avr_terminate( m_avrProcessor );
    }
    m_avrProcessor = 0l;
}

bool AvrProcessor::loadFirmware( QString fileN )
{
    if( fileN == "" ) return false;
    QFileInfo fileInfo(fileN);

    if( fileInfo.completeSuffix().isEmpty() ) fileN.append(".hex");

    if( !QFile::exists(fileN) )     // File not found
    {
        QMessageBox::warning( 0, tr("File Not Found"),
                                 tr("The file \"%1\" was not found.").arg(fileN) );
        return false;
    }

    char name[20] = "";
    strncpy( name, m_device.toLatin1(), sizeof(name)-1 );
    *(name + sizeof(name) -1) = 0;

    char filename[1000]="";
    strncpy( filename, fileN.toLatin1(), sizeof(filename)-1 );
    *(filename + sizeof(filename) -1) = 0;

    elf_firmware_t f = {{0}};

    if( fileN.endsWith("hex") )
    {
        ihex_chunk_p chunk = NULL;
        int cnt = read_ihex_chunks( filename, &chunk );

        if( cnt <= 0 )
        {
            QMessageBox::warning(0,tr("Error:"), tr(" Unable to load IHEX file %1\n").arg(fileN) );
            return false;
        }

        int lastFChunk = 0;

        for( int ci=0; ci<cnt; ci++ )
        {
            if( chunk[ci].baseaddr < (1*1024*1024) ) lastFChunk = ci;
        }
        f.flashbase = chunk[ 0 ].baseaddr;
        f.flashsize = chunk[ lastFChunk ].baseaddr + chunk[ lastFChunk ].size;
        f.flash = (uint8_t*) malloc( f.flashsize+1 );

        for( int ci=0; ci<cnt; ci++ )
        {
            if( chunk[ci].baseaddr < (1*1024*1024) )
            {
                memcpy( f.flash + chunk[ci].baseaddr,
                        chunk[ci].data,
                        chunk[ci].size );
            }
            if( chunk[ci].baseaddr >= AVR_SEGMENT_OFFSET_EEPROM )
            {
                f.eeprom = chunk[ci].data;
                f.eesize = chunk[ci].size;
            }
        }
    }
//#ifndef _WIN32
    else if( fileN.endsWith(".elf") )
    {
        f.flashsize = 0;
        elf_read_firmware_ext( filename, &f );
        
        if( !f.flashsize )
        {
            QMessageBox::warning(0,tr("Failed to load firmware: "), tr("File %1 is not in valid ELF format\n").arg(fileN) );
            return false;
        }
    }
//#endif
    else                                    // File extension not valid
    {
        QMessageBox::warning(0,tr("Error:"), tr("%1 should be .hex or .elf\n").arg(fileN) );
        return false;
    }

    QString mmcu( f.mmcu );
    if( !mmcu.isEmpty() )
    {
        if( mmcu != m_device ) 
        {
            QMessageBox::warning(0,tr("Warning on load firmware: "), tr("Incompatible firmware: compiled for %1 and your processor is %2\n").arg(mmcu).arg(m_device) );
            return false;
        }
    } 
    else 
    {
        if( !strlen( name ) )
        {
            QMessageBox::warning( 0,tr("Failed to load firmware: "), tr("The processor model is not specified.\n") );
            return false;
        }
        strcpy( f.mmcu, name );
    }
    if( !m_avrProcessor )
    {
        m_avrProcessor = avr_make_mcu_by_name(f.mmcu);

        if( !m_avrProcessor )
        {
            QMessageBox::warning( 0, tr("Unkown Error:")
                                   , tr("Could not Create AVR Processor: \"%1\"").arg(f.mmcu) );
            return false;
        }
        int started = avr_init( m_avrProcessor );

        m_uartInIrq.resize( 6 );
        m_uartInIrq.fill(0);
        for( int i=0; i<6; i++ )// Uart interface
        {
            avr_irq_t* src = avr_io_getirq(m_avrProcessor, AVR_IOCTL_UART_GETIRQ('0'+i), UART_IRQ_OUTPUT);
            if( src )
            {
                qDebug() << "    UART"<<i;
                intptr_t uart = i;
                avr_irq_register_notify( src, uart_pty_out_hook, (void*)uart ); // Irq to get data coming from AVR

                // Irq to send data to AVR:
                m_uartInIrq[i] = avr_io_getirq(m_avrProcessor, AVR_IOCTL_UART_GETIRQ('0'+i), UART_IRQ_INPUT);
            }
        }
        qDebug() << "\nAvrProcessor::loadFirmware Avr Init: "<< name << (started==0);
    }
    
    /// TODO: Catch possible abort signal here, otherwise application will crash on the invalid firmware load
    /// Done: Modified simavr to not call abort(), instead it returns error code.
    if( avr_load_firmware( m_avrProcessor, &f ) != 0 )
    {
        QMessageBox::warning(0,tr("Error:"), tr("Wrong firmware!!").arg(f.mmcu) );
        return false;
    }
    if( f.flashbase ) m_avrProcessor->pc = f.flashbase;

    setEeprom( m_eeprom ); // Load EEPROM

    m_avrProcessor->frequency = 16000000;
    m_avrProcessor->cycle = 0;
    m_avrProcessor->gdb_port = 1212;
    m_symbolFile = fileN;

    if( m_initGdb )
    {
        int ok = avr_gdb_init( m_avrProcessor );
        if( ok < 0 ) qDebug() << "avr_gdb_init ERROR " << ok;
        else         qDebug() << "avr_gdb_init OK";
    }

    initialized();

    return true;
}

void AvrProcessor::reset()
{
    if( !m_loadStatus ) return;
    //qDebug() << "AvrProcessor::reset("<< eeprom();
    
    for( int i=0; i<m_avrProcessor->ramend; i++ ) m_avrProcessor->data[i] = 0;

    avr_reset( m_avrProcessor );
    m_avrProcessor->pc = 0;
    m_avrProcessor->cycle = 0;
    m_nextCycle = m_mcuStepsPT;
    m_extraCycle = 0;
}

void AvrProcessor::stepOne()
{
    //qDebug() <<"AvrProcessor::stepOne()"<<m_avrProcessor->cycle << m_nextCycle;

    if( m_avrProcessor->state < cpu_Done )
        m_avrProcessor->run( m_avrProcessor );

    while( m_avrProcessor->cycle >= m_nextCycle )
    {
        m_nextCycle += m_mcuStepsPT; //McuComponent::self()->freq(); //
        runSimuStep(); // 1 simu step = 1uS
    }
}

int AvrProcessor::pc()
{
    return m_avrProcessor->pc;
}

int AvrProcessor::getRamValue( int address )
{
    return m_avrProcessor->data[address];
}

int AvrProcessor::validate( int address )
{
    if( address < 64 ) address += 32;
    return address;
}

void AvrProcessor::uartIn( int uart, uint32_t value ) // Receive one byte on Uart
{
    if( !m_avrProcessor ) return;

    avr_irq_t* uartInIrq = m_uartInIrq[uart];
    if( uartInIrq )
    {
        BaseProcessor::uartIn( uart, value );
        avr_raise_irq( uartInIrq, value );
        //qDebug() << "AvrProcessor::uartIn: " << value;
    }
}

QVector<int> AvrProcessor::eeprom()
{
    if( m_avrProcessor )
    {
        int rom_size = m_avrProcessor->e2end + 1;
        m_eeprom.resize( rom_size );

        avr_eeprom_desc_t ee;
        ee.ee = 0;
        ee.offset = 0;
        ee.size = rom_size;
        int ok = avr_ioctl( m_avrProcessor, AVR_IOCTL_EEPROM_GET, &ee );
        //qDebug() << "avr epprom read ok =" ;//<< ok ;//<< m_eeprom;
        if( ok )
        {
            //qDebug() << "avr epprom Reading...";
            uint8_t* src = ee.ee;

            for( int i=0; i<rom_size; i++ ) m_eeprom[i] = src[i];
        }
    }
    return m_eeprom;
}

void AvrProcessor::setEeprom( QVector<int> eep )
{
    m_eeprom = eep;
    //qDebug() << "BaseProcessor::setEeprom" <<eep.size()<< eep;

    if( !m_avrProcessor ) return;

    int rom_size = m_avrProcessor->e2end+1;
    int eep_size = m_eeprom.size();

    //qDebug() << "eeprom size at Load:" << rom_size << eep_size;

    if( eep_size < rom_size ) rom_size = eep_size;

    if( rom_size )
    {
        uint8_t rep[rom_size];

        for( int i=0; i<rom_size; i++ )
        {
            uint8_t val = m_eeprom[i];
            rep[i] = val;
            //qDebug() << i << val;
        }
        avr_eeprom_desc_t ee;
        ee.offset = 0;
        ee.size = rom_size;
        ee.ee = &rep[0];
        avr_ioctl( m_avrProcessor, AVR_IOCTL_EEPROM_SET, &ee );
    }
}

bool AvrProcessor::initGdb()
{
    return m_initGdb;
}

void AvrProcessor::setInitGdb( bool init )
{
    m_initGdb = init;
}

#include "moc_avrprocessor.cpp"


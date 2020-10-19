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

#include <QtGui>

#include "picprocessor.h"
#include "mcucomponent.h"
#include "simulator.h"
#include "utils.h"

// GpSim includes
#include "uart.h"
#include "pir.h"
#include "eeprom.h"
#include "hexutils.h"

PicProcessor::PicProcessor( QObject* parent ) 
            : BaseProcessor( parent )
            , m_hexLoader()
{
    m_pSelf = this;
    m_pPicProcessor = 0l;
    m_loadStatus    = false;
}
PicProcessor::~PicProcessor(){}

void PicProcessor::terminate()
{
    BaseProcessor::terminate();
 
    m_pPicProcessor = 0l;
}

bool PicProcessor::loadFirmware( QString fileN )
{
    if( fileN == "" ) return false;
    
    m_symbolFile = fileN; 
    
    if( !QFile::exists( m_symbolFile ) )
    {
        QMessageBox::warning( 0, tr("File Not Found")
                               , tr("The file \"%1\" was not found.").arg(m_symbolFile) );
        return false;
    }
    QByteArray symbolFile = m_symbolFile.toLocal8Bit();
    QByteArray device     = m_device.toLocal8Bit();
    
    m_loadStatus = false;
    
    if( !m_pPicProcessor )
    {
        qDebug() << "Creating Proccessor:    "<<m_device<<"\n" ;

        Processor* p = ProcessorConstructor::CreatePic( device.constData() );
        m_pPicProcessor = dynamic_cast<pic_processor*>( p );

        if( !m_pPicProcessor )
        {
            QMessageBox::warning( 0, tr("Unkown Error:")
                                   , tr("Could not Create Pic Processor: \"%1\"").arg(m_device) );
            return false;
        }
    }
    qDebug() << "Loading HexFile:\n"<<m_symbolFile<<"\n" ;
    
    FILE* pFile  = fopen( symbolFile.constData(), "r" );
    int load = m_hexLoader.readihex16( m_pPicProcessor, pFile );
    if( load == IntelHexProgramFileType::SUCCESS ) m_loadStatus = true;
    
    if( !m_loadStatus )
    {
        QMessageBox::warning( 0, tr("Unkown Error:")
                               , tr("Could not Load: \"%1\"").arg(m_symbolFile) );
        return false;
    }
    //m_pPicProcessor->get_Vdd();
    m_pPicProcessor->set_Vdd( 5 );
    setEeprom( m_eeprom ); // Load EEPROM

    //m_pPicProcessor->set_frequency( (double)McuComponent::self()->freq()*1e6 );
    qDebug() <<"\nProcessor Ready:\n    Device    ="<<m_pPicProcessor->name().c_str();
    qDebug() << "    Freq. MHz =" <<  McuComponent::self()->freq();
    qDebug() << "    Int. OSC  =" << (m_pPicProcessor->get_int_osc() ? "true" : "false");
    qDebug() << "    Use PLLx4 =" << (m_pPicProcessor->get_pplx4_osc() ? "true" : "false");

    int address = getRegAddress( "OSCCAL" );
    if( address > 0 ) // Initialize Program Memory at 0x3FF for OSCCAL
    {
        qDebug() << "    OSCCAL    = true";
        m_pPicProcessor->init_program_memory_at_index(0x3FF, 0x3400);
    }
    m_rcsta.resize(6);
    m_rcsta.fill(0);
    for( int i=0; i<6; i++ ) // Get UARTs
    {
        QString rc = "RCSTA";
        if( i > 0) rc += QString::number(i);
        int address = getRegAddress( rc );
        if( address > 0 )
        {
            int uart = i;
            if( i > 0 ) uart--;  // Uart 0 and 1 are the same
            qDebug() << "    UART"<<uart;
            Register* rcsta = m_pPicProcessor->rma.get_register( address );
            m_rcsta[uart] = dynamic_cast<_RCSTA*>(rcsta);
        }
    }
    initialized();
    return true;
}

void PicProcessor::reset()
{
    if( !m_loadStatus ) return;

    if( m_pPicProcessor->is_sleeping() ) m_pPicProcessor->exit_sleep();
    m_pPicProcessor->reset( POR_RESET ); // POR_RESET MCLR_RESET EXIT_RESET IO_RESET
    m_nextCycle = m_mcuStepsPT;
    m_extraCycle = 0;
}

void PicProcessor::stepOne() 
{
    m_pPicProcessor->step_cycle();
    m_nextCycle -= 1;

    while( m_nextCycle < 1 )
    {
        runSimuStep(); // 1 simu step = 1uS
        m_nextCycle += McuComponent::self()->freq()/4;
    }
}

int PicProcessor::pc() { return m_pPicProcessor->pc->get_value(); }

int PicProcessor::getRamValue( int address )
{ 
    return m_pPicProcessor->rma[address].get_value();
}

int PicProcessor::validate( int address ) { return address; }

void PicProcessor::uartIn( int uart, uint32_t value ) // Receive one byte on Uart
{
     if( !m_pPicProcessor) return;

    _RCSTA* rcsta = m_rcsta[uart];
    if( rcsta )
    {
        BaseProcessor::uartIn( uart, value );
        rcsta->queueData( value );
        //qDebug() << "PicProcessor::uartIn: " << value<<m_pPicProcessor->rma[26].get_value();
    }
}

QVector<int> PicProcessor::eeprom()
{
    //qDebug() <<"ROM size" << rom_size;

    //m_hexLoader.writeihex8( m_pPicProcessor->eeprom->rom, rom_size, eepFile, 0xF00000 );

    /*QString qrFile = "/home/user/eep.hex";
    QByteArray rFile = qrFile.toLocal8Bit();
    FILE* eepFile  = fopen( rFile.constData(), "r" );
    //int loa = m_hexLoader.readihex16( m_pPicProcessor, eepFile );
    int loa = m_hexLoader.readihex8( m_pPicProcessor->eeprom->rom, rom_size, eepFile, 0 );
    if( loa == IntelHexProgramFileType::SUCCESS ) qDebug() << "eeprom loaded";
    fclose( eepFile );

    m_eeprom.resize( rom_size );*/

    if( m_pPicProcessor )
    {
        int rom_size = m_pPicProcessor->eeprom->get_rom_size();
        m_eeprom.resize( rom_size );

        for( int i=0; i<rom_size; i++ )
        {
            m_eeprom[i] = m_pPicProcessor->eeprom->rom[i]->get_value();
        }
     }
    //qDebug() << m_eeprom;

    //m_pPicProcessor->eeprom->dump();
    return m_eeprom;
}

void PicProcessor::setEeprom( QVector<int> eep )
{
    m_eeprom = eep;

    if( !m_pPicProcessor ) return;

    int rom_size = m_pPicProcessor->eeprom->get_rom_size();
    int eep_size = m_eeprom.size();

    if( eep_size < rom_size ) rom_size = eep_size;

    for( int i=0; i<rom_size; i++ )
    {
        m_pPicProcessor->eeprom->rom[i]->put_value( m_eeprom[i] );
    }
}

#include "moc_picprocessor.cpp"


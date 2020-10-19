/***************************************************************************
 *   Copyright (C) 2019 by santiago González                               *
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

#include <math.h>

#include "avrcompbase.h"
#include "avr_twi.h"


AvrCompBase::AvrCompBase( QObject* parent, QString type, QString id )
           : McuComponent( parent, type, id )
           , m_avrI2C("avrI2C")
{
    m_avrI2C.setEnabled( false );
    m_avrI2C.setComponent( this );
    m_sda = 0l;
    m_scl = 0l;
}
AvrCompBase::~AvrCompBase() { }

/*void AvrCompBase::remove()
{

}*/

void AvrCompBase::attachPins()
{
    AvrProcessor* ap = dynamic_cast<AvrProcessor*>( m_processor );
    avr_t* cpu = ap->getCpu();

    for( int i = 0; i < m_numpins; i++ )
    {
        AVRComponentPin *pin = dynamic_cast<AVRComponentPin*>( m_pinList[i] );
        pin->attach( cpu );
    }
    cpu->vcc  = 5000;
    cpu->avcc = 5000;
    
    // ADC irq
    avr_irq_t* adcIrq = avr_io_getirq( cpu, AVR_IOCTL_ADC_GETIRQ, ADC_IRQ_OUT_TRIGGER );
    avr_irq_register_notify( adcIrq, adc_hook, this );

    if( m_sda && m_scl )
    {
        // I2C Out irq
        avr_irq_t* i2cOutIrq = avr_io_getirq( cpu, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_OUTPUT );
        avr_irq_register_notify( i2cOutIrq, i2c_out_hook, this );

        // TWEN change irq
        int twcrAddr = BaseProcessor::self()->getRegAddress( "TWCR" );
        if( twcrAddr < 0 ) qDebug()<<"AvrCompBase::attachPins: TWCR Register Not found";
        else
        {
            qDebug()<<"AvrCompBase::attachPins Found SDA SCL";
            m_avrI2C.setInput( 0, m_sda );         // Input SDA
            m_avrI2C.setClockPin( m_scl );         // Input SCL

            avr_irq_t* twenIrq = avr_iomem_getirq( cpu, twcrAddr, 0l, 2 );
            avr_irq_register_notify( twenIrq, twen_hook, this );

            m_i2cInIrq = avr_io_getirq( cpu, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_INPUT);
        }
    }
    m_attached = true;
}

void AvrCompBase::addPin( QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
{
    if( m_initialized )
    {
        updatePin( id, type, label, pos, xpos, ypos, angle );
    }
    else
    {
        //qDebug()<<pos<<id<<label;
        AVRComponentPin* newPin = new AVRComponentPin( this, id, type, label, pos, xpos, ypos, angle );
        m_pinList.append( newPin );

        QString ty = getType( type, "adc" );
        if( !ty.isEmpty() ) m_ADCpinList[ty.remove("adc").toInt()] = newPin;

        ty = getType( type, "sda" );
        if( !ty.isEmpty() ) m_sda = newPin;

        ty = getType( type, "scl" );
        if( !ty.isEmpty() ) m_scl = newPin;
    }
}

QString AvrCompBase::getType( QString type, QString t )
{
    if( type.contains( t ) )
    {
        QStringList types = type.split( "," );
        for( QString ty : types )
        {
            if( ty.startsWith( t ) ) return ty;
        }
    }
    return "";
}

void AvrCompBase::adcread( int channel )
{
    //qDebug() << "ADC Read channel:" << channel;
    AVRComponentPin* pin = m_ADCpinList.value(channel);
    if( pin ) pin->adcread();
}

void AvrCompBase::i2cOut( uint32_t value )
{
    avr_twi_msg_irq_t v;
    v.u.v = value;
    uint32_t msg = v.u.twi.msg;

//qDebug() << "AvrCompBase::i2cOut" << value<< msg;

    if( msg & TWI_COND_START )
    {
        //m_slvAddr = v.u.twi.addr;
        m_avrI2C.masterStart( 0 );
    }
    else if( msg & TWI_COND_ADDR )
    {
        m_slvAddr = v.u.twi.addr;
        m_avrI2C.masterWrite( m_slvAddr );
    }
    else if( msg & TWI_COND_WRITE )
    {
        m_avrI2C.masterWrite( v.u.twi.data );
    }
    else if( msg & TWI_COND_STOP )
    {
        m_avrI2C.masterStop();
    }
    else if( msg & TWI_COND_READ )
    {
        m_avrI2C.masterRead();
    }
    else
        qDebug() << "AvrCompBase::i2cOut UNKNOWN ACTION";
}

void AvrCompBase::inStateChanged( int value )
{
    if( value < 128 ) return;

    if( value & 256 ) // ACK received
    {
        uint32_t irqMsg = avr_twi_irq_msg( TWI_COND_ACK, m_slvAddr, value & 1 );
        avr_raise_irq( m_i2cInIrq, irqMsg );
    }
    else if( value == 128 ) // Start Condition sent
    {
        uint32_t irqMsg = avr_twi_irq_msg( TWI_COND_START, m_slvAddr, 0 );
        avr_raise_irq( m_i2cInIrq, irqMsg );
    }
    else if( value == 130 ) // Received a byte
    {
        uint32_t irqMsg = avr_twi_irq_msg( TWI_COND_READ, m_slvAddr, m_avrI2C.byteReceived() );
        avr_raise_irq( m_i2cInIrq, irqMsg );
    }
    else if( value == 132 ) // Stop Condition sent
    {
        uint32_t irqMsg = avr_twi_irq_msg( TWI_COND_STOP, m_slvAddr, 0 );
        avr_raise_irq( m_i2cInIrq, irqMsg );
    }
}

void AvrCompBase::twenChanged( uint32_t value )
{
    if( !(m_sda && m_scl) ) return;

    qDebug() << "AvrCompBase::twenChanged Enable:" << value;

    if( value )
    {
        m_avrI2C.setEnabled( true );
        m_avrI2C.setMaster( true );
        m_sda->enableIO( false );
        m_scl->enableIO( false );

        double i2cFreq = 4e5;

        int twbr = BaseProcessor::self()->getRamValue( "TWBR" );
        int twsr = BaseProcessor::self()->getRamValue( "TWSR" );

        if( (twbr<0) || (twsr<0) ) qDebug() << "AvrCompBase::twenChanged: TWBR or TWSR not found";
        else                       // Calculate Prescaler and Frequency
        {
            int pr = pow( 4, (twsr & 3) );
            double dpr = 16+2*twbr*pr;
            i2cFreq = this->freq()*1e6/dpr;
        }
        m_avrI2C.setFreq( i2cFreq );

        qDebug() << "AvrCompBase::twenChanged i2cFreq:" << i2cFreq;
    }
    else
    {
        m_avrI2C.setEnabled( false );
        m_avrI2C.setMaster( false );
        m_sda->enableIO( true );
        m_scl->enableIO( true );
    }
}

bool AvrCompBase::initGdb()
{
    AvrProcessor* ap = dynamic_cast<AvrProcessor*>( m_processor );
    return ap->initGdb();
}

void AvrCompBase::setInitGdb( bool init )
{
    AvrProcessor* ap = dynamic_cast<AvrProcessor*>( m_processor );
    ap->setInitGdb( init );
}

#include "moc_avrcompbase.cpp"

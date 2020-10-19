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

#include "avrcomponentpin.h"
#include "baseprocessor.h"
#include "simulator.h"

AVRComponentPin::AVRComponentPin( McuComponent* mcu, QString id, QString type, QString label, int pos, int xpos, int ypos, int angle )
               : McuComponentPin( mcu, id, type, label, pos, xpos, ypos, angle )
{
    m_channel = -1;
}
AVRComponentPin::~AVRComponentPin(){}

void AVRComponentPin::attach( avr_t*  AvrProcessor )
{
    m_avrProcessor = AvrProcessor;

    if( m_id.startsWith("P") )
    {
        m_pinType = 1;
        m_port = m_id.at(1).toLatin1();
        m_pinN = m_id.mid(2,1).toInt();
        
        // PORTX Register change irq
        QString portName = "PORT";
        portName.append( m_id.at(1) );

        int portAddr = BaseProcessor::self()->getRegAddress( portName );
        if( portAddr < 0 )
        {
            qDebug()<<"PORT Register for"<<AvrProcessor->mmcu<<"Not found\nAVRComponentPin::attach"<<portName<<m_pinN;
            return;
        }
        m_PortRegChangeIrq = avr_iomem_getirq( AvrProcessor, portAddr, &m_port, m_pinN );
        avr_irq_register_notify( m_PortRegChangeIrq, port_reg_hook, this );
        
        // By now need this for pwm to work
        m_PortChangeIrq = avr_io_getirq( AvrProcessor, AVR_IOCTL_IOPORT_GETIRQ( m_port ),  m_pinN );
        avr_irq_register_notify( m_PortChangeIrq, port_hook, this );
        
        // DDRX Register change irq
        QString ddrName = "DDR";
        ddrName.append( m_id.at(1) );
        int ddrAddr = BaseProcessor::self()->getRegAddress( ddrName );
        if( ddrAddr < 0 )
        {
            qDebug()<<"DDR Register for"<<AvrProcessor->mmcu<<"Not found\nAVRComponentPin::attach"<<ddrName<<m_pinN;
            return;
        }
        m_DdrRegChangeIrq = avr_iomem_getirq( AvrProcessor, ddrAddr, " ", m_pinN );
        avr_irq_register_notify( m_DdrRegChangeIrq, ddr_hook, this );
        
        //qDebug()  << m_port << m_pinN;
        
        m_Write_stat_irq = avr_io_getirq( AvrProcessor, AVR_IOCTL_IOPORT_GETIRQ(m_port), m_pinN );
        
        if( m_type.contains("adc") )
        {
            QStringList types = m_type.split( "," );
            for( QString ty : types )
            {
                if( ty.startsWith("adc") )
                {
                    m_channel = ty.remove( "adc" ).toInt();
                    m_Write_adc_irq = avr_io_getirq( m_avrProcessor, AVR_IOCTL_ADC_GETIRQ, m_channel );
                    break;
                }
            }
        }

#ifdef AVR_IOPORT_INTRN_PULLUP_IMP
        // If we wish to take full control over pull-ups,
        // we can turn off the simavr internall pull-ups support.
        avr_irq_t *src_imp_irq = avr_io_getirq( m_avrProcessor, AVR_IOCTL_IOPORT_GETIRQ(m_port), m_pinN + IOPORT_IRQ_PIN0_SRC_IMP);
        avr_raise_irq_float( src_imp_irq, 0, true );
        // Otherwise it is active and can "override" our handling in some situations.
        // Verify tests/pullup_disconnect/pullup_disconnect.ino with digitalWrite(1, x) lines uncommented
#endif //AVR_IOPORT_INTRN_PULLUP_IMP
    }
    else if( m_type == "reset" ) 
    {
        m_pinType = 21;
    }
    else if( m_type == "vcc"  ) 
    {
        m_pinType = 22;
    }
    else if( m_type == "avcc" ) 
    {
        m_pinType = 23;
    }
    else if( m_type == "aref" )
    {
         m_pinType = 24;
    }
    m_attached = true;
    
    resetState();
}

void AVRComponentPin::setVChanged()
{
    float volt = m_ePin[0]->getVolt();

    //qDebug() << m_id << m_type << volt;
    if( m_pinType == 1 )                                 // Is an IO Pin
    {
        if( volt  > 2.5 ) avr_raise_irq(m_Write_stat_irq, 1);
        else              avr_raise_irq(m_Write_stat_irq, 0);
    }
    else if( m_pinType == 21 ) // reset
    {
        if( volt < 3 )  BaseProcessor::self()->hardReset( true );
        else            BaseProcessor::self()->hardReset( false );
    }
    else if( m_pinType == 22 ) { m_avrProcessor->vcc  = volt*1000;}
    else if( m_pinType == 23 ) { m_avrProcessor->avcc = volt*1000;}
    else if( m_pinType == 24 ) { m_avrProcessor->aref = volt*1000;}
}

void AVRComponentPin::resetState()
{
    if( m_pinType == 1 )                         // Initialize irq flags
    {
        if( m_PortRegChangeIrq && m_DdrRegChangeIrq )
        {
            m_PortChangeIrq->flags    |= IRQ_FLAG_INIT;
            m_PortRegChangeIrq->flags |= IRQ_FLAG_INIT;
            m_DdrRegChangeIrq->flags  |= IRQ_FLAG_INIT;
        }
        else
        {
            qDebug()<< "Pin not properly initialized:" << m_port << m_pinN;
        }
        McuComponentPin::resetState();
    }
}

void AVRComponentPin::pullupNotConnected( bool up )
{
    avr_raise_irq( m_Write_stat_irq, up? 1:0 );
}

void AVRComponentPin::adcread()
{
    //qDebug() << "ADC Read channel:    pin: " << m_id <<m_ePin[0]->getVolt()*1000 ;
    avr_raise_irq( m_Write_adc_irq, m_ePin[0]->getVolt()*1000 );
}

void AVRComponentPin::enableIO( bool en )
{
    if( !(m_ePin[0]->isConnected() && m_attached) ) return;

    if( en )
    {
        if( m_isInput ) m_ePin[0]->getEnode()->addToChangedFast(this);
    }
    else
    {
        m_ePin[0]->getEnode()->remFromChangedFast(this);
    }
}

#include "moc_avrcomponentpin.cpp"

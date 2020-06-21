;
;    Copyright (C) 2013  Milan Kupcevic
;
;    You can redistribute and/or modify this file under the
;    terms of the GNU General Public License version 2, or
;    (at your option) any later version, as published by
;    the Free Software Foundation.
;
;    This file is distributed in the hope that it will be useful,
;    but WITHOUT ANY WARRANTY; without even the implied warranty of
;    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;    GNU General Public License for more details.
;
;    You should have received a copy of the GNU General Public License
;    along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;
; Based on ATtiny4/5/9/10 Datasheet 8127F-AVR-02/2013.
;

.device ATtiny4

.equ SIGNATURE_000 = 0x1e
.equ SIGNATURE_001 = 0x8f
.equ SIGNATURE_002 = 0x0a

; Port B Input Pins
.equ PINB  = 0x00
.equ PINB0 = 0
.equ PINB1 = 1
.equ PINB2 = 2
.equ PINB3 = 3

; Port B Data Direction Register
.equ DDRB  = 0x01
.equ DDRB0 = 0
.equ DDB0  = 0
.equ DDRB1 = 1
.equ DDB1  = 1
.equ DDRB2 = 2
.equ DDB2  = 2
.equ DDRB3 = 3
.equ DDB3  = 3

; Port B Data Register
.equ PORTB  = 0x02
.equ PORTB0 = 0
.equ PB0 = 0
.equ PORTB1 = 1
.equ PB1 = 1
.equ PORTB2 = 2
.equ PB2 = 2
.equ PORTB3 = 3
.equ PB3 = 3

; Port B Pull-up Enable Control Register
.equ PUEB  = 0x03
.equ PUEB0 = 0
.equ PUEB1 = 1
.equ PUEB2 = 2
.equ PUEB3 = 3

; Port Control Register
.equ PORTCR = 0x0c
.equ BBMB   = 1

; Pin Change Mask Register
.equ PCMSK  = 0x10
.equ PCINT0 = 0
.equ PCINT1 = 1
.equ PCINT2 = 2
.equ PCINT3 = 3

; Pin Change Interrupt Flag Register
.equ PCIFR = 0x11
.equ PCIF0 = 0

; Pin Change Interrupt Control Register
.equ PCICR = 0x12
.equ PCIE0 = 0

; External Interrupt Mask Register
.equ EIMSK = 0x13
.equ INT0  = 0

; External Interrupt Flag Register
.equ EIFR  = 0x14
.equ INTF0 = 0

; External Interrupt Control Register A
.equ EICRA = 0x15
.equ ISC00 = 0
.equ ISC01 = 1

; Digital Input Disable Register 0
.equ DIDR0 = 0x17
.equ AIN0D = 0
.equ AIN1D = 1

; Analog Comparator Control and Status Register
.equ ACSR  = 0x1f
.equ ACIS0 = 0
.equ ACIS1 = 1
.equ ACIC  = 2
.equ ACIE  = 3
.equ ACI   = 4
.equ ACO   = 5
.equ ACD   = 7

; Input Capture Register 0
.equ ICR0L = 0x22
.equ ICR0H = 0x23

; Output Compare Register 0 B
.equ OCR0BL = 0x24
.equ OCR0BH = 0x25

; Output Compare Register 0 A
.equ OCR0AL = 0x26
.equ OCR0AH = 0x27

; Timer/Counter0
.equ TCNT0L = 0x28
.equ TCNT0H = 0x29

; Timer/Counter Interrupt Flag Register 0
.equ TIFR0 = 0x2a
.equ TOV0  = 0
.equ OCF0A = 1
.equ OCF0B = 2
.equ ICF0  = 5

; Timer/Counter Interrupt Mask Register 0
.equ TIMSK0 = 0x2b
.equ TOIE0  = 0
.equ OCIE0A = 1
.equ OCIE0B = 2
.equ ICIE0  = 5

; Timer/Counter0 Control Register C
.equ TCCR0C = 0x2c
.equ FOC0B  = 6
.equ FOC0A  = 7

; Timer/Counter0 Control Register B
.equ TCCR0B = 0x2d
.equ CS00   = 0
.equ CS01   = 1
.equ CS02   = 2
.equ WGM02  = 3
.equ WGM03  = 4
.equ ICES0  = 6
.equ ICNC0  = 7

; Timer/Counter0 Control Register A
.equ TCCR0A = 0x2e
.equ WGM00  = 0
.equ WGM01  = 1
.equ COM0B0 = 4
.equ COM0B1 = 5
.equ COM0A0 = 6
.equ COM0A1 = 7

; General Timer/Counter Control Register
.equ GTCCR = 0x2f
.equ PSR   = 0
.equ TSM   = 7

; Watchdog Timer Control and Status Register
.equ WDTCSR = 0x31
.equ WDP0   = 0
.equ WDP1   = 1
.equ WDP2   = 2
.equ WDE    = 3
.equ WDP3   = 5
.equ WDIE   = 6
.equ WDIF   = 7

; Non-Volatile Memory Control and Status Register
.equ NVMCSR = 0x32
.equ NVMBSY = 7

; Non-Volatile Memory Command Register
.equ NVMCMD  = 0x33
.equ NVMCMD0 = 0
.equ NVMCMD1 = 1
.equ NVMCMD2 = 2
.equ NVMCMD3 = 3
.equ NVMCMD4 = 4
.equ NVMCMD5 = 5

; Vcc Level Monitoring Control and Status Register
.equ VLMCSR = 0x34
.equ VLM0   = 0
.equ VLM1   = 1
.equ VLM2   = 2
.equ VLMIE  = 6
.equ VLMF   = 7

; Power Reduction Register
.equ PRR    = 0x35
.equ PRTIM0 = 0
.equ PRADC  = 1

; Clock Prescale Register
.equ CLKPSR = 0x36
.equ CLKPS0 = 0
.equ CLKPS1 = 1
.equ CLKPS2 = 2
.equ CLKPS3 = 3

; Clock Main Settings Register
.equ CLKMSR = 0x37
.equ CLKMS0 = 0
.equ CLKMS1 = 1

; Oscillator Calibration Register
.equ OSCCAL = 0x39
.equ CAL0   = 0
.equ CAL1   = 1
.equ CAL2   = 2
.equ CAL3   = 3
.equ CAL4   = 4
.equ CAL5   = 5
.equ CAL6   = 6
.equ CAL7   = 7

; Sleep Mode Control Register
.equ SMCR = 0x3a
.equ SE   = 0
.equ SM0  = 1
.equ SM1  = 2
.equ SM2  = 3

; Reset Flag Register
.equ RSTFLR = 0x3b
.equ PORF   = 0
.equ EXTRF  = 1
.equ WDRF   = 3

; Configuration Change Protection Register
.equ CCP  = 0x3c
.equ CCP0 = 0
.equ CCP1 = 1
.equ CCP2 = 2
.equ CCP3 = 3
.equ CCP4 = 4
.equ CCP5 = 5
.equ CCP6 = 6
.equ CCP7 = 7

; Stack Pointer Register
.equ SPL = 0x3d
.equ SPH = 0x3e

; Status Register
.equ SREG   = 0x3f
.equ SREG_C = 0
.equ SREG_Z = 1
.equ SREG_N = 2
.equ SREG_V = 3
.equ SREG_S = 4
.equ SREG_H = 5
.equ SREG_T = 6
.equ SREG_I = 7

; Indirect address registers
.def XL = r26
.def XH = r27
.def YL = r28
.def YH = r29
.def ZL = r30
.def ZH = r31

; Non-Volatile Memory Lock Bits
.equ MAPPED_LOCKBITS_0 = 0x3f00
.equ NVLB1 = 0
.equ LB1 = 0
.equ NVLB2 = 1
.equ LB2 = 1

; Flash Memory
.equ PAGESIZE = 16
.equ FLASHEND = 0x00ff
.equ MAPPED_FLASH_START = 0x4000
.equ MAPPED_FLASH_SIZE  = 0x0200
.equ MAPPED_FLASH_END   = 0x41ff

; RAM Memory
.equ IOEND  = 0x003f
.equ RAMEND = 0x005f
.equ SRAM_START = 0x0040
.equ SRAM_SIZE  = 32

; Configuration
.equ MAPPED_CONFIG_0 = 0x3f40
.equ RSTDISBL = 0
.equ WDTON = 1
.equ CKOUT = 2

; Calibration
.equ MAPPED_CALIB_0 = 0x3f80

; Signature
.equ MAPPED_SIGN_0 = 0x3fc0
.equ MAPPED_SIGN_1 = 0x3fc1
.equ MAPPED_SIGN_2 = 0x3fc2

; Interrupt Vectors
.equ INT_VECTORS_SIZE = 10
.equ INT0addr = 0x0001
.equ PCI0addr = 0x0002
.equ ICP0addr = 0x0003
.equ OVF0addr = 0x0004
.equ OC0Aaddr = 0x0005
.equ OC0Baddr = 0x0006
.equ ACIaddr  = 0x0007
.equ WDTaddr  = 0x0008
.equ VLMaddr  = 0x0009


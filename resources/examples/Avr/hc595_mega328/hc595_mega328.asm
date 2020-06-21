;Program compiled by Great Cow BASIC (0.94 2015-10-27)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: MEGA328P
;Assembler header file
.INCLUDE "m328pdef.inc"

;SREG bit names (for AVR Assembler compatibility, GCBASIC uses different names)
#define I 7
#define T 6
#define H 5
#define S 4
#define V 3
#define N 2
#define Z 1
#define C 0

;********************************************************************************

;Set aside memory locations for variables
.EQU	BITOUT=256
.EQU	INDEX=257

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SysBitTest=r5
.DEF	SysCalcTempA=r22
.DEF	SysValueCopy=r21
.DEF	SysWaitTempMS=r29
.DEF	SysWaitTempMS_H=r30
.DEF	SysTemp1=r0

;********************************************************************************

;Vectors
;Interrupt vectors
	nop
	rjmp	BASPROGRAMSTART ;Reset
	nop
	reti	;INT0
	nop
	reti	;INT1
	nop
	reti	;PCINT0
	nop
	reti	;PCINT1
	nop
	reti	;PCINT2
	nop
	reti	;WDT
	nop
	reti	;TIMER2_COMPA
	nop
	reti	;TIMER2_COMPB
	nop
	reti	;TIMER2_OVF
	nop
	reti	;TIMER1_CAPT
	nop
	reti	;TIMER1_COMPA
	nop
	reti	;TIMER1_COMPB
	nop
	reti	;TIMER1_OVF
	nop
	reti	;TIMER0_COMPA
	nop
	reti	;TIMER0_COMPB
	nop
	reti	;TIMER0_OVF
	nop
	reti	;SPI_STC
	nop
	reti	;USART_RX
	nop
	reti	;USART_UDRE
	nop
	reti	;USART_TX
	nop
	reti	;ADC
	nop
	reti	;EE_READY
	nop
	reti	;ANALOG_COMP
	nop
	reti	;TWI
	nop
	reti	;SPM_READY

;********************************************************************************

;Start of program memory page 0
nop
BASPROGRAMSTART:
;Initialise stack
	ldi	SysValueCopy,high(RAMEND)
	out	SPH, SysValueCopy
	ldi	SysValueCopy,low(RAMEND)
	out	SPL, SysValueCopy
;Call initialisation routines
	rcall	INITSYS

;Start of the main program
	rcall	INIT595
SysDoLoop_S1:
	ldi	SysValueCopy,1
	sts	BITOUT,SysValueCopy
	rcall	SHIFTBIT
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	ldi	SysValueCopy,0
	sts	INDEX,SysValueCopy
SysForLoop1:
	lds	SysTemp1,INDEX
	inc	SysTemp1
	sts	INDEX,SysTemp1
	ldi	SysValueCopy,0
	sts	BITOUT,SysValueCopy
	rcall	SHIFTBIT
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	lds	SysCalcTempA,INDEX
	cpi	SysCalcTempA,7
	brlo	SysForLoop1
SysForLoopEnd1:
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

Delay_MS:
	inc	SysWaitTempMS_H
DMS_START:
	ldi	DELAYTEMP2,254
DMS_OUTER:
	ldi	DELAYTEMP,20
DMS_INNER:
	dec	DELAYTEMP
	brne	DMS_INNER
	dec	DELAYTEMP2
	brne	DMS_OUTER
	dec	SysWaitTempMS
	brne	DMS_START
	dec	SysWaitTempMS_H
	brne	DMS_START
	ret

;********************************************************************************

INIT595:
	sbi	DDRB,5
	sbi	DDRB,4
	sbi	DDRB,3
	sbi	DDRB,2
	sbi	DDRB,1
	cbi	PORTB,5
	cbi	PORTB,4
	cbi	PORTB,3
	cbi	PORTB,2
	cbi	PORTB,1
	sbi	PORTB,5
	ret

;********************************************************************************

INITSYS:
	ldi	SysValueCopy,0
	out	PORTB,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTC,SysValueCopy
	ldi	SysValueCopy,0
	out	PORTD,SysValueCopy
	ret

;********************************************************************************

SHIFTBIT:
	cbi	PORTB,3
	cbi	PORTB,4
	cbi	PORTB,1
	lds	SysBitTest,BITOUT
	sbrc	SysBitTest,0
	sbi	PORTB,1
	sbi	PORTB,4
	sbi	PORTB,3
	ret

;********************************************************************************



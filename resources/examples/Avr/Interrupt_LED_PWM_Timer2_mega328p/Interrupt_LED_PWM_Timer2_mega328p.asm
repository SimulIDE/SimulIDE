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
.EQU	LEDSPEED=256
.EQU	PWMCOUNTER=257
.EQU	SaveSREG=258
.EQU	SaveSysCalcTempA=259
.EQU	SaveSysCalcTempB=260
.EQU	SaveSysTemp1=261
.EQU	SaveSysValueCopy=262
.EQU	SysIntOffCount=263
.EQU	TMR0PRES=264
.EQU	TMR1PRES=265
.EQU	TMR2POST=266
.EQU	TMR2PRES=267
.EQU	TMRNUMBER=268

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SysCalcTempA=r22
.DEF	SysCalcTempB=r28
.DEF	SysValueCopy=r21
.DEF	SysWaitTempMS=r29
.DEF	SysWaitTempMS_H=r30
.DEF	SysWaitTempS=r31
.DEF	SysTemp1=r0
.DEF	SysTemp2=r16
.DEF	SysTemp3=r1

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
	rjmp	IntTIMER2_OVF ;TIMER2_OVF
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
;Enable interrupts
	clr	SysValueCopy
	sts	SysIntOffCount,SysValueCopy
	sei

;Start of the main program
	sbi	DDRB,5
	rcall	INITLEDCONTROL
SysDoLoop_S1:
	ldi	SysValueCopy,255
	sts	LEDSPEED,SysValueCopy
SysForLoop1:
	lds	SysTemp1,LEDSPEED
	inc	SysTemp1
	sts	LEDSPEED,SysTemp1
	ldi	SysWaitTempMS,25
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	lds	SysCalcTempA,LEDSPEED
	cpi	SysCalcTempA,100
	brlo	SysForLoop1
SysForLoopEnd1:
	ldi	SysWaitTempS,1
	rcall	Delay_S
	ldi	SysValueCopy,101
	sts	LEDSPEED,SysValueCopy
SysForLoop2:
	lds	SysTemp1,LEDSPEED
	dec	SysTemp1
	sts	LEDSPEED,SysTemp1
	ldi	SysWaitTempMS,25
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	ldi	SysCalcTempA,0
	lds	SysCalcTempB,LEDSPEED
	cp	SysCalcTempA,SysCalcTempB
	brlo	SysForLoop2
SysForLoopEnd2:
	ldi	SysWaitTempS,1
	rcall	Delay_S
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
	rjmp	BASPROGRAMEND
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

Delay_S:
DS_START:
	ldi	SysWaitTempMS,232
	ldi	SysWaitTempMS_H,3
	rcall	Delay_MS
	dec	SysWaitTempS
	brne	DS_START
	ret

;********************************************************************************

INITLEDCONTROL:
	cbi	PORTB,5
	ldi	SysValueCopy,0
	sts	LEDSPEED,SysValueCopy
	ldi	SysValueCopy,0
	sts	PWMCOUNTER,SysValueCopy
	lds	SysValueCopy,TIMSK2
	sbr	SysValueCopy,1<<TOIE2
	sts	TIMSK2,SysValueCopy
	ldi	SysValueCopy,1
	sts	TMR2PRES,SysValueCopy
	ldi	SysValueCopy,1
	sts	TMR2POST,SysValueCopy
	rcall	INITTIMER2
	ldi	SysValueCopy,2
	sts	TMRNUMBER,SysValueCopy
	rjmp	STARTTIMER

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

INITTIMER2:
	lds	SysCalcTempA,TMR2PRES
	cpi	SysCalcTempA,2
	brne	ENDIF8
	ldi	SysValueCopy,7
	sts	TMR2POST,SysValueCopy
ENDIF8:
	ret

;********************************************************************************

IntTIMER2_OVF:
	rcall	SysIntContextSave
	rcall	PWMHANDLER
	cbi	TIFR2,TOV2
	rjmp	SysIntContextRestore

;********************************************************************************

PWMHANDLER:
	lds	SysCalcTempA,PWMCOUNTER
	lds	SysCalcTempB,LEDSPEED
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE3_1
	sbi	PORTB,5
	rjmp	ENDIF3
ELSE3_1:
	cbi	PORTB,5
ENDIF3:
	lds	SysTemp1,PWMCOUNTER
	inc	SysTemp1
	sts	PWMCOUNTER,SysTemp1
	lds	SysCalcTempA,PWMCOUNTER
	cpi	SysCalcTempA,100
	brne	ENDIF4
	ldi	SysValueCopy,0
	sts	PWMCOUNTER,SysValueCopy
ENDIF4:
	ret

;********************************************************************************

STARTTIMER:
	lds	SysCalcTempA,TMRNUMBER
	tst	SysCalcTempA
	brne	ENDIF5
	ldi	SysTemp2,248
	in	SysTemp3,TCCR0B
	and	SysTemp3,SysTemp2
	mov	SysTemp1,SysTemp3
	lds	SysTemp3,TMR0PRES
	or	SysTemp3,SysTemp1
	out	TCCR0B,SysTemp3
ENDIF5:
	lds	SysCalcTempA,TMRNUMBER
	cpi	SysCalcTempA,1
	brne	ENDIF6
	ldi	SysTemp2,248
	lds	SysTemp3,TCCR1B
	and	SysTemp3,SysTemp2
	mov	SysTemp1,SysTemp3
	lds	SysTemp3,TMR1PRES
	or	SysTemp3,SysTemp1
	sts	TCCR1B,SysTemp3
ENDIF6:
	lds	SysCalcTempA,TMRNUMBER
	cpi	SysCalcTempA,2
	brne	ENDIF7
	ldi	SysTemp2,248
	lds	SysTemp3,TCCR2B
	and	SysTemp3,SysTemp2
	mov	SysTemp1,SysTemp3
	lds	SysTemp3,TMR2POST
	or	SysTemp3,SysTemp1
	sts	TCCR2B,SysTemp3
ENDIF7:
	ret

;********************************************************************************

SysIntContextRestore:
;Allow interrupt to be re-enabled
	clr	SysValueCopy
	sts	SysIntOffCount,SysValueCopy
;Restore registers
	lds	SysCalcTempA,SaveSysCalcTempA
	lds	SysCalcTempB,SaveSysCalcTempB
	lds	SysTemp1,SaveSysTemp1
;Restore SREG
	lds	SysValueCopy,SaveSREG
	out	SREG,SysValueCopy
;Restore SysValueCopy
	lds	SysValueCopy,SaveSysValueCopy
	reti

;********************************************************************************

SysIntContextSave:
;Store SysValueCopy
	sts	SaveSysValueCopy,SysValueCopy
;Store SREG
	in	SysValueCopy,SREG
	sts	SaveSREG,SysValueCopy
;Store registers
	sts	SaveSysCalcTempA,SysCalcTempA
	sts	SaveSysCalcTempB,SysCalcTempB
	sts	SaveSysTemp1,SysTemp1
;Prevent interrupt from being re-enabled
	ldi	SysValueCopy,1
	sts	SysIntOffCount,SysValueCopy
	ret

;********************************************************************************



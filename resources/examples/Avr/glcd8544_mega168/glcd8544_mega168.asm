;Program compiled by Great Cow BASIC (0.94 2015-10-27)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Chip Model: MEGA168
;Assembler header file
.INCLUDE "m168def.inc"

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
.EQU	SYSSTRINGPARAM1=256
.EQU	CC_X=296
.EQU	CHARLINE=297
.EQU	C_X=298
.EQU	C_Y=299
.EQU	D_X=300
.EQU	D_Y=301
.EQU	GLCDCHAR=302
.EQU	GLCDCOMDAT=303
.EQU	GLCDDAT=304
.EQU	GLCDINDEX=305
.EQU	GLCDP_X=306
.EQU	GLCDP_Y=307
.EQU	SYSPOINTERX=308
.EQU	StringPointer=309
.EQU	SysGLCDSTRDATHandler=310
.EQU	SysGLCDSTRDATHandler_H=311
.EQU	SysRepeatTemp1=312
.EQU	SysRepeatTemp2=313
.EQU	SysRepeatTemp3=314
.EQU	SysRepeatTemp3_H=315
.EQU	SysRepeatTemp4=316

;********************************************************************************

;Register variables
.DEF	DELAYTEMP=r25
.DEF	DELAYTEMP2=r26
.DEF	SYSCALCTEMPX=r0
.DEF	SYSSTRINGLENGTH=r25
.DEF	SysBYTETempA=r22
.DEF	SysBYTETempB=r28
.DEF	SysBYTETempX=r0
.DEF	SysBitTest=r5
.DEF	SysCalcTempA=r22
.DEF	SysCalcTempB=r28
.DEF	SysReadA=r30
.DEF	SysReadA_H=r31
.DEF	SysStringA=r26
.DEF	SysStringA_H=r27
.DEF	SysStringB=r28
.DEF	SysStringB_H=r29
.DEF	SysValueCopy=r21
.DEF	SysWaitTempMS=r29
.DEF	SysWaitTempMS_H=r30
.DEF	SysWaitTempS=r31
.DEF	SysTemp1=r1
.DEF	SysTemp1_H=r2
.DEF	SysTemp2=r16
.DEF	SysTemp3=r17

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
;Automatic pin direction setting
	sbi	DDRB,5
	sbi	DDRB,4
	sbi	DDRB,3
	sbi	DDRB,2
	sbi	DDRB,0
	sbi	DDRB,7
	sbi	DDRB,6

;Start of the main program
	cbi	PORTB,3
	cbi	PORTB,2
	rcall	INITGLCD
	ldi	SysValueCopy,3
	sts	SysRepeatTemp1,SysValueCopy
SysRepeatLoop1:
	cbi	PORTB,3
	sbi	PORTB,2
	rcall	GLCDCLEAR
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	sbi	PORTB,3
	cbi	PORTB,2
	rcall	GLCDCLEAR
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	cbi	PORTB,3
	sbi	PORTB,2
	rcall	GLCDCLEAR
	rcall	GLCDDRAWFRAME
	ldi	SysValueCopy,1
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable1<<1)
	ldi	SysReadA_H,high(StringTable1<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	sbi	PORTB,3
	cbi	PORTB,2
	rcall	GLCDCLEAR
	rcall	GLCDDRAWFRAME
	rcall	GLCDDRAWFRAME
	ldi	SysValueCopy,1
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable2<<1)
	ldi	SysReadA_H,high(StringTable2<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
	ldi	SysWaitTempMS,244
	ldi	SysWaitTempMS_H,1
	rcall	Delay_MS
	lds	SysTemp1,SysRepeatTemp1
	dec	SysTemp1
	sts	SysRepeatTemp1,SysTemp1
	breq	PC + 2
	rjmp	SysRepeatLoop1
SysRepeatLoopEnd1:
	ldi	SysWaitTempS,1
	rcall	Delay_S
	ldi	SysValueCopy,0
	sts	CC_X,SysValueCopy
	ldi	SysValueCopy,0
	sts	C_X,SysValueCopy
	ldi	SysValueCopy,0
	sts	C_Y,SysValueCopy
	ldi	SysValueCopy,0
	sts	D_X,SysValueCopy
	ldi	SysValueCopy,0
	sts	D_Y,SysValueCopy
SysDoLoop_S1:
	sbi	PORTB,0
	rcall	ANIMATE
	ldi	SysWaitTempMS,200
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	cbi	PORTB,0
	rcall	ANIMATE
	ldi	SysWaitTempMS,200
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	rjmp	SysDoLoop_S1
SysDoLoop_E1:
BASPROGRAMEND:
	sleep
	rjmp	BASPROGRAMEND

;********************************************************************************

ANIMATE:
	ldi	SysCalcTempA,78
	lds	SysCalcTempB,C_X
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE1_1
	sbi	PORTB,3
	cbi	PORTB,2
	rjmp	ENDIF1
ELSE1_1:
	cbi	PORTB,3
	sbi	PORTB,2
ENDIF1:
	lds	SysValueCopy,CC_X
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,C_Y
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable3<<1)
	ldi	SysReadA_H,high(StringTable3<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rcall	GLCDPRINT9
	lds	SysCalcTempA,D_X
	tst	SysCalcTempA
	brne	ELSE2_1
	lds	SysTemp1,C_X
	ldi	SysTemp2,6
	add	SysTemp1,SysTemp2
	sts	C_X,SysTemp1
	rjmp	ENDIF2
ELSE2_1:
	lds	SysTemp1,C_X
	ldi	SysTemp2,6
	sub	SysTemp1,SysTemp2
	sts	C_X,SysTemp1
ENDIF2:
	lds	SysCalcTempA,D_Y
	tst	SysCalcTempA
	brne	ELSE3_1
	lds	SysTemp1,C_Y
	inc	SysTemp1
	sts	C_Y,SysTemp1
	rjmp	ENDIF3
ELSE3_1:
	lds	SysTemp1,C_Y
	dec	SysTemp1
	sts	C_Y,SysTemp1
ENDIF3:
	lds	SysCalcTempA,C_Y
	cpi	SysCalcTempA,5
	brne	ENDIF4
	ldi	SysValueCopy,1
	sts	D_Y,SysValueCopy
ENDIF4:
	lds	SysCalcTempA,C_Y
	tst	SysCalcTempA
	brne	ENDIF5
	ldi	SysValueCopy,0
	sts	D_Y,SysValueCopy
ENDIF5:
	lds	SysCalcTempA,C_X
	cpi	SysCalcTempA,162
	brne	ENDIF6
	ldi	SysValueCopy,1
	sts	D_X,SysValueCopy
ENDIF6:
	lds	SysCalcTempA,C_X
	tst	SysCalcTempA
	brne	ENDIF7
	ldi	SysValueCopy,0
	sts	D_X,SysValueCopy
ENDIF7:
	ldi	SysCalcTempA,78
	lds	SysCalcTempB,C_X
	cp	SysCalcTempA,SysCalcTempB
	brsh	ELSE8_1
	lds	SysTemp1,C_X
	ldi	SysTemp2,84
	sub	SysTemp1,SysTemp2
	sts	CC_X,SysTemp1
	sbi	PORTB,3
	cbi	PORTB,2
	rjmp	ENDIF8
ELSE8_1:
	lds	SysValueCopy,C_X
	sts	CC_X,SysValueCopy
	cbi	PORTB,3
	sbi	PORTB,2
ENDIF8:
	lds	SysValueCopy,CC_X
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,C_Y
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysStringB,low(SYSSTRINGPARAM1)
	ldi	SysStringB_H,high(SYSSTRINGPARAM1)
	ldi	SysReadA,low(StringTable4<<1)
	ldi	SysReadA_H,high(StringTable4<<1)
	rcall	SysReadString
	ldi	SysValueCopy,low(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler,SysValueCopy
	ldi	SysValueCopy,high(SYSSTRINGPARAM1)
	sts	SysGLCDSTRDATHandler_H,SysValueCopy
	rjmp	GLCDPRINT9

;********************************************************************************

ASCII_TABLE_H:
	cpi	SysStringA, 241
	brlo	PC + 3
	clr	SysByteTempX
	ret
	ldi	SysReadA, low(TableASCII_TABLE_H<<1)
	ldi	SysReadA_H, high(TableASCII_TABLE_H<<1)
	add	SysReadA, SysStringA
	brcc	PC + 2
	inc	SysReadA_H
	lpm
	ret
TableASCII_TABLE_H:
	.DB	240,127,9,9,9,6,62,65,81,33,94,127,9,25,41,70,70,73,73,73,49,1,1,127,1,1,63,64
	.DB	64,64,63,31,32,64,32,31,63,64,56,64,63,99,20,8,20,99,7,8,112,8,7,97,81,73,69,67
	.DB	0,127,65,65,0,2,4,8,16,32,0,65,65,127,0,4,2,1,2,4,64,64,64,64,64,0,1,2,4,0,32,84
	.DB	84,84,120,127,72,68,68,56,56,68,68,68,32,56,68,68,72,127,56,84,84,84,24,8,126,9
	.DB	1,2,12,82,82,82,62,127,8,4,4,120,0,68,125,64,0,32,64,68,61,0,127,16,40,68,0,0
	.DB	65,127,64,0,124,4,24,4,120,124,8,4,4,120,56,68,68,68,56,124,20,20,20,8,8,20,20,24
	.DB	124,124,8,4,4,8,72,84,84,84,32,4,63,68,64,32,60,64,64,32,124,28,32,64,32,28,60,64
	.DB	48,64,60,68,40,16,40,68,12,80,80,80,60,68,100,84,76,68,0,8,54,65,0,0,0,127,0,0
	.DB	0,65,54,8,0,16,8,8,16,8,0,6,9,9,6

;********************************************************************************

ASCII_TABLE_L:
	cpi	SysStringA, 241
	brlo	PC + 3
	clr	SysByteTempX
	ret
	ldi	SysReadA, low(TableASCII_TABLE_L<<1)
	ldi	SysReadA_H, high(TableASCII_TABLE_L<<1)
	add	SysReadA, SysStringA
	brcc	PC + 2
	inc	SysReadA_H
	lpm
	ret
TableASCII_TABLE_L:
	.DB	240,0,0,0,0,0,0,0,95,0,0,0,7,0,7,0,20,127,20,127,20,36,42,127,42,18,35,19,8,100
	.DB	98,54,73,85,34,80,0,5,3,0,0,0,28,34,65,0,0,65,34,28,0,20,8,62,8,20,8,8,62,8,8,0
	.DB	80,48,0,0,8,8,8,8,8,0,96,96,0,0,32,16,8,4,2,62,81,73,69,62,0,66,127,64,0,66,97,81
	.DB	73,70,33,65,69,75,49,24,20,18,127,16,39,69,69,69,57,60,74,73,73,48,1,113,9,5,3,54
	.DB	73,73,73,54,6,73,73,41,30,0,54,54,0,0,0,86,54,0,0,8,20,34,65,0,20,20,20,20,20,0
	.DB	65,34,20,8,2,1,81,9,6,50,73,121,65,62,126,17,17,17,126,127,73,73,73,54,62,65,65,65
	.DB	34,127,65,65,34,28,127,73,73,73,65,127,9,9,9,1,62,65,73,73,122,127,8,8,8,127,0,65
	.DB	127,65,0,32,64,65,63,1,127,8,20,34,65,127,64,64,64,64,127,2,12,2,127,127,4,8,16,127
	.DB	62,65,65,65,62

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

GLCDCHARACTER:
	lds	SysCalcTempA,GLCDCHAR
	cpi	SysCalcTempA,80
	brsh	ELSE17_1
	lds	SysTemp2,GLCDCHAR
	ldi	SysTemp3,32
	sub	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	mov	SysBYTETempA,SysTemp1
	ldi	SysBYTETempB,5
	mul	SysByteTempA,SysByteTempB
	sts	GLCDCHAR,SysByteTempX
	ldi	SysValueCopy,5
	sts	SysRepeatTemp4,SysValueCopy
SysRepeatLoop4:
	lds	SysTemp1,GLCDCHAR
	inc	SysTemp1
	sts	GLCDCHAR,SysTemp1
	lds	SysStringA,GLCDCHAR
	rcall	ASCII_TABLE_L
	sts	CHARLINE,SysByteTempX
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	lds	SysValueCopy,CHARLINE
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysValueCopy,GLCDDAT
	sts	CHARLINE,SysValueCopy
	lds	SysTemp1,SysRepeatTemp4
	dec	SysTemp1
	sts	SysRepeatTemp4,SysTemp1
	brne	SysRepeatLoop4
SysRepeatLoopEnd4:
	rjmp	ENDIF17
ELSE17_1:
	lds	SysTemp2,GLCDCHAR
	ldi	SysTemp3,80
	sub	SysTemp2,SysTemp3
	mov	SysTemp1,SysTemp2
	mov	SysBYTETempA,SysTemp1
	ldi	SysBYTETempB,5
	mul	SysByteTempA,SysByteTempB
	sts	GLCDCHAR,SysByteTempX
	ldi	SysValueCopy,5
	sts	SysRepeatTemp4,SysValueCopy
SysRepeatLoop5:
	lds	SysTemp1,GLCDCHAR
	inc	SysTemp1
	sts	GLCDCHAR,SysTemp1
	lds	SysStringA,GLCDCHAR
	rcall	ASCII_TABLE_H
	sts	CHARLINE,SysByteTempX
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	lds	SysValueCopy,CHARLINE
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysValueCopy,GLCDDAT
	sts	CHARLINE,SysValueCopy
	lds	SysTemp1,SysRepeatTemp4
	dec	SysTemp1
	sts	SysRepeatTemp4,SysTemp1
	brne	SysRepeatLoop5
SysRepeatLoopEnd5:
ENDIF17:
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDDAT,SysValueCopy
	rjmp	GLCDWRITE

;********************************************************************************

GLCDCLEAR:
	ldi	SysValueCopy,247
	sts	SysRepeatTemp3,SysValueCopy
	ldi	SysValueCopy,2
	sts	SysRepeatTemp3_H,SysValueCopy
SysRepeatLoop3:
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysTemp1,SysRepeatTemp3
	lds	SysTemp1_H,SysRepeatTemp3_H
	dec	SysTemp1
	sts	SysRepeatTemp3,SysTemp1
	brne	SysRepeatLoop3
	dec	SysTemp1_H
	sts	SysRepeatTemp3_H,SysTemp1_H
	brne	SysRepeatLoop3
SysRepeatLoopEnd3:
	ret

;********************************************************************************

GLCDDRAWFRAME:
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop2:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,0
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,1
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,83
	brlo	SysForLoop2
SysForLoopEnd2:
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop3:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_X,SysValueCopy
	ldi	SysValueCopy,5
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,128
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,83
	brlo	SysForLoop3
SysForLoopEnd3:
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop4:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
	ldi	SysValueCopy,83
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,255
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,5
	brlo	SysForLoop4
SysForLoopEnd4:
	ldi	SysValueCopy,255
	sts	GLCDINDEX,SysValueCopy
SysForLoop5:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
	ldi	SysValueCopy,0
	sts	GLCDP_X,SysValueCopy
	lds	SysValueCopy,GLCDINDEX
	sts	GLCDP_Y,SysValueCopy
	rcall	GLCDGOTOXY
	ldi	SysValueCopy,1
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,255
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	lds	SysCalcTempA,GLCDINDEX
	cpi	SysCalcTempA,5
	brlo	SysForLoop5
SysForLoopEnd5:
	ret

;********************************************************************************

GLCDGOTOXY:
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysTemp2,128
	lds	SysTemp1,GLCDP_X
	or	SysTemp1,SysTemp2
	sts	GLCDDAT,SysTemp1
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysTemp2,64
	lds	SysTemp1,GLCDP_Y
	or	SysTemp1,SysTemp2
	sts	GLCDDAT,SysTemp1
	rjmp	GLCDWRITE

;********************************************************************************

;Overloaded signature: STRING:
GLCDPRINT9:
	ldi	SysValueCopy,0
	sts	GLCDINDEX,SysValueCopy
	lds	SysStringA,SysGLCDSTRDATHandler
	lds	SysStringA_H,SysGLCDSTRDATHandler_H
	ld	SysCalcTempA,X
	cpi	SysCalcTempA,1
	brlo	SysForLoopEnd1
SysForLoop1:
	lds	SysTemp1,GLCDINDEX
	inc	SysTemp1
	sts	GLCDINDEX,SysTemp1
	lds	SysTemp1,SysGLCDSTRDATHandler
	lds	SysTemp2,GLCDINDEX
	add	SysTemp1,SysTemp2
	mov	SysStringA,SysTemp1
	lds	SysTemp1,SysGLCDSTRDATHandler_H
	ldi	SysTemp2,0
	adc	SysTemp1,SysTemp2
	mov	SysStringA_H,SysTemp1
	ld	SysValueCopy,X
	sts	GLCDCHAR,SysValueCopy
	rcall	GLCDCHARACTER
	lds	SysTemp1,SysGLCDSTRDATHandler
	lds	SysTemp2,GLCDINDEX
	add	SysTemp1,SysTemp2
	mov	SysStringA,SysTemp1
	lds	SysTemp1,SysGLCDSTRDATHandler_H
	ldi	SysTemp2,0
	adc	SysTemp1,SysTemp2
	mov	SysStringA_H,SysTemp1
	lds	SysValueCopy,GLCDCHAR
	st	X,SysValueCopy
	lds	SysStringA,SysGLCDSTRDATHandler
	lds	SysStringA_H,SysGLCDSTRDATHandler_H
	lds	SysCalcTempA,GLCDINDEX
	ld	SysCalcTempB,X
	cp	SysCalcTempA,SysCalcTempB
	brlo	SysForLoop1
SysForLoopEnd1:
	ret

;********************************************************************************

GLCDWRITE:
	lds	SysCalcTempA,GLCDCOMDAT
	cpi	SysCalcTempA,1
	brne	ELSE9_1
	sbi	PORTB,6
	rjmp	ENDIF9
ELSE9_1:
	cbi	PORTB,6
ENDIF9:
	ldi	SysValueCopy,8
	sts	SysRepeatTemp2,SysValueCopy
SysRepeatLoop2:
	cbi	PORTB,4
	lds	SysBitTest,GLCDDAT
	sbrs	SysBitTest,7
	rjmp	ELSE10_1
	sbi	PORTB,5
	rjmp	ENDIF10
ELSE10_1:
	cbi	PORTB,5
ENDIF10:
	lds	SysBYTETempA,GLCDDAT
	rol	SysBYTETempA
	sts	GLCDDAT,SysBYTETempA
	sbi	PORTB,4
	lds	SysTemp1,SysRepeatTemp2
	dec	SysTemp1
	sts	SysRepeatTemp2,SysTemp1
	brne	SysRepeatLoop2
SysRepeatLoopEnd2:
	ret

;********************************************************************************

INITGLCD:
	cbi	PORTB,7
	ldi	SysWaitTempMS,100
	ldi	SysWaitTempMS_H,0
	rcall	Delay_MS
	sbi	PORTB,7
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,33
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,191
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,4
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,20
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,12
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,32
	sts	GLCDDAT,SysValueCopy
	rcall	GLCDWRITE
	ldi	SysValueCopy,0
	sts	GLCDCOMDAT,SysValueCopy
	ldi	SysValueCopy,12
	sts	GLCDDAT,SysValueCopy
	rjmp	GLCDWRITE

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

SYSREADSTRING:
	lpm
	mov	SYSCALCTEMPA, SYSCALCTEMPX
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
	st	Y+, SYSCALCTEMPA
	rjmp	SYSSTRINGREADCHECK
SYSREADSTRINGPART:
	lpm
	mov	SYSCALCTEMPA, SYSCALCTEMPX
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
	add	SYSSTRINGLENGTH, SYSCALCTEMPA
SYSSTRINGREADCHECK:
	cpi	SYSCALCTEMPA, 0
	brne	SYSSTRINGREAD
	ret
SYSSTRINGREAD:
	lpm
	inc	SYSREADA
	brne	PC + 2
	inc	SYSREADA_H
	st	Y+, SYSCALCTEMPX
	dec	SYSCALCTEMPA
	brne	SYSSTRINGREAD
	ret

;********************************************************************************

SysStringTables:

StringTable1:
.DB	11,32,32,68,105,115,112,108,97,121,32,49


StringTable2:
.DB	11,32,32,68,105,115,112,108,97,121,32,50


StringTable3:
.DB	1,32


StringTable4:
.DB	1,42


;********************************************************************************



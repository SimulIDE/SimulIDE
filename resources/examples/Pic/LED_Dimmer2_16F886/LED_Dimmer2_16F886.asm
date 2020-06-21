;Program compiled by Great Cow BASIC (0.94 2015-10-27)
;Need help? See the GCBASIC forums at http://sourceforge.net/projects/gcbasic/forums,
;check the documentation or email w_cholmondeley at users dot sourceforge dot net.

;********************************************************************************

;Set up the assembler options (Chip type, clock source, other bits and pieces)
 LIST p=16F886, r=DEC
#include <P16F886.inc>
 __CONFIG _CONFIG1, _HS_OSC & _WDT_OFF & _MCLRE_OFF & _LVP_OFF

;********************************************************************************

;Set aside memory locations for variables
CNT	EQU	32
DOPWM	EQU	33
PWMCHANNEL	EQU	34
PWMDUR	EQU	35
SOFTPWMCYCLES	EQU	36
SOFTPWMDUTY	EQU	37

;********************************************************************************

;Vectors
	ORG	0
	goto	BASPROGRAMSTART
	ORG	4
	retfie

;********************************************************************************

;Start of program memory page 0
	ORG	5
BASPROGRAMSTART
;Call initialisation routines
	call	INITSYS
;Automatic pin direction setting
	banksel	TRISB
	bcf	TRISB,5
	bcf	TRISB,2

;Start of the main program
SysDoLoop_S1
	banksel	CNT
	clrf	CNT
SysForLoop1
	movlw	10
	addwf	CNT,F
	movlw	1
	movwf	PWMCHANNEL
	movf	CNT,W
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movlw	2
	movwf	PWMCHANNEL
	movf	CNT,W
	sublw	250
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movlw	250
	subwf	CNT,W
	btfss	STATUS, C
	goto	SysForLoop1
SysForLoopEnd1
	movlw	4
	movwf	CNT
SysForLoop2
	movlw	10
	subwf	CNT,F
	movlw	1
	movwf	PWMCHANNEL
	movf	CNT,W
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movlw	2
	movwf	PWMCHANNEL
	movf	CNT,W
	sublw	250
	movwf	SOFTPWMDUTY
	movlw	10
	movwf	SOFTPWMCYCLES
	call	PWMOUT
	movf	CNT,W
	sublw	10
	btfss	STATUS, C
	goto	SysForLoop2
SysForLoopEnd2
	goto	SysDoLoop_S1
SysDoLoop_E1
BASPROGRAMEND
	sleep
	goto	BASPROGRAMEND

;********************************************************************************

INITSYS
	banksel	ADCON1
	bcf	ADCON1,ADFM
	banksel	ADCON0
	bcf	ADCON0,ADON
	banksel	ANSEL
	clrf	ANSEL
	clrf	ANSELH
	banksel	CM2CON0
	bcf	CM2CON0,C2ON
	bcf	CM1CON0,C1ON
	banksel	PORTA
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC
	clrf	PORTE
	return

;********************************************************************************

PWMOUT
	clrf	PWMDUR
	movlw	1
	subwf	SOFTPWMCYCLES,W
	btfss	STATUS, C
	goto	SysForLoopEnd3
SysForLoop3
	incf	PWMDUR,F
	clrf	DOPWM
SysForLoop4
	incf	DOPWM,F
	movf	SOFTPWMDUTY,W
	subwf	DOPWM,W
	btfsc	STATUS, C
	goto	ELSE4_1
	decf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bsf	PORTB,5
	movlw	2
	subwf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bsf	PORTB,2
	goto	ENDIF4
ELSE4_1
	decf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bcf	PORTB,5
	movlw	2
	subwf	PWMCHANNEL,W
	btfsc	STATUS, Z
	bcf	PORTB,2
ENDIF4
	movlw	255
	subwf	DOPWM,W
	btfss	STATUS, C
	goto	SysForLoop4
SysForLoopEnd4
	movf	SOFTPWMCYCLES,W
	subwf	PWMDUR,W
	btfss	STATUS, C
	goto	SysForLoop3
SysForLoopEnd3
	return

;********************************************************************************

;Start of program memory page 1
	ORG	2048
;Start of program memory page 2
	ORG	4096
;Start of program memory page 3
	ORG	6144

 END

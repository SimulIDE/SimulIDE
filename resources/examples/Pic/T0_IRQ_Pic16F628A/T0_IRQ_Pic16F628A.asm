

;**********************************************************************
; Filename: TMR0_IRQ.asm
; Date:  Nov. 11, 2013 
; Author: Lewis Loflin 
; Demonstrates how to use interrupts with TMR0	                                                     
; State of LED on Ra0 is inverted by toggle every 61 interrupts
; PIC16F628A and 16 mHz crystal                                                     
;**********************************************************************




	list      p=16f628A           ; list directive to define processor
	#include <p16F628A.inc>       ; processor specific variable definitions
	errorlevel  -302              ; suppress message 302 from list file

__CONFIG   _CP_OFF & _LVP_OFF & _BOREN_OFF & _MCLRE_OFF & _WDT_OFF & _PWRTE_ON & _HS_OSC 
	
; Use  _INTOSC_OSC_NOCLKOUT  for 
; internal 4 mHz osc and no ext reset, use pin Ra5 as an input
; Use _HS_OSC for a 16 mHz ext crystal. 
; Use _XT_OSC for 4 mHz ext crystal. Page 95 in spec sheet. 


	cblock 0x20	; Begin General Purpose-Register
	count1
	count2
	count3
	CNT
	endc


	;***** VARIABLE DEFINITIONS
w_temp        EQU     0x71        ; variable used for context saving 
status_temp   EQU     0x72        ; variable used for context saving

;-----------------------------------------------
RA0       EQU     0 ;

;**********************************************************************
	ORG     0x000             ; processor reset vector
	goto    setup              ; go to beginning of program
	
	ORG     0x004             ; interrupt vector location
	movwf   w_temp            ; save off current W register contents
	movf	STATUS,w          ; move status register into W register
	movwf	status_temp       ; save off contents of STATUS register
; isr code can go here or be located as a call subroutine elsewhere

	movlw d'61' 
	; approx 61 interrupts = 1 sec. at 16 mHz crystal
	subwf CNT, w
	btfss STATUS, Z ; i = 10
	goto $+4
	call toggle ; state = !state PORTA, 0
	clrf CNT
	goto $+2
  	incf CNT
	bcf INTCON, T0IF ; clr TMR0 interrupt flag	

	movf    status_temp,w     ; retrieve copy of STATUS register
	movwf	STATUS            ; restore pre-isr STATUS register contents
	swapf   w_temp,f
	swapf   w_temp,w          ; restore pre-isr W register contents
	retfie                    ; return from interrupt

;**********************************************************************

setup ; init PIC16F628A

	movlw	0x07		; Turn comparators off and enable pins for I/O 
	movwf	CMCON
	banksel TRISA    ; BSF	STATUS,RP0 Jump to bank 1 use BANKSEL instead
	clrf    TRISA
	clrf    TRISB
	banksel INTCON ; back to bank 0
	clrf	PORTA
	clrf	PORTB

	; setp TMR0 interrupts
	banksel OPTION_REG ; Reg. 0x81 BANK1
	movlw b'10000111' ; internal clock, pos edge, prescale 256
	movwf OPTION_REG
	banksel INTCON ; 
	bsf INTCON, GIE ; enable global interrupt
	bsf INTCON, PEIE ; enable all unmasked interrupts
	bsf INTCON, T0IE ; enable TMR0 interrupt
	bcf INTCON, T0IF ; clr TMR0 interrupt flag 
	; to turn on, must be cleared after interrupt
	clrf	CNT ; RAM location 0x23

goto main

main
	; do nothing
	goto	main 
	
;************************************************************************

toggle
	btfss PORTA, RA0
	goto $+3
	bcf PORTA, RA0
	return	
	bsf PORTA, RA0
	return
	
END  ; directive 'end of program'


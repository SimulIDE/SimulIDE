;********************************************************************** 
; Filename: PIC16F628A_TMR1.asm 
; Date: 12 Nov. 2013  
; Author: Lewis Loflin   
; Uses TMR1 with ext. 32 kHz crystal to generate interrupt
; that toggles the state of RA5 LED   
; 16 mHz crystal                                                                     
;**********************************************************************


	list      p=16f628A           ; list directive to define processor
	#include <p16F628A.inc>       ; processor specific variable definitions

	errorlevel  -302              ; suppress message 302 from list file

__CONFIG   _CP_OFF & _LVP_OFF & _BOREN_OFF & _MCLRE_OFF & _WDT_OFF & _PWRTE_ON & _HS_OSC 
	
; Use  _INTOSC_OSC_NOCLKOUT  for 
; internal 4 mHz osc, no ext reset, use pin RA5 as an input
; Use _HS_OSC for a 16 mHz ext crystal. 
; Use _XT_OSC for 4 mHz ext crystal. Page 95 in spec sheet. 

;------------------------------------------------------------
	cblock 0x20	; Begin General Purpose-Register
;-------------------------- counters	
	count1
	count2
	count3
	count4
	CNT

	endc
;--------------------------
#DEFINE pwmu	PORTB,3
;--------------------------

;***** VARIABLE DEFINITIONS

w_temp        EQU     0x71        ; variable used for context saving 
status_temp   EQU     0x72        ; variable used for context saving

;-----------------------------------------------
RA5       EQU     5 ;

;**********************************************************************

	ORG     0x000             ; processor reset vector
	goto    setup              ; go to beginning of program
	

	ORG     0x004             ; interrupt vector location
	movwf   w_temp            ; save off current W register contents
	movf	STATUS,w          ; move status register into W register
	movwf	status_temp       ; save off contents of STATUS register

; isr code can go here or be located as a call subroutine elsewhere

	movlw 0x80 ; reload TMR1
	movwf TMR1H
	bcf PIR1, TMR1IF ; clr TMR1 interrupt flag
	call toggle ; flip state on RA0


	movf    status_temp,w     ; retrieve copy of STATUS register
	movwf	STATUS            ; restore pre-isr STATUS register contents
	swapf   w_temp,f
	swapf   w_temp,w          ; restore pre-isr W register contents
	retfie                    ; return from interrupt

;**********************************************************************

setup ; init PIC16F628A

	movlw	0x07	; Turn comparators off and enable pins for I/O 
	movwf	CMCON
	banksel TRISA    ; BSF	STATUS,RP0 Jump to bank 1 use BANKSEL instead
	clrf    TRISA
	clrf    TRISB

	bsf	PIE1, TMR1IE ; enable TMR1 interrupt
	banksel INTCON
	clrf PORTA
	clrf PORTB
	bsf INTCON, GIE ; enable global interrupt
	bsf INTCON, PEIE ; enable all unmasked interrupts

	bcf PIR1, TMR1IF ; clr TMR1 interrupt flag
	clrf T1CON
	bsf T1CON, TMR1ON ; Timer1 On bit
	bsf T1CON, TMR1CS ; external oscillator crystal
	bsf T1CON, 2 ; T1SYNC set to asynchronous 
	bsf T1CON, T1OSCEN ; Timer1 Oscillator Enable Control bit
	; two lines below same as 5 lines above
	; movlw b'00001111' 
	; movwf T1CON

	clrf TMR1L ; set to 0
	movlw 0x80 ; value for 1 sec. delay
	movwf TMR1H ; must be reloaded after every interrupt

	goto main

main
; remaining code goes here
	; do nothing
	goto	main 


toggle
	btfss PORTA, RA5
	goto $+3
	bcf PORTA, RA5
	return	
	bsf PORTA, RA5
	return

	END  ; directive 'end of program'



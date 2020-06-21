/***************************************************************************
 *   Copyright (C) 2018 by hovercraft:                                     *
 *		https://github.com/hovercraft-github                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <pic16f689.h>
#include <stdint.h>

/***************************************************************************
*									   *
* This source code is optimized for use with SDCC compiler and piklab IDE  *
*									   *
****************************************************************************/

/* ----------------------------------------------------------------------- */
/* Configuration bits: adapt to your setup and needs */
typedef unsigned int word;
word __at 0x2007 CONFIG = _INTRC_OSC_NOCLKOUT & _WDT_ON & _PWRTE_ON & _MCLRE_OFF & _CP_OFF & _CPD_OFF & _BOR_OFF & _IESO_ON & _FCMEN_ON;

//#define GPSIM

#define COL_Msd RA1
#define COL_Lsd RA5
#define COL_ON 1
#define COL_OFF 0
#define COL_IX_Msd 0
#define COL_IX_Lsd 1

/*
#define SEGM_A RC0
#define SEGM_B RC1
#define SEGM_C RC2
#define SEGM_D RC3
#define SEGM_E RC4
#define SEGM_F RC5
#define SEGM_G RC6
#define SEGM_ON 1
#define SEGM_OFF 0
*/

#define SEGMENTS_OFF 0
#define SEGMENTS_0 0x3F
#define SEGMENTS_1 0x06
#define SEGMENTS_2 0x5B
#define SEGMENTS_3 0x4F
#define SEGMENTS_4 0x66
#define SEGMENTS_5 0x6D
#define SEGMENTS_6 0x7D
#define SEGMENTS_7 0x07
#define SEGMENTS_8 0x7F
#define SEGMENTS_9 0x6F
#define SEGMENTS_E 0x79

#define LOAD1 RB4
#define LOAD2 RB5
#define LOAD3 RB6
#define LOAD_ON 1
#define LOAD_OFF 0

#define BUZZER RA0
#define BUZZER_ON 1
#define BUZZER_OFF 0

#define KEY_TOGGLE_MASK 2 // RC1
#define KEY_UP_MASK 0x10 // RC4
#define KEY_DOWN_MASK  0x20 // RC5
#define KEY_OK_MASK 0x40 // RC6
#define KEYS_MASK (KEY_TOGGLE_MASK | KEY_UP_MASK | KEY_DOWN_MASK | KEY_OK_MASK)

#define EEPROM_IX_FLAG 0
#define EEPROM_IX_MINUTES 1

#define clrwdt() \
    __asm \
    CLRWDT \
    __endasm

typedef enum KeyPressedComb {
  PRESSED_NONE=0,
  PRESSED_TOGGLE=KEY_TOGGLE_MASK,
  PRESSED_UP=KEY_UP_MASK,
  PRESSED_DOWN=KEY_DOWN_MASK,
  PRESSED_CANCEL=KEY_UP_MASK | KEY_DOWN_MASK,
  PRESSED_OK=KEY_OK_MASK,
};

typedef enum OpModes {
  MODE_Idle = 0,
  MODE_Work,
  MODE_EditMsd, // edit the most significant digit
  MODE_EditLsd, // edit the least significant digit
};

volatile uint8_t clk = 0;
volatile uint8_t sec = 0;
volatile uint8_t min_left = 10;
uint8_t mode = MODE_Work;

/* Adjust to your clock frequency (in Hz). */
/* Instructions per millisecond. */
#define INSNS_PER_MS    ((4U*1000U/4000U*1000U))
/* Delay loop is about 10 cycles per iteration. */
#define LOOPS_PER_MS    (INSNS_PER_MS / 10U)
void delay_ms(uint16_t ms) {
    uint16_t u;
    while (ms--) {
        /* Inner loop takes about 10 cycles per iteration + 4 cycles setup. */
        for (u = 0; u < LOOPS_PER_MS; u++) {
            /* Prevent this loop from being optimized away. */
            __asm nop __endasm;
        }
    }
}

uint8_t ee_write_byte(uint8_t address, uint8_t *_data){
  uint8_t int_enabled = GIE;
  EEDATA = *_data;
  EEADR = address;
  // start write sequence as described in datasheet, page 91
  EEPGD = 0;
  //CFGS = 0;
  WREN = 1; // enable writes to data EEPROM
  GIE = 0;  // disable interrupts
  EECON2 = 0x55;
  EECON2 = 0x0AA;
  WR = 1;   // start writing
  while (WR) {
      __asm nop __endasm;}
  if (WRERR) {
      return 0;
  }
  WREN = 0;
  GIE = int_enabled;  // restore the interrupts enable state
  return 1;
}

void ee_read_byte(uint8_t address, uint8_t *_data){
  EEADR = address;
  //CFGS = 0;
  EEPGD = 0;
  RD = 1;
  *_data = EEDATA;
}

void store_minutes(void) {
  uint8_t flag;
  flag = 'T';
  ee_write_byte(EEPROM_IX_FLAG, &flag);
  ee_write_byte(EEPROM_IX_MINUTES, &min_left);
}

void retrieve_minutes(void) {
  uint8_t flag;
  ee_read_byte(EEPROM_IX_FLAG, &flag);
  if (flag != 'T') {
    min_left = 10;
    store_minutes();
  } else {
    ee_read_byte(EEPROM_IX_MINUTES, &min_left);
  }
}

  uint8_t scanKeys(void) {
  uint8_t keys_state;
  // Switch key pins to input mode
  TRISC |= KEYS_MASK;
  delay_ms(1);
  keys_state = PORTC;
  TRISC &= ~KEYS_MASK;
  PORTC = SEGMENTS_OFF;
  keys_state = (~keys_state) & KEYS_MASK;
  // Switch key pins to output mode
  if ((keys_state & KEY_UP_MASK) && (keys_state & KEY_DOWN_MASK))
    return PRESSED_OK;
  switch (keys_state) {
    case PRESSED_TOGGLE:
      return PRESSED_TOGGLE;
      break;
    case PRESSED_CANCEL:
      return PRESSED_CANCEL;
      break;
    case PRESSED_OK:
      return PRESSED_OK;
      break;
    case PRESSED_UP:
      return PRESSED_UP;
      break;
    case PRESSED_DOWN:
      return PRESSED_DOWN;
      break;
    default:
      return PRESSED_NONE;
      break;
  }
}

uint8_t bin2decimal(uint8_t n, uint8_t pos) {
  //n  - binary number
  //d2...d0 - decimal numbers
  uint8_t d2 = 0, d1, d0;
  d1 = n / 10;
  d2 = d1 / 10;
  if (d1 > 9) {
    d1 = d1 % 10;
  }
  d0 = n - d2 * 100 - d1 * 10;
  // d2 always ignored
  if (pos == COL_IX_Lsd)
    return d0;
  return d1;
}

void segments_off(void) {
  COL_Msd = COL_OFF;
  COL_Lsd = COL_OFF;
  PORTC = SEGMENTS_OFF;
}

void segments_on(char c, uint8_t col_ix) {
  switch (c) {
    case '0':
      PORTC = SEGMENTS_0;
      break;
    case '1':
      PORTC = SEGMENTS_1;
      break;
    case '2':
      PORTC = SEGMENTS_2;
      break;
    case '3':
      PORTC = SEGMENTS_3;
      break;
    case '4':
      PORTC = SEGMENTS_4;
      break;
    case '5':
      PORTC = SEGMENTS_5;
      break;
    case '6':
      PORTC = SEGMENTS_6;
      break;
    case '7':
      PORTC = SEGMENTS_7;
      break;
    case '8':
      PORTC = SEGMENTS_8;
      break;
    case '9':
      PORTC = SEGMENTS_9;
      break;
    case 'E':
      PORTC = SEGMENTS_E;
      break;
    default:
      PORTC = SEGMENTS_OFF;
      break;
  }
  if (col_ix == COL_IX_Msd)
    COL_Msd = COL_ON;
  else
    COL_Lsd = COL_ON;
}

volatile uint8_t toggle = 0;
volatile uint8_t kbstate_prev = PRESSED_NONE;

void show_digit(void) {
  uint8_t value;
  uint8_t kbstate;
  segments_off();
  kbstate = scanKeys();
  if ((kbstate != PRESSED_NONE) && (kbstate != kbstate_prev)) {
    BUZZER = BUZZER_OFF;
    switch (mode) {
      case MODE_EditLsd:
	if (toggle < 2)
	  toggle = 2;
	switch (kbstate) {
	  case PRESSED_CANCEL:
	    mode = MODE_Idle;
	    toggle = 2;
	    break;
	  case PRESSED_OK:
	    store_minutes();
	    mode = MODE_Idle;
	    toggle = 2;
	    break;
	  case PRESSED_TOGGLE:
	    mode = MODE_EditMsd;
	    break;
	  case PRESSED_UP:
	    value = bin2decimal(min_left, COL_IX_Lsd) + 1;
	    if (value > 9)
	      value = 0;
	    min_left = bin2decimal(min_left, COL_IX_Msd)*10 + value;
	    break;
	  case PRESSED_DOWN:
	    value = bin2decimal(min_left, COL_IX_Lsd);
	    if (value > 0)
	      value -= 1;
	    else
	      value = 9;
	    min_left = bin2decimal(min_left, COL_IX_Msd)*10 + value;
	    break;
	};
	break;
      case MODE_EditMsd:
	if (toggle < 2)
	  toggle = 2;
	switch (kbstate) {
	  case PRESSED_CANCEL:
	    mode = MODE_Idle;
	    toggle = 2;
	    break;
	  case PRESSED_OK:
	    store_minutes();
	    mode = MODE_Idle;
	    toggle = 2;
	    break;
	  case PRESSED_TOGGLE:
	    mode = MODE_EditLsd;
	    break;
	  case PRESSED_UP:
	    value = bin2decimal(min_left, COL_IX_Msd) + 1;
	    if (value > 9)
	      value = 0;
	    min_left = bin2decimal(min_left, COL_IX_Lsd) + value*10;
	    break;
	  case PRESSED_DOWN:
	    value = bin2decimal(min_left, COL_IX_Msd);
	    if (value > 0)
	      value -= 1;
	    else
	      value = 9;
	    min_left = bin2decimal(min_left, COL_IX_Lsd) + value*10;
	    break;
	};
	break;
      case MODE_Idle:
	retrieve_minutes();
	if (kbstate == PRESSED_OK) {
	    mode = MODE_Work;
	    toggle = 0;
	    LOAD1 = LOAD_ON;
	    LOAD2 = LOAD_ON;
	    LOAD3 = LOAD_ON;
	    sec = 0;
	} else if (kbstate == PRESSED_TOGGLE) {
	    mode = MODE_EditLsd;
	    toggle = 2;
	}
	break;
      case MODE_Work:
      default:
	LOAD1 = LOAD_OFF;
	LOAD2 = LOAD_OFF;
	LOAD3 = LOAD_OFF;
	toggle = 2;
	if (kbstate == PRESSED_TOGGLE) {
	  mode = MODE_EditLsd;
	}
	else
	  mode = MODE_Idle;
	break;
    };
  }
  kbstate_prev = kbstate;
  if (toggle == 0) {
    toggle = 1;
    value = bin2decimal(min_left, toggle);
    segments_on('0' + value, toggle);
  } else if (toggle == 1) {
    toggle = 0;
    value = bin2decimal(min_left, toggle);
    segments_on('0' + value, toggle);
  } else {
    toggle++;
    if (mode == MODE_EditLsd || mode == MODE_EditMsd) {
      if (toggle < 100) { // dim the edited position
	if (mode == MODE_EditLsd) {
	  if (toggle % 2 == COL_IX_Msd)
	    segments_on('0' + bin2decimal(min_left, COL_IX_Msd), COL_IX_Msd); // show only msd
	} else if (mode == MODE_EditMsd) {
	  if (toggle % 2 == COL_IX_Lsd)
	    segments_on('0' + bin2decimal(min_left, COL_IX_Lsd), COL_IX_Lsd); // show only lsd
	} else {
	  segments_on('0' + bin2decimal(min_left, toggle % 2), toggle % 2); // show both digits
	}
      } else if (toggle < 200) { // lit the edited position
	  segments_on('0' + bin2decimal(min_left, toggle % 2), toggle % 2); // show both digits
      } else {
	toggle = 2;
      }
    } else if (mode == MODE_Idle) {
      if (toggle < 100) { // dim both positions
	if (!min_left)
	  BUZZER = BUZZER_ON;
      } else if (toggle < 200) { // lit both positions
	  BUZZER = BUZZER_OFF;
	  segments_on('0' + bin2decimal(min_left, toggle % 2), toggle % 2); // show both digits
      } else {
	toggle = 2;
      }
    }
  };
}

/* interrupt service routine */
void isr() __interrupt 0 {
  clrwdt();
  TMR1H = 0xF6;
  TMR1L = 0x3C;
  clk++;
  if (clk == 200) {
    clk = 0;
    sec++;
    if (sec == 60) {
      sec = 0;
      if (mode == MODE_Work) {
	if (min_left)
	  --min_left;
	if (!min_left) {
	  LOAD1 = LOAD_OFF;
	  LOAD2 = LOAD_OFF;
	  LOAD3 = LOAD_OFF;
	  //BUZZER = BUZZER_ON;
	  toggle = 2;
	  mode = MODE_Idle;
	}
      }
    }
  }
  show_digit();
  TMR1IF = 0;
}

volatile uint16_t cnt = 0;

void main() {
  GIE = 0;
  WDTPS0 = 1;
  WDTPS1 = 1;
  WDTPS2 = 1;
  // Select 8Mhz internall oscillator with 1/2 prescaler to produce 4MHz clock.
  // This is the default.
  /*
  IRCF0 = 0;
  IRCF1 = 1;
  IRCF2 = 1;
  SCS = 1;
  //SCS = OSTS;
  */
  
  SSPEN = 0;
  INTE = 0;
  T0IE = 0;
  IOCA = 0;
  IOCB = 0;
  RABIE = 0;
  RABIF = 0;
  
  ADIE = 0;
  ANSEL = 0;
  ANSELH = 0;
  TRISA = 0x3F;
  TRISA0 = 0;	// buzzer
  TRISA1 = 0;	// column 1
  TRISA5 = 0;	// column 2

  TRISB = 0;	// All outputs
  TRISC = 0x80; // All outputs, except RC7
  segments_off();

  BUZZER = BUZZER_OFF;	// buzzer off

  T1CON = 0;
  T1CKPS0 = 1;
  T1CKPS1 = 0;
  
  PIE1 = 0;
  INTCON = 0;
  
  // setup timer1 interrupt frequency
  // 4000000/4/2/(2^16-63036) = 200Hz
  //TMR1 = 63036L;
  TMR1H = 0xF6;
  TMR1L = 0x3C;
  TMR1IE = 1;
  TMR1CS = 0;
  TMR1GE = 0;
  T1OSCEN = 0;
  //T1SYNC = 1;
  TMR1ON = 1;
  PEIE = 1;
  retrieve_minutes();
  TMR1IF = 0;
  GIE = 1;
  LOAD1 = LOAD_ON;
  LOAD2 = LOAD_ON;
  LOAD3 = LOAD_ON;
  while (1) {
    cnt++;
#ifdef GPSIM
  if (cnt > 512) {
    cnt = 0;
    isr();
  }
#endif
  };
}

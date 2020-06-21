#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <SPI.h>

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define DISPLAY_WIDTH 84
#define DISPLAY_HEIGHT 48

#define ARDUINO_PRECISION 1023.0
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
 

//Analog Pins
int channelAI = A0;      // probe

#define DELAY_POTENTIMETER //disabled it I don't have it connected
#ifdef DELAY_POTENTIMETER
int delayAI = A1;       // delay potentiometer
#endif

int height = DISPLAY_HEIGHT-1;
unsigned int delayVariable = 0;

int xCounter = 0;
int yPosition = 0;
uint8_t readings[DISPLAY_WIDTH+1];

unsigned long drawtime = 0;
unsigned long lastdraw = 0;
unsigned long frames = 0;

void setup(void) 
{
  //FASTADC: set prescale to 16
  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  
  display.begin();
  display.setContrast(60);// you might have a slightly different display so it might not be the optimal value for you
  display.clearDisplay();
}

void loop() 
{  
  #ifdef DELAY_POTENTIMETER
  delayVariable = analogRead( delayAI );
  delayVariable = delayVariable*2+1;
  #endif
  
  // Get Rissing Edge:
  while( analogRead(channelAI) > 512 );
  while( analogRead(channelAI) < 512 );
    
  //record readings
  unsigned long readTime = micros();
  unsigned long readTime2 = 0;
  unsigned long totTime = 0;
  bool positive = true;
  int  numCicles = 0;
  for(xCounter = 0; xCounter <= DISPLAY_WIDTH; xCounter++)
  {
    yPosition = analogRead(channelAI);
    
    // Detect rising edges:
    if( !positive && (yPosition > 512) ) 
    {
      numCicles++; 
      readTime2 = micros();
    }
    positive = yPosition > 512;

    // Record New Reading:
    readings[xCounter] = yPosition>>5; // 0-32
    
    // Wait 
    #ifdef DELAY_POTENTIMETER
    delayMicroseconds( delayVariable );
    #endif
  }
  totTime = (micros()-readTime)/1000; // ms
  readTime = readTime2-readTime;
  
  
  display.clearDisplay();
  display.drawLine( 0, 24, 83, 24, BLACK); // 0 line

  // Draw Readings
  int base = height-8;
  int lastreading = base-readings[0];
  int reading;
  for(xCounter = 1; xCounter <= DISPLAY_WIDTH; xCounter++)
  {
    reading = base-readings[xCounter];
    display.drawLine(xCounter-1, lastreading, xCounter, reading, BLACK);
    lastreading = reading;
  }
  // Draw Frequency
  unsigned long freq = numCicles*2000000/readTime;
  display.setCursor(1,0);
  display.print(freq);
  display.print( " Hz" );
  
  // Draw reading Time
  display.setCursor(42,0);
  display.print(totTime);
  display.print( " mS" );
  
  //Draw FPS
  drawtime = micros();
  frames=1000000/*a second*//(drawtime-lastdraw);
  lastdraw = drawtime;
  display.setCursor(1,41);
  display.print(frames);
  display.print( " FPS" );
  
  // Draw Sample rate
  unsigned long rate = 85000/totTime;
  display.setCursor(41,41);
  display.print(rate);
  display.print( " SPS" );
  
  display.display();
}

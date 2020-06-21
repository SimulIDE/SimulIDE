/*
 * Conway's "Life"
 *
 * Adapted from the Life example
 * on the Processing.org site
 *
 * Needs FrequencyTimer2 library
 */

#include <FrequencyTimer2.h>

byte col = 0;
byte leds[8][8];

// pin[xx] on led matrix connected to nn on Arduino (-1 is dummy to make array start at pos 1)
int pins[17]= {-1, 19, 18, 17, 16, 15, 14, 13, 12, 0, 1, 2, 3, 4, 5, 6, 7};

// col[xx] of leds = pin yy on led matrix
int cols[8] = {pins[1], pins[2], pins[3], pins[4], pins[5], pins[6], pins[7], pins[8]};

// row[xx] of leds = pin yy on led matrix
int rows[8] = {pins[9], pins[10], pins[11], pins[12], pins[13], pins[14], pins[15], pins[16]};

#define SIZE 8
extern byte leds[SIZE][SIZE];
byte world[SIZE][SIZE];
long density = 50;

void setup() 
{
  for( int i = 1; i <= 16; i++ ) pinMode(pins[i], OUTPUT);

  for( int i=0; i<SIZE; i++ ) 
  {
    digitalWrite(cols[i], HIGH);
    digitalWrite(rows[i], LOW);
  }

  FrequencyTimer2::disable();
  FrequencyTimer2::setPeriod(2000);
  FrequencyTimer2::setOnOverflow( updateScreen );
  
  initialize();
}

void loop() 
{
  for( int i = 0; i < SIZE; i++ ) 
  {
    for( int j = 0; j < SIZE; j++ ) 
      world[i][j] = leds[i][j];
  }

  for( int x=0; x<SIZE; x++ ) 
  {
    if( digitalRead( 8 ) ) 
    {
      initialize();
      break;
    }
    int nx = x+1;
    if( nx >= SIZE ) nx -= SIZE;
    
    int px = x-1;
    if( px < 0 )     px += SIZE;
    
    for (int y=0; y<SIZE; y++) 
    {
      int ny = y+1;
      if( ny >= SIZE ) ny -= SIZE;
    
      int py = y-1;
      if( py < 0 )     py += SIZE;
    
      int count = 
         world[nx] [y]+
         world[x]  [ny]+
         world[px] [y]+
         world[x]  [py];
    
      if( count == 2 ) leds[x][y] = 1;
      if(( count < 2 
        || count > 3) )  leds[x][y] = 0;
        
      delay(10);
    }
  }
}

void initialize()
{
  randomSeed( TCNT0 );

  for( int i = 0; i < SIZE; i++ ) 
  {
    for( int j = 0; j < SIZE; j++ ) 
    {
      if( random(100) < density ) leds[i][j] = 1;
      else                        leds[i][j] = 0;
    }
  }
  delay(500);
}

void updateScreen() 
{
  digitalWrite(cols[col], HIGH);
  col++;
  if( col == SIZE ) col = 0;
 
  for( int row=0; row<SIZE; row++ ) 
    digitalWrite(rows[row], leds[col][row]);

  digitalWrite(cols[col], LOW); 
}
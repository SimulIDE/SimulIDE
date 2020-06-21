#define trigPin 10
#define echoPin 9

int counter;
float duration;
float distance;
unsigned long time;


void setup() 
{
  Serial.begin ( 9600);
  pinMode( trigPin, OUTPUT );
  pinMode( echoPin, INPUT );
}

void loop() 
{
  digitalWrite( trigPin, LOW ); 
  delayMicroseconds( 2 );
 
  digitalWrite( trigPin, HIGH );
  delayMicroseconds( 10 );
  digitalWrite( trigPin, LOW );
  
  //duration = pulseIn( echoPin, HIGH ); 

  // Get Pulse duration with more accuracy than pulseIn()
  duration = 0;
  counter = 0;
  while(  --counter!=0  )
  {
	if( PINB & 2 ) 
	{
	  time = micros();
	  break;
	}
  }
  while( --counter!=0 )
  {
	if( (PINB & 2)==0 ) 
	{
	  duration = micros()-time;
	  break;
	}
  }

  distance = ( duration/2 ) * 0.0344;
  
  Serial.print("Distance: ");

  if     ( distance > 400 ) Serial.print("> 400");
  else if( distance < 2 )   Serial.print("< 2");
  else                      Serial.print( distance );

  Serial.println( " cm" );

  delay( 1000 );
}

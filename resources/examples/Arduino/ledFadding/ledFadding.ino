/*
 Fade
 
 This example shows how to fade an LED on all pwm pins
 using the analogWrite() function.
 
 This example code is in the public domain.
 */
int brightness[] = { 0, 40, 80, 120, 160, 200 };// how bright the LED is
int led[]        = { 3, 5, 6, 9, 10, 11 };    // the pin that the LED is attached to
int fadeAmount[] = { 5, 5, 5, 5, 5, 5 };      // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup()  { 
  // declare pwm pins to be an output:
  for( int i=0; i<6; i++ ) pinMode(led[i], OUTPUT);
} 

// the loop routine runs over and over again forever:
void loop()  { 
  
  for( int i=0; i<6; i++ )
  {
    // set the brightness of pin 9:
    analogWrite(led[i], brightness[i]);   
  
    // change the brightness for next time through the loop:
    brightness[i] += fadeAmount[i];
  
    // reverse the direction of the fading at the ends of the fade: 
    if (brightness[i] == 0 || brightness[i] == 255) {
      fadeAmount[i] = -fadeAmount[i] ; 
    }     
    // wait for 30 milliseconds to see the dimming effect    
    delay(1);        
  }    
  
}


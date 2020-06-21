//This file has been modified from: http://wiki.t-o-f.info/Arduino/ExempleMutliplexeurDentr%C3%A9esNum%C3%A9riques74HC165#toc3
// HARDWARE CONNECTIONS 
int LATCH = 11; 
int CLOCK = 10; 
int DATA = 9; 

byte a=0;

void setup() { 
        Serial.begin(57600); 
        pinMode(LATCH, OUTPUT); 
        pinMode(CLOCK, OUTPUT); 
        pinMode(DATA, INPUT); 
        digitalWrite(CLOCK,LOW); 
        digitalWrite(LATCH,LOW); 
} 

void loop() { 
		 digitalWrite(LATCH,HIGH); 
         byte a_temp = shiftInFixed(DATA,CLOCK); 
        digitalWrite(LATCH,LOW);
        // Envoie la valeur lue si elle change 
        if ( a_temp != a ) { 
                a = a_temp; 
                Serial.print("Value: "); 
                Serial.println(a,DEC); 
        } 
		delayMicroseconds(1000);
} 
 
byte shiftInFixed(byte dataPin, byte clockPin) { 
        byte value = 0; 
		int j= 7; 
        for (byte i = 0; i <8; ++i) { 
             value =   value |  (digitalRead(dataPin) << j);
			 j--;			
             digitalWrite(clockPin, HIGH); 
             digitalWrite(clockPin, LOW); 
		        } 
        return value; 
} 

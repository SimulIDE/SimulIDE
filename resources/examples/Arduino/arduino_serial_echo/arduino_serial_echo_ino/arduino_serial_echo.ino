
byte byteRead;

void setup() 
{
    Serial.begin(9600);
    
    Serial.write("Serial echo Test\n");
}

void loop() 
{
    if( Serial.available() ) 
    {
        byteRead = Serial.read();
    
        Serial.write(byteRead);
    }
}
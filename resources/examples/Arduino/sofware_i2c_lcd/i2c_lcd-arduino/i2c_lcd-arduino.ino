#include <Soft_LCD-I2C.h>

uint8_t address = 80;
uint8_t sda = 4;
uint8_t scl = 3;

Soft_Lcd_I2C i2cLcd( address, 16, 2, sda, scl );

void setup()
{
    pinMode( 13, OUTPUT );
    i2cLcd.init();

    i2cLcd.printstr("Hello, world!");
}

void loop()
{
    i2cLcd.scrollDisplayLeft();
    delay( 100 );
}




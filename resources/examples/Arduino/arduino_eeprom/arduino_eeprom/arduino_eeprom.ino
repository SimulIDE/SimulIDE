#include <LiquidCrystal.h>
#include <EEPROM.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

 
void setup()
{
  lcd.begin(16, 2);
  
  byte val = 0;
  
  lcd.setCursor( 0, 0 );
  lcd.print("EEP size = ");
  lcd.print(EEPROM.length());
  delay(2000);
  
  // WRITE EEPROM
  /*lcd.setCursor( 0, 0 );
  lcd.print("Writting EEPROM...\n");
  for( int i=0; i<EEPROM.length(); i++ )
  {
    lcd.setCursor(0, 1);
    EEPROM.put( i, val );
    lcd.print( i );
    lcd.print(" - ");
    lcd.print( val );
    lcd.print("\n");
    val++;
    //delay(200);
  }*/
  
  lcd.setCursor(0, 0);
  lcd.print("Reading EEPROM...\n");
  for( int i=0; i<EEPROM.length(); i++ )
  {
    lcd.setCursor(0, 1);
    EEPROM.get( i, val );
    lcd.print("AD=");
    lcd.print( i );
    lcd.print(" Val=");
    lcd.print( val );
    lcd.print("   \n");
    delay(100);
  } 
  lcd.setCursor( 0, 0 );
  lcd.print("END...\n");
} 
 
void loop()
{
  delay(1000);
}
/*
  LiquidCrystal Library - Hello World

 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the
 Hitachi HD44780 driver. There are many of them out there, and you
 can usually tell them by the 16-pin interface.

 This sketch prints "Hello World!" to the LCD
 and shows the time.

  The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)

 Library originally added 18 Apr 2008
 by David A. Mellis
 library modified 5 Jul 2009
 by Limor Fried (http://www.ladyada.net)
 example added 9 Jul 2009
 by Tom Igoe
 modified 22 Nov 2010
 by Tom Igoe
 modified 7 Nov 2016
 by Arturo Guadalupi

 This example code is in the public domain.

 http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystal.h>

struct time_t
{
  short hours;
  short minutes;
  short seconds;
  bool operator==(time_t current)
  {
    if ((current.hours == hours) && (current.minutes == minutes) && (current.seconds == seconds))
      return true;
    else
      return false;
  }
} alarm;

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
const int ledPin =  LED_BUILTIN;// the number of the LED pin
enum state {TIME, SET_ALARM, SET_ALARM_HOURS, SET_ALARM_MINUTES, SET_ALARM_SECONDS};
const int buttonMode = 6;
const int buttonSet = 7;

// Variables will change:
enum state mode = TIME;
bool alarmOn = false;
int ledState = LOW;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonMode, INPUT);
  pinMode(buttonSet, INPUT);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Time");
  alarm = {.hours = 0, .minutes = 0, .seconds = 7};
}

void printTime(struct time_t t)
{
  if (t.hours < 10)
    lcd.print("0");
  lcd.print(t.hours);
  lcd.print(":");
  if (t.minutes < 10)
    lcd.print("0");
  lcd.print(t.minutes);
  lcd.print(":");
  if (t.seconds < 10)
  {
    lcd.print("0");
  }
  lcd.print(t.seconds);
}

void loop() {
  if (digitalRead(buttonMode))
  {
    if ((mode == SET_ALARM) && (!alarmOn))
      mode = TIME;
    else
      mode = static_cast<state>((mode + 1) % 5);
    delay(100);
  }
  if (mode == TIME)
  {
    lcd.setCursor(0, 0);
    lcd.print("Time ");
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    time_t t = { .hours = static_cast<short>(millis()/1000/60/60 % 24), .minutes = static_cast<short>(millis()/1000/60 % 60), .seconds = static_cast<short>(millis()/1000 % 60) };
    printTime(t);
    if (alarmOn && (t == alarm))
      digitalWrite(ledPin, HIGH);
  }
  else if (mode == SET_ALARM)
  {
    if (digitalRead(buttonSet) == HIGH)
    {
      alarmOn = !alarmOn;
      delay(100);
    }
    lcd.setCursor(0, 0);
    lcd.print("Alarm");
    lcd.setCursor(0, 1);
    if (millis()%500 < 250)
    {
      if (alarmOn)
        lcd.print("ON      ");
      else
        lcd.print("OFF     ");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("   ");
    }
  }
  else if (mode == SET_ALARM_HOURS)
  {
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.hours = (alarm.hours + 1) % 24;
      delay(200);
    }
    lcd.setCursor(0, 1);
    if (millis()%500 < 250)
    {
      printTime(alarm);
    }
    else
    {
      lcd.print("  ");
    }
  }
  else if (mode == SET_ALARM_MINUTES)
  {
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.minutes = (alarm.minutes + 1) % 60;
      delay(200);
    }
    lcd.setCursor(0, 1);
    if (millis()%500 < 250)
    {
      printTime(alarm);
    }
    else
    {
      lcd.setCursor(3, 1);
      lcd.print("  ");
    }
  }
  else if (mode == SET_ALARM_SECONDS)
  {
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.seconds = (alarm.seconds + 1) % 60;
      delay(200);
    }
    lcd.setCursor(0, 1);
    if (millis()%500 < 250)
    {
      printTime(alarm);
    }
    else
    {
      lcd.setCursor(6, 1);
      lcd.print("  ");
    }
  }
}

/*******************************************************************************
 * Awana Car Timing
 * Author: David Trotz
 * email: david.trotz@gmail.com
 ******************************************************************************/

#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"

////////////////////////////////////////////////////////////////////////////////
// Method Prototypes
////////////////////////////////////////////////////////////////////////////////
void start_timing();
void midpoint_timing();
void end_timing();


////////////////////////////////////////////////////////////////////////////////
// Global Variables 
////////////////////////////////////////////////////////////////////////////////
RTC_DS1307 RTC;
// The following interrupt pins are different based on the Arduino being used.
// In this case we are targeting the Arduino Mega
const int BEG_BREAK_SENSOR_INTERRUPT_PIN = 19;
const int MID_BREAK_SENSOR_INTERRUPT_PIN = 20;
const int END_BREAK_SENSOR_INTERRUPT_PIN = 21;

LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);

int backLight = 13;
long iter = 0;
char dateBuffer[256];
char timeBuffer[256];

void setup()
{
  attachInterrupt(digitalPinToInterrupt(BEG_BREAK_SENSOR_INTERRUPT_PIN), start_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(MID_BREAK_SENSOR_INTERRUPT_PIN), midpoint_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(END_BREAK_SENSOR_INTERRUPT_PIN), end_timing, FALLING);
  
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  } else {
    Serial.println("RTC IS running!");
  }
}
  
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();
}

void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(testVal);
//  if(++iter % 5000 == 0) {
//    DateTime now = RTC.now(); 
//
//    
//    sprintf(dateBuffer, "D %02d-%02d-%02d", now.year(), now.month(), now.day());
//    sprintf(timeBuffer, "T %02d:%02d:%02d:%03d", now.hour(), now.minute(), now.second(), (millis() - milsOffset) % 1000);
//    lcd.clear();
//    lcd.setCursor(0,0);
//    lcd.print(dateBuffer);
//    lcd.setCursor(0,1);
//    lcd.print(timeBuffer);
//  }
}

void start_timing() {
}

void midpoint_timing() {
}

void end_timing() {
}


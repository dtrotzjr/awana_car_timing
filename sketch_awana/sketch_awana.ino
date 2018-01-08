/*******************************************************************************
 * Awana Car Timing
 * Author: David Trotz
 * email: david.trotz@gmail.com
 ******************************************************************************/
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include "RTClib.h"

////////////////////////////////////////////////////////////////////////////////
// Method Prototypes
////////////////////////////////////////////////////////////////////////////////
void start_00_timing();
void mid_01_timing();
void mid_02_timing();
void end_03_timing();


////////////////////////////////////////////////////////////////////////////////
// Global Variables 
////////////////////////////////////////////////////////////////////////////////
RTC_DS1307 RTC;
// The following interrupt pins are different based on the Arduino being used.
// In this case we are targeting the Arduino Mega
const int BREAK_SENSOR_00_INTERRUPT_PIN = 18;
const int BREAK_SENSOR_01_INTERRUPT_PIN = 19;
const int BREAK_SENSOR_02_INTERRUPT_PIN = 2;
const int BREAK_SENSOR_03_INTERRUPT_PIN = 3;

const int SD_CARD_CHIP_SELECT           = 53;

const char CAR_OWNERS[4][16] = {"ABBY", "BEN", "WILLIAM", "JAMES"}; 

// This board can be run in 8 bit or 4 bit modes. 
// The bits indicate the width of the datapipe. 
// 8-bit LCD (we want the added performance of 8 bits)
LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2);

int backLight   = 13;
long iter       = 0;
char dateBuffer[256];
char timeBuffer[256];

volatile signed long start_00_millis    = -1;
volatile signed long mid_01_millis      = -1;
volatile signed long mid_02_millis      = -1;
volatile signed long end_03_millis      = -1;

void setup()
{
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_00_INTERRUPT_PIN), start_00_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_01_INTERRUPT_PIN), mid_01_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_02_INTERRUPT_PIN), mid_02_timing, FALLING);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_03_INTERRUPT_PIN), end_03_timing, FALLING);
  
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
  
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); // turn backlight on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();

  if (!SD.begin(SD_CARD_CHIP_SELECT)) {
    lcd.setCursor(0,0);
    lcd.print("SD Card Failed!");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");

}

void loop() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("");

  if(iter++ < 10) {
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (dataFile) {
        DateTime now = RTC.now(); 
        sprintf(timeBuffer, "%02d:%02d:%02d,%d", now.hour(), now.minute(), now.second(),millis());
        dataFile.println(timeBuffer);
        dataFile.close();
        lcd.setCursor(0,1);
        lcd.print(timeBuffer);
      }
  }

  delay(1000);
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

void start_00_timing() {
    start_00_millis = millis();
}

void mid_01_timing() {
    mid_01_millis = millis();
}

void mid_02_timing() {
    mid_02_millis = millis();
}

void end_03_timing() {
    end_03_millis = millis();
}


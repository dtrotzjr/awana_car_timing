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
void end_02_timing();


////////////////////////////////////////////////////////////////////////////////
// Global Variables 
////////////////////////////////////////////////////////////////////////////////
RTC_DS1307 RTC;
// The following interrupt pins are different based on the Arduino being used.
// In this case we are targeting the Arduino Mega
const int BREAK_SENSOR_00_INTERRUPT_PIN = 18;
const int BREAK_SENSOR_01_INTERRUPT_PIN = 19;
const int BREAK_SENSOR_02_INTERRUPT_PIN = 2;

const int SD_CARD_CHIP_SELECT           = 53;

const int YEL_LED_PIN                   = 22;
const int BTN_01_PIN                    = 23;
const int BTN_02_PIN                    = 24;
const int BTN_03_PIN                    = 25;
const int WHT_LED_PIN                   = 26;
const int GRN_LED_PIN                   = 27;
const int RED_LED_PIN                   = 28 ;

const char CAR_OWNERS[4][16] = {"ABBY", "BEN", "WILLIAM", "JAMES"}; 

// This board can be run in 8 bit or 4 bit modes. 
// The bits indicate the width of the datapipe. 
// 8-bit LCD (we want the added performance of 8 bits)
LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 14);

int LCD_BACKLIGHT_PIN   = 13;
long iter       = 0;
char dataFilename[256];
char timeBuffer[256];
char floatBuffer[16];

volatile signed long start_00_millis    = -1;
volatile signed long mid_01_millis      = -1;
volatile signed long end_02_millis      = -1;

bool handledStart = false;
bool handledMid = false;
bool handledEnd = false;

int sensorState = 0, lastState=0; 

void setup()
{
  digitalWrite(BREAK_SENSOR_00_INTERRUPT_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_00_INTERRUPT_PIN), start_00_timing, FALLING);
  digitalWrite(BREAK_SENSOR_01_INTERRUPT_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_01_INTERRUPT_PIN), mid_01_timing, FALLING);
  digitalWrite(BREAK_SENSOR_02_INTERRUPT_PIN, HIGH);
  attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_02_INTERRUPT_PIN), end_02_timing, FALLING);

  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  pinMode(GRN_LED_PIN, OUTPUT);
  digitalWrite(GRN_LED_PIN, LOW);
  pinMode(YEL_LED_PIN, OUTPUT);
  digitalWrite(YEL_LED_PIN, LOW);
  pinMode(WHT_LED_PIN, OUTPUT);
  digitalWrite(WHT_LED_PIN, LOW);

  pinMode(BTN_01_PIN, INPUT);
  pinMode(BTN_02_PIN, INPUT);
  pinMode(BTN_03_PIN, INPUT);
  
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
  
  pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
  digitalWrite(LCD_BACKLIGHT_PIN, HIGH); // turn LCD_BACKLIGHT_PIN on. Replace 'HIGH' with 'LOW' to turn it off.
  lcd.begin(16,2);              // columns, rows.  use 16,2 for a 16x2 LCD, etc.
  lcd.clear();

  if (!SD.begin(SD_CARD_CHIP_SELECT)) {
    lcd.setCursor(0,0);
    lcd.print("SD Card Failed!");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  DateTime now = RTC.now();
  sprintf(dataFilename, "%02d-%02d-%02d.csv", now.year() % 2000, now.month(), now.day());

  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("TEST");
}

void loop() {
  digitalWrite(YEL_LED_PIN, HIGH);
  
  if(!handledStart && start_00_millis > 0) {
    lcd.setCursor(9,0);
    lcd.print("RUNNING");
    File dataFile = SD.open(dataFilename, FILE_WRITE);
    if (dataFile) {
      DateTime now = RTC.now(); 
      sprintf(timeBuffer, "%02d-%02d-%02d,%ld,", now.year(), now.month(), now.day(), start_00_millis);
      dataFile.print(timeBuffer);
      dataFile.close();
    }
    handledStart = true;
    digitalWrite(WHT_LED_PIN, HIGH);
  }

  if(!handledMid && mid_01_millis > 0) {
    float seconds = ((((float)mid_01_millis) - ((float)start_00_millis))/(1000.0f));
    
    lcd.setCursor(0,1);
    sprintf(timeBuffer, "M:%s", dtostrf(seconds, 4, 3, floatBuffer));
    lcd.print(timeBuffer);
    Serial.print(timeBuffer);
      File dataFile = SD.open(dataFilename, FILE_WRITE);
      if (dataFile) {
        DateTime now = RTC.now(); 
        sprintf(timeBuffer, "%ld,%s,", mid_01_millis, dtostrf(seconds, 4, 3, floatBuffer));
        dataFile.print(timeBuffer);
        dataFile.close();
      }
      handledMid = true;
      digitalWrite(WHT_LED_PIN, LOW);
      digitalWrite(GRN_LED_PIN, HIGH);
  }

    if(!handledEnd && end_02_millis > 0) {
      float seconds = ((((float)end_02_millis) - ((float)start_00_millis))/(1000.0f));
      lcd.setCursor(9,1);
      sprintf(timeBuffer, "E:%s", dtostrf(seconds, 4, 3, floatBuffer));
      Serial.print(timeBuffer);
      lcd.print(timeBuffer);
        File dataFile = SD.open(dataFilename, FILE_WRITE);
        if (dataFile) {
          DateTime now = RTC.now(); 
          sprintf(timeBuffer, "%ld,%s", end_02_millis, dtostrf(seconds, 4, 3, floatBuffer));
          dataFile.println(timeBuffer);
          dataFile.close();
        }
        handledEnd = true;
        digitalWrite(GRN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);
  }

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

void end_02_timing() {
    end_02_millis = millis();
}


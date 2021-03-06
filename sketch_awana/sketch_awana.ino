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

const int TOTAL_BTNS  = 3;

const int YEL_LED_PIN                   = 22;
const int BTN_PINS[TOTAL_BTNS]          = {23, 24, 25};
const int WHT_LED_PIN                   = 26;
const int GRN_LED_PIN                   = 27;
const int RED_LED_PIN                   = 28 ;

const int TOTAL_CAR_OWNERS = 6;
const char CAR_OWNERS[TOTAL_CAR_OWNERS][16] = {"AB17", "BN17", "ABBY", "BEN", "WILLY", "JAMES"}; 
int CAR_OWNERS_TEST_NUMBER[TOTAL_CAR_OWNERS] = {0}; 

int curCarIndex = 0;

// This board can be run in 8 bit or 4 bit modes. 
// The bits indicate the width of the datapipe. 
// 8-bit LCD (we want the added performance of 8 bits)
LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6, 5, 4, 15, 14);

int LCD_BACKLIGHT_PIN   = 13;
long iter       = 0;
char dataFilename[256];
char messageBuffer[64];
char floatBuffer[16];
char startInfo00Buffer[256] = {0};
char midInfo01Buffer[256] = {0};
char midInfo02Buffer[256] = {0};
char endInfo03Buffer[256] = {0};

volatile signed long start_00_millis    = -1;
volatile signed long mid_01_millis      = -1;
volatile signed long mid_02_millis      = -1;
volatile signed long end_03_millis      = -1;

bool handledStart00 = false;
bool handledMid01 = false;
bool handledMid02 = false;
bool handledEnd03 = false;

int btnStates[TOTAL_BTNS];
int lastBtnStates[TOTAL_BTNS];

unsigned long lastDebounceTimes[TOTAL_BTNS] = {0};
const unsigned long DEBOUNCE_DELAY = 50;

void setup()
{
	digitalWrite(BREAK_SENSOR_00_INTERRUPT_PIN, HIGH);
	attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_00_INTERRUPT_PIN), start_00_timing, FALLING);
	digitalWrite(BREAK_SENSOR_01_INTERRUPT_PIN, HIGH);
	attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_01_INTERRUPT_PIN), mid_01_timing, FALLING);
	digitalWrite(BREAK_SENSOR_02_INTERRUPT_PIN, HIGH);
	attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_02_INTERRUPT_PIN), mid_02_timing, FALLING);
	digitalWrite(BREAK_SENSOR_03_INTERRUPT_PIN, HIGH);
	attachInterrupt(digitalPinToInterrupt(BREAK_SENSOR_03_INTERRUPT_PIN), end_03_timing, FALLING);

	pinMode(RED_LED_PIN, OUTPUT);
	pinMode(GRN_LED_PIN, OUTPUT);
	pinMode(YEL_LED_PIN, OUTPUT);
	pinMode(WHT_LED_PIN, OUTPUT);
  
  
	for(int i = 0; i < TOTAL_BTNS; i++) {
		pinMode(BTN_PINS[i], INPUT);
		btnStates[i] = LOW;
		lastBtnStates[i] = LOW;
	}
	
	for(int i = 0; i < TOTAL_CAR_OWNERS; i++) {
		CAR_OWNERS_TEST_NUMBER[i] = 1;
	}
  
	Wire.begin();
	RTC.begin();
	if (! RTC.isrunning()) {
		// following line sets the RTC to the date & time this sketch was compiled
		RTC.adjust(DateTime(__DATE__, __TIME__));
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
	
	DateTime now = RTC.now();

	sprintf(dataFilename, "%02d-%02d-%02d.csv", now.year() % 2000, now.month(), now.day());
	if (!SD.exists(dataFilename)) {
		File dataFile = SD.open(dataFilename, FILE_WRITE);
		if (dataFile) {
			dataFile.print("date,car,test,start_00,mid_01,mid_01_len,mid_02,mid_02_len,end_03,end_03_len\n");
			dataFile.close();
		} else {
			lcd.setCursor(0,0);
			lcd.print("File Error!");
			lcd.setCursor(0,1);
			lcd.print(dataFilename);
		}
	}
	resetState(true);
}

void loop() {
	checkBtnStates();
	
	if(!handledStart00 && start_00_millis > 0) {
		lcd.setCursor(9,0);
		lcd.print("REC");
		DateTime now = RTC.now();
		sprintf(startInfo00Buffer, "%02d-%02d-%02d,%s,%d,%ld", now.year(), now.month(), now.day(), CAR_OWNERS[curCarIndex], CAR_OWNERS_TEST_NUMBER[curCarIndex], start_00_millis);
		handledStart00 = true;
		digitalWrite(WHT_LED_PIN, HIGH);
	}

	if(handledStart00 && !handledMid01 && mid_01_millis > 0) {
		float seconds = ((((float)mid_01_millis) - ((float)start_00_millis))/(1000.0f));
		lcd.setCursor(9,0);
		sprintf(messageBuffer, "A:%s", dtostrf(seconds, 4, 3, floatBuffer));
		lcd.print(messageBuffer);
		sprintf(midInfo01Buffer, "%ld,%s", mid_01_millis, dtostrf(seconds, 4, 3, floatBuffer));
		handledMid01 = true;
		digitalWrite(WHT_LED_PIN, LOW);
		digitalWrite(GRN_LED_PIN, HIGH);
	}
	
	if(handledMid01 && !handledMid02 && mid_02_millis > 0) {
		float seconds = ((((float)mid_02_millis) - ((float)start_00_millis))/(1000.0f));
		lcd.setCursor(0,1);
		sprintf(messageBuffer, "B:%s", dtostrf(seconds, 4, 3, floatBuffer));
		lcd.print(messageBuffer);
		sprintf(midInfo02Buffer, "%ld,%s", mid_02_millis, dtostrf(seconds, 4, 3, floatBuffer));
		handledMid02 = true;
		digitalWrite(GRN_LED_PIN, LOW);
	}

	if(handledMid02 && !handledEnd03 && end_03_millis > 0) {
		float seconds = ((((float)end_03_millis) - ((float)start_00_millis))/(1000.0f));
		lcd.setCursor(9,1);
		sprintf(messageBuffer, "C:%s", dtostrf(seconds, 4, 3, floatBuffer));
		lcd.print(messageBuffer);
		File dataFile = SD.open(dataFilename, FILE_WRITE);
		if (dataFile) {
			sprintf(endInfo03Buffer, "%s,%s,%s,%ld,%s\n", startInfo00Buffer, midInfo01Buffer, midInfo02Buffer, end_03_millis, dtostrf(seconds, 4, 3, floatBuffer));
			dataFile.print(endInfo03Buffer);
			dataFile.close();
		} else {
			lcd.setCursor(0,0);
			lcd.print("File Error!");
			lcd.setCursor(0,1);
			lcd.print(dataFilename);
		}
		handledEnd03 = true;
		digitalWrite(RED_LED_PIN, HIGH);
	}
}

void checkBtnStates() {
	for(int i = 0; i < TOTAL_BTNS; i++) {
		int btnReading = digitalRead(BTN_PINS[i]);

		// If the switch changed, due to noise or pressing:
		if (btnReading != lastBtnStates[i]) {
			// reset the debouncing timer
			lastDebounceTimes[i] = millis();
		}

		if ((millis() - lastDebounceTimes[i]) > DEBOUNCE_DELAY) {
			// whatever the reading is at, it's been there for longer than the debounce
			// delay, so take it as the actual current state:
    
			// if the button state has changed:
			if (btnReading != btnStates[i]) {
				btnStates[i] = btnReading;
				// only toggle the LED if the new button state is HIGH
				if (btnStates[i] == HIGH) {
					switch(i) {
						case 0:
						resetState(false);
						break;
						case 1:
						switchCar();
						break;
						case 2:
						incrementTest();
						break;
					}
				}
			}
		}
		lastBtnStates[i] = btnReading;
	}
}

void resetState(bool resetCar) {
	digitalWrite(YEL_LED_PIN, HIGH);
	digitalWrite(RED_LED_PIN, LOW);
	digitalWrite(GRN_LED_PIN, LOW);
	digitalWrite(WHT_LED_PIN, LOW);
	
	if (resetCar) {
		curCarIndex = 0;
	}
	resetLCD();

	start_00_millis    = -1;
	mid_01_millis      = -1;
	mid_02_millis      = -1;
	end_03_millis      = -1;

	handledStart00 = false;
	handledMid01 = false;
	handledMid02 = false;
	handledEnd03 = false;
}

void switchCar() {
	resetState(false);
	curCarIndex += 1;
	if (curCarIndex >= TOTAL_CAR_OWNERS) {
		curCarIndex = 0;
	}
	resetLCD();
}

void incrementTest() {
	resetState(false);
	CAR_OWNERS_TEST_NUMBER[curCarIndex] += 1;
	resetLCD();
}

void resetLCD() {
	lcd.clear();
	lcd.setCursor(0,0);
	sprintf(messageBuffer, "%s(%03d)", CAR_OWNERS[curCarIndex], CAR_OWNERS_TEST_NUMBER[curCarIndex]);
	lcd.print(messageBuffer);
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


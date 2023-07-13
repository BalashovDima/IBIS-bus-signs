#include <LiquidCrystal.h>
#include "RTClib.h"


#define LCD_columns 16
#define LCD_rows 2
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define rs 8
#define en 9
#define d4 4 
#define d5 5
#define d6 6
#define d7 7
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void setup() {
    lcd.begin(LCD_columns, LCD_rows); // set up the LCD's number of columns and rows

    if (!rtc.begin()) {
        // code to do if failed to connect to rtc module
        // Serial.println("Couldn't find RTC");
    }

    // rtc.adjust(DateTime()); // set rtc module time

}

void loop() {
    DateTime now = rtc.now();

}
#include <LiquidCrystal.h>
#include "RTClib.h"
#include <AnalogKey.h>
#include <EncButton.h>


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

int16_t sigs[5] = {640, 411, 257, 100, 0}; // array of signal value of buttons
AnalogKey<A0, 5, sigs> keys; // pin of button, number of buttons, array of signal

EncButton<EB_TICK, VIRT_BTN> select_btn;
EncButton<EB_TICK, VIRT_BTN> left_btn;
EncButton<EB_TICK, VIRT_BTN> down_btn;
EncButton<EB_TICK, VIRT_BTN> up_btn;
EncButton<EB_TICK, VIRT_BTN> right_btn;

void setup() {
    lcd.begin(LCD_columns, LCD_rows); // set up the LCD's number of columns and rows

    if (!rtc.begin()) {
        // code to do if failed to connect to rtc module
        // Serial.println("Couldn't find RTC");
    }

    // rtc.adjust(DateTime()); // set rtc module time

}

void loop() {
    select_btn.tick(keys.status(0));
    left_btn.tick(keys.status(1));
    down_btn.tick(keys.status(2));
    up_btn.tick(keys.status(3));
    right_btn.tick(keys.status(4));

    DateTime now = rtc.now();

}
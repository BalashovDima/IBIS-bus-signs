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

// each state marks certain 'state'. for each state (each number) the text of the menu is different 
// the codes are as follows:
// 0 -- home | stand-by mode. not in menu. date, time and line are shown
// 1 -- main menu | pointed at 'Line' option 1st row . 'Other text' 2nd row
// 2 -- main menu | 'Line' option 1st row . pointed at 'Other text' 2nd row
// 3 -- main menu | pointed at 'Time settings' option 1st row. 2nd row empty

uint16_t state = 0;
String date_and_time;
String current_sign_text = "Line 1";

uint8_t second = 0;

void updateMenu(bool state_changed = false) {
    if(state_changed) lcd.clear();

    switch (state) {
        case 0: 
            lcd.home();
            lcd.print(date_and_time);
            lcd.setCursor(0, 1);
            lcd.print(current_sign_text);
            break;
        case 1:
            lcd.setCursor(0,0);
            lcd.print("> Line");
            lcd.setCursor(0,1);
            lcd.print("  Other text");
            break;
        case 2:
            lcd.setCursor(0,0);
            lcd.print("  Line");
            lcd.setCursor(0,1);
            lcd.print("> Other text");
            break;
        case 3:
            lcd.setCursor(0,0);
            lcd.print("> Time settings");
            break;
        default:
            break;
    }
}

void lcd_pointer(bool first_row = true) {
    if(first_row) {
        lcd.setCursor(0,0);
        lcd.print("> ");
        lcd.setCursor(0,1);
        lcd.print("  ");
    } else {
        lcd.setCursor(0,0);
        lcd.print("  ");
        lcd.setCursor(0,1);
        lcd.print("> ");
    }
}


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

    if(state == 0 && select_btn.click()) {
        state = 1; // enters menu
        updateMenu(true);
    } else if(state <= 3) {
        if(select_btn.click()) {
            switch(state) {
                case 1:
                    // state = ;
                    // updateMenu(true);
                    break;
                case 2:
                    // state = ;
                    // updateMenu(true);
                    break;
                case 3:
                    // state = ;
                    // updateMenu(true);
                    break;
            }
        } else if(down_btn.click()) {
            state + 1 > 3 ? state = 1 : state++;
            updateMenu(true);
        } else if(up_btn.click()) {
            state - 1 < 1 ? state = 3 : state--;
            updateMenu(true);
        }
    }

    DateTime now = rtc.now();

    if(state == 0 && second != now.second()) {
        char date_time[] = "DD/MM  hh:mm:ss";
        now.toString(date_time);
        date_and_time = date_time;
        updateMenu();
    }

}
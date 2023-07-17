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
// 100 -- line setting | Line <number>. Number of line can be change with up and down buttons 
// 99 -- time setting | DD/MM/YY on 1st row. hh:mm:ss 2nd row. LEFT, RIGHT to go to previous, next number to change. UP, DOWN to increase, decrease

uint16_t state = 0;
String date_and_time;
String current_sign_text = "Line 1";

uint8_t second = 0;

DateTime setting_time;
uint8_t setting_time_row = 1;
uint8_t setting_time_col = 0;
// 01234567
// DD/MM/YY
// hh:mm:ss

int8_t lines[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 19, 20, -1, -2, -3, -4, -5, -8, -16};
uint8_t line_index = 0;

void updateMenu(bool state_changed = false) {
    if(state_changed) {
        lcd.noBlink();
        lcd.clear();
    }

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
        case 100:
            lcd.setCursor(0,0);
            lcd.print("Line");
            lcd.setCursor(5,0);
            lcd.print("    ");
            lcd.setCursor(5,0);
            lcd.print(lines[line_index]);
            lcd.setCursor(0,1);
            lcd.print("use UP&DOWN");
            break;
        case 99:
            lcd.setCursor(0,0);
            char first_row[] = "DD/MM/YY";
            char second_row[] = "hh:mm:ss";
            lcd.print(setting_time.toString(first_row));
            lcd.setCursor(0,1);
            lcd.print(setting_time.toString(second_row));
            lcd.setCursor(setting_time_col, setting_time_row);
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

    DateTime now = rtc.now();

    if(state == 0 && select_btn.click()) {
        state = 1; // enters menu
        updateMenu(true);
    } else if(state <= 3) { // when in main menu
        if(select_btn.click()) {
            switch(state) {
                case 1: // enter line settings
                    state = 100;
                    updateMenu(true);
                    break;
                case 2:
                    // state = ;
                    // updateMenu(true);
                    break;
                case 3:
                    state = 99;
                    setting_time = now;
                    updateMenu(true);
                    lcd.blink();
                    break;
            }
        } else if(down_btn.click()) {
            state + 1 > 3 ? state = 1 : state++;
            updateMenu(true);
        } else if(up_btn.click()) {
            state - 1 < 1 ? state = 3 : state--;
            updateMenu(true);
        }
    } else if(state == 100) { // when in line setting
        if(select_btn.click()) {
            current_sign_text = "Line " + String(lines[line_index]);
            state = 0;
            updateMenu(true);
        } else if(up_btn.click()) {
            line_index + 1 >= sizeof(lines) / sizeof(lines[0]) ? line_index = 0 : line_index++;
            updateMenu();
        } else if(down_btn.click()) {
            line_index - 1 <= 0 ? line_index = sizeof(lines) / sizeof(lines[0]) - 1 : line_index--;
            updateMenu();
        }
    } else if(state = 99) { // when setting time
        // 01234567
        // DD/MM/YY
        // hh:mm:ss
        if(select_btn.click()) {
            if(setting_time.isValid()) {
                state = 0;
                rtc.adjust(setting_time);
                updateMenu(true);
            } else {
                lcd.setCursor(0,0);
                lcd.print("Invalid ");
                lcd.setCursor(0,1);
                lcd.print("Date    ");
            }
        } else if(up_btn.click()) {
            if(setting_time_row == 0) { // 'date' row
                uint8_t day_setting = setting_time.day();
                uint8_t month_setting = setting_time.month();
                uint16_t year_setting = setting_time.year();
                
                if(setting_time_col == 0) {
                    DateTime add_ten_day = setting_time.operator+(TimeSpan(864000));
                    if(add_ten_day.month() == month_setting) {
                        setting_time = add_ten_day;
                    } else if(day_setting%10 == 0){
                        setting_time = DateTime(year_setting, month_setting, 1, setting_time.hour(), setting_time.minute(), setting_time.second());
                    } else {
                        setting_time = DateTime(year_setting, month_setting, day_setting%10, setting_time.hour(), setting_time.minute(), setting_time.second());
                    }
                }
                switch(setting_time_col) {
                    case 1:
                        if(day_setting % 10 < 9) {
                            DateTime add_day = setting_time.operator+(TimeSpan(86400));
                            if(add_day.month() == month_setting) {
                                setting_time = add_day;
                                break;
                            }
                        }
                        if(day_setting < 10) {
                            setting_time = DateTime(year_setting, month_setting, 1, setting_time.hour(), setting_time.minute(), setting_time.second());
                        } else {
                            setting_time = DateTime(year_setting, month_setting, int(day_setting/10)*10, setting_time.hour(), setting_time.minute(), setting_time.second());
                        }
                        break;
                    case 4:
                        month_setting < 12 ? month_setting++ : month_setting = 1;

                        setting_time = DateTime(year_setting, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        break;
                    case 6:
                        if(year_setting < 2090) {
                            setting_time = DateTime(year_setting+10, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        } else {
                            setting_time = DateTime(2000+year_setting%10, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        }
                        break;
                    case 7:
                        if(year_setting % 10 < 9) {
                            setting_time = DateTime(year_setting+1, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        } else {
                            setting_time = DateTime(year_setting/10*10, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        }
                        break;
                }

            } else { // 'time' row
                uint8_t hour_setting = setting_time.hour();
                uint8_t minute_setting = setting_time.minute();
                uint8_t second_setting = setting_time.second();
                switch(setting_time_col) {
                    case 0:
                        if(hour_setting + 10 < 24) {
                            setting_time = setting_time.operator+(TimeSpan(36000));
                        } else if (hour_setting < 20) {
                            setting_time = setting_time.operator-(TimeSpan(36000));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(72000));
                        }
                        break;
                    case 1:
                        if(hour_setting > 20) {
                            hour_setting = hour_setting % 10;
                            if(hour_setting < 3) {
                                setting_time = setting_time.operator+(TimeSpan(3600));
                            } else {
                                setting_time = setting_time.operator-(TimeSpan(10800));
                            }
                            break;
                        }
                        if(hour_setting > 9) hour_setting = hour_setting % 10;
                        if(hour_setting < 9) {
                            setting_time = setting_time.operator+(TimeSpan(3600));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(32400));
                        }
                        break;
                    case 3:
                        if(minute_setting < 50) {
                            setting_time = setting_time.operator+(TimeSpan(600));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(3000));
                        }
                        break;
                    case 4:
                        if(minute_setting > 9) minute_setting = minute_setting % 10;
                        if(minute_setting < 9) {
                            setting_time = setting_time.operator+(TimeSpan(60));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(540));
                        }
                        break;
                    case 6:
                        if(second_setting < 50) {
                            setting_time = setting_time.operator+(TimeSpan(10));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(50));
                        }
                        break;
                    case 7:
                        if(second_setting > 9) second_setting = second_setting % 10;
                        if(second_setting < 9) {
                            setting_time = setting_time.operator+(TimeSpan(1));
                        } else {
                            setting_time = setting_time.operator-(TimeSpan(9));
                        }
                        break;
                }

            }

            updateMenu();
        } else if(down_btn.click()) {
            if(setting_time_row == 0) { // 'date' row
                uint8_t day_setting = setting_time.day();
                uint8_t month_setting = setting_time.month();
                uint16_t year_setting = setting_time.year();

                if(setting_time_col == 0) {
                    DateTime take_ten_day = setting_time.operator-(TimeSpan(864000));
                    if(take_ten_day.month() == month_setting) {
                        setting_time = take_ten_day;
                    } else if(day_setting%10 == 0){
                        setting_time = DateTime(year_setting, month_setting, 1, setting_time.hour(), setting_time.minute(), setting_time.second());
                    } else {
                        if(month_setting < 12) {
                            month_setting++;
                        } else {
                            month_setting = 1;
                            year_setting++;
                        }
                        setting_time = DateTime(year_setting, month_setting, 1, setting_time.hour(), setting_time.minute(), setting_time.second()).operator-(TimeSpan(86400));
                    }
                }
                switch(setting_time_col) {
                    case 1:
                        if(day_setting % 10 > 0 && day_setting != 1) {
                            setting_time = setting_time.operator-(TimeSpan(86400));
                        } else if(day_setting < 10) {
                            setting_time = setting_time.operator+(TimeSpan(691200));
                        } else {
                            DateTime add_nine_day = setting_time.operator+(TimeSpan(777600));
                            
                            if(add_nine_day.month() == month_setting) {
                                setting_time = add_nine_day;
                                break;
                            } 

                            if(month_setting < 12) {
                                month_setting++;
                            } else {
                                month_setting = 1;
                                year_setting++;
                            }
                            setting_time = DateTime(year_setting, month_setting, 1, setting_time.hour(), setting_time.minute(), setting_time.second()).operator-(TimeSpan(86400));
                        }
                        break;
                    case 4:
                        month_setting > 1 ? month_setting-- : month_setting = 12;

                        setting_time = DateTime(year_setting, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        break;
                    case 6:
                        if(year_setting > 2009) {
                            setting_time = DateTime(year_setting-10, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        } else {
                            setting_time = DateTime(2090+year_setting%10, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        }
                        break;
                    case 7:
                        if(year_setting % 10 > 0) {
                            setting_time = DateTime(year_setting-1, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        } else {
                            setting_time = DateTime(year_setting/10*10+9, month_setting, day_setting, setting_time.hour(), setting_time.minute(), setting_time.second());
                        }
                        break;
                }

            } else { // 'time' row
                uint8_t hour_setting = setting_time.hour();
                uint8_t minute_setting = setting_time.minute();
                uint8_t second_setting = setting_time.second();
                switch(setting_time_col) {
                    case 0:
                        if(hour_setting > 9) {
                            setting_time = setting_time.operator-(TimeSpan(36000));
                        } else if(hour_setting % 10 < 4) {
                            setting_time = setting_time.operator+(TimeSpan(72000));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(36000));
                        } 
                        break;
                    case 1:
                        if(hour_setting > 19) {
                            hour_setting = hour_setting % 10;
                            if(hour_setting > 0) {
                                setting_time = setting_time.operator-(TimeSpan(3600));
                            } else {
                                setting_time = setting_time.operator+(TimeSpan(10800));
                            }
                            break;
                        }
                        if(hour_setting > 9) hour_setting = hour_setting % 10;
                        if(hour_setting > 0) {
                            setting_time = setting_time.operator-(TimeSpan(3600));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(32400));
                        }
                        break;
                    case 3:
                        if(minute_setting > 9) {
                            setting_time = setting_time.operator-(TimeSpan(600));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(3000));
                        }
                        break;
                    case 4:
                        if(minute_setting > 9) minute_setting = minute_setting % 10;
                        if(minute_setting > 0) {
                            setting_time = setting_time.operator-(TimeSpan(60));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(540));
                        }
                        break;
                    case 6:
                        if(second_setting > 9) {
                            setting_time = setting_time.operator-(TimeSpan(10));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(50));
                        }
                        break;
                    case 7:
                        if(second_setting > 9) second_setting = second_setting % 10;
                        if(second_setting > 0) {
                            setting_time = setting_time.operator-(TimeSpan(1));
                        } else {
                            setting_time = setting_time.operator+(TimeSpan(9));
                        }
                        break;
                }

            }
            updateMenu();
        } else if(right_btn.click()) { // to next date/time parameter
            if(setting_time_row == 0) {
                if(setting_time_col == 0 || setting_time_col == 3 || setting_time_col == 6) {
                    setting_time_col++;
                } else if(setting_time_col == 7) {
                    setting_time_col = 0;
                    setting_time_row = 1;
                } else if(setting_time_col == 1) {
                    setting_time_col = 4;
                } else {
                    setting_time_col += 2;
                }
            } else {
                if(setting_time_col == 0 || setting_time_col == 3 || setting_time_col == 6) {
                    setting_time_col++;
                } else if(setting_time_col == 7) {
                    setting_time_col = 0;
                    setting_time_row = 0;
                } else {
                    setting_time_col +=2;
                }
            }
            updateMenu();
        } else if(left_btn.click()) { // to previous date/time parameter
            if(setting_time_row == 0) {
                if(setting_time_col == 1 || setting_time_col == 7) {
                    setting_time_col--;
                } else if(setting_time_col == 0) {
                    setting_time_col = 7;
                    setting_time_row = 1;
                } else if(setting_time_col == 4) {
                    setting_time_col = 1;
                } else {
                    setting_time_col -= 2;
                }
            } else {
                if(setting_time_col == 1 || setting_time_col == 4 || setting_time_col == 7) {
                    setting_time_col--;
                } else if(setting_time_col == 0) {
                    setting_time_col = 7;
                    setting_time_row = 0;
                } else {
                    setting_time_col -= 2;
                }
            }
            updateMenu();
        }
    }


    if(state == 0 && second != now.second()) {
        char date_time[] = "DD/MM  hh:mm:ss";
        now.toString(date_time);
        date_and_time = date_time;
        updateMenu();
    }

}
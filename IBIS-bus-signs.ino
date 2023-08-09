#include <LiquidCrystal.h>
#include "RTClib.h"
#include <AnalogKey.h>
#include <EncButton.h>
#include <EEPROM.h>


#define LCD_columns 16
#define LCD_rows 2
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define rs 12
#define en 11
#define d4 5 
#define d5 4
#define d6 3
#define d7 2
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

RTC_DS1307 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int16_t sigs[5] = {732, 496, 314, 138, 0}; // array of signal value of buttons
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
// 3 -- main menu | pointed at 'Interior sign' option 1st row. 'Time settings' 2nd row
// 4 -- main menu | 'Interior sign' option 1st row. pointed at 'Time settings' 2nd row
// 10 -- home(2) || cycle and text of interior sign are shown
// 100 -- line setting | Line <number>. Number of line can be change with up and down buttons 
// 99 -- time setting | DD/MM/YY on 1st row. hh:mm:ss 2nd row. LEFT, RIGHT to go to previous, next number to change. UP, DOWN to increase, decrease
// 200 -- text or functions setting || text_n_functions is show and can be changed by either of the buttons, SELECT saves the choise. The choise is shown instead of 'Line #'
// 300 -- interior sign submenu || menu of interior sign, pointed at 'Cycle' 1st row, 'Text'  2nd row
// 301 -- interior sign submenu || menu of interior sign, 'Cycle' 1st row, pointed at 'Text'  2nd row
// 302 -- interior sign submenu || menu of interior sign, pointed at 'Turn off'  1nd row, 2nd row is empty
// 310 -- cycle setting || 'Cycle' and cycle_number. cycle_number can be change by user clicking either of the buttons, SELECT saves the choise
// 320 -- interior sign text setting || currect_InteriorSign_text_index is show and can be change by either of the buttons, SELECT saves the choise

uint16_t state = 0;
String date_and_time;
String current_sign_text;

uint8_t second = 0;

DateTime setting_time;
uint8_t setting_time_row = 1;
uint8_t setting_time_col = 0;
// 01234567
// DD/MM/YY
// hh:mm:ss

int8_t lines[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 19, 20, -1, -2, -3, -4, -5, -8, -16};
int8_t destinations[] = {1, 1, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 19, 20, 101, 101, 103, 104, 105, 108, 116};
uint8_t line_index = 0; // 254 when other text is displayed, otherwise the index of the line in the 'lines' array

#define number_of_other_texts 7
char text_n_functions[number_of_other_texts][16];
uint8_t currect_text_n_function_index = 254; // 254 when line number is displayed, otherwise the index of the text in the 'text_n_functions' array 
uint16_t l__text_n_functions[number_of_other_texts] = {989, 987, 986, 985, 0, 993, 0};
uint16_t z__text_n_functions[number_of_other_texts] = {999, 998, 997, 994, 993, 996, 0};

uint8_t cycle_number = 1;
uint8_t currect_InteriorSign_text_index = 0;
#define number_of_interiorSign_texts 3
char interiorSign_text[number_of_other_texts][50];

bool updateIBIS = true;
uint32_t IBIS_timer = 0;

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
            lcd.print("  Text|functions");
            break;
        case 2:
            lcd.setCursor(0,0);
            lcd.print("  Line");
            lcd.setCursor(0,1);
            lcd.print("> Text|functions");
            break;
        case 3:
            lcd.setCursor(0,0);
            lcd.print("> Interior sign");
            lcd.setCursor(0,1);
            lcd.print("  Time settings");
            break;
        case 4:
            lcd.setCursor(0,0);
            lcd.print("  Interior sign");
            lcd.setCursor(0,1);
            lcd.print("> Time settings");
            break;
        case 10:
            lcd.setCursor(0,0);
            lcd.print("Cycle ");
            lcd.print(cycle_number);
            lcd.setCursor(0,1);
            lcd.print(interiorSign_text[currect_InteriorSign_text_index]);
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
        case 300:
            lcd.setCursor(0,0);
            lcd.print("> Cycle");
            lcd.setCursor(0,1);
            lcd.print("  Text");
            break;
        case 301:
            lcd.setCursor(0,0);
            lcd.print("  Cycle");
            lcd.setCursor(0,1);
            lcd.print("> Text");
            break;
        case 302:
            lcd.setCursor(0,0);
            lcd.print("> Turn off");
            break;
        case 310:
            lcd.setCursor(0,0);
            lcd.print("Cycle ");
            lcd.setCursor(6,0);
            lcd.print(cycle_number);
            break;
        case 320:
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(interiorSign_text[currect_InteriorSign_text_index]);
            break;
        case 99:
            lcd.setCursor(0,0);
            char first_row[] = "DD/MM/YY";
            char second_row[] = "hh:mm:ss";
            lcd.print(setting_time.toString(first_row));
            lcd.setCursor(0,1);
            lcd.print(setting_time.toString(second_row));
            lcd.setCursor(setting_time_col, setting_time_row);
            break;
        default:
            break;
    }

    if(state == 200) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(text_n_functions[currect_text_n_function_index]);
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

    strcpy(text_n_functions[0], "All Ukraine");
    strcpy(text_n_functions[1], "On-Duty");
    strcpy(text_n_functions[2], "Children!");
    strcpy(text_n_functions[3], "Evacuation");
    strcpy(text_n_functions[4], "To Depot");
    strcpy(text_n_functions[5], "# Fill");
    strcpy(text_n_functions[6], "# Clear");

    strcpy(interiorSign_text[0], "We sell best apples");
    strcpy(interiorSign_text[1], "Some other ad");
    strcpy(interiorSign_text[2], "This is bs120 sign");

    if(EEPROM.read(0) != 255) line_index = EEPROM.read(0);
    if(EEPROM.read(1) != 255) currect_text_n_function_index = EEPROM.read(1);
    if(EEPROM.read(2) != 255) cycle_number = EEPROM.read(2);
    if(EEPROM.read(3) != 255) currect_InteriorSign_text_index = EEPROM.read(3);

    if(line_index != 254) {
        current_sign_text = "Line "+ String(lines[line_index]);
    } else {
        current_sign_text = text_n_functions[currect_text_n_function_index];
    }

    IBIS_init();
}

void loop() {
    select_btn.tick(keys.status(2));
    left_btn.tick(keys.status(4));
    down_btn.tick(keys.status(0));
    up_btn.tick(keys.status(1));
    right_btn.tick(keys.status(3));

    bool SELECT = select_btn.click() || select_btn.step();
    bool LEFT = left_btn.click() || left_btn.step();
    bool DOWN = down_btn.click() || down_btn.step();
    bool UP = up_btn.click() || up_btn.step();
    bool RIGHT = right_btn.click() || right_btn.step();

    DateTime now = rtc.now();

    if(state == 0 || state == 10) {
        if(SELECT) {
            state = 1; // enters menu
            updateMenu(true);
        } else if(RIGHT || DOWN || LEFT || UP) {
            if(state == 0) state = 10;
            else state = 0;
            updateMenu(true);
        } 
    } else if(state > 0 && state <= 4) { // when in main menu
        if(SELECT) {
            switch(state) {
                case 1: // enter line settings
                    state = 100;
                    line_index = 0;
                    updateMenu(true);
                    break;
                case 2: // enter 'text & functions' setting
                    state = 200;
                    currect_text_n_function_index = 0;
                    updateMenu(true);
                    break;
                case 3: // enter 'Interior sign' settings
                    state = 300;
                    updateMenu(true);
                    break;
                case 4:
                    state = 99;
                    setting_time = now;
                    updateMenu(true);
                    lcd.blink();
                    break;
            }
        } else if(DOWN) {
            state + 1 > 4 ? state = 1 : state++;
            updateMenu(true);
        } else if(UP) {
            state - 1 < 1 ? state = 4 : state--;
            updateMenu(true);
        }
    } else if(state == 100) { // when in line setting
        if(SELECT) {
            current_sign_text = "Line " + String(lines[line_index]);
            state = 0;
            currect_text_n_function_index = 254;
            EEPROM.update(0, line_index); // save what line_index was chosen
            EEPROM.update(1, 254); // currect_text_n_function_index = 254, meaning that line is displayed
            updateMenu(true);
            updateIBIS = true;
        } else if(UP) {
            line_index + 1 >= sizeof(lines) / sizeof(lines[0]) ? line_index = 0 : line_index++;
            updateMenu();
        } else if(DOWN) {
            line_index - 1 <= 0 ? line_index = sizeof(lines) / sizeof(lines[0]) - 1 : line_index--;
            updateMenu();
        }
    } else if(state == 200) { // when setting 'Text & functions'
        if(SELECT) {
            current_sign_text = text_n_functions[currect_text_n_function_index];
            state = 0;
            line_index = 254;
            EEPROM.update(0, 254); // line_index = 254, meaning that text or function is displayed
            EEPROM.update(1, currect_text_n_function_index); // save what text or function was chosen
            updateMenu(true);
            updateIBIS = true;
        } else if(UP || LEFT) {
            currect_text_n_function_index < number_of_other_texts - 1 ? currect_text_n_function_index++ : currect_text_n_function_index = 0;
            updateMenu();
        } else if(DOWN  || RIGHT) {
            currect_text_n_function_index > 0 ? currect_text_n_function_index-- : currect_text_n_function_index = number_of_other_texts - 1;
            updateMenu();
        }
    } else if(state >= 300 && state <= 302) { // when in 'Interior sign' settings
        if(SELECT) {
            switch(state) {
                case 300: // enter in 'cycle' setting of 'Interior sign'
                    state = 310;
                    cycle_number = 1;
                    updateMenu(true);
                    break;
                case 301: // enter in 'text' setting of 'Interior sign'
                    state = 320;
                    currect_InteriorSign_text_index = 0;
                    updateMenu(true);
                    break;
                case 302: // turn off interior sign 
                    cycle_number = 0;
                    state = 0;
                    updateMenu(true);
                    break;
            }
        } else if(DOWN || RIGHT) {
            state == 302 ? state = 300 : state++;
            updateMenu(true);
        } else if(UP || LEFT) {
            state == 300 ? state = 302 : state--;
            updateMenu(true);
        } 
    } else if(state == 310) { // setting 'cycle' parameter of 'Interior sign' setting
        if(SELECT) {
            state = 0;
            EEPROM.update(2, cycle_number); // save what cycle was chosen
            updateMenu(true);
        } else if(UP || RIGHT) {
            cycle_number == 9 ? cycle_number = 1 : cycle_number++;
            updateMenu();
        } else if(DOWN || LEFT) {
            cycle_number == 1 ? cycle_number = 9 : cycle_number--;
            updateMenu();
        } 
    }  else if(state == 320) { // choosing text setting of 'Interior sign' setting
        if(SELECT) {
            state = 0;
            EEPROM.update(3, currect_InteriorSign_text_index);
            updateMenu(true);
        } else if(DOWN || RIGHT) {
            currect_InteriorSign_text_index == number_of_interiorSign_texts - 1 ? currect_InteriorSign_text_index = 0 : currect_InteriorSign_text_index++;
            updateMenu();
        } else if(UP || LEFT) {
            currect_InteriorSign_text_index == 0 ? currect_InteriorSign_text_index = number_of_interiorSign_texts - 1 : currect_InteriorSign_text_index--;
            updateMenu();
        } 
    } else if(state == 99) { // when setting time
        // 01234567
        // DD/MM/YY
        // hh:mm:ss
        if(SELECT) {
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
        } else if(UP) {
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
        } else if(DOWN) {
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
        } else if(RIGHT) { // to next date/time parameter
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
        } else if(LEFT) { // to previous date/time parameter
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

    if(millis() - IBIS_timer >= 10000) updateIBIS = true;

    if((state == 0 || state == 10) && updateIBIS) {
        uint16_t l_index;
        uint16_t z_index;
        if(line_index != 254) {
            if(lines[line_index] > 0) {
                l_index = lines[line_index];
                z_index = destinations[line_index];
            } else {
                l_index = abs(lines[line_index]);
                z_index = destinations[line_index];
            }
        } else {
            l_index = l__text_n_functions[currect_text_n_function_index];
            z_index = z__text_n_functions[currect_text_n_function_index];
        }

        IBIS_l(l_index);
        IBIS_z(z_index);
        updateIBIS = false;
        IBIS_timer = millis();
    }
}
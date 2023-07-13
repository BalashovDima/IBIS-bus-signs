#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
#define rs 8
#define en 9
#define d4 4 
#define d5 5
#define d6 6
#define d7 7
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
#define LCD_columns 16
#define LCD_rows 2

void setup() {
    lcd.begin(LCD_columns, LCD_rows); // set up the LCD's number of columns and rows

}

void loop() {

}
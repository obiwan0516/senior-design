#include <LiquidCrystal.h>

// Initialize the library with the numbers of the interface pins
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);

void setup() {
  // Set up the LCD's number of columns and rows (16x2)
  lcd.begin(16, 2);
  
  // Print a message to the LCD
  lcd.setCursor(0, 0);  // Set the cursor to column 0, row 0
  lcd.print("Hello, World!");  // Print "Hello, World!" on the first row
  
  lcd.setCursor(0, 1);  // Set the cursor to column 0, row 1
  lcd.print("LCD Test");  // Print "LCD Test" on the second row
}

void loop() {
  // Nothing to do here
}

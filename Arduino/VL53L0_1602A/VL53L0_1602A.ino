#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>

// Initialize the LCD screen
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
VL53L0X sensor;

void setup()
{
  // Initialize serial communication and LCD
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2); // Initializes the interface to the LCD, specifies dimensions (16x2)

  sensor.setTimeout(500);
  if (!sensor.init())
  {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Init Fail!");
    while (1) {}
  }

#if defined LONG_RANGE
  sensor.setSignalRateLimit(0.1);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  sensor.setMeasurementTimingBudget(200000);
#endif
}

void loop()
{
  // Read the distance in millimeters
  int distance = sensor.readRangeSingleMillimeters();

  // Check if the reading was successful
  if (sensor.timeoutOccurred()) {
    lcd.setCursor(0, 0);
    lcd.print("Timeout Occurred");
  } else {
    // Display the distance in millimeters on the LCD
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Distance: ");
    lcd.print(distance);
    lcd.print(" mm");
  }

  delay(50); // Delay between readings
}


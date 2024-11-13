#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>

// Initialize the LCD screen
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
VL53L0X sensor;

// TMAG5273 I2C address
#define TMAG5273_ADDR 0x22 // Replace with your TMAG5273's I2C address

void setup()
{
  // Initialize serial communication and LCD
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2);

  // Initialize time-of-flight sensor
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Init Fail!");
    while (1) {}
  }

  // Configuration for TOF sensor
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

  // Setup TMAG5273 (if needed, add any necessary initialization here)
  // Ensure correct communication
  Wire.beginTransmission(TMAG5273_ADDR);
  if (Wire.endTransmission() != 0) {
    lcd.setCursor(0, 1);
    lcd.print("TMAG Init Fail!");
    while (1) {}
  }
}

void loop()
{
  // Read distance from VL53L0X
  int distance = sensor.readRangeSingleMillimeters();

  // Read data from TMAG5273 (example read process)
  int hall_data = readHallSensor();

  // Check if the reading was successful
  lcd.clear();
  if (sensor.timeoutOccurred()) {
    lcd.setCursor(0, 0);
    lcd.print("Timeout Occurred");
  } else {
    // Display distance and Hall sensor data
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print(" mm");

    lcd.setCursor(0, 1);
    lcd.print("Hall: ");
    lcd.print(hall_data);
  }

  delay(50); // Delay between readings
}

// Function to read from the Hall effect sensor
int readHallSensor() {
  Wire.beginTransmission(TMAG5273_ADDR);
  Wire.write(0x44); // Register address (update based on TMAG5273's datasheet)
  Wire.endTransmission(false);

  Wire.requestFrom(TMAG5273_ADDR, 2); // Request data from sensor
  int value = 0;
  if (Wire.available() >= 2) {
    value = Wire.read() << 8; // MSB
    value |= Wire.read();     // LSB
  }
  return value;
}

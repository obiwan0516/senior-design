#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>

// LCD and Distance Sensor Setup
LiquidCrystal lcd(6, 7, 5, 4, 3, 2);
VL53L0X sensor1;
VL53L0X sensor2;


// Motor Driver Pins
#define MOTOR_CW 9   // Clockwise (forward) PWM
#define MOTOR_CCW 10  // Counterclockwise (reverse) PWM
#define BUZZER_PIN 12 // Positive lead for Passive Buzzer PWM
#define XSHUT_1 13 // To temporarily shut it off after init
#define XSHUT_2 8 // To temporarily shut it off after init

// Distance Mapping Parameters
const int MIN_DIST = 50;   // Minimum distance (mm)
const int MAX_DIST = 300;  // Maximum distance (mm)
const int MAX_PWM = 255;   // Max PWM value

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2); // Initialize LCD (16x2)

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(XSHUT_1, OUTPUT);
  pinMode(XSHUT_2, OUTPUT);

  lcd.clear();
  lcd.print("Beginning Initialization");
  delay(2000);

  // 🔹 Step 1: Power down both sensors
  digitalWrite(XSHUT_1, LOW);
  digitalWrite(XSHUT_2, LOW);
  delay(10);

  
  // 🔹 Step 2: Initialize Sensor 1
  digitalWrite(XSHUT_1, HIGH);  // Power ON sensor 1
  delay(100);
  Serial.print("XSHUT_1: "); Serial.println(digitalRead(XSHUT_1));

  // Initialize Distance Sensor 1
  sensor1.setTimeout(500);
  if (!sensor1.init()) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor 1 Init Fail!");
    while (1) {}
  }

  sensor1.setAddress(0x12); // Set unique address for sensor1
  Serial.print("Sensor 1 new address: ");
  Serial.println(sensor1.getAddress(), HEX);
  delay(10);

  lcd.clear();
  lcd.print("Sensor 1 Init yay!");
  delay(2000);
  

  // 🔹 Step 3: Initialize Sensor 2
  digitalWrite(XSHUT_2, HIGH);  // Power ON sensor 2
  delay(100);
  Serial.print("XSHUT_2: "); Serial.println(digitalRead(XSHUT_2));

  sensor2.setAddress(0x29); // need to change address here
  Serial.print("Sensor 2 new address: ");
  Serial.println(sensor2.getAddress(), HEX);
  delay(10);

  // Initialize Distance Sensor 2
  sensor2.setTimeout(500);
  if (!sensor2.init()) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor 2 Init Fail!");
    while (1) {}
  }

  lcd.clear();
  lcd.print("Both Sensors OK!");


  // Configure Long-Range or High-Speed if needed
#if defined LONG_RANGE
  sensor1.setSignalRateLimit(0.1);
  sensor1.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor1.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
  
  sensor2.setSignalRateLimit(0.1);
  sensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
  sensor1.setMeasurementTimingBudget(20000);
  sensor2.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  sensor1.setMeasurementTimingBudget(200000);
  sensor2.setMeasurementTimingBudget(200000);
#endif

  // Set up motor pins
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);
  analogWrite(MOTOR_CW, 0);
  analogWrite(MOTOR_CCW, 0);
}

void loop() {
  int distance1 = sensor1.readRangeSingleMillimeters();
  int distance2 = sensor2.readRangeSingleMillimeters();

  // Handle timeouts
  if (sensor1.timeoutOccurred() || sensor2.timeoutOccurred()) {
    lcd.setCursor(0, 0);
    lcd.print("Timeout Occurred");
    return;
  }

  // Compute average distance
  int avgDistance = (distance1 + distance2) / 2;

  // Map distance to PWM (closer = higher PWM)
  int pwmValue = map(avgDistance, MIN_DIST, MAX_DIST, MAX_PWM, 0);
  pwmValue = constrain(pwmValue, 0, MAX_PWM); // Ensure within valid range

  // Control Motor Speed Based on Distance
  if (avgDistance < 150) {  
    // Forward motion when close
    analogWrite(MOTOR_CW, pwmValue);
    analogWrite(MOTOR_CCW, 0);
  } else {  
    // Reverse motion when far
    analogWrite(MOTOR_CW, 0);
    analogWrite(MOTOR_CCW, pwmValue);
  }

  // **Buzzer Activation when distance <= 60mm**
  if (avgDistance <= 60) {
    tone(BUZZER_PIN, 1000); // Play buzzer at 1kHz
  } else {
    noTone(BUZZER_PIN); // Stop buzzer
  }

  // Display on Serial Monitor
  Serial.print("Distance1: ");
  Serial.print(distance1);
  Serial.print(" mm | Distance2: ");
  Serial.print(distance2);
  Serial.print(" mm | Avg: ");
  Serial.print(avgDistance);
  Serial.print(" mm | PWM: ");
  Serial.println(pwmValue);

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg Dist: ");
  lcd.print(avgDistance);
  lcd.print("mm");

  lcd.setCursor(0, 1);
  lcd.print("PWM: ");
  lcd.print(pwmValue);

  delay(50);
}


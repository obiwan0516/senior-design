#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>

// LCD and Distance Sensor Setup
LiquidCrystal lcd(6, 12, 5, 4, 3, 2);
VL53L0X sensor1;
VL53L0X sensor2;
VL53L0X sensor3;
VL53L0X sensor4;

// Motor Driver Pins
#define MOTOR_CW 9   // Clockwise (forward) PWM
#define MOTOR_CCW 10  // Counterclockwise (reverse) PWM
#define BUZZER_PIN 1 // Positive lead for Passive Buzzer PWM
#define XSHUT_1 13 // To temporarily shut it off after init (pin 13)
#define XSHUT_2 8 // To temporarily shut it off after init (pin 8)
#define XSHUT_3 11 // To temporarily shut it off after init (pin 11)
#define XSHUT_4 7 // To temporarily shut it off after init (pin 7)

// Distance Mapping Parameters
const int MIN_DIST = 50;   // Minimum distance (mm)
const int MAX_DIST = 300;  // Maximum distance (mm)
const int MAX_PWM = 255;   // Max PWM value

// Variables to track system state
int distance1 = 0;
int distance2 = 0;
int distance3 = 0;
int distance4 = 0;
int avgDistance = 0;
int pwmValue = 0;
bool buzzerActive = false;
String motorDirection = "IDLE";

// Serial communication timing
unsigned long lastTransmitTime = 0;
const int TRANSMIT_INTERVAL = 100; // Transmit data every 100ms

void setup() {
  Serial.begin(9600);
  Wire.begin();
  lcd.begin(16, 2); // Initialize LCD (16x2)

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(XSHUT_1, OUTPUT);
  pinMode(XSHUT_2, OUTPUT);
  pinMode(XSHUT_3, OUTPUT);
  pinMode(XSHUT_4, OUTPUT);

  lcd.clear();
  lcd.print("Beginning Init");
  
  // Initialize all sensors (same as original code)
  initializeSensors();
  
  // Set up motor pins
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);
  analogWrite(MOTOR_CW, 0);
  analogWrite(MOTOR_CCW, 0);
  
  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
}

void loop() {
  // Read all sensor data
  readSensors();
  
  // Calculate values and control hardware
  processData();
  
  // Display on local LCD
  updateLocalDisplay();
  
  // Transmit data to Mega
  if (millis() - lastTransmitTime >= TRANSMIT_INTERVAL) {
    transmitData();
    lastTransmitTime = millis();
  }
  
  delay(50);
}

void initializeSensors() {
  // Power down all sensors
  digitalWrite(XSHUT_1, LOW);
  digitalWrite(XSHUT_2, LOW);
  digitalWrite(XSHUT_3, LOW);
  digitalWrite(XSHUT_4, LOW);
  delay(10);
  
  // Initialize Sensor 1
  digitalWrite(XSHUT_1, HIGH);
  delay(100);
  sensor1.setTimeout(500);
  if (!sensor1.init()) {
    lcd.clear();
    lcd.print("Sensor 1 Failed!");
    while (1) {}
  }
  sensor1.setAddress(0x12);
  
  // Initialize Sensor 2
  digitalWrite(XSHUT_2, HIGH);
  delay(100);
  sensor2.setAddress(0x24);
  sensor2.setTimeout(500);
  if (!sensor2.init()) {
    lcd.clear();
    lcd.print("Sensor 2 Failed!");
    while (1) {}
  }
  
  // Initialize Sensor 3
  digitalWrite(XSHUT_3, HIGH);
  delay(100);
  sensor3.setAddress(0x18);
  sensor3.setTimeout(500);
  if (!sensor3.init()) {
    lcd.clear();
    lcd.print("Sensor 3 Failed!");
    while (1) {}
  }
  
  // Initialize Sensor 4
  digitalWrite(XSHUT_4, HIGH);
  delay(100);
  sensor4.setAddress(0x27);
  sensor4.setTimeout(500);
  if (!sensor4.init()) {
    lcd.clear();
    lcd.print("Sensor 4 Failed!");
    while (1) {}
  }
  
  lcd.clear();
  lcd.print("All Sensors OK!");
  delay(1000);
}

void readSensors() {
  distance1 = sensor1.readRangeSingleMillimeters();
  distance2 = sensor2.readRangeSingleMillimeters();
  distance3 = sensor3.readRangeSingleMillimeters();
  distance4 = sensor4.readRangeSingleMillimeters();
  
  // Handle timeouts
  if (sensor1.timeoutOccurred() || sensor2.timeoutOccurred() || 
      sensor3.timeoutOccurred() || sensor4.timeoutOccurred()) {
    lcd.clear();
    lcd.print("Sensor Timeout!");
    return;
  }
  
  // Compute average distance
  avgDistance = (distance1 + distance2 + distance3 + distance4) / 4;
}

void processData() {
  // Map distance to PWM (closer = higher PWM)
  pwmValue = map(avgDistance, MIN_DIST, MAX_DIST, MAX_PWM, 0);
  pwmValue = constrain(pwmValue, 0, MAX_PWM);
  
  // Control Motor Speed Based on Distance
  if (avgDistance < 150) {
    motorDirection = "FWD";
    analogWrite(MOTOR_CW, pwmValue);
    analogWrite(MOTOR_CCW, 0);
  } else {
    motorDirection = "REV";
    analogWrite(MOTOR_CW, 0);
    analogWrite(MOTOR_CCW, pwmValue);
  }
  
  // Buzzer control
  if (avgDistance <= 60) {
    buzzerActive = true;
    tone(BUZZER_PIN, 1000);
  } else {
    buzzerActive = false;
    noTone(BUZZER_PIN);
  }
}

void updateLocalDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg: ");
  lcd.print(avgDistance);
  lcd.print("mm ");
  lcd.print(motorDirection);
  
  lcd.setCursor(0, 1);
  lcd.print("PWM: ");
  lcd.print(pwmValue);
  if (buzzerActive) {
    lcd.print(" ALERT!");
  }
}

void transmitData() {
  // Format: D1,D2,D3,D4,AVG,PWM,DIR,BUZ;
  // Example: 120,135,118,142,129,160,FWD,0;
  
  String dataPacket = String(distance1) + "," +
                     String(distance2) + "," +
                     String(distance3) + "," +
                     String(distance4) + "," +
                     String(avgDistance) + "," +
                     String(pwmValue) + "," +
                     motorDirection + "," +
                     String(buzzerActive ? 1 : 0) + ";";
  
  Serial.println(dataPacket);
}

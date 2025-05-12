#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>
#include <PID_v1.h>


// LCD and Distance Sensor Setup
LiquidCrystal lcd(6, 12, 5, 4, 3, 2);
VL53L0X sensor1;
VL53L0X sensor2;
VL53L0X sensor3;
//VL53L0X sensor4;

// Motor Driver Pins
#define MOTOR_CW 9   // Clockwise (forward) PWM
#define MOTOR_CCW 10  // Counterclockwise (reverse) PWM
#define BUZZER_PIN 1 // Positive lead for Passive Buzzer PWM
#define XSHUT_1 13 // To temporarily shut it off after init (pin 13)
#define XSHUT_2 8 // To temporarily shut it off after init (pin 8)
#define XSHUT_3 11 // To temporarily shut it off after init (pin 11)
#define XSHUT_4 7 // To temporarily shut it off after init (pin 7)
#define CSensor_Pin A0

// // Distance Mapping Parameters
// const int MIN_DIST = 50;   // Minimum distance (mm)
// const int MAX_DIST = 300;  // Maximum distance (mm)
// const int MAX_PWM = 255;   // Max PWM value

const float vcc = 5;  // Reference voltage (12V power supply)

// Target distance in mm
const double Setpoint = 191;

// Variables to track system state
int distance1 = 0;
int distance2 = 0;
// int distance3 = 0;
//int distance4 = 0;
// int avgDistance = 0;
// double pwm = 0;
double avgDistance = 0;
double Output = 0;
double pwm = 0;
double Input = 0;
double error = 0;
double previousError = 0;
double integral = 0;
bool buzzerActive = false;
String motorDirection = "IDLE";

// Serial communication timing
unsigned long lastTransmitTime = 0;
const int TRANSMIT_INTERVAL = 100; // Transmit data every 100ms

// PID gains - adjust as needed
double Kp = 8.9507;
double Ki = 1024.6145;//224.6145;
double Kd = 0.08917;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  Serial.begin(9600);

  // Set up motor pins
  pinMode(MOTOR_CW, OUTPUT);
  pinMode(MOTOR_CCW, OUTPUT);
  analogWrite(MOTOR_CW, 205);
  analogWrite(MOTOR_CCW, 0);

  Wire.begin();
  lcd.begin(16, 2); // Initialize LCD (16x2)

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(XSHUT_1, OUTPUT);
  pinMode(XSHUT_2, OUTPUT);
  // pinMode(XSHUT_3, OUTPUT);
  //pinMode(XSHUT_4, OUTPUT);

  lcd.clear();
  lcd.print("Beginning Init");
  
  // Initialize all sensors
  initializeSensors();

  sensor1.startContinuous();
  sensor2.startContinuous();
  // sensor3.startContinuous();
  
  // // Set up motor pins
  // pinMode(MOTOR_CW, OUTPUT);
  // pinMode(MOTOR_CCW, OUTPUT);
  // analogWrite(MOTOR_CW, 205);
  // analogWrite(MOTOR_CCW, 0);
  
  lcd.clear();
  lcd.print("System Ready");
  delay(1000);
  
  myPID.SetMode(AUTOMATIC);

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
  //digitalWrite(XSHUT_4, LOW);
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
  
  // // Initialize Sensor 3
  // digitalWrite(XSHUT_3, HIGH);
  // delay(100);
  // sensor3.setAddress(0x18);
  // sensor3.setTimeout(500);
  // if (!sensor3.init()) {
  //   lcd.clear();
  //   lcd.print("Sensor 3 Failed!");
  //   while (1) {}
  // }
  
  // Initialize Sensor 4
  // digitalWrite(XSHUT_4, HIGH);
  // delay(100);
  // sensor4.setAddress(0x27);
  // sensor4.setTimeout(500);
  // if (!sensor4.init()) {
  //   lcd.clear();
  //   lcd.print("Sensor 4 Failed!");
  //   while (1) {}
  // }
  
  lcd.clear();
  lcd.print("All Sensors OK!");
  delay(1000);
}

void readSensors() {
  distance1 = sensor1.readRangeContinuousMillimeters();
  distance2 = sensor2.readRangeContinuousMillimeters();
  // distance3 = sensor3.readRangeContinuousMillimeters();
  //distance4 = sensor4.readRangeSingleMillimeters();
  
  // Handle timeouts but ensure we still send data
  if (sensor1.timeoutOccurred() || sensor2.timeoutOccurred() /*|| 
      sensor3.timeoutOccurred() || sensor4.timeoutOccurred()*/) {
    lcd.clear();
    lcd.print("Sensor Timeout!");
    // Instead of returning, we'll continue with whatever data we have
  }
  
  // Compute average distance (even with timeouts)
  avgDistance = (distance1 + distance2 /*+ distance3 + distance4*/) / 2;
}

  // Input = avgDistance;
  // myPID.SetOutputLimits(50, 165); 
  // myPID.Compute();
  // pwm = map(Output, 50, 165, 205, 90);
  // analogWrite(MOTOR_CW, pwm);

// void processData() {
//   // Set PWM to maximum value for testing
//   pwmValue = 169; // Force to maximum (255)
  
//   // Choose which direction you want to test
//   // Option 1: Test forward direction only
//   // motorDirection = "FWD";
//   // analogWrite(MOTOR_CW, pwmValue);
//   // analogWrite(MOTOR_CCW, 0);
  
//   // Option 2 (comment out Option 1 if using this): Test reverse direction only
//   motorDirection = "REV";
//   analogWrite(MOTOR_CW, 0);
//   analogWrite(MOTOR_CCW, pwmValue);
  
//   // Optionally disable the buzzer for testing
//   buzzerActive = false;
//   noTone(BUZZER_PIN);
// }

// Old Control Code

void processData() {
  Input = avgDistance;
  myPID.SetOutputLimits(50, 165); 
  myPID.Compute();
  pwm = map(Output, 50, 165, 205, 90);
  analogWrite(MOTOR_CW, pwm);
//   // Map distance to PWM (closer = higher PWM)
//   pwmValue = map(avgDistance, MIN_DIST, MAX_DIST, MAX_PWM, 0);
//   pwmValue = constrain(pwmValue, 0, MAX_PWM);
  
//   // Control Motor Speed Based on Distance
//   if (avgDistance < 150) {
//     motorDirection = "FWD";
//     analogWrite(MOTOR_CW, pwmValue);
//     analogWrite(MOTOR_CCW, 0);
//   } else {
//     motorDirection = "REV";
//     analogWrite(MOTOR_CW, 0);
//     analogWrite(MOTOR_CCW, pwmValue);
  }
  
//   // Buzzer control
//   if (avgDistance <= 60) {
//     buzzerActive = true;
//     tone(BUZZER_PIN, 1000);
//   } else {
//     buzzerActive = false;
//     noTone(BUZZER_PIN);
//   }
// }

void updateLocalDisplay() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg: ");
  lcd.print(avgDistance);
  lcd.print("mm ");
  lcd.print(motorDirection);
  
  lcd.setCursor(0, 1);
  lcd.print("PWM: ");
  lcd.print(255-pwm);
  if (buzzerActive) {
    lcd.print(" ALERT!");
  }

  int rawValue = analogRead(CSensor_Pin);
  float current = (((rawValue / 1023.0) * vcc) - 2.5)/0.2;  // Convert ADC value to voltage
  lcd.print("A: ");
  lcd.print(current);
  
  // Removed Serial.println(current) to avoid interference
}

void transmitData() {
  // Format: D1,D2,D3,D4,AVG,PWM,DIR,BUZ;
  // Example: 120,135,118,142,129,160,FWD,0;
  
  String dataPacket = String(distance1) + "," +
                     String(distance2) + "," +
                     "0" /*String(distance3)*/ + "," +
                     "0"/*String(distance4)*/ + "," +
                     String(avgDistance) + "," +
                     String(pwm) + "," +
                     motorDirection + "," +
                     String(buzzerActive ? 1 : 0) + ";";
  
  Serial.println(dataPacket);
}

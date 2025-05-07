#include <PID_v1.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <LiquidCrystal.h>




// LCD and Distance Sensor Setup
LiquidCrystal lcd(6, 12, 5, 4, 3, 2);
VL53L0X sensor1;
VL53L0X sensor2;
VL53L0X sensor3;
//VL53L0X sensor4;

//Current sensor Setup
// const int sensorPin = A0;       // Analog pin connected to VIOUT
const float VCC = 5.0;          // Supply voltage to the sensor
const float ZERO_CURRENT_V = 2.5;  // Voltage at 0A current (midpoint)
const float SENSITIVITY = 0.2;  // 200 mV/A = 0.2 V/A

// Motor Driver Pins
#define MOTOR_CW 9   // Clockwise (forward) PWM
#define MOTOR_CCW 10  // Counterclockwise (reverse) PWM
#define BUZZER_PIN 1 // Positive lead for Passive Buzzer PWM
#define XSHUT_1 13 // To temporarily shut it off after init (pin 13)
#define XSHUT_2 8 // To temporarily shut it off after init (pin 8)
#define XSHUT_3 11 // To temporarily shut it off after init (pin 11)
//#define XSHUT_4 7 // To temporarily shut it off after init (pin 7)


// // Distance Mapping Parameters
// const int MIN_DIST = 50;   // Minimum distance (mm)
// const int MAX_DIST = 300;  // Maximum distance (mm)
// const int MAX_PWM = 255;   // Max PWM value




// Target distance in mm
const double Setpoint = 215;


// PID variables
double avgDistance = 0;
double Output = 0;
double pwm = 0;
double Input = 0;
double error = 0;
double previousError = 0;
double integral = 0;


// Time tracking
unsigned long lastTime;
double elapsedTime;


// PID gains - adjust as needed
double Kp = 8.9507;
double Ki = 224.6145;//224.6145;
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
  pinMode(XSHUT_3, OUTPUT);
  //pinMode(XSHUT_4, OUTPUT);


  lcd.clear();
  lcd.print("Beginning Initialization");
  delay(1000);


  // 🔹 Step 1: Power down both sensors
  digitalWrite(XSHUT_1, LOW);
  delay(10);
  digitalWrite(XSHUT_2, LOW);
  delay(10);
  digitalWrite(XSHUT_3, LOW);
  delay(10);
  // digitalWrite(XSHUT_4, LOW);
  // delay(10);


 
  // 🔹 Step 2: Initialize Sensor 1
  digitalWrite(XSHUT_1, HIGH);  // Power ON sensor 1
  delay(100);
  // Serial.print("XSHUT_1: "); Serial.println(digitalRead(XSHUT_1));


  // Initialize Distance Sensor 1
  sensor1.setTimeout(500);
  if (!sensor1.init()) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor 1 Init Fail!");
    while (1) {}
  }


  sensor1.setAddress(0x12); // Set unique address for sensor1


  // Serial.print("Sensor 1 new address: ");
  // Serial.println(sensor1.getAddress(), HEX);
  delay(10);


  lcd.clear();
  lcd.print("Sensor 1 Init yay!");
  delay(2000);
 


  // 🔹 Step 3: Initialize Sensor 2
  digitalWrite(XSHUT_2, HIGH);  // Power ON sensor 2
  delay(100);
  // Serial.print("XSHUT_2: "); Serial.println(digitalRead(XSHUT_2));


  sensor2.setAddress(0x24); // need to change address here
  // Serial.print("Sensor 2 new address: ");
  // Serial.println(sensor2.getAddress(), HEX);
  delay(10);


  // Initialize Distance Sensor 2
  sensor2.setTimeout(500);
  if (!sensor2.init()) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor 2 Init Fail!");
    while (1) {}
  }


  lcd.clear();
  lcd.print("Sensor 2 Init yay!");
  delay(1000);


  // 🔹 Step 4: Initialize Sensor 3
  digitalWrite(XSHUT_3, HIGH);  // Power ON sensor 3
  delay(100);
  // Serial.print("XSHUT_3: "); Serial.println(digitalRead(XSHUT_3));


  sensor3.setAddress(0x18); // need to change address here
  // Serial.print("Sensor 3 new address: ");
  // Serial.println(sensor3.getAddress(), HEX);
  delay(10);


  // Initialize Distance Sensor 3
  sensor3.setTimeout(500);
  if (!sensor3.init()) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor 3 Init Fail!");
    while (1) {}
  }


  lcd.clear();
  lcd.print("Sensor 3 Init yay!");
  delay(1000);


  // // 🔹 Step 5: Initialize Sensor 4
  // digitalWrite(XSHUT_4, HIGH);  // Power ON sensor 3
  // delay(100);
  // Serial.print("XSHUT_4: "); Serial.println(digitalRead(XSHUT_4));


  // sensor4.setAddress(0x27); // need to change address here
  // Serial.print("Sensor 4 new address: ");
  // Serial.println(sensor4.getAddress(), HEX);
  // delay(10);


  // // Initialize Distance Sensor 4
  // sensor4.setTimeout(500);
  // if (!sensor4.init()) {
  //   lcd.setCursor(0, 0);
  //   lcd.print("Sensor 4 Init Fail!");
  //   while (1) {}
  // }


  // lcd.clear();
  // lcd.print("Sensor 4 Init yay!");
  // delay(1000);


  lcd.clear();
  lcd.print("All Sensors OK!");
  delay(1000);




  // Configure Long-Range or High-Speed if needed
#if defined LONG_RANGE
  sensor1.setSignalRateLimit(0.1);
  sensor1.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor1.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
 
  sensor2.setSignalRateLimit(0.1);
  sensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor2.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);


  sensor3.setSignalRateLimit(0.1);
  sensor3.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  sensor3.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);


  // sensor4.setSignalRateLimit(0.1);
  // sensor4.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
  // sensor4.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif


#if defined HIGH_SPEED
  sensor1.setMeasurementTimingBudget(20000);
  sensor2.setMeasurementTimingBudget(20000);
  sensor3.setMeasurementTimingBudget(20000);
  //sensor4.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
  sensor1.setMeasurementTimingBudget(20000);
 sensor2.setMeasurementTimingBudget(20000);
 sensor3.setMeasurementTimingBudget(20000);
  //sensor4.setMeasurementTimingBudget(200000);
#endif


  // // Set up motor pins
  // pinMode(MOTOR_CW, OUTPUT);
  // pinMode(MOTOR_CCW, OUTPUT);
  // digitalWrite(MOTOR_CW, 0);
  // digitalWrite(MOTOR_CCW, 0);


    myPID.SetMode(AUTOMATIC);

}


void loop() {
  int distance1 = sensor1.readRangeSingleMillimeters();
  int distance2 = sensor2.readRangeSingleMillimeters();
  int distance3 = sensor3.readRangeSingleMillimeters();
  //int distance4 = sensor4.readRangeSingleMillimeters();


  // Handle timeouts
  if (sensor1.timeoutOccurred()) {
  lcd.setCursor(0, 0);
  lcd.print("Timeout Sensor 1");
  // Serial.println("Timeout Sensor 1");
  return;
}
if (sensor2.timeoutOccurred()) {
  lcd.setCursor(0, 0);
  lcd.print("Timeout Sensor 2");
  // Serial.println("Timeout Sensor 2");
  return;
}
if (sensor3.timeoutOccurred()) {
  lcd.setCursor(0, 0);
  lcd.print("Timeout Sensor 3");
  // Serial.println("Timeout Sensor 3");
  return;
}
// if (sensor4.timeoutOccurred()) {
//   lcd.setCursor(0, 0);
//   lcd.print("Timeout Sensor 4");
//   Serial.println("Timeout Sensor 4");
//   return;
// }




  // Compute average distance
  int avgDistance = (distance1 + distance2 + distance3 /*+ distance4*/) / 3;
  



//New Control System
  Input = avgDistance;
  myPID.SetOutputLimits(50, 165); 
  myPID.Compute();
  pwm = map(Output, 50, 165, 205, 90);
  analogWrite(MOTOR_CW, pwm);

// //Alternate Control System
//   if (avgDistance >= Setpoint){
//     pwm = 205;
//   }
//   else{
//     pwm = 90;
//   }
//   analogWrite(MOTOR_CW, pwm);




//Old Control System
  // // Map distance to PWM (closer = higher PWM)
  // int pwmValue = map(avgDistance, MIN_DIST, MAX_DIST, MAX_PWM, 0);
  // pwmValue = constrain(pwmValue, 0, MAX_PWM); // Ensure within valid range


  // // Control Motor Speed Based on Distance
  // if (avgDistance < 150) {  
  //   // Forward motion when close
  //   analogWrite(MOTOR_CW, pwmValue);
  //   analogWrite(MOTOR_CCW, 0);
  // } else {  
  //   // Reverse motion when far
  //   analogWrite(MOTOR_CW, 0);
  //   analogWrite(MOTOR_CCW, pwmValue);
  // }


  // **Buzzer Activation when distance <= 60mm**
  if (avgDistance <= 60) {
    tone(BUZZER_PIN, 1000); // Play buzzer at 1kHz
  } else {
    noTone(BUZZER_PIN); // Stop buzzer
  }


  // Display on Serial Monitor
  // Serial.print("Distance1: ");
  // Serial.println(distance1);
  // Serial.print(" mm | Distance2: ");
  // Serial.println(distance2);
  // Serial.print("Distance3: ");
  // Serial.println(distance3);
  // //Serial.print(" mm | Distance4: ");
  //Serial.print(distance4);
  // Serial.print(" Avg: ");
  // Serial.println(avgDistance);
  // Serial.print(" PWM: ");
  // Serial.println(pwm);


  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Avg Dist: ");
  lcd.print(avgDistance);
  lcd.print("mm");


  lcd.setCursor(0, 1);
  lcd.print("PWM: ");
  lcd.print(pwm);

// // Sensor Calibration

//   int rawADC = analogRead(sensorPin);
//   float voltage = (rawADC / 1023.0) * VCC;
//   float current = (voltage - ZERO_CURRENT_V) / SENSITIVITY;

//   Serial.print("Raw ADC: ");
//   Serial.print(rawADC);
//   Serial.print(" | Voltage: ");
//   Serial.print(voltage, 3);
//   Serial.print(" V | Current: ");
//   Serial.print(current, 3);
  // Serial.println(" A");

  delay(50);
}


// int mapOutputToPWM(double controlSignal) {
//   double maxCurrent = 7.2;
//   double scaledPWM = (controlSignal / maxCurrent) * 255.0;
//   return (int)scaledPWM;
// }


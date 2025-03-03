#include <Wire.h>            // Used to establish serial communication on the I2C bus
#include "SparkFun_TMAG5273_Arduino_Library.h" // Used to send and receive specific information from our sensor

TMAG5273 sensor; // Initialize hall-effect sensor

// I2C default address
uint8_t i2cAddress = TMAG5273_I2C_ADDRESS_INITIAL;

void setup() 
{
  Wire.begin();
  // Start serial communication at 115200 baud
  Serial.begin(115200);  

  // Begin example of the magnetic sensor code (and add whitespace for easy reading)
  Serial.println("TMAG5273 Example 1: Basic Readings");
  Serial.println("");

  // Check if the sensor is connected
  if(sensor.isConnected() == 0)
  {
    Serial.println("Sensor is connected.");
  }
  else
  {
    Serial.println("Sensor is not connected.");
    // Print the Manufacturer ID even if not connected
    uint16_t manufacturerID = sensor.getManufacturerID();
    Serial.print("Manufacturer ID: 0x");
    Serial.println(manufacturerID, HEX);

    // Directly read raw data from the device ID registers using Wire library
    Wire.beginTransmission(i2cAddress);
    Wire.write(TMAG5273_REG_MANUFACTURER_ID_LSB);
    Wire.endTransmission();
    Wire.requestFrom((uint8_t)TMAG5273_ADDR, 2); // Explicit cast to uint8_t
    uint8_t lsb = Wire.read();
    uint8_t msb = Wire.read();
    uint16_t rawManufacturerID = (msb << 8) | lsb;
    Serial.print("Raw Device ID LSB: 0x");
    Serial.println(lsb, HEX);
    Serial.print("Raw Device ID MSB: 0x");
    Serial.println(msb, HEX);
    Serial.print("Raw Manufacturer ID: 0x");
    Serial.println(rawManufacturerID, HEX);

    while(1); // Runs forever
  }

  // If begin is successful (0), then start example
  if(sensor.begin(i2cAddress, Wire) == 1)
  {
    Serial.println("Begin");
  }
  else // Otherwise, infinite loop
  {
    Serial.println("Device failed to setup - Freezing code.");
    while(1); // Runs forever
  }
}

void loop() 
{
  // Checks if mag channels are on - turns on in setup
  if(sensor.getMagneticChannel() != 0) 
  {
    sensor.setTemperatureEn(true);

    float magX = sensor.getXData();
    float magY = sensor.getYData();
    float magZ = sensor.getZData();
    float temp = sensor.getTemp();

    Serial.print("(");
    Serial.print(magX);
    Serial.print(", ");
    Serial.print(magY);
    Serial.print(", ");
    Serial.print(magZ);
    Serial.println(") mT");
    Serial.print(temp);
    Serial.println(" C");
  }
  else
  {
    // If there is an issue, stop the magnetic readings and restart sensor/example
    Serial.println("Mag Channels disabled, stopping..");
    while(1);
  }

  delay(100);
}

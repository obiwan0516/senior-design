// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library

// The control pins for the LCD
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Color definitions
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define ORANGE  0xFD20
#define DARKGREEN 0x03E0

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Variables to store data from Uno
int distance1 = 0;
int distance2 = 0;
int distance3 = 0;
int distance4 = 0;
int avgDistance = 0;
int pwmValue = 0;
String motorDirection = "IDLE";
bool buzzerActive = false;

// System variables
unsigned long lastPacketTime = 0;
int packetCount = 0;
bool connectionActive = false;
String rawData = "";

void setup(void) {
  Serial.begin(9600);   // Communication with PC
  Serial1.begin(9600);  // Communication with Uno on pins 18(TX1) and 19(RX1)
  
  Serial.println(F("Distance Sensor Monitor"));

  // Initialize the display
  tft.reset();
  
  uint16_t identifier = tft.readID();
  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x4535) {
    Serial.println(F("Found LGDP4535 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else if(identifier == 0x0101) {
    identifier = 0x9341;
    Serial.println(F("Found 0x9341 LCD driver"));
  } else if(identifier == 0x1111) {
    identifier = 0x9328;
    Serial.println(F("Found 0x9328 LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    identifier = 0x9328;
  }
  
  tft.begin(identifier);
  
  // Initial display setup
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("Distance Sensor");
  tft.println("Monitor System");
  tft.setTextColor(WHITE);
  tft.println("---------------");
  tft.println("");
  tft.setTextColor(CYAN);
  tft.println("Waiting for data...");
}

void loop(void) {
  // Check for connection status
  if (millis() - lastPacketTime > 3000) {
    if (connectionActive) {
      connectionActive = false;
      drawStatusScreen();
    }
  }
  
  // Check for messages from Uno
  if (Serial1.available()) {
    char c = Serial1.read();
    
    // Add to buffer until end of packet
    if (c != ';') {
      rawData += c;
    } else {
      // Process complete packet
      processDataPacket(rawData);
      rawData = "";
      
      // Update connection status
      lastPacketTime = millis();
      if (!connectionActive) {
        connectionActive = true;
      }
      
      packetCount++;
      updateDisplay();
    }
  }
  
  // Brief delay
  delay(10);
}

void processDataPacket(String data) {
  // Parse data packet
  // Format: D1,D2,D3,D4,AVG,PWM,DIR,BUZ;
  
  int values[6];
  int valueIndex = 0;
  int commaIndex = -1;
  
  // Parse numeric values
  for (int i = 0; i < 6; i++) {
    int nextComma = data.indexOf(',', commaIndex + 1);
    if (nextComma != -1) {
      values[i] = data.substring(commaIndex + 1, nextComma).toInt();
      commaIndex = nextComma;
    } else {
      break;
    }
  }
  
  // Get motor direction
  int nextComma = data.indexOf(',', commaIndex + 1);
  if (nextComma != -1) {
    motorDirection = data.substring(commaIndex + 1, nextComma);
    commaIndex = nextComma;
  }
  
  // Get buzzer status
  int buzzerValue = data.substring(commaIndex + 1).toInt();
  buzzerActive = (buzzerValue == 1);
  
  // Assign values
  distance1 = values[0];
  distance2 = values[1];
  distance3 = values[2];
  distance4 = values[3];
  avgDistance = values[4];
  pwmValue = values[5];
  
  // Debug output
  Serial.print("Received: ");
  Serial.println(data);
}

void updateDisplay() {
  // Clear screen for new data
  tft.fillRect(0, 50, tft.width(), tft.height() - 50, BLACK);
  
  // Set starting position
  tft.setCursor(0, 50);
  
  // Display individual sensor readings
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Sensor 1: ");
  tft.print(distance1);
  tft.println(" mm");
  
  tft.print("Sensor 2: ");
  tft.print(distance2);
  tft.println(" mm");
  
  tft.print("Sensor 3: ");
  tft.print(distance3);
  tft.println(" mm");
  
  tft.print("Sensor 4: ");
  tft.print(distance4);
  tft.println(" mm");
  
  // Display average distance with larger text
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.setCursor(0, 100);
  tft.print("Avg: ");
  tft.print(avgDistance);
  tft.println(" mm");
  
  // Display PWM and direction
  tft.setTextColor(GREEN);
  tft.setCursor(0, 130);
  tft.print("PWM: ");
  tft.print(pwmValue);
  
  // Display motor direction
  tft.setCursor(0, 150);
  tft.print("Dir: ");
  if (motorDirection == "FWD") {
    tft.setTextColor(CYAN);
    tft.println("FORWARD ");
  } else if (motorDirection == "REV") {
    tft.setTextColor(MAGENTA);
    tft.println("REVERSE ");
  } else {
    tft.setTextColor(WHITE);
    tft.println(motorDirection);
  }
  
  // Display alert status
  if (buzzerActive) {
    tft.fillRect(0, 170, tft.width(), 30, RED);
    tft.setCursor(20, 175);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("PROXIMITY ALERT!");
  }
  
  // Display connection info at bottom
  tft.setCursor(0, 210);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Packets: ");
  tft.println(packetCount);
  
  tft.print("Last update: ");
  tft.print(lastPacketTime / 1000);
  tft.println("s");
}

void drawStatusScreen() {
  tft.fillRect(0, 50, tft.width(), tft.height() - 50, BLACK);
  tft.setCursor(0, 80);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.println("CONNECTION LOST");
  tft.println("");
  tft.setTextColor(YELLOW);
  tft.setTextSize(1);
  tft.println("Please check:");
  tft.println("- Uno power supply");
  tft.println("- Serial connections");
  tft.println("- Sensor operation");
  tft.println("");
  tft.println("Last data received at:");
  tft.print(lastPacketTime / 1000);
  tft.println(" seconds");
}
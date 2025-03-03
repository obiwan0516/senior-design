// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.
// SEE RELEVANT COMMENTS IN Elegoo_TFTLCD.h FOR SETUP.

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

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Variables to store received messages
String lastMessage = "Waiting for Uno...";
unsigned long lastMessageTime = 0;
int messageCount = 0;

void setup(void) {
  Serial.begin(9600);   // Communication with PC
  Serial1.begin(9600);  // Communication with Uno on pins 18(TX1) and 19(RX1)
  
  Serial.println(F("TFT LCD test with Arduino communication"));

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
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Arduino Communication");
  tft.println("-------------------");
  tft.println("");
  tft.setTextColor(YELLOW);
  tft.println(lastMessage);
}

void loop(void) {
  // Check for messages from Uno
  if (Serial1.available()) {
    // Read the incoming message
    String message = Serial1.readStringUntil('\n');
    messageCount++;
    
    // Update message and timestamp
    lastMessage = message;
    lastMessageTime = millis();
    
    // Print to serial monitor
    Serial.println("Received: " + message);
    
    // Update the display
    updateDisplay();
  } else {
    // If no message for 5 seconds, show waiting status
    if (millis() - lastMessageTime > 5000 && lastMessage != "Waiting for Uno...") {
      lastMessage = "Waiting for Uno...";
      updateDisplay();
    }
  }
  
  // Brief delay to prevent display flickering
  delay(100);
}

void updateDisplay() {
  // Clear the message area (not the whole screen to avoid flicker)
  tft.fillRect(0, 80, tft.width(), tft.height() - 80, BLACK);
  
  // Set cursor position for new message
  tft.setCursor(0, 80);
  
  // Display the message count
  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.print("Messages: ");
  tft.println(messageCount);
  tft.println("");
  
  // Display the last message
  tft.setTextColor(CYAN);
  tft.setTextSize(2);
  tft.println("Last message:");
  
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
  tft.println(lastMessage);
  
  // Display timestamp (seconds since last message)
  tft.setTextColor(MAGENTA);
  tft.setTextSize(1);
  tft.print("Received at: ");
  tft.print(lastMessageTime / 1000);
  tft.println(" seconds");
}
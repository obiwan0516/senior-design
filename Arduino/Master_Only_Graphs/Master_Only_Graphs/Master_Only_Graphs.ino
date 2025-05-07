// IMPORTANT: ELEGOO_TFTLCD LIBRARY MUST BE SPECIFICALLY
// CONFIGURED FOR EITHER THE TFT SHIELD OR THE BREAKOUT BOARD.

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>   // Touch screen library

// The control pins for the LCD
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0  
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// Touch screen pins
#define YP A2  // must be an analog pin
#define XM A3  // must be an analog pin
#define YM 8   // can be a digital pin
#define XP 9   // can be a digital pin

// Touch screen parameters
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940
#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define BUZZER_PIN 40

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
#define GRAY    0x8410
#define DARKBLUE 0x0010
#define LIGHTBLUE 0xAEDC

// Graph definitions - ADJUSTED POSITIONS TO MOVE UP
#define GRAPH_HEIGHT 60
#define GRAPH_WIDTH 180
#define GRAPH_X 30
#define GRAPH_Y1 75  // Moved up from 85
#define GRAPH_Y2 165 // Moved up from 185
#define MAX_DATA_POINTS 40

// Maximum distance value - CHANGED FROM 500 TO 250
#define MAX_DISTANCE 250

// Filtering settings
#define FILTER_ALPHA 0.2  // Low-pass filter coefficient (0-1): lower = more filtering
#define CALIBRATION_OFFSET 0  // Offset to match LCD display readings
#define CALIBRATION_SCALE 1.0  // Scale factor to match LCD display readings

// Initialize touch screen
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Variables to store data from Uno
int distance1 = 0;
int distance2 = 0;
int distance3 = 0;
int distance4 = 0;
int avgDistance = 0;
float filteredDistance = 0;  // Filtered average distance (float for better precision)
int pwmValue = 0;
float filteredPwm = 0;       // Filtered PWM value (float for better filtering)
int previousDistances[5] = {0, 0, 0, 0, 0};  // For median filtering
String motorDirection = "IDLE";
bool buzzerActive = false;

// System variables
unsigned long lastPacketTime = 0;
unsigned long responseTime = 0;
int packetCount = 0;
bool connectionActive = false;
String rawData = "";

// Graph variables
int distanceHistory[MAX_DATA_POINTS];
int pwmHistory[MAX_DATA_POINTS];
int historyIndex = 0;
bool graphFull = false;

//************************************************ SETUP ******************************************************************

void setup(void) {
  // Only keep Serial1 for Uno communication
  Serial1.begin(9600);  // Communication with Uno on pins 18(TX1) and 19(RX1)
  
  // Initialize the buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  
  // Initialize the display
  tft.reset();
  
  uint16_t identifier = tft.readID();
  // Identify LCD without Serial debugging
  if(identifier == 0x0101) {
    identifier = 0x9341;
  } else if(identifier == 0x1111) {
    identifier = 0x9328;
  }
  
  tft.begin(identifier);
  tft.setRotation(3);  // Set to landscape mode (0-3 for different rotations)
  
  // Initialize history arrays
  for(int i = 0; i < MAX_DATA_POINTS; i++) {
    distanceHistory[i] = 0;
    pwmHistory[i] = 0;
  }
  
  // Display loading animation
  showLoadingAnimation();
  
  // Show sensor graphs screen directly
  drawScreenLayout();
}

//************************************************ LOOP ******************************************************************

void loop(void) {
  // Handle sensor graphs mode directly
  handleGraphsMode();
  
  // Brief delay
  delay(10);
}

//************************************************ LOADING ANIMATION STARTUP ******************************************************************

void showLoadingAnimation() {
  // Clear the screen
  tft.fillScreen(BLACK);
  
  // Draw title
  tft.setCursor(50, 30);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("Distance Sensor");
  tft.setCursor(60, 50);
  tft.println("Monitor System");
  
  // Draw loading text
  tft.setCursor(90, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.println("LOADING...");
  
  // Draw progress bar outline
  tft.drawRect(60, 120, 200, 20, WHITE);
  
  // Animate progress bar
  for (int i = 0; i < 198; i += 2) {
    tft.fillRect(61, 121, i, 18, CYAN);
    delay(10);
  }
  
  // Display ready message
  tft.fillRect(60, 150, 200, 30, BLACK);
  tft.setCursor(90, 150);
  tft.setTextColor(GREEN);
  tft.setTextSize(2);
  tft.println("READY!");
  delay(500);
}

//************************************************ HandleGraphsMode ******************************************************************

void handleGraphsMode() {
  // Check for connection status
  if (millis() - lastPacketTime > 3000) {
    if (connectionActive) {
      connectionActive = false;
      drawStatusScreen();
    }
  }
  
  // Check for messages from Uno
  while (Serial1.available()) {
    char c = Serial1.read();
    
    // Add to buffer until end of packet
    if (c != ';') {
      rawData += c;
    } else {
      // Process complete packet
      unsigned long startProcess = millis();
      processDataPacket(rawData);
      rawData = "";
      
      // Calculate response time (processing time)
      responseTime = millis() - startProcess;
      
      // Update connection status
      lastPacketTime = millis();
      if (!connectionActive) {
        connectionActive = true;
        drawScreenLayout(); // Redraw layout after reconnection
      }
      
      packetCount++;
      
      // Add new data to history (use filtered values)
      distanceHistory[historyIndex] = filteredDistance;
      pwmHistory[historyIndex] = filteredPwm;
      historyIndex = (historyIndex + 1) % MAX_DATA_POINTS;
      if(historyIndex == 0) {
        graphFull = true;
      }
      
      // Update display with new data
      updateDisplay();
    }
  }
}

//************************************************ Draws Sensor Graphs ******************************************************************

void drawScreenLayout() {
  tft.fillScreen(BLACK);
  
  // Draw title - make it more compact
  tft.setCursor(70, 5);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("Distance Monitor");
  
  // Draw Distance Graph box
  tft.drawRect(GRAPH_X-5, GRAPH_Y1-15, GRAPH_WIDTH+10, GRAPH_HEIGHT+20, WHITE);
  tft.setCursor(GRAPH_X-3, GRAPH_Y1-13);
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.println("Distance (mm)");
  
  // Draw PWM Graph box
  tft.drawRect(GRAPH_X-5, GRAPH_Y2-15, GRAPH_WIDTH+10, GRAPH_HEIGHT+20, WHITE);
  tft.setCursor(GRAPH_X-3, GRAPH_Y2-13);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(1);
  tft.println("PWM Value (255-PWM)");
  
  // Draw grid lines for distance graph
  for(int y = 0; y < GRAPH_HEIGHT; y += 15) {
    for(int x = 0; x < GRAPH_WIDTH; x += 5) {
      tft.drawPixel(GRAPH_X + x, GRAPH_Y1 + y, GRAY);
    }
  }
  
  // Draw grid lines for PWM graph
  for(int y = 0; y < GRAPH_HEIGHT; y += 15) {
    for(int x = 0; x < GRAPH_WIDTH; x += 5) {
      tft.drawPixel(GRAPH_X + x, GRAPH_Y2 + y, GRAY);
    }
  }
  
  // Move indicators and readouts to ensure they fit on screen
  int rightColumnX = 220;
  
  // Direction indicators (initially both off) - MOVED UP
  tft.fillRect(rightColumnX, 80, 30, 20, DARKGREEN); // FWD button (off) - moved up from 90
  tft.fillRect(rightColumnX + 40, 80, 30, 20, DARKBLUE);  // REV button (off) - moved up from 90
  
  tft.setCursor(rightColumnX + 5, 85);  // Moved up from 95
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("FWD");
  
  tft.setCursor(rightColumnX + 45, 85);  // Moved up from 95
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("REV");
  
  // Sensor readouts area - MOVED UP
  tft.setCursor(rightColumnX, 110);  // Moved up from 120
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("S1:");
  
  tft.setCursor(rightColumnX + 40, 110);  // Moved up from 120
  tft.print("S2:");
  
  tft.setCursor(rightColumnX, 125);  // Moved up from 135
  tft.print("S3:");
  
  tft.setCursor(rightColumnX + 40, 125);  // Moved up from 135
  tft.print("S4:");
  
  // Display range indicator for distance graph
  tft.setCursor(GRAPH_X + GRAPH_WIDTH + 10, GRAPH_Y1);
  tft.setTextColor(CYAN);
  tft.print("0-");
  tft.print(MAX_DISTANCE);
  tft.print("mm");
}

//************************************************ Processes the packets from Arduino Uno ******************************************************************

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
  
  // Shift previous readings and add new one for median filtering
  for (int i = 0; i < 4; i++) {
    previousDistances[i] = previousDistances[i+1];
  }
  previousDistances[4] = avgDistance;
  
  // Make a copy for sorting
  int sortedDistances[5];
  for (int i = 0; i < 5; i++) {
    sortedDistances[i] = previousDistances[i];
  }
  
  // Simple bubble sort to find median
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4-i; j++) {
      if (sortedDistances[j] > sortedDistances[j+1]) {
        int tempVal = sortedDistances[j];
        sortedDistances[j] = sortedDistances[j+1];
        sortedDistances[j+1] = tempVal;
      }
    }
  }
  
  // Use median value (middle value) for filtering - better spike rejection
  int medianDistance = sortedDistances[2];
  
  // Apply low-pass filter to smooth out spikes
  if (filteredDistance == 0) {
    // First reading - initialize filter
    filteredDistance = medianDistance;
    filteredPwm = pwmValue;
  } else {
    // Apply exponential moving average filter
    filteredDistance = (1 - FILTER_ALPHA) * filteredDistance + FILTER_ALPHA * medianDistance;
    filteredPwm = (1 - FILTER_ALPHA) * filteredPwm + FILTER_ALPHA * pwmValue;
  }
  
  // Apply calibration to match LCD display readings
  filteredDistance = (filteredDistance * CALIBRATION_SCALE) + CALIBRATION_OFFSET;
  
  // Constrain filtered values to valid ranges
  filteredDistance = constrain(filteredDistance, 0, MAX_DISTANCE);
  filteredPwm = constrain(filteredPwm, 0, 255);

  // **Buzzer Activation when distance <= 60mm**
  if (filteredDistance <= 60) {  // Use filtered distance for more stable alerts
    tone(BUZZER_PIN, 1000); // Play buzzer at 1kHz
  } else {
    noTone(BUZZER_PIN); // Stop buzzer
  }
}

//************************************************ Updates the display when a measurement is logged ******************************************************************

void updateDisplay() {
  int rightColumnX = 220;
  
  // Update sensor readings - MOVED UP
  tft.fillRect(rightColumnX + 20, 110, 20, 8, BLACK);  // Moved up from 120
  tft.setCursor(rightColumnX + 20, 110);  // Moved up from 120
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(distance1);
  
  tft.fillRect(rightColumnX + 60, 110, 20, 8, BLACK);  // Moved up from 120
  tft.setCursor(rightColumnX + 60, 110);  // Moved up from 120
  tft.print(distance2);
  
  tft.fillRect(rightColumnX + 20, 125, 20, 8, BLACK);  // Moved up from 135
  tft.setCursor(rightColumnX + 20, 125);  // Moved up from 135
  tft.print(distance3);
  
  tft.fillRect(rightColumnX + 60, 125, 20, 8, BLACK);  // Moved up from 135
  tft.setCursor(rightColumnX + 60, 125);  // Moved up from 135
  tft.print(distance4);
  
  // Update average distance display - MOVED UP
  tft.fillRect(rightColumnX, 145, 95, 15, BLACK);  // Moved up from 155
  tft.setCursor(rightColumnX, 145);  // Moved up from 155
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.print("Raw: ");
  tft.print(avgDistance);
  tft.print(" mm");
  
  // Display filtered distance
  tft.fillRect(rightColumnX, 160, 95, 15, BLACK);
  tft.setCursor(rightColumnX, 160);
  tft.setTextColor(CYAN);
  tft.print("Filt: ");
  tft.print(int(filteredDistance));
  tft.print(" mm");
  
  // Update PWM display - MOVED UP and INVERTED (255-PWM)
  tft.fillRect(rightColumnX, 175, 95, 15, BLACK);  // Moved up position
  tft.setCursor(rightColumnX, 175);  // Moved up position
  tft.setTextColor(MAGENTA);
  tft.print("PWM: ");
  tft.print(255 - int(filteredPwm));  // Display inverted PWM value (255-PWM)
  
  // Update direction indicators - MOVED UP
  if (motorDirection == "FWD") {
    tft.fillRect(rightColumnX, 80, 30, 20, GREEN);  // FWD on - moved up from 90
    tft.fillRect(rightColumnX + 40, 80, 30, 20, DARKBLUE); // REV off - moved up from 90
  } else if (motorDirection == "REV") {
    tft.fillRect(rightColumnX, 80, 30, 20, DARKGREEN); // FWD off - moved up from 90
    tft.fillRect(rightColumnX + 40, 80, 30, 20, RED);     // REV on - moved up from 90
  } else {
    tft.fillRect(rightColumnX, 80, 30, 20, DARKGREEN); // FWD off - moved up from 90
    tft.fillRect(rightColumnX + 40, 80, 30, 20, DARKBLUE);  // REV off - moved up from 90
  }
  
  // Update direction text - MOVED UP
  tft.setCursor(rightColumnX + 5, 85);  // Moved up from 95
  tft.setTextColor(WHITE);
  tft.print("FWD");
  tft.setCursor(rightColumnX + 45, 85);  // Moved up from 95
  tft.print("REV");
  
  // Update proximity alert - MOVED UP
  if (buzzerActive) {
    tft.fillRect(rightColumnX, 195, 70, 30, RED);  // Moved down slightly to make room
    tft.setCursor(rightColumnX + 3, 205);  
    tft.setTextColor(WHITE);
    tft.print("ALERT!");
  } else {
    tft.fillRect(rightColumnX, 195, 70, 30, BLACK);  // Moved down slightly
  }
  
  // Update log - MOVED UP
  tft.fillRect(5, 250, tft.width()-10, 45, BLACK);  // Moved up from 270
  tft.setCursor(5, 250);  // Moved up from 270
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Packets: ");
  tft.print(packetCount);
  
  tft.setCursor(5, 265);  // Moved up from 285
  tft.print("Response: ");
  tft.print(responseTime);
  tft.print(" ms");
  
  tft.setCursor(5, 280);  // Moved up from 300
  tft.print("Last update: ");
  tft.print((millis() - lastPacketTime) / 1000.0, 1);
  tft.print(" s ago");
  
  // Update the distance graph with filtered data
  updateGraph(distanceHistory, GRAPH_X, GRAPH_Y1, GRAPH_WIDTH, GRAPH_HEIGHT, 0, MAX_DISTANCE, CYAN);
  
  // Update the PWM graph with filtered data - inverted (255-PWM)
  // Create temporary array with inverted PWM values
  int invertedPwmHistory[MAX_DATA_POINTS];
  for (int i = 0; i < MAX_DATA_POINTS; i++) {
    invertedPwmHistory[i] = 255 - pwmHistory[i];
  }
  updateGraph(invertedPwmHistory, GRAPH_X, GRAPH_Y2, GRAPH_WIDTH, GRAPH_HEIGHT, 0, 255, MAGENTA);
}

//************************************************ Clears the distance sensor graphs ******************************************************************

void updateGraph(int dataArray[], int x, int y, int width, int height, int minValue, int maxValue, uint16_t color) {
  // Clear the graph area
  tft.fillRect(x, y, width, height, BLACK);
  
  // Redraw grid lines
  for(int gy = 0; gy < height; gy += 15) {
    for(int gx = 0; gx < width; gx += 5) {
      tft.drawPixel(x + gx, y + gy, GRAY);
    }
  }
  
  // Draw axis values
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.setCursor(x - 25, y);
  tft.print(maxValue);
  tft.setCursor(x - 25, y + height - 8);
  tft.print(minValue);
  
  // Number of points to plot
  int numPoints = graphFull ? MAX_DATA_POINTS : historyIndex;
  
  // Don't try to plot if we have no data
  if(numPoints == 0) return;
  
  // Calculate scaling factors
  float xScale = (float)width / (numPoints - 1);
  float yScale = (float)height / (maxValue - minValue);
  
  // Plot the data points
  for (int i = 0; i < numPoints - 1; i++) {
    int idx1 = (historyIndex - numPoints + i) % MAX_DATA_POINTS;
    if (idx1 < 0) idx1 += MAX_DATA_POINTS;
    
    int idx2 = (idx1 + 1) % MAX_DATA_POINTS;
    
    int value1 = dataArray[idx1];
    int value2 = dataArray[idx2];
    
    // Constrain values to the range
    value1 = constrain(value1, minValue, maxValue);
    value2 = constrain(value2, minValue, maxValue);
    
    // Calculate plot coordinates
    int x1 = x + i * xScale;
    int y1 = y + height - (value1 - minValue) * yScale;
    int x2 = x + (i + 1) * xScale;
    int y2 = y + height - (value2 - minValue) * yScale;
    
    // Draw the line segment
    tft.drawLine(x1, y1, x2, y2, color);
  }
}

//************************************************ Draws the CONNECTION LOST screen ******************************************************************

void drawStatusScreen() {
  tft.fillRect(0, 50, tft.width(), 180, BLACK);  // Adjusted size for new layout
  tft.setCursor(80, 100);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.println("CONNECTION LOST");
  tft.setCursor(50, 130);
  tft.setTextColor(YELLOW);
  tft.setTextSize(1);
  tft.println("Please check device connections");
  
  tft.setCursor(5, 250);  // Moved up from 270
  tft.setTextColor(WHITE);
  tft.print("Last packet received: ");
  tft.print(lastPacketTime / 1000);
  tft.println(" s");
  
  tft.setCursor(5, 265);  // Moved up from 285
  tft.print("Total packets: ");
  tft.println(packetCount);
}
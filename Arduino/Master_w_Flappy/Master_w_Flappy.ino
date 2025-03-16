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

// Menu button definitions
#define MENU_BUTTON_X 5
#define MENU_BUTTON_Y 5
#define MENU_BUTTON_W 50
#define MENU_BUTTON_H 30

// Main menu button definitions
#define MAIN_BTN_X 40
#define MAIN_BTN_W 240
#define MAIN_BTN_H 40
#define MAIN_BTN_Y1 70
#define MAIN_BTN_Y2 130
#define MAIN_BTN_Y3 190

// Application state
#define STATE_MENU 0
#define STATE_GRAPHS 1
#define STATE_FLAPPY_BIRD 2
#define STATE_MAG_LEVITATION 3
int currentState = STATE_MENU;

//*************************************************** Flappy bird game defines ********************************************************
#define BIRD_WIDTH 20
#define BIRD_HEIGHT 15
#define PIPE_WIDTH 30
#define PIPE_GAP 70
#define GRAVITY 0.6
#define FLAP_FORCE -8
#define PIPE_SPEED 3
#define MAX_PIPES 3

// Bird variables
float birdY = 120;
float birdVelocity = 0;
int birdX = 60;
bool gameActive = false;
int gameScore = 0;
float prevBirdY = 120;  // Track previous position for proper erasing

// Pipe variables
struct Pipe {
  int x;
  int prevX;  // Previous x position for proper erasing
  int gapY;
  bool counted;
};
Pipe pipes[MAX_PIPES];

unsigned long lastFrameTime = 0;
#define FRAME_DELAY 20 // ~23 fps
// ********************************************************************************************************************************

// Initialize touch screen
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Touch debounce variables
unsigned long lastTouchTime = 0;
#define TOUCH_DEBOUNCE 300 // ms

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
unsigned long responseTime = 0;
int packetCount = 0;
bool connectionActive = false;
String rawData = "";

// Graph variables
#define GRAPH_HEIGHT 60
#define GRAPH_WIDTH 180
#define GRAPH_X 30
#define GRAPH_Y1 85
#define GRAPH_Y2 185
#define MAX_DATA_POINTS 40

int distanceHistory[MAX_DATA_POINTS];
int pwmHistory[MAX_DATA_POINTS];
int historyIndex = 0;
bool graphFull = false;

//************************************************ SETUP ******************************************************************

void setup(void) {
  // Add this line to the top of your setup() function, after initializing Serial1:
  randomSeed(analogRead(A2)); // Use an unconnected analog pin for randomness

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
  tft.setRotation(1);  // Set to landscape mode (0-3 for different rotations)
  
  // Initialize history arrays
  for(int i = 0; i < MAX_DATA_POINTS; i++) {
    distanceHistory[i] = 0;
    pwmHistory[i] = 0;
  }
  
  // Display loading animation
  showLoadingAnimation();
  
  // Show main menu
  drawMainMenu();
}

//************************************************ LOOP ******************************************************************

void loop(void) {
  // Check touch input for menu navigation
  handleTouchInput();
  
  // Handle current state
  switch(currentState) {
    case STATE_MENU:
      // No continuous updates needed for menu
      break;
      
    case STATE_GRAPHS:
      handleGraphsMode();
      break;
      
    case STATE_FLAPPY_BIRD:
      handleFlappyBirdGame();
      break;
      
    case STATE_MAG_LEVITATION:
      // Placeholder for magnetic levitation animation
      // handleMagLevitation();
      break;
  }
  
  // Brief delay
  delay(10);
}

//************************************************ HandleTouchInput ******************************************************************

void handleTouchInput() {
  // Read touch input
  TSPoint p = ts.getPoint();
  
  // Restore pins that were used for touchscreen
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  
  // Check if there's a valid touch
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    // Debounce touch
    if (millis() - lastTouchTime < TOUCH_DEBOUNCE) {
      return;
    }
    lastTouchTime = millis();
    
    // Map touch coordinates to screen coordinates
    // For rotation 1 (landscape)
    int x = map(p.y, TS_MINY, TS_MAXY, 0, tft.width()); 
    int y = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());
    
    // Handle touch based on current state
    switch(currentState) {
      case STATE_MENU:
        // Check which menu button was pressed
        if (isTouchInRect(x, y, MAIN_BTN_X, MAIN_BTN_Y1, MAIN_BTN_W, MAIN_BTN_H)) {
          // Sensor Graphs button
          currentState = STATE_GRAPHS;
          drawScreenLayout();
        } 
        else if (isTouchInRect(x, y, MAIN_BTN_X, MAIN_BTN_Y2, MAIN_BTN_W, MAIN_BTN_H)) {
          // Flappy Bird button
          currentState = STATE_FLAPPY_BIRD;
          drawFlappyBirdScreen();
        }
        else if (isTouchInRect(x, y, MAIN_BTN_X, MAIN_BTN_Y3, MAIN_BTN_W, MAIN_BTN_H)) {
          // Magnetic Levitation button
          currentState = STATE_MAG_LEVITATION;
          drawMagLevitationScreen();
        }
        break;
        
      case STATE_GRAPHS:
      case STATE_MAG_LEVITATION:
        // Check if menu button was pressed
        if (isTouchInRect(x, y, MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H)) {
          // Return to main menu
          currentState = STATE_MENU;
          drawMainMenu();
        }
        break;
      
      case STATE_FLAPPY_BIRD:
        // Check if menu button was pressed
        if (isTouchInRect(x, y, MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H)) {
          // Return to main menu
          currentState = STATE_MENU;
          noTone(BUZZER_PIN); // Stop any game sounds
          drawMainMenu();
        } else {
          // Any other touch in Flappy Bird either starts the game or makes the bird flap
          if (!gameActive) {
            // Clear game over messages
            tft.fillRect(60, 120, 200, 100, LIGHTBLUE);
            
            // Reset bird and pipes
            initFlappyBird();
            
            // Display initial score
            tft.fillRect(150, 20, 80, 20, LIGHTBLUE);
            tft.setCursor(150, 20);
            tft.setTextColor(BLACK);
            tft.setTextSize(2);
            tft.print("Score: 0");
            
            // Start the game
            gameActive = true;
          } else {
            // Make the bird flap (jump)
            birdVelocity = FLAP_FORCE;
            // Play flap sound
            tone(BUZZER_PIN, 800, 30);
          }
        }
        break;
    }
  }
}

// Helper function to check if touch is within a rectangle
bool isTouchInRect(int touchX, int touchY, int rectX, int rectY, int rectW, int rectH) {
  return (touchX >= rectX && touchX <= rectX + rectW && 
          touchY >= rectY && touchY <= rectY + rectH);
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
      
      // Add new data to history
      distanceHistory[historyIndex] = avgDistance;
      pwmHistory[historyIndex] = pwmValue;
      historyIndex = (historyIndex + 1) % MAX_DATA_POINTS;
      if(historyIndex == 0) {
        graphFull = true;
      }
      
      // Update display with new data
      updateDisplay();
    }
  }
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

//************************************************ MAIN MENU ******************************************************************

void drawMainMenu() {
  // Clear the screen
  tft.fillScreen(BLACK);
  
  // Draw title
  tft.setCursor(70, 20);
  tft.setTextColor(YELLOW);
  tft.setTextSize(2);
  tft.println("MAIN MENU");
  
  // Draw menu buttons
  // Button 1: Sensor Graphs
  drawMenuButton(MAIN_BTN_X, MAIN_BTN_Y1, MAIN_BTN_W, MAIN_BTN_H, "Sensor Graphs", BLUE);
  
  // Button 2: Flappy Bird Game
  drawMenuButton(MAIN_BTN_X, MAIN_BTN_Y2, MAIN_BTN_W, MAIN_BTN_H, "Flappy Bird", GREEN);
  
  // Button 3: Magnetic Levitation
  drawMenuButton(MAIN_BTN_X, MAIN_BTN_Y3, MAIN_BTN_W, MAIN_BTN_H, "Mag Levitation", MAGENTA);
}

//************************************************ Draws the buttons ******************************************************************

void drawMenuButton(int x, int y, int w, int h, const char* label, uint16_t color) {
  tft.fillRoundRect(x, y, w, h, 8, color);
  tft.drawRoundRect(x, y, w, h, 8, WHITE);
  
  // Calculate text length
  int textLength = strlen(label) * 12; // Approximate width with text size 2
  
  // Center text on button
  int textX = x + (w / 2) - (textLength / 2);
  int textY = y + (h / 2) - 8;
  
  tft.setCursor(textX, textY);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print(label);
}

//************************************************ Draws Sensor Graphs ******************************************************************

void drawScreenLayout() {
  tft.fillScreen(BLACK);
  
  // Draw "Menu" button in top-left corner
  tft.fillRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, BLUE);
  tft.drawRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, WHITE);
  tft.setCursor(MENU_BUTTON_X + 7, MENU_BUTTON_Y + 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("MENU");
  
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
  tft.println("PWM Value");
  
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
  
  // Direction indicators (initially both off)
  tft.fillRect(rightColumnX, 90, 30, 20, DARKGREEN); // FWD button (off)
  tft.fillRect(rightColumnX + 40, 90, 30, 20, DARKBLUE);  // REV button (off)
  
  tft.setCursor(rightColumnX + 5, 95);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("FWD");
  
  tft.setCursor(rightColumnX + 45, 95);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("REV");
  
  // Sensor readouts area
  tft.setCursor(rightColumnX, 120);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("S1:");
  
  tft.setCursor(rightColumnX + 40, 120);
  tft.print("S2:");
  
  tft.setCursor(rightColumnX, 135);
  tft.print("S3:");
  
  tft.setCursor(rightColumnX + 40, 135);
  tft.print("S4:");
  
  // Log area at bottom
  tft.drawRect(0, 255, tft.width(), 65, WHITE);
  tft.setCursor(5, 258);
  tft.setTextColor(YELLOW);
  tft.print("SYSTEM LOG");
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

  // **Buzzer Activation when distance <= 60mm**
  if (avgDistance <= 60) {
    tone(BUZZER_PIN, 1000); // Play buzzer at 1kHz
  } else {
    noTone(BUZZER_PIN); // Stop buzzer
  }
}

//************************************************ Updates the display when a measurement is logged ******************************************************************

void updateDisplay() {
  int rightColumnX = 220;
  
  // Update sensor readings
  tft.fillRect(rightColumnX + 20, 120, 20, 8, BLACK);
  tft.setCursor(rightColumnX + 20, 120);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print(distance1);
  
  tft.fillRect(rightColumnX + 60, 120, 20, 8, BLACK);
  tft.setCursor(rightColumnX + 60, 120);
  tft.print(distance2);
  
  tft.fillRect(rightColumnX + 20, 135, 20, 8, BLACK);
  tft.setCursor(rightColumnX + 20, 135);
  tft.print(distance3);
  
  tft.fillRect(rightColumnX + 60, 135, 20, 8, BLACK);
  tft.setCursor(rightColumnX + 60, 135);
  tft.print(distance4);
  
  // Update average distance display
  tft.fillRect(rightColumnX, 155, 95, 15, BLACK);
  tft.setCursor(rightColumnX, 155);
  tft.setTextColor(CYAN);
  tft.setTextSize(1);
  tft.print("Avg: ");
  tft.print(avgDistance);
  tft.print(" mm");
  
  // Update PWM display
  tft.fillRect(rightColumnX, 170, 95, 15, BLACK);
  tft.setCursor(rightColumnX, 170);
  tft.setTextColor(MAGENTA);
  tft.print("PWM: ");
  tft.print(pwmValue);
  
  // Update direction indicators
  if (motorDirection == "FWD") {
    tft.fillRect(rightColumnX, 90, 30, 20, GREEN);  // FWD on
    tft.fillRect(rightColumnX + 40, 90, 30, 20, DARKBLUE); // REV off
  } else if (motorDirection == "REV") {
    tft.fillRect(rightColumnX, 90, 30, 20, DARKGREEN); // FWD off
    tft.fillRect(rightColumnX + 40, 90, 30, 20, RED);     // REV on
  } else {
    tft.fillRect(rightColumnX, 90, 30, 20, DARKGREEN); // FWD off
    tft.fillRect(rightColumnX + 40, 90, 30, 20, DARKBLUE);  // REV off
  }
  
  // Update direction text
  tft.setCursor(rightColumnX + 5, 95);
  tft.setTextColor(WHITE);
  tft.print("FWD");
  tft.setCursor(rightColumnX + 45, 95);
  tft.print("REV");
  
  // Update proximity alert
  if (buzzerActive) {
    tft.fillRect(rightColumnX, 200, 70, 30, RED);
    tft.setCursor(rightColumnX + 3, 210);
    tft.setTextColor(WHITE);
    tft.print("ALERT!");
  } else {
    tft.fillRect(rightColumnX, 200, 70, 30, BLACK);
  }
  
  // Update log
  tft.fillRect(5, 270, tft.width()-10, 45, BLACK);
  tft.setCursor(5, 270);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Packets: ");
  tft.print(packetCount);
  
  tft.setCursor(5, 285);
  tft.print("Response: ");
  tft.print(responseTime);
  tft.print(" ms");
  
  tft.setCursor(5, 300);
  tft.print("Last update: ");
  tft.print((millis() - lastPacketTime) / 1000.0, 1);
  tft.print(" s ago");
  
  // Update the distance graph
  updateGraph(distanceHistory, GRAPH_X, GRAPH_Y1, GRAPH_WIDTH, GRAPH_HEIGHT, 0, 500, CYAN);
  
  // Update the PWM graph
  updateGraph(pwmHistory, GRAPH_X, GRAPH_Y2, GRAPH_WIDTH, GRAPH_HEIGHT, 0, 255, MAGENTA);
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
  tft.fillRect(0, 50, tft.width(), 200, BLACK);
  tft.setCursor(80, 100);
  tft.setTextColor(RED);
  tft.setTextSize(2);
  tft.println("CONNECTION LOST");
  tft.setCursor(50, 130);
  tft.setTextColor(YELLOW);
  tft.setTextSize(1);
  tft.println("Please check device connections");
  
  tft.setCursor(5, 270);
  tft.setTextColor(WHITE);
  tft.print("Last packet received: ");
  tft.print(lastPacketTime / 1000);
  tft.println(" s");
  
  tft.setCursor(5, 285);
  tft.print("Total packets: ");
  tft.println(packetCount);
}

//************************************************ Placeholder screen for flappy bird ******************************************************************

// Placeholder screens for other features
// void drawFlappyBirdScreen() {
//   tft.fillScreen(BLACK);
  
//   // Draw "Menu" button in top-left corner
//   tft.fillRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, BLUE);
//   tft.drawRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, WHITE);
//   tft.setCursor(MENU_BUTTON_X + 7, MENU_BUTTON_Y + 10);
//   tft.setTextColor(WHITE);
//   tft.setTextSize(1);
//   tft.print("MENU");
  
//   // Draw game title
//   tft.setCursor(80, 20);
//   tft.setTextColor(YELLOW);
//   tft.setTextSize(2);
//   tft.print("FLAPPY BIRD");
  
//   // Placeholder for game screen
//   tft.drawRect(20, 50, 280, 160, GREEN);
//   tft.setCursor(50, 120);
//   tft.setTextColor(WHITE);
//   tft.print("Game implementation");
// }

// Initialize the Flappy Bird game
void initFlappyBird() {
  // Reset bird position and velocity
  birdY = 120;
  prevBirdY = 120;
  birdVelocity = 0;
  gameActive = false;
  gameScore = 0;
  
  // Initialize pipes off screen
  for (int i = 0; i < MAX_PIPES; i++) {
    pipes[i].x = tft.width() + (i * (tft.width() + PIPE_WIDTH)) / 2;
    pipes[i].prevX = pipes[i].x;  // Initialize prevX
    pipes[i].gapY = 50 + random(100);
    pipes[i].counted = false;
  }
}

// Draw the Flappy Bird game screen
void drawFlappyBirdScreen() {
  tft.fillScreen(LIGHTBLUE);
  
  // Draw "Menu" button in top-left corner
  tft.fillRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, BLUE);
  tft.drawRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, WHITE);
  tft.setCursor(MENU_BUTTON_X + 7, MENU_BUTTON_Y + 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("MENU");
  
  // Draw game title
  tft.setCursor(80, 20);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.print("FLAPPY BIRD");
  
  // Draw start instructions
  tft.setCursor(60, 160);
  tft.setTextColor(BLACK);
  tft.setTextSize(1);
  tft.print("Touch screen to start");
  tft.setCursor(60, 180);
  tft.print("and make the bird flap");
  
  // Initialize the game
  initFlappyBird();
  
  // Draw initial bird
  drawBird();
}

// Draw the bird and erase previous position
void drawBird() {
  // Erase old bird position (only erase what's needed)
  if (prevBirdY != birdY) {
    // If moving up, erase bottom part
    if (prevBirdY > birdY) {
      tft.fillRect(birdX, birdY + BIRD_HEIGHT, BIRD_WIDTH, prevBirdY - birdY, LIGHTBLUE);
    } 
    // If moving down, erase top part
    else if (prevBirdY < birdY) {
      tft.fillRect(birdX, prevBirdY, BIRD_WIDTH, birdY - prevBirdY, LIGHTBLUE);
    }
    
    prevBirdY = birdY;  // Update previous position
  }
  
  // Draw new bird
  tft.fillRoundRect(birdX, birdY, BIRD_WIDTH, BIRD_HEIGHT, 5, YELLOW);
  // Draw eye
  tft.fillCircle(birdX + 15, birdY + 5, 2, BLACK);
  // Draw wing
  tft.fillRoundRect(birdX + 5, birdY + 10, 10, 5, 2, ORANGE);
}

// Draw a pipe
void drawPipe(Pipe &pipe) {
  // Erase old pipe position (only what's needed)
  if (pipe.prevX != pipe.x) {
    // Erase the trailing edge
    tft.fillRect(pipe.x + PIPE_WIDTH, 0, pipe.prevX - pipe.x, tft.height(), LIGHTBLUE);
    // If moving fast, erase the full previous area
    if (pipe.prevX - pipe.x > PIPE_SPEED * 2) {
      tft.fillRect(pipe.prevX, 0, PIPE_WIDTH, tft.height(), LIGHTBLUE);
    }
    pipe.prevX = pipe.x;
  }
  
  // Draw upper pipe
  tft.fillRect(pipe.x, 0, PIPE_WIDTH, pipe.gapY - PIPE_GAP/2, GREEN);
  // Cap for upper pipe
  tft.fillRect(pipe.x - 3, pipe.gapY - PIPE_GAP/2 - 5, PIPE_WIDTH + 6, 5, DARKGREEN);
  
  // Draw lower pipe
  int lowerPipeY = pipe.gapY + PIPE_GAP/2;
  tft.fillRect(pipe.x, lowerPipeY, PIPE_WIDTH, tft.height() - lowerPipeY, GREEN);
  // Cap for lower pipe
  tft.fillRect(pipe.x - 3, lowerPipeY, PIPE_WIDTH + 6, 5, DARKGREEN);
}

// Check for collision with pipes
bool checkCollision() {
  // Check for floor/ceiling collision
  if (birdY <= 0 || birdY + BIRD_HEIGHT >= tft.height()) {
    return true;
  }
  
  // Check for pipe collisions
  for (int i = 0; i < MAX_PIPES; i++) {
    if (birdX + BIRD_WIDTH > pipes[i].x && birdX < pipes[i].x + PIPE_WIDTH) {
      // Bird is within pipe x-range, check y-range
      if (birdY < pipes[i].gapY - PIPE_GAP/2 || birdY + BIRD_HEIGHT > pipes[i].gapY + PIPE_GAP/2) {
        return true; // Collision detected
      }
    }
  }
  
  return false; // No collision
}

// Update game state
void updateFlappyBird() {
  // Only update game at appropriate frame rate
  if (millis() - lastFrameTime < FRAME_DELAY) {
    return;
  }
  lastFrameTime = millis();
  
  if (!gameActive) {
    return;
  }
  
  // Apply gravity
  birdVelocity += GRAVITY;
  birdY += birdVelocity;
  
  // Update pipes
  bool needScoreUpdate = false;
  for (int i = 0; i < MAX_PIPES; i++) {
    // Store previous position
    pipes[i].prevX = pipes[i].x;
    
    // Move pipe
    pipes[i].x -= PIPE_SPEED;
    
    // If pipe moves off screen, reset it
    if (pipes[i].x + PIPE_WIDTH < 0) {
      // Erase fully before resetting
      tft.fillRect(pipes[i].x, 0, PIPE_WIDTH + 4, tft.height(), LIGHTBLUE);
      
      pipes[i].x = tft.width();
      pipes[i].prevX = pipes[i].x;
      pipes[i].gapY = 50 + random(100);
      pipes[i].counted = false;
    }
    
    // Check if bird has passed pipe
    if (!pipes[i].counted && birdX > pipes[i].x + PIPE_WIDTH) {
      gameScore++;
      pipes[i].counted = true;
      needScoreUpdate = true;
      
      // Play a beep for score
      tone(BUZZER_PIN, 1500, 50);
    }
    
    // Draw new pipe position
    drawPipe(pipes[i]);
  }
  
  // Update score display if needed
  if (needScoreUpdate) {
    tft.fillRect(150, 20, 80, 20, LIGHTBLUE);
    tft.setCursor(150, 20);
    tft.setTextColor(BLACK);
    tft.setTextSize(2);
    tft.print("Score: ");
    tft.print(gameScore);
  }
  
  // Draw bird
  drawBird();
  
  // Check for collision
  if (checkCollision()) {
    gameActive = false;
    
    // Play game over sound
    tone(BUZZER_PIN, 300, 500);
    
    // Display game over message
    tft.setCursor(90, 120);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.print("GAME OVER");
    
    tft.setCursor(60, 150);
    tft.setTextColor(BLACK);
    tft.setTextSize(1);
    tft.print("Touch screen to play again");
    
    tft.setCursor(80, 180);
    tft.print("Final score: ");
    tft.print(gameScore);
  }
}

// Handle Flappy Bird game
void handleFlappyBirdGame() {
  updateFlappyBird();
}

//************************************************ Placeholder screen for animation ******************************************************************

void drawMagLevitationScreen() {
  tft.fillScreen(BLACK);
  
  // Draw "Menu" button in top-left corner
  tft.fillRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, BLUE);
  tft.drawRoundRect(MENU_BUTTON_X, MENU_BUTTON_Y, MENU_BUTTON_W, MENU_BUTTON_H, 4, WHITE);
  tft.setCursor(MENU_BUTTON_X + 7, MENU_BUTTON_Y + 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("MENU");
  
  // Draw feature title
  tft.setCursor(40, 20);
  tft.setTextColor(MAGENTA);
  tft.setTextSize(2);
  tft.print("MAG LEVITATION");
  
  // Placeholder for animation area
  tft.drawRect(20, 50, 280, 160, MAGENTA);
  tft.setCursor(50, 120);
  tft.setTextColor(WHITE);
  tft.print("Animation coming soon");
}
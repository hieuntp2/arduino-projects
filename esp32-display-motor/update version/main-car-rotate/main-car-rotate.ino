#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Adafruit_MPU6050 mpu;

// Variables to store initial angles and accelerations
float initialAngleX = 0;
float initialAngleY = 0;
float initialAngleZ = 0;
unsigned long lastTime = 0;
float angleX = 0;
float angleY = 0;
float angleZ = 0;
float accelerationX = 0;
float temperature = 0;

int16_t screenWidth, screenHeight;
int16_t centerX1, centerY1, centerX2, centerY2;
int16_t radius, diameter;

void setup(void) {
  tft.init();
  tft.setRotation(1);
  setup_mpu();

  // Get initial MPU6050 readings
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Set initial angles
  initialAngleX = g.gyro.x;
  initialAngleY = g.gyro.y;
  initialAngleZ = g.gyro.z;
  lastTime = millis();

  // Initialize screen dimensions and circle parameters
  screenWidth = tft.width();
  screenHeight = tft.height();
  centerX1 = screenWidth / 4;
  centerY1 = screenHeight / 2;
  centerX2 = 3 * screenWidth / 4;
  centerY2 = screenHeight / 2;
  radius = screenWidth / 4;
  diameter = 2 * radius;
  
  // Set initial screen background
  tft.fillScreen(TFT_BLACK);
}

void loop() {
  getMpuData();
  updateDisplay();
  delay(1000); // Adjust delay to balance power consumption and update frequency
}

void setup_mpu(){
  if (!mpu.begin()) {
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  delay(100);
}

void getMpuData(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // time in seconds
  lastTime = currentTime;

  angleX += (g.gyro.x - initialAngleX) * dt;
  angleY += (g.gyro.y - initialAngleY) * dt;
  angleZ += (g.gyro.z - initialAngleZ) * dt;

  accelerationX = a.acceleration.x;
  temperature = temp.temperature;
}

void updateDisplay() {
  // Clear only the areas that need to be updated
  tft.fillCircle(centerX1, centerY1, radius, TFT_BLACK);
  tft.fillCircle(centerX2, centerY2, radius, TFT_BLACK);
  
  // Draw circles and lines
  drawCircleAndLine(centerX1, centerY1, radius, angleX, TFT_YELLOW);
  drawCircleAndLine(centerX2, centerY2, radius, angleZ, TFT_WHITE);

  // Display X acceleration value and temperature between circles
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(4);
  tft.setCursor(screenWidth / 2 - 30, 0); // Adjust position if necessary
  tft.printf("%.2f m/s", accelerationX);
  
  tft.setCursor(screenWidth / 2 - 30, 110); // Position below acceleration text
  tft.printf("%.2f C", temperature);

  // Display rotation values at the center of each circle
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextFont(4);
  tft.setCursor(centerX1 - 30, centerY1 - 15);
  tft.printf("%.2f", angleX);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(4);
  tft.setCursor(centerX2 - 30, centerY2 - 15);
  tft.printf("%.2f", angleZ);
}

void drawCircleAndLine(int centerX, int centerY, int radius, float angle, uint16_t color) {
  // Draw the circle
  tft.drawCircle(centerX, centerY, radius, color);

  // Draw rotating line
  float rad = angle; // Assuming angle is in radians
  int16_t lineX1 = centerX - radius * cos(rad);
  int16_t lineY1 = centerY - radius * sin(rad);
  int16_t lineX2 = centerX + radius * cos(rad);
  int16_t lineY2 = centerY + radius * sin(rad);

  drawThickLine(lineX1, lineY1, lineX2, lineY2, color, 10);
}

void drawThickLine(int x0, int y0, int x1, int y1, uint16_t color, uint8_t thickness) {
  float dx = x1 - x0;
  float dy = y1 - y0;
  float d = sqrt(dx*dx + dy*dy);
  float ux = dx / d;
  float uy = dy / d;
  int16_t x2 = x0 - uy * thickness / 2;
  int16_t y2 = y0 + ux * thickness / 2;
  int16_t x3 = x1 - uy * thickness / 2;
  int16_t y3 = y1 + ux * thickness / 2;
  int16_t x4 = x0 + uy * thickness / 2;
  int16_t y4 = y0 - ux * thickness / 2;
  int16_t x5 = x1 + uy * thickness / 2;
  int16_t y5 = y1 - ux * thickness / 2;

  tft.fillTriangle(x2, y2, x3, y3, x4, y4, color);
  tft.fillTriangle(x3, y3, x4, y4, x5, y5, color);
}

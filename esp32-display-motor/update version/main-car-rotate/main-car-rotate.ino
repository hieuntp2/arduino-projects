#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
Adafruit_MPU6050 mpu;

// Variables to store initial angles
float initialAngleX = 0;
float initialAngleY = 0;
unsigned long lastTime = 0;
float angleX = 0;
float angleY = 0;

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
  lastTime = millis();
}

void loop() {
  getMpuData();
  delay(200); // Adjust delay to balance power consumption and update frequency
}

void setup_mpu(){
  Serial.begin(115200);
  while (!Serial)
    delay(10); // will pause Zero, Leonardo, etc until serial console opens

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
  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // time in seconds
  lastTime = currentTime;

  /* Integrate the gyroscope data -> angle */
  angleX += (g.gyro.x - initialAngleX) * dt;
  angleY += (g.gyro.y - initialAngleY) * dt;

  /* Clear only the part of the display we are updating */
  int16_t screenWidth = tft.width();
  int16_t screenHeight = tft.height();
  int16_t centerX1 = screenWidth / 4;
  int16_t centerY1 = screenHeight / 2;
  int16_t centerX2 = 3 * screenWidth / 4;
  int16_t centerY2 = screenHeight / 2;
  int16_t radius = screenWidth / 4;
  int16_t diameter = 2 * radius;

  tft.fillScreen(TFT_BLACK); // Black background

  // Draw circles
  tft.fillCircle(centerX1, centerY1, radius, TFT_BLACK);
  tft.drawCircle(centerX1, centerY1, radius, TFT_YELLOW);
  tft.fillCircle(centerX2, centerY2, radius, TFT_BLACK);
  tft.drawCircle(centerX2, centerY2, radius, TFT_WHITE);

  // Draw thicker lines for X rotation
  int16_t lineX1Start = centerX1 - radius * cos(angleX);
  int16_t lineY1Start = centerY1 - radius * sin(angleX);
  int16_t lineX1End = centerX1 + radius * cos(angleX);
  int16_t lineY1End = centerY1 + radius * sin(angleX);
  drawThickLine(centerX1, centerY1, lineX1End, lineY1End, TFT_YELLOW, 10);

  // Draw thicker lines for Y rotation
  int16_t lineX2Start = centerX2 - radius * cos(angleY);
  int16_t lineY2Start = centerY2 - radius * sin(angleY);
  int16_t lineX2End = centerX2 + radius * cos(angleY);
  int16_t lineY2End = centerY2 + radius * sin(angleY);
  drawThickLine(centerX2, centerY2, lineX2End, lineY2End, TFT_WHITE, 10);

  // Display rotation values at the center of each circle
  tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.setTextFont(4);
  tft.setCursor(centerX1 - 30, centerY1 - 15);
  tft.printf("%.2f", angleX);

  tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setTextFont(4);
  tft.setCursor(centerX2 - 30, centerY2 - 15);
  tft.printf("%.2f", angleY);
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

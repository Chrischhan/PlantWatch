
#include "My_MicroOLED.h"

#include "Images.h"

My_MicroOLED::My_MicroOLED(int8_t RST)
: Adafruit_SSD1306(RST)
{
  clearDisplay();
  drawXBitmap(0, 0, logoCHKR, logoCHKR_width, logoCHKR_height, WHITE);
}

void My_MicroOLED::version()
{
  DEBUG_MICROOLED("My_MicroOLED: Version 0.1");
}

void My_MicroOLED::drawPixel(int16_t x, int16_t y, uint16_t color)
{
  Adafruit_SSD1306::drawPixel(x, y, color);
}

void My_MicroOLED::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
  Adafruit_SSD1306::drawFastHLine(x, y, w, color);
}

void My_MicroOLED::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  Adafruit_SSD1306::fillCircle(x0, y0, r, color);
}

void My_MicroOLED::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  Adafruit_SSD1306::fillRect(x, y, w, h, color);
}

void My_MicroOLED::drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress) {
  uint16_t radius = height / 2;
  uint16_t xRadius = x + radius;
  uint16_t yRadius = y + radius;
  uint16_t doubleRadius = 2 * radius;
  uint16_t innerRadius = radius - 2;

  setTextColor(WHITE);
  drawCircleQuads(xRadius, yRadius, radius, 0b00000110);
  drawFastHLine(xRadius, y, width - doubleRadius + 1);
  drawFastHLine(xRadius, y + height, width - doubleRadius + 1);
  drawCircleQuads(x + width - radius, yRadius, radius, 0b00001001);

  uint16_t maxProgressWidth = (width - doubleRadius - 1) * progress / 100;

  fillCircle(xRadius, yRadius, innerRadius);
  fillRect(xRadius + 1, y + 2, maxProgressWidth, height - 3);
  fillCircle(xRadius + maxProgressWidth, yRadius, innerRadius);
}

void My_MicroOLED::drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads) {
  int16_t x = 0, y = radius;
  int16_t dp = 1 - radius;
  while (x < y) {
    if (dp < 0)
      dp = dp + 2 * (++x) + 3;
    else
      dp = dp + 2 * (++x) - 2 * (--y) + 5;
    if (quads & 0x1) {
      drawPixel(x0 + x, y0 - y);
      drawPixel(x0 + y, y0 - x);
    }
    if (quads & 0x2) {
      drawPixel(x0 - y, y0 - x);
      drawPixel(x0 - x, y0 - y);
    }
    if (quads & 0x4) {
      drawPixel(x0 - y, y0 + x);
      drawPixel(x0 - x, y0 + y);
    }
    if (quads & 0x8) {
      drawPixel(x0 + x, y0 + y);
      drawPixel(x0 + y, y0 + x);
    }
  }
  if (quads & 0x1 && quads & 0x8) {
    drawPixel(x0 + radius, y0);
  }
  if (quads & 0x4 && quads & 0x8) {
    drawPixel(x0, y0 + radius);
  }
  if (quads & 0x2 && quads & 0x4) {
    drawPixel(x0 - radius, y0);
  }
  if (quads & 0x1 && quads & 0x2) {
    drawPixel(x0, y0 - radius);
  }
}

void My_MicroOLED::drawTemperature(const float& temperature)
{
  drawXBitmap(0, 0, ico_temp, ico_width, ico_height, WHITE);
  String tempText = String(temperature, 1);
  String text = tempText + " C";

  char charBuf[10];
  tempText.toCharArray(charBuf, 10);

  int16_t newx, newy;
  uint16_t wid, hig;

  setCursor(6,0);
  getTextBounds(charBuf, getCursorX(), getCursorY(), &newx, &newy, &wid, &hig);
  drawChar(newx + wid + 1, newy, (unsigned char)247, WHITE, BLACK, 1);
  print(text);
}

void My_MicroOLED::drawAirHumidity(const float& airHumidity)
{
  drawXBitmap(0, 10, ico2_cloud, ico2_width, ico2_height, WHITE);
  String tempText = String(airHumidity, 1);
  String text = tempText + "%";
  setCursor(ico2_width + 1, 10);
  print(text);
}

void My_MicroOLED::updateDisplay(const float& voltage, const float& temperature, const float& airHumidity)
{
  clearDisplay();
  if (voltage >= 3.9)
  {
    drawXBitmap(52, 0, logo_bat_3_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else if (voltage >= 3.6)
  {
    drawXBitmap(52, 0, logo_bat_2_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else if (voltage >= 3.3)
  {
    drawXBitmap(52, 0, logo_bat_1_3, logo_bat_width, logo_bat_height, WHITE);
  }
  else
  {
    drawXBitmap(52, 0, logo_bat_0_3, logo_bat_width, logo_bat_height, WHITE);
  }

  // text display tests
  setTextSize(1);
  setTextColor(WHITE);
  drawTemperature(temperature);
  drawAirHumidity(airHumidity);

  drawXBitmap(0, 20, ico_drop, ico_width, ico_height, WHITE);


  display();
}
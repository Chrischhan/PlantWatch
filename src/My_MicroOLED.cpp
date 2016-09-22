
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

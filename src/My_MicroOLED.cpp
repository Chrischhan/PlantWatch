
#include "My_MicroOLED.h"

#define logoisax_width 64
#define logoisax_height 24
static const unsigned char PROGMEM logoisax[] = {
   0x00, 0x00, 0x1f, 0x00, 0xe0, 0x01, 0x00, 0x00, 0x1e, 0xe0, 0xff, 0x00,
   0xfc, 0x0f, 0x00, 0x3e, 0x1e, 0xf8, 0xff, 0x03, 0xff, 0x1f, 0x00, 0x1e,
   0x1e, 0xfc, 0xff, 0x03, 0xff, 0x3f, 0x00, 0x1f, 0x00, 0x7c, 0xc0, 0x03,
   0x0f, 0x7c, 0x80, 0x0f, 0x00, 0x3e, 0x00, 0x01, 0x02, 0xf8, 0xc0, 0x07,
   0x00, 0x1e, 0x00, 0x00, 0x00, 0xf0, 0xc1, 0x03, 0x00, 0x3e, 0x00, 0x00,
   0x00, 0xe0, 0xe3, 0x01, 0x1e, 0x3e, 0x00, 0x00, 0x00, 0xc0, 0xf7, 0x01,
   0x1e, 0xfc, 0x01, 0x00, 0x00, 0x80, 0xff, 0x00, 0x1e, 0xfc, 0x1f, 0x00,
   0xf8, 0x00, 0x7f, 0x00, 0x1e, 0xf8, 0x7f, 0x00, 0xfe, 0x00, 0x3f, 0x00,
   0x1e, 0xe0, 0xff, 0x01, 0xff, 0x01, 0x3e, 0x00, 0x1e, 0x00, 0xff, 0x83,
   0x1f, 0x00, 0x7f, 0x00, 0x1e, 0x00, 0xf0, 0xc7, 0x07, 0x80, 0x7f, 0x00,
   0x1e, 0x00, 0xc0, 0xc7, 0x03, 0x80, 0xff, 0x00, 0x1e, 0x00, 0x80, 0xc7,
   0x03, 0xc0, 0xf7, 0x01, 0x1e, 0x00, 0x80, 0xc7, 0x03, 0xe0, 0xe3, 0x03,
   0x1e, 0x0c, 0x80, 0xc7, 0x03, 0xf0, 0xc1, 0x07, 0x1e, 0x1e, 0xc0, 0x87,
   0x07, 0xf8, 0x80, 0x07, 0x1e, 0xff, 0xff, 0x83, 0x1f, 0x7f, 0x00, 0x0f,
   0x1e, 0xfe, 0xff, 0x01, 0xff, 0x3f, 0x00, 0x1f, 0x1e, 0xf8, 0xff, 0x00,
   0xfe, 0x1f, 0x00, 0x3e, 0x00, 0xe0, 0x3f, 0x00, 0xf8, 0x07, 0x00, 0x7c };

My_MicroOLED::My_MicroOLED(int8_t RST)
: Adafruit_SSD1306(RST)
{
  clearDisplay();
  drawXBitmap(0, 0, logoisax, 64, 24, WHITE);
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

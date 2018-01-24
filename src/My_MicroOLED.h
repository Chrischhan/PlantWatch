#ifndef MY_MICROOLED_H
#define MY_MICROOLED_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef DEBUG_MICROOLED
#define DEBUG_MICROOLED(...)
#endif

#if (SSD1306_LCDHEIGHT != 48)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

class My_MicroOLED : public Adafruit_SSD1306{
public:
  // Constructor(s)
  My_MicroOLED(int8_t RST = -1);

  void version();

  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color = WHITE);
  // Fill circle
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color = WHITE);
  // Fill the rectangle
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color = WHITE);

  void drawPixel(int16_t x, int16_t y, uint16_t color = WHITE);

  // Draws a rounded progress bar with the outer dimensions given by width and height. Progress is
  // a unsigned byte value between 0 and 100
  void drawProgressBar(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t progress);
  // Draw all Quadrants specified in the quads bit mask
  void drawCircleQuads(int16_t x0, int16_t y0, int16_t radius, uint8_t quads);

  void drawTemperature(const float& temperature);
  void drawAirHumidity(const float& airHumidity);
  void updateDisplay(const float& voltage, const float& temperature, const float& airHumidity);
};

#endif // MY_MICROOLED_H

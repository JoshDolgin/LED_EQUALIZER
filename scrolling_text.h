
#include "wifi_setup.h"
#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#define NUM_LEDS (XRES * YRES)

CRGB leds[NUM_LEDS];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(leds, XRES, YRES,
    NEO_MATRIX_BOTTOM    + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG );


void scroll_msg(String msg, uint8_t scroll_count)
{
  uint8_t len = msg.length();
  if (scroll_count)
  {
    uint8_t count = 0;
    while (count < scroll_count)
    {
      matrix->setBrightness(brightness);
      matrix->setTextColor(matrix->Color(red, green, blue));
      matrix->fillScreen(0);
      matrix->setCursor(x, 0);
      matrix->print(msg);
      if (--x < -(len * 6)) {
        count++;
        x = matrix->width();
        if (++pass >= 3) pass = 0;
      }
      matrix->show();
      delay(210 - scroll_speed);
    }
  }
  else
  {
    matrix->setBrightness(brightness);
    matrix->setTextColor(matrix->Color(red, green, blue));
    matrix->fillScreen(0);
    matrix->setCursor(x, 0);
    matrix->print(msg);
    if (--x < -(len * 6)) {
      x = matrix->width();
      if (++pass >= 3) pass = 0;
    }
    matrix->show();
    delay(210 - scroll_speed);
  }
}

#include <arduinoFFT.h>
#include "scrolling_text.h"
#define EEPROM_SIZE 1

#define SAMPLES         1024          // Must be a power of 2
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE       1000          // Sensitivity Control
#define AUDIO_IN_PIN    35            // Signal in on this pin
#define LED_PIN         5             // LED strip data


#define COLOR_ORDER     GRB           // If colours look wrong, play with this
#define CHIPSET         WS2812B       // LED strip type
#define MAX_MILLIAMPS   2000          //Max Amperage
#define BLACK CRGB(0, 0, 0)
#define WHITE CRGB(255, 255, 255)

#define LED_VOLTS       5             // Usually 5 or 12
#define NUM_BANDS       8            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE           500           // Used as a crude noise filter, values below this are ignored


// Sampling and FFT stuff
unsigned int sampling_period_us;
int bandValues[XRES] = {};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;
arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);
int mapping_values_max[XRES] = {100, 400, 350, 440, 450, 500, 500, 625};
int mapping_values_min[XRES] = {5, 10, 10, 15, 30, 30, 60, 140};
int barHeights[XRES] = {};

// FastLED stuff
CRGB rainbow_pallet[XRES] = {CRGB(255, 0, 0), CRGB(255, 80, 0), CRGB(255, 175, 0), CRGB(255, 255, 0), CRGB(0, 255, 0), CRGB(0, 148, 254), CRGB(0, 0, 255), CRGB(125, 0, 255)};
CRGB purple_pallet[XRES] = {CRGB(132, 51, 172), CRGB(160, 66, 198), CRGB(176, 97, 208), CRGB(192, 129, 217), CRGB(208, 160, 227), CRGB(223, 192, 236), CRGB(239, 223, 246), CRGB(255, 255, 255)};
CRGB blue_pallet[XRES] = {CRGB(151, 249, 253), CRGB(5, 230, 255), CRGB(0, 208, 255), CRGB(0, 184, 255), CRGB(0, 157, 255), CRGB(0, 127, 255), CRGB(0, 88, 255), CRGB(42, 0, 255)};
CRGB green_pallet[XRES] = {CRGB(147, 255, 129), CRGB(136, 255, 117), CRGB(124, 255, 105), CRGB(111, 255, 92), CRGB(96, 255, 78), CRGB(78, 255, 62), CRGB(55, 255, 42), CRGB(0, 255, 0)};
CRGB yellow_pallet[XRES] = {CRGB(255, 251, 138), CRGB(255, 250, 125), CRGB(255, 250, 112), CRGB(255, 249, 98), CRGB(255, 248, 83), CRGB(255, 248, 66), CRGB(255, 247, 45), CRGB(255, 246, 0)};
CRGB orange_pallet[XRES] = {CRGB(255, 193, 138), CRGB(255, 182, 119), CRGB(255, 170, 101), CRGB(255, 158, 83), CRGB(255, 145, 66), CRGB(255, 132, 48), CRGB(255, 118, 28), CRGB(255, 102, 0)};
CRGB red_pallet[XRES] = {CRGB(255, 0, 0), CRGB(238, 0, 1), CRGB(221, 0, 2), CRGB(204, 0, 2), CRGB(188, 0, 2), CRGB(172, 0, 2), CRGB(156, 0, 2), CRGB(140, 0, 0)};
CRGB blank_pallet[XRES] = {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};
int hue = 0;

CRGB pallet_list[NUM_PALLETS][8] = {rainbow_pallet, purple_pallet, blue_pallet, red_pallet, blank_pallet, rainbow_pallet, blank_pallet, blank_pallet, blank_pallet, blank_pallet, blank_pallet};
CRGB peak_list[NUM_PALLETS] = {WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE};
CRGB back_list[NUM_PALLETS] = {BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK};

int peaks[XRES] = {};
uint8_t count = 1;
#define COUNT_DELAY 5;

#define BUTTON_OUT 2
#define BUTTON_IN 15

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(2000);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
  FastLED.setMaxPowerInVoltsAndMilliamps(LED_VOLTS, MAX_MILLIAMPS);
  FastLED.setBrightness(brightness);
  FastLED.clear();

  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(brightness);
  matrix->setTextColor(matrix->Color(red, green, blue));

  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQ));

  pinMode(BUTTON_OUT, OUTPUT);
  digitalWrite(BUTTON_OUT, HIGH);
  pinMode(BUTTON_IN, INPUT_PULLDOWN);

  //Read pallet count from eeprom
  EEPROM.begin(EEPROM_SIZE);
  pallet_count = EEPROM.read(0);
] //setup webserver and scroll IP once
  setupWebServer();
  show_IP(1);
}

void loop() {
  //readjust brightness
  FastLED.setBrightness(brightness);
  if (digitalRead(BUTTON_IN))
  {
    //increase pallet count and write new value to eeprom
    delay(250);
    pallet_count = (pallet_count + 1) % NUM_PALLETS;
    EEPROM.write(0, pallet_count);
    EEPROM.commit();
  }
  if (pallet_count <= 10)
  {
    //equalize
    get_bar_heights();
    visualize();
  }
  else if(pallet_count == 11)
  {
    //keep scrolling text
    scroll_msg(text, 0);
  }
  else
  {
    //keep scrolling IP
    show_IP(0);
  }
  FastLED.show();

}

void show_IP(uint8_t num_times)
{
  char strIP[16] = "               ";
  IPAddress ip = WiFi.localIP();
  ip.toString().toCharArray(strIP, 16);
  scroll_msg(strIP, num_times);
}

void get_bar_heights()
{
  // Reset bandValues[]
  for (int i = 0; i < NUM_BANDS; i++) {
    bandValues[i] = 0;
  }

  // Sample the audio pin
  for (int i = 0; i < SAMPLES; i++) {
    newTime = micros();
    vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
    vImag[i] = 0;
    while ((micros() - newTime) < sampling_period_us) {
      /* chill */
    }
  }

  // Compute FFT
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();

  // Analyse FFT results
  for (int i = 2; i < (SAMPLES / 2); i++) {    
    if (vReal[i] > NOISE) {                   

      //8 bands, 12kHz top band
      if (i <= 3 )           bandValues[0]  += (int)vReal[i];
      if (i > 3   && i <= 6  ) bandValues[1]  += (int)vReal[i];
      if (i > 6   && i <= 13 ) bandValues[2]  += (int)vReal[i];
      if (i > 13  && i <= 27 ) bandValues[3]  += (int)vReal[i];
      if (i > 27  && i <= 55 ) bandValues[4]  += (int)vReal[i];
      if (i > 55  && i <= 112) bandValues[5]  += (int)vReal[i];
      if (i > 112 && i <= 229) bandValues[6]  += (int)vReal[i];
      if (i > 229          ) bandValues[7]  += (int)vReal[i];
    }
  }

  // Process the FFT data into bar heights
  for (byte band = 0; band < NUM_BANDS; band++) {

    // Scale the bars for the display
    int barHeight = constrain(bandValues[band] / AMPLITUDE, mapping_values_min[band], mapping_values_max[band]);
    barHeight = map(barHeight, mapping_values_min[band], mapping_values_max[band], 0, YRES - 1);

    barHeights[band] = barHeight;
  }
}

void visualize()
{
  CRGB pallet[XRES] = pallet_list[pallet_count];
  CRGB peak_color = peak_list[pallet_count];
  CRGB back_color = back_list[pallet_count];
  //Serial.println(pallet_count);
  if (pallet_count < 5)
  {
    //adjust the peaks
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        //push the peaks higher
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        //drops the peaks 1 square
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        //annoying to deal with the serpentine LEDS, if the column is even its easy if it is odd you have to backtrack
        if (i % 2 == 0)
        {
          if (j < barHeights[i])
          {
            leds[(i * XRES) + j] = pallet[i];
          }
          else if (j == peaks[i])
          {
            leds[(i * XRES) + j] = peak_color;
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j < barHeights[i])
          {
            leds[((i + 1) * XRES) - j - 1] = pallet[i];
          }
          else if (j == peaks[i])
          {
            leds[((i + 1) * XRES) - j - 1] =  peak_color;
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count < 6)
  {
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j == peaks[i])
          {
            leds[(i * XRES) + j] = pallet[i];
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j == peaks[i])
          {
            leds[((i + 1) * XRES) - j - 1] =  pallet[i];
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count == 6)
  {
    EVERY_N_MILLISECONDS(50)
    {
      hue += 1;
    }
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j == peaks[i])
          {
            leds[(i * XRES) + j] = CHSV(hue, 255, brightness);
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j == peaks[i])
          {
            leds[((i + 1) * XRES) - j - 1] =  CHSV(hue, 255, brightness);
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count == 7)
  {
    EVERY_N_MILLISECONDS(5)
    {
      hue += 1;
    }
    int peak_color = hue;
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j == peaks[i])
          {
            peak_color = (peak_color + 255 / XRES) % 255;
            leds[(i * XRES) + j] = CHSV(peak_color, 255, brightness);
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j == peaks[i])
          {
            peak_color = (peak_color + 255 / XRES) % 255;
            leds[((i + 1) * XRES) - j - 1] =  CHSV(peak_color, 255, brightness);
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count == 8)
  {
    EVERY_N_MILLISECONDS(50)
    {
      hue += 1;
    }
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j < barHeights[i])
          {
            leds[(i * XRES) + j] = CHSV(hue, 255, brightness);
          }
          else if (j == peaks[i])
          {
            leds[(i * XRES) + j] = peak_color;
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j < barHeights[i])
          {
            leds[((i + 1) * XRES) - j - 1] = CHSV(hue, 255, brightness);
          }
          else if (j == peaks[i])
          {
            leds[((i + 1) * XRES) - j - 1] =  peak_color;
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count == 9)
  {
    EVERY_N_MILLISECONDS(50)
    {
      hue += 1;
    }
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j < barHeights[i])
          {
            leds[(i * XRES) + j] = CHSV(hue, 255, brightness);
          }
          else if (j == peaks[i])
          {
            leds[(i * XRES) + j] = CHSV(255 - hue, 255, brightness);
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j < barHeights[i])
          {
            leds[((i + 1) * XRES) - j - 1] = CHSV(hue, 255, brightness);
          }
          else if (j == peaks[i])
          {
            leds[((i + 1) * XRES) - j - 1] = CHSV(255 - hue, 255, brightness);
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }
      }
    }
  }
  else if (pallet_count == 10)
  {
    EVERY_N_MILLISECONDS(5)
    {
      hue += 1;
    }
    int peak_color = hue;
    for (uint8_t i = 0; i < XRES; i++)
    {
      if (barHeights[i] > peaks[i])
      {
        peaks[i] = barHeights[i];
      }
      else if (count == 0 && peaks[i] != 0)
      {
        peaks[i] = peaks[i] - 1;
      }


      for (uint8_t j = 0; j < YRES; j++)
      {
        if (i % 2 == 0)
        {
          if (j < barHeights[i])
          {
            leds[(i * XRES) + j] = WHITE;
          }
          else if (j == peaks[i])
          {
            peak_color = (peak_color + 255 / XRES) % 255;
            leds[(i * XRES) + j] = CHSV(peak_color, 255, brightness);
          }
          else
          {
            leds[(i * XRES) + j] = back_color;
          }
        }
        else
        {
          if (j < barHeights[i])
          {
            leds[((i + 1) * XRES) - j - 1] = WHITE;
          }
          else if (j == peaks[i])
          {
            peak_color = (peak_color + 255 / XRES) % 255;
            leds[((i + 1) * XRES) - j - 1] = CHSV(peak_color, 255, brightness);
          }
          else
          {
            leds[((i + 1) * XRES) - j - 1] = back_color;
          }
        }

      }
    }
  }
  count = (count + 1) % COUNT_DELAY;
}

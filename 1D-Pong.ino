#include <FastLED.h>

#define NUM_LEDS 72
#define BRIGHTNESS 64

#define DATA_PIN 2

CRGB ledsBall[NUM_LEDS];
CRGB leds[NUM_LEDS];

void setup() {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(1000);

  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200);
}

unsigned long lastTime = 0;

const int minSpeed = 7;
const int maxSpeed = 30;
int limit = NUM_LEDS << 8;
long pos = 0;
int speed = 10;

void loop() {
  auto time = millis();

  auto delta = (int)(time - lastTime);
  pos += speed * delta;

  if (pos >= limit) {
    pos = limit - 1;
    speed = -random(minSpeed, maxSpeed);
    Serial.println(speed);
  }
  if (pos < 0) {
    pos = 0;
    speed = random(minSpeed, maxSpeed);
    Serial.println(speed);
  }

  fadeToBlackBy(ledsBall, NUM_LEDS, 10);

  auto ledPos = (pos >> 8) % NUM_LEDS;
  ledsBall[(ledPos) % NUM_LEDS] = CRGB::Aqua;

  FastLED.clear();
  for (int i = 0; i < 9; i++) {
    leds[i] = CRGB::DarkGreen;
    leds[NUM_LEDS - i - 1] = CRGB::DarkGreen;
  }

  leds[NUM_LEDS / 2] = CRGB::DarkRed;
  leds[(NUM_LEDS / 2) - 1] = CRGB::DarkRed;

  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] += ledsBall[i];

  FastLED.show();

  lastTime = time;
}

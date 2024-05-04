#include <FastLED.h>

#define NUM_LEDS 72
#define BRIGHTNESS 255

const int ledDataPin = 2;
const int pinA = 3;
const int pinB = 4;

CRGB ledsBall[NUM_LEDS];
CRGB leds[NUM_LEDS];

enum State {
  WaitForStart,
  AtoB,
  BtoA,
};

State state = WaitForStart;

void setup() {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(100);

  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);

  FastLED.addLeds<WS2811, ledDataPin, RGB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.begin(115200);
}

unsigned long lastTime = 0;

const int minSpeed = 7;
const int maxSpeed = 30;
int limit = (NUM_LEDS << 8) - 1;
long pos = 0;
int speed = 0;

void loop() {
  auto aPressed = digitalRead(pinA) == LOW;
  auto bPressed = digitalRead(pinB) == LOW;

  switch (state) {
    case WaitForStart:
      if (aPressed && pos == 0) {
        speed = 10;
        state = AtoB;
      }
      if (bPressed && pos == limit) {
        speed = -10;
        state = BtoA;
      }
      break;
    case AtoB:
      if (bPressed && pos > limit - (9 << 8)) {
        auto dif = limit - pos;
        speed = -GetSpeed(dif);
        state = BtoA;
      }
      break;
    case BtoA:
      if (aPressed && pos < (9 << 8)) {
        auto dif = pos;
        speed = GetSpeed(dif);
        state = AtoB;
      }
      break;
  }

  auto time = millis();

  auto delta = (int)(time - lastTime);
  pos += speed * delta;

  if (pos > limit) {
    Win(0);
  }
  if (pos < 0) {
    Win(limit);
  }

  fadeToBlackBy(ledsBall, NUM_LEDS, 10);

  auto ledPos = (pos >> 8) % NUM_LEDS;
  ledsBall[(ledPos) % NUM_LEDS] = CRGB::Aqua;

  SendToLeds();

  lastTime = time;
}

void SendToLeds() {
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
}

int GetSpeed(int dif) {
  return map(dif, 9 << 8, 0, 10, 30);
}

void Win(int newPos) {
  FastLED.clear();
  auto colorA = state == BtoA?CRGB::Red:CRGB::Green;
  auto colorB = state == BtoA?CRGB::Green:CRGB::Red;

  for (int i = 0; i < NUM_LEDS / 3; i++) {
    leds[i] = colorA;
    leds[NUM_LEDS - i - 1] = colorB;
  }

  FastLED.show();
  delay(1200);

  pos = newPos;
  speed = 0;
  state = WaitForStart;
  lastTime = millis();
}
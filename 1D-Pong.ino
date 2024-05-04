#include <FastLED.h>

#define NUM_LEDS 72
#define BRIGHTNESS 63

const int ledDataPin = 2;
const int pinA = 4;
const int pinB = 3;

CRGB ledsBall[NUM_LEDS];
CRGB leds[NUM_LEDS];

enum State {
  WaitForStart,
  AtoB,
  BtoA
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

const int minSpeed = 10;
const int maxSpeed = 30;
const int startSpeed = 20;
const int winMax = 8;
int limit = (NUM_LEDS << 8) - 1;
long pos = 0;
int speed = 0;
int invalidPos = -1;

bool wasPressed = false;

int winA = 0;
int winB = 0;


void loop() {
  auto aPressed = digitalRead(pinA) == LOW;
  auto bPressed = digitalRead(pinB) == LOW;

  switch (state) {
    case WaitForStart:
      if (aPressed && pos == 0) {
        speed = startSpeed;
        state = AtoB;
      }
      if (bPressed && pos == limit) {
        speed = -startSpeed;
        state = BtoA;
      }
      lastTime = millis();
      break;
    case AtoB:
      if (bPressed && !wasPressed)
        if (pos > limit - (9 << 8)) {
          auto dif = limit - pos;
          speed = -GetSpeed(dif);
          state = BtoA;
        } else if (pos > limit / 2) {
          invalidPos = pos;
          wasPressed = true;
        }

      break;
    case BtoA:
      if (aPressed && !wasPressed)
        if (pos < (9 << 8)) {
          auto dif = pos;
          speed = GetSpeed(dif);
          state = AtoB;
        } else if (pos < limit / 2) {
          invalidPos = pos;
          wasPressed = true;
        }
      break;
  }

  auto time = millis();

  auto delta = (int)(time - lastTime);

  pos += speed * delta;

  if (pos > limit) {
    winA++;
    Win(0);
  } else if (pos < 0) {
    winB++;
    Win(limit);
  } else {
    fadeToBlackBy(ledsBall, NUM_LEDS, 20);

    ledsBall[PosToLed(pos)] = CRGB::Aqua;

    SendToLeds();
  }
  lastTime = time;
}

void SendToLeds() {
  FastLED.clear();
  for (int i = 0; i < 9; i++) {
    leds[i] = CRGB::DarkGreen;
    leds[NUM_LEDS - i - 1] = CRGB::DarkGreen;
  }

  if (invalidPos > -1) {
    auto color = (millis() / 100) % 2 == 0 ? CRGB::HotPink : CRGB::Black;
    leds[PosToLed(invalidPos)] = color;
  }

  leds[NUM_LEDS / 2] = CRGB::DarkRed;
  leds[(NUM_LEDS / 2) - 1] = CRGB::DarkRed;

  for (int i = 0; i < NUM_LEDS; i++)
    leds[i] += ledsBall[i];

  FastLED.show();
}

int GetSpeed(int dif) {
  if (dif < 256)
    return maxSpeed * 2;
  return map(dif, 9 << 8, 0, minSpeed, maxSpeed);
}

int PosToLed(int pos) {
  return (pos >> 8) % NUM_LEDS;
}

void Win(int newPos) {
  FastLED.clear();
  auto colorA = state == BtoA ? CRGB::Red : CRGB::Green;
  auto colorB = state == BtoA ? CRGB::Green : CRGB::Red;

  for (int i = 0; i < winA; i++)
    leds[i] = colorA;

  for (int i = 0; i < winB; i++)
    leds[NUM_LEDS - i - 1] = colorB;


  FastLED.show();
  for (int i = 0; i < 12; i++) {
    if (invalidPos > -1) {
      auto color = (millis() / 100) % 2 == 0 ? CRGB::HotPink : CRGB::Black;
      leds[PosToLed(invalidPos)] = color;
      FastLED.show();
    }
    delay(100);
  }

  if (winA == winMax || winB == winMax)
    GameWinAnimation();

  pos = newPos;
  speed = 0;
  state = WaitForStart;
  lastTime = millis();
  invalidPos = -1;
  wasPressed = false;
}

void GameWinAnimation() {
  bool aWon = (winA == winMax);
  for (int i = 0; i < 48; i++) {
    FastLED.clear();

    auto r = i < 24 ? i : 48 - i;
    for (int j = 0; j < r; j++) {
      auto idx = aWon ? j : NUM_LEDS - j - 1;
      leds[idx] = CRGB::Orange;
    }
    FastLED.show();
    delay(40);
  }
  winA = winB = 0;
}

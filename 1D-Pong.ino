#include <FastLED.h>

const int numLeds = 72;

const int ledDataPin = 2;
const int pinButtonA = 3;
const int pinButtonB = 5;
const int pinLedA = 4;
const int pinLedB = 6;
const int pinBrightness = 7;

CRGB ledsBall[numLeds];
CRGB leds[numLeds];

int brightness = 6;

enum State {
  WaitForStart,
  AtoB,
  BtoA
};

State state = WaitForStart;

void setup() {
  // sanity check delay - allows reprogramming if accidently blowing power w/leds
  delay(100);

  pinMode(pinButtonA, INPUT_PULLUP);
  pinMode(pinButtonB, INPUT_PULLUP);
  pinMode(pinLedA, OUTPUT);
  pinMode(pinLedB, OUTPUT);
  pinMode(pinBrightness, INPUT_PULLUP);

  FastLED.addLeds<WS2811, ledDataPin, RGB>(leds, numLeds);
  SetBrightness();

  Serial.begin(115200);
}

unsigned long lastTime = 0;
unsigned long lastBrightnessTime = 0;

const int minSpeed = 10;
const int maxSpeed = 30;
const int startSpeed = 20;
const int winMax = 8;
int limit = (numLeds << 8) - 1;
long pos = 0;
int speed = 0;
int invalidPos = -1;

bool wasPressed = false;

int winA = 0;
int winB = 0;


void loop() {
  CheckBrightness();

  auto aPressed = digitalRead(pinButtonA) == LOW;
  auto bPressed = digitalRead(pinButtonB) == LOW;

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

  digitalWrite(pinLedA, pos < limit / 2);
  digitalWrite(pinLedB, pos > limit / 2);


  auto time = millis();
  auto delta = (int)(time - lastTime);
  lastTime = time;

  pos += speed * delta;

  if (pos > limit) {
    winA++;
    Win(0);
  } else if (pos < 0) {
    winB++;
    Win(limit);
  } else {
    fadeToBlackBy(ledsBall, numLeds, 20);

    ledsBall[PosToLed(pos)] = CRGB::Aqua;

    SendToLeds();
  }
}

void CheckBrightness() {
  if ((millis() - lastBrightnessTime) > 400 && digitalRead(pinBrightness) == LOW) {
    brightness++;
    if (brightness > 8) brightness = 4;
    SetBrightness();

    lastBrightnessTime = millis();
  }
}

void SetBrightness() {
  FastLED.setBrightness((1 << brightness) - 1);
}

void SendToLeds() {
  FastLED.clear();
  for (int i = 0; i < 9; i++) {
    leds[i] = CRGB::DarkGreen;
    leds[numLeds - i - 1] = CRGB::DarkGreen;
  }

  leds[numLeds / 2] = CRGB::DarkRed;
  leds[(numLeds / 2) - 1] = CRGB::DarkRed;

  if (invalidPos > -1) {
    auto color = (millis() / 100) % 2 == 0 ? CRGB::HotPink : CRGB::Black;
    leds[PosToLed(invalidPos)] = color;
  }

  for (int i = 0; i < numLeds; i++)
    leds[i] += ledsBall[i];

  FastLED.show();
}

int GetSpeed(int dif) {
  if (dif < 256)
    return maxSpeed * 2;
  return map(dif, 9 << 8, 0, minSpeed, maxSpeed);
}

int PosToLed(int pos) {
  return (pos >> 8) % numLeds;
}

void Win(int newPos) {
  FastLED.clear();
  auto colorA = state == BtoA ? CRGB::Yellow : CRGB::Green;
  auto colorB = state == BtoA ? CRGB::Green : CRGB::Yellow;

  leds[numLeds / 2 - 1] = CRGB::DarkRed;
  leds[numLeds / 2] = CRGB::DarkRed;

  for (int i = 0; i < winA; i++)
    leds[numLeds / 2 - i - 2] = colorA;

  for (int i = 0; i < winB; i++)
    leds[numLeds / 2 + i + 1] = colorB;


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
      auto idx = aWon ? j : numLeds - j - 1;
      leds[idx] = CRGB::Green;
    }
    FastLED.show();
    delay(40);
  }
  winA = winB = 0;
}

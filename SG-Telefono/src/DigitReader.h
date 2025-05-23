#pragma once
#include <Arduino.h>

class DigitReader {
public:
  void begin();
  void update();
  bool isReady();
  int getDigit();
  void reset();

private:
  enum State { IDLE, COUNTING, DONE };
  State state = IDLE;
  unsigned long lastChange = 0;
  int pulseCount = 0;
  int lastRead = LOW;
  int stable = LOW;
  bool ready = false;
  int digitValue = -1;
};

#include "DigitReader.h"
#include "PinConfig.h"

void DigitReader::begin() {
  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
}

void DigitReader::update() {
  int rawInput2 = !digitalRead(INPUT_2);
  int read1 = !digitalRead(INPUT_1);

  switch (state) {
    case IDLE:
      if (rawInput2 == LOW) {
        state = COUNTING;
        pulseCount = 0;
        lastRead = read1;
        stable = read1;
        lastChange = millis();
      }
      break;
    case COUNTING:
      if (read1 != lastRead) {
        lastChange = millis();
        lastRead = read1;
      }
      if ((millis() - lastChange) > 10) {
        if (read1 != stable) {
          stable = read1;
          if (stable == HIGH)
            pulseCount++;
        }
      }
      if (rawInput2 == HIGH) {
        state = DONE;
        digitValue = (pulseCount ) % 10; 
        ready = true;
      }
      break;
    case DONE:
      break;
  }
}

bool DigitReader::isReady() {
  return ready;
}

int DigitReader::getDigit() {
  ready = false;
  state = IDLE;
  return digitValue;
}

void DigitReader::reset() {
  state = IDLE;
  ready = false;
  pulseCount = 0;
}

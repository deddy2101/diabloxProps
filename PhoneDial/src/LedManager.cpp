#include "LedManager.h"

#include <TimerOne.h>

void LedManager::begin() {
  pinMode(ERROR_LED, OUTPUT);
  pinMode(OK_LED, OUTPUT);
  digitalWrite(ERROR_LED, LOW);
  digitalWrite(OK_LED, LOW);
}

void LedManager::startBlinking() {
    if (blinking) return;
    blinking = true;
  Timer1.initialize(300000);
  Timer1.attachInterrupt([]() {
    static bool state = false;
    state = !state;
    digitalWrite(OK_LED, state ? HIGH : LOW);
    digitalWrite(ERROR_LED, state ? LOW : HIGH);
  });
}

void LedManager::stopBlinking() {
  if (blinking) {
    Timer1.detachInterrupt();
    blinking = false;
  }
  
  digitalWrite(OK_LED, LOW);
  digitalWrite(ERROR_LED, LOW);
}

#pragma once
#include <Arduino.h>
#include "pinConfig.h"
class LedManager
{
public:
    void begin();
    void startBlinking();
    void stopBlinking();
    void toggleLeds();
    void ok()
    {
        digitalWrite(OK_LED, HIGH);
        digitalWrite(ERROR_LED, LOW);
    }
    void error()
    {
        digitalWrite(OK_LED, LOW);
        digitalWrite(ERROR_LED, HIGH);
    }
    void reset()
    {
        digitalWrite(OK_LED, LOW);
        digitalWrite(ERROR_LED, LOW);
    }

private:
    bool state = false;
    bool blinking = false;
};

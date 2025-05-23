#pragma once
#include "DFRobotDFPlayerMini.h"


class AudioManager {
public:
  void begin(HardwareSerial& serial);
  void playSuccess();
  void stop();

private:
  DFRobotDFPlayerMini player;
};

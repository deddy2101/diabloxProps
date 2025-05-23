#include "AudioManager.h"
#include "pinConfig.h"
void AudioManager::begin(HardwareSerial& serial) {
  if (player.begin(serial)) {
    Serial.println("DFPlayer pronto");
    player.volume(0);
  } else {
    Serial.println("Errore DFPlayer");
    while (1) digitalWrite(ERROR_LED, HIGH);
  }
}

void AudioManager::playSuccess() {
  player.volume(30);
  player.play(1);
}

void AudioManager::stop() {
  player.stop();
  player.volume(0);
}

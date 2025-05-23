#include "CodeManager.h"

void CodeManager::begin() {
  prefs.begin("config", true);
}

void CodeManager::load(int code[6]) {
  if (prefs.isKey("code"))
    prefs.getBytes("code", code, sizeof(int) * 6);
  else {
    int defaultCode[6] = {4, 2, 3, 6, 1, 0};
    memcpy(code, defaultCode, sizeof(int) * 6);
  }
  prefs.end();
}

void CodeManager::save(int code[6]) {
  prefs.begin("config", false);
  prefs.putBytes("code", code, sizeof(int) * 6);
  prefs.end();
}

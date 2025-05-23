#pragma once
#include <Preferences.h>

class CodeManager {
public:
  void begin();
  void save(int code[6]);
  void load(int code[6]);

private:
  Preferences prefs;
};

#include "pinConfig.h"
#include "DigitReader.h"
#include "LedManager.h"
#include "AudioManager.h"
#include "CodeManager.h"

DigitReader digit;
LedManager leds;
AudioManager audio;
CodeManager codeStorage;

HardwareSerial dfSerial(1);

int correctCode[6];
int inputCode[6];
int inputIndex = 0;

int programmingSequence[4] = {0, 1, 1, 1};
int programmingBuffer[4];
int progIndex = 0;

bool inProgrammingMode = false;
bool lastPhoneState = false;

void setup() {
  Serial.begin(115200);
  dfSerial.begin(9600, SERIAL_8N1, 8, 9);

  digit.begin();
  leds.begin();
  audio.begin(dfSerial);
  codeStorage.begin();
  codeStorage.load(correctCode);

  pinMode(INPUT_5, INPUT_PULLUP); // cornetta
  lastPhoneState = !digitalRead(INPUT_5);
}

void loop() {
  bool phoneState = !digitalRead(INPUT_5);
  digit.update();

  if (phoneState != lastPhoneState) {
    lastPhoneState = phoneState;

    audio.stop();
    if (!phoneState) {
      // cornetta giù
      inputIndex = progIndex = 0;
      inProgrammingMode = false;
      digit.reset();
      leds.stopBlinking();
      Serial.println("Cornetta abbassata, reset stato");
    } else {
      // cornetta su
      inputIndex = progIndex = 0;
      inProgrammingMode = false;
      digit.reset();
      leds.stopBlinking();
      Serial.println("Cornetta alzata, inizio lettura");
    }
    delay(100);
  }

  if (digit.isReady()) {
    leds.reset();
    int val = digit.getDigit();
    val++; // correzione impulsi
    if (val > 9) val = 0;

    Serial.print("Letto: ");
    Serial.println(val);

    if (phoneState) {
      // === Modalità NORMALE ===
      inputCode[inputIndex++] = val;

      if (inputIndex == 6) {
        bool correct = true;
        for (int i = 0; i < 6; i++) {
          if (inputCode[i] != correctCode[i]) {
            correct = false;
            break;
          }
        }

        if (correct) {
          Serial.println("Codice corretto!");
          leds.ok();
          audio.playSuccess();
        } else {
          leds.error();
          Serial.println("Codice errato");
        }

        inputIndex = 0;
      }
    } else {
      // === Modalità PROGRAMMAZIONE ===
      if (!inProgrammingMode) {
        programmingBuffer[progIndex++] = val;
        if (progIndex == 4) {
          bool trigger = true;
          for (int i = 0; i < 4; i++) {
            if (programmingBuffer[i] != programmingSequence[i]) {
              trigger = false;
              break;
            }
          }

          if (trigger) {
            inProgrammingMode = true;
            inputIndex = 0;
            progIndex = 0;
            leds.startBlinking();
            Serial.println("ENTRATA IN PROGRAMMAZIONE");
          } else {
            progIndex = 0;
            Serial.println("Sequenza errata");
          }
        }
      } else {
        correctCode[inputIndex++] = val;
        if (inputIndex == 6) {
          codeStorage.save(correctCode);
          leds.stopBlinking();
          Serial.println("Nuovo codice salvato!");
          inputIndex = 0;
          inProgrammingMode = false;
        }
      }
    }
  }

  delay(10);
}

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <Preferences.h>
#include <TimerOne.h>

Preferences prefs;

// Definizione pin
#define INPUT_1 39
#define INPUT_2 40
#define INPUT_3 21
#define INPUT_4 18
#define INPUT_5 17
#define ERROR_LED INPUT_3
#define OK_LED INPUT_4

bool ledState = false;
#define TX 2
#define RX 1
HardwareSerial mySerial(1);

const int debounceTime = 10;
const int input2DebounceDelay = 50;
const int ledPin = LED_BUILTIN;

int correctCode[6] = {4, 2, 3, 6, 1, 0};
int inputCode[6];
int inputIndex = 0;

DFRobotDFPlayerMini player;

bool dialingInProgress = false;
bool digitCaptured = false;
bool inProgrammingMode = false;

int programmingSequence[4] = {0, 1, 1, 1};
int programmingBuffer[4];
int progIndex = 0;

int lastInput2State = HIGH;
int stableInput2State = HIGH;
unsigned long input2LastChangeTime = 0;
void toggleLeds()
{
  ledState = !ledState;
  digitalWrite(OK_LED, ledState ? HIGH : LOW);
  digitalWrite(ERROR_LED, ledState ? LOW : HIGH);
}
bool lastPhoneState = LOW;

// === NON BLOCCANTE readDigit ===
enum DigitReadState
{
  IDLE,
  COUNTING,
  DONE
};
DigitReadState digitState = IDLE;

unsigned long digitLastChange = 0;
int digitPulseCount = 0;
int digitLastRead1 = LOW;
int digitStable1 = LOW;

bool digitReady = false;
int digitValue = -1;

void updateReadDigit()
{
  int rawInput2 = !digitalRead(INPUT_2);
  int read1 = !digitalRead(INPUT_1);

  switch (digitState)
  {
  case IDLE:
    if (rawInput2 == LOW)
    {
      digitState = COUNTING;
      digitPulseCount = 0;
      digitLastRead1 = read1;
      digitStable1 = read1;
      digitLastChange = millis();
    }
    break;

  case COUNTING:
    if (read1 != digitLastRead1)
    {
      digitLastChange = millis();
      digitLastRead1 = read1;
    }

    if ((millis() - digitLastChange) > debounceTime)
    {
      if (read1 != digitStable1)
      {
        digitStable1 = read1;
        if (digitStable1 == HIGH)
        {
          digitPulseCount++;
        }
      }
    }

    if (rawInput2 == HIGH)
    {
      digitState = DONE;
      digitValue = digitPulseCount % 10;
      digitReady = true;
    }
    break;

  case DONE:
    break;
  }
}

void resetDigitReader()
{
  digitState = IDLE;
  digitReady = false;
  digitPulseCount = 0;
}

// === SETUP ===
void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 8, 9);

  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
  pinMode(INPUT_3, OUTPUT);
  pinMode(INPUT_4, OUTPUT);
  pinMode(INPUT_5, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ERROR_LED, LOW);
  digitalWrite(OK_LED, LOW);

  if (player.begin(mySerial))
  {
    Serial.println("DFPlayer Mini connesso correttamente!");
    player.volume(0);
  }
  else
  {
    Serial.println("Errore nella connessione con DFPlayer Mini.");
    while (1)
    {
      digitalWrite(ERROR_LED, HIGH);
    }
  }

  prefs.begin("config", true);
  if (prefs.isKey("code"))
  {
    prefs.getBytes("code", correctCode, sizeof(correctCode));
    Serial.println("Codice caricato da memoria");
  }
  else
  {
    Serial.println("Nessun codice salvato, uso codice predefinito");
  }
  prefs.end();

  lastPhoneState = !digitalRead(INPUT_5);
}

// === LOOP ===
void loop()
{
  bool phoneState = !digitalRead(INPUT_5);
  updateReadDigit();

  if (phoneState != lastPhoneState)
  {
    lastPhoneState = phoneState;
    player.stop();
    player.volume(0);
    if (!phoneState)
    {

      dialingInProgress = false;
      digitCaptured = false;
      stableInput2State = HIGH;
      digitStable1 = LOW;
      digitLastRead1 = LOW;
      digitPulseCount = 0;
      inputIndex = 0;
      progIndex = 0;
      resetDigitReader();
      digitalWrite(ERROR_LED, LOW);
      digitalWrite(OK_LED, LOW);
      Serial.println("Cornetta abbassata, reset stato");
    }
    else
    {
      if (inProgrammingMode)
      {
        inProgrammingMode = false;
        Serial.println("Programmazione annullata");
        Timer1.detachInterrupt();
        digitalWrite(ERROR_LED, LOW);
        digitalWrite(OK_LED, LOW);
      }
      dialingInProgress = false;
      digitCaptured = false;
      inputIndex = 0;
      
      progIndex = 0;
      resetDigitReader();
      Serial.println("Cornetta alzata, inizio lettura");
    }
    delay(100);
  }

  if (digitReady)
  {
    digitValue++;
    if (digitValue > 9)
    {
      digitValue = 0;
    }
    Serial.print("Letto: ");
    Serial.println(digitValue);

    if (phoneState)
    {
      // modalità NORMALE
      inputCode[inputIndex++] = digitValue;
      if (inputIndex == 6)
      {
        bool correct = true;
        for (int i = 0; i < 6; i++)
        {
          if (inputCode[i] != correctCode[i])
          {
            correct = false;
            break;
          }
        }

        if (correct)
        {
          Serial.println("Codice corretto!");
          digitalWrite(ledPin, HIGH);
          digitalWrite(OK_LED, HIGH);
          player.volume(30);
          player.play(1);
          for (int i = 0; i < 6; i++)
            inputCode[i] = 0;
          inputIndex = 0;
          delay(1000);
        }
        else
        {
          Serial.println("Codice errato. Reset.");
          digitalWrite(ERROR_LED, HIGH);
          delay(1000);
          digitalWrite(ERROR_LED, LOW);
          inputIndex = 0;
        }
      }
    }
    else
    {
      // modalità PROGRAMMAZIONE
      if (!inProgrammingMode)
      {
        programmingBuffer[progIndex++] = digitValue;
        Serial.print("Programmazione - Letto: ");
        Serial.println(digitValue);

        if (progIndex == 4)
        {
          bool trigger = true;
          for (int i = 0; i < 4; i++)
          {
            if (programmingBuffer[i] != programmingSequence[i])
            {
              trigger = false;
              break;
            }
          }

          if (trigger)
          {
            inProgrammingMode = true;
            progIndex = 0;
            inputIndex = 0;
            Serial.println("ENTRATA IN PROGRAMMAZIONE");

            Timer1.initialize(300000); // 300ms
            Timer1.attachInterrupt(toggleLeds);
            digitalWrite(ERROR_LED, LOW);
          }
          else
          {
            Serial.println("Sequenza errata.");
            progIndex = 0;
          }
        }
      }
      else
      {
        correctCode[inputIndex++] = digitValue;
        Serial.print("Nuovo codice[");
        Serial.print(inputIndex - 1);
        Serial.print("] = ");
        Serial.println(digitValue);

        if (inputIndex == 6)
        {
          Serial.println("Nuovo codice salvato!");
          prefs.begin("config", false);
          prefs.putBytes("code", correctCode, sizeof(correctCode));
          prefs.end();
          // Ferma lampeggio e spegni LED
          Timer1.detachInterrupt();
          digitalWrite(OK_LED, LOW);
          digitalWrite(ERROR_LED, LOW);
          digitalWrite(OK_LED, HIGH);
          delay(1000);
          digitalWrite(OK_LED, LOW);
          inputIndex = 0;
          inProgrammingMode = false;
        }
      }
    }

    // pulizia
    digitReady = false;
    digitState = IDLE;
  }

  delay(10);
}

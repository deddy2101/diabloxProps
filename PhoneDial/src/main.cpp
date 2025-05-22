#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

#define INPUT_1 39 // impulsi
#define INPUT_2 40 // stato composizione (basso = attivo)
#define INPUT_3 21 // 021
#define INPUT_4 18 // 18
#define INPUT_5 17 // 16
#define ERROR_LED INPUT_3
#define TX 2
#define RX 1
HardwareSerial mySerial(1); // Puoi usare anche 2, se 1 è già usata altrove

const int ledPin = LED_BUILTIN;

const int debounceTime = 10;
const int input2DebounceDelay = 50; // tempo minimo per considerare la fine della cifra (ms)

int pulseCount = 0;
int lastReadState = LOW;
int stableState = LOW;
unsigned long lastStateChangeTime = 0;

bool dialingInProgress = false;
bool digitCaptured = false;

int inputCode[6];
int inputIndex = 0;

const int correctCode[6] = {4, 2, 3, 6, 1, 0};

// Per debounce su INPUT_2
int lastInput2State = HIGH;
int stableInput2State = HIGH;
unsigned long input2LastChangeTime = 0;
// Create the Player object
DFRobotDFPlayerMini player;

void setup()
{
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, 8, 9); // RX = 8, TX = 9

  delay(1000);
  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
  pinMode(INPUT_3, OUTPUT);
  pinMode(INPUT_4, OUTPUT);
  pinMode(INPUT_5, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  if (player.begin(mySerial))
  {
    Serial.println("DFPlayer Mini connesso correttamente!");
  }
  else
  {
    Serial.println("Errore nella connessione con DFPlayer Mini.");
    while (1)
    {
      digitalWrite(ERROR_LED, HIGH);
    }
  }
}

void loop()
{

  bool phoneState = !digitalRead(INPUT_5);
  if (phoneState)
  {
    Serial.println("Telefono in uso");
    digitalWrite(ledPin, LOW);
    digitalWrite(INPUT_3, HIGH); // accende led error
    return;
  }
  else
  {
    Serial.println("Telefono non in uso");
  }
  
}

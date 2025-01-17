#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <TimerOne.h>

#define RELAY_PIN 42
#define LASER_PIN_1 39
#define LASER_PIN_2 40
#define LASER_PIN_3 41
#define LASER_PIN_4 38

#define ETH_RESET_PIN 46

#define INPUT_LASER_1 4
#define INPUT_LASER_2 5
#define INPUT_LASER_3 6
#define INPUT_LASER_4 7
#define RESET_PROP_BUTTON 16
#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];

volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
volatile bool toggle4 = false;
bool relayState = false;

IPAddress staticIP(192, 168, 1, 5);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13801;
EthernetConnection eth(&staticIP, &dnsServer, &gateway, &subnetMask, &serverIP, &serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

void handleResetButtonPress()
{
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay)
  {
    relayState = !relayState;
    lastInterruptTime = currentTime;
  }
}

void setLedColor(CRGB color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
  FastLED.show();
}

// Task per Ethernet Loop
void ethernetTask(void *parameter)
{
  for (;;)
  {
    eth.loop();
    delay(10);  // Aggiungi un piccolo delay per evitare di saturare il core
  }
}

void setup()
{
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  setLedColor(CRGB::Red);

  pinMode(LASER_PIN_1, OUTPUT);
  pinMode(LASER_PIN_2, OUTPUT);
  pinMode(LASER_PIN_3, OUTPUT);
  pinMode(LASER_PIN_4, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(INPUT_LASER_1, INPUT);
  pinMode(INPUT_LASER_2, INPUT);
  pinMode(INPUT_LASER_3, INPUT);
  pinMode(INPUT_LASER_4, INPUT);
  pinMode(RESET_PROP_BUTTON, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(RESET_PROP_BUTTON), handleResetButtonPress, FALLING);

  eth.init(&relayState);
  setLedColor(CRGB::Green);

  // Crea il task Ethernet sul core 0
  xTaskCreatePinnedToCore(
    ethernetTask,      // Nome della funzione
    "Ethernet Task",   // Nome del task (per debug)
    10000,             // Stack size
    NULL,              // Parametro passato al task
    1,                 // Priorità del task
    NULL,              // Handle del task (opzionale)
    0                  // Core su cui eseguire il task (0 per Core 0)
  );
}

void loop()
{
  digitalWrite(RELAY_PIN, relayState);
  digitalWrite(LASER_PIN_1, HIGH);
  delay(4);
  toggle1 = digitalRead(INPUT_LASER_1);
  digitalWrite(LASER_PIN_1, LOW);
  delay(4);
  digitalWrite(LASER_PIN_2, HIGH);
  delay(4);
  toggle2 = digitalRead(INPUT_LASER_2);
  digitalWrite(LASER_PIN_2, LOW);
  delay(4);
  digitalWrite(LASER_PIN_3, HIGH);
  delay(4);
  toggle3 = digitalRead(INPUT_LASER_3);
  digitalWrite(LASER_PIN_3, LOW);
  delay(4);
  digitalWrite(LASER_PIN_4, HIGH);
  delay(4);
  toggle4 = digitalRead(INPUT_LASER_4);
  digitalWrite(LASER_PIN_4, LOW);
  delay(4);

  if (toggle1 && toggle2 && toggle3 && toggle4 && !relayState)
  {
    relayState = true;
    eth.apiCall("{846e92d0-299c-454b-a799-3b4227ddb862}");
  }

  if (relayState)
  {
    setLedColor(CRGB::Blue);
  }
  else
  {
    setLedColor(CRGB::Green);
}
}

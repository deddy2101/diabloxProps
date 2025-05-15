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

#define INPUT_LASER_1 7
#define INPUT_LASER_2  6
#define INPUT_LASER_3  5
#define INPUT_LASER_4  4
#define RESET_PROP_BUTTON 16
#define NUM_LEDS 12
#define DATA_PIN 47
CRGB leds[NUM_LEDS];

volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
volatile bool toggle4 = false;
volatile bool buttonpressed = false;
bool relayState = false;

bool toggleState1 = false;
bool toggleState2 = false;
bool toggleState3 = false;
bool toggleState4 = false;

unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;
unsigned long timer4 = 0;

const unsigned long toggleDuration = 90000; // 90 secondi in millisecondi

IPAddress staticIP(192, 168, 1, 5);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

void handleResetButtonPress()
{
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay)
  {
    relayState = !relayState;
    lastInterruptTime = currentTime;
    buttonpressed = true;
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
  setLedColor(CRGB::Yellow);
  eth.setLEDS(leds, NUM_LEDS);
  eth.init(&relayState);


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
void updateToggleStateLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  if(relayState) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
    }
    FastLED.show();
    return;
  }

  if (toggleState1) {
    for (int i = 0; i <= 2; i++) leds[i] = CRGB::Blue;
  }
  if (toggleState2) {
    for (int i = 3; i <= 5; i++) leds[i] = CRGB::Blue;
  }
  if (toggleState3) {
    for (int i = 6; i <= 8; i++) leds[i] = CRGB::Blue;
  }
  if (toggleState4) {
    for (int i = 9; i <= 11; i++) leds[i] = CRGB::Blue;
  }

  FastLED.show();
}

void loop() {
  unsigned long now = millis();
  digitalWrite(RELAY_PIN, relayState);

  // LASER 1
  digitalWrite(LASER_PIN_1, HIGH);
  delay(4);
  toggle1 = digitalRead(INPUT_LASER_1) == LOW;
  digitalWrite(LASER_PIN_1, LOW);
  delay(4);
  if (toggle1) {
    toggleState1 = true;
    timer1 = now;
  }
  if (toggleState1 && now - timer1 >= toggleDuration) {
    toggleState1 = digitalRead(INPUT_LASER_1) == LOW;
    timer1 = now; // Riavvia se ancora LOW
  }

  // LASER 2
  digitalWrite(LASER_PIN_2, HIGH);
  delay(4);
  toggle2 = digitalRead(INPUT_LASER_2) == LOW;
  digitalWrite(LASER_PIN_2, LOW);
  delay(4);
  if (toggle2) {
    toggleState2 = true;
    timer2 = now;
  }
  if (toggleState2 && now - timer2 >= toggleDuration) {
    toggleState2 = digitalRead(INPUT_LASER_2) == LOW;
    timer2 = now;
  }

  // LASER 3
  digitalWrite(LASER_PIN_3, HIGH);
  delay(4);
  toggle3 = digitalRead(INPUT_LASER_3) == LOW;
  digitalWrite(LASER_PIN_3, LOW);
  delay(4);
  if (toggle3) {
    toggleState3 = true;
    timer3 = now;
  }
  if (toggleState3 && now - timer3 >= toggleDuration) {
    toggleState3 = digitalRead(INPUT_LASER_3) == LOW;
    timer3 = now;
  }

  // LASER 4
  digitalWrite(LASER_PIN_4, HIGH);
  delay(4);
  toggle4 = digitalRead(INPUT_LASER_4) == LOW;
  digitalWrite(LASER_PIN_4, LOW);
  delay(4);
  if (toggle4) {
    toggleState4 = true;
    timer4 = now;
  }
  if (toggleState4 && now - timer4 >= toggleDuration) {
    toggleState4 = digitalRead(INPUT_LASER_4) == LOW;
    timer4 = now;
  }

  // Stampa per debug
  //printf("ToggleStates: %d %d %d %d\n", toggleState1, toggleState2, toggleState3, toggleState4);
  //print tggle
// printf("Toggle: %d %d %d %d\n", toggle1, toggle2, toggle3, toggle4);

  // Se tutti i toggle state sono attivi, attiva il relay
  if (toggleState1 && toggleState2 && toggleState3 && toggleState4 && !relayState) {
    relayState = true;
    eth.apiCall("{846e92d0-299c-454b-a799-3b4227ddb862}"); //api call for the porta opened

  }

  // Stato relay
  digitalWrite(RELAY_PIN, relayState);

  if (buttonpressed) {
    buttonpressed = false;
    printf("Button pressed\n");
  }
  updateToggleStateLEDs();

}

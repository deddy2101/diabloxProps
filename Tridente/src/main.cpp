#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <CardReader.h>


#define RELAY_PIN 42
#define INT_1 41
#define RST_1 2
#define CS_1 1

#define INT_2 38
#define RST_2 39
#define CS_2 40


#define ETH_RESET_PIN 46

CardReader cardReader(CS_1, RST_1);
CardReader cardReader2(CS_2, RST_2);

byte serial_1[5] = {0x43, 0x0D, 0x8D, 0x18, 0xDB};
byte serial_2[5] = {0xD3, 0x85, 0x9C, 0xED, 0x27};


#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];
bool relayState = false;
EthernetConnection eth;
SPIClass spi;

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
    //eth.loop();
    delay(10);  // Aggiungi un piccolo delay per evitare di saturare il core
  }
}

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  //set led 1 to red
  setLedColor(CRGB::Red);

  spi.begin(18, 16, 17, 40);
  
  pinMode(RELAY_PIN, OUTPUT);
  cardReader.begin(&spi);  
  cardReader2.begin(&spi);

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
void loop() {
  if (cardReader.readCardAndHoldPresence(serial_1) && cardReader2.readCardAndHoldPresence(serial_2) && !relayState) {
    relayState=true;
    
    printf("\033[1;32m[I] The relay is on\n\033[0m");
    digitalWrite(RELAY_PIN, relayState);
    eth.apiCall("1", "{080ffce7-f73e-4932-a7e3-c09a62701323}");
  };
  digitalWrite(RELAY_PIN, relayState);

  if (relayState) {
    setLedColor(CRGB::Blue);
  } else {
    setLedColor(CRGB::Green);
  }
}
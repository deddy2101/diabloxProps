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
#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];
bool relayState = false;
EthernetConnection eth;
SPIClass spi;


void setup() {
  spi.begin(18, 16, 17, 40);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  cardReader.begin(&spi);  
  cardReader2.begin(&spi);

  //eth.init();
  

}
void loop() {
  cardReader2.loop();
  cardReader.loop();
}
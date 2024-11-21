#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>

#define RELAY_PIN 42
#define LASER_PIN_1 39
#define LASER_PIN_2 40
#define LASER_PIN_3 41

#define INPUL_LASER_1 4
#define INPUL_LASER_2 5
#define INPUL_LASER_3 6
#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];

EthernetConnection eth;

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LASER_PIN_1, OUTPUT);
  pinMode(LASER_PIN_2, OUTPUT);
  pinMode(LASER_PIN_3, OUTPUT);
  pinMode(INPUL_LASER_1, INPUT);
  pinMode(INPUL_LASER_2, INPUT);
  pinMode(INPUL_LASER_3, INPUT);
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical

  delay(1000);



  printf("\033[1;33mDNS Server :\033[0m\n");
 
}

void loop() {
  // TURN on relay and leds
  //digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(LASER_PIN_1, HIGH);
  digitalWrite(LASER_PIN_2, HIGH);
  digitalWrite(LASER_PIN_3, HIGH);

  if(digitalRead(INPUL_LASER_1) == HIGH){
    leds[0] = CRGB::Green;
    FastLED.show();
  }else{
    leds[0] = CRGB::Red;
    FastLED.show();
  }
  
}

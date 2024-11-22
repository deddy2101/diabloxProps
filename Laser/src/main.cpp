#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <TimerOne.h>


#define RELAY_PIN 42
#define LASER_PIN_1 39
#define LASER_PIN_2 40
#define LASER_PIN_3 41

#define ETH_RESET_PIN 46

#define INPUT_LASER_1 4
#define INPUT_LASER_2 5
#define INPUT_LASER_3 6
#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];
volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
bool relayState = false;
EthernetConnection eth;



void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  pinMode(LASER_PIN_1, OUTPUT);
  pinMode(LASER_PIN_2, OUTPUT);
  pinMode(LASER_PIN_3, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(INPUT_LASER_1, INPUT);
  pinMode(INPUT_LASER_2, INPUT);
  pinMode(INPUT_LASER_3, INPUT);

}
void loop() {
  digitalWrite(RELAY_PIN, relayState);
  digitalWrite(LASER_PIN_1, HIGH);
  delay(4);
  //read the input 1
  toggle1 = digitalRead(INPUT_LASER_1);
  digitalWrite(LASER_PIN_1, LOW);
  delay(4);
  //read the input 2
  digitalWrite(LASER_PIN_2, HIGH);
  delay(4);
  toggle2 = digitalRead(INPUT_LASER_2);
  digitalWrite(LASER_PIN_2, LOW);
delay(4);
  //read the input 3
  digitalWrite(LASER_PIN_3, HIGH);
  delay(4);
  toggle3 = digitalRead(INPUT_LASER_3);
  digitalWrite(LASER_PIN_3, LOW);
  delay(4);
  if(toggle1 == LOW){
    leds[0] = CRGB::Red;
    FastLED.show();
  }
  if(toggle2 == LOW){
    leds[0] = CRGB::Green;
    FastLED.show();
  }
  if(toggle3 == LOW){
    leds[0] = CRGB::Blue;
    FastLED.show();
  }
  if (toggle1 == LOW && toggle2 == LOW && toggle3 == LOW){
    leds[0] = CRGB::Green;
    FastLED.show();
    relayState = true;
    
  } else {
    relayState = false;
  }
}
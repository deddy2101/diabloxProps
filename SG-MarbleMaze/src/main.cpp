#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <TimerOne.h>




#define ETH_RESET_PIN 8

#define INPUT_LASER_1 4
#define INPUT_LASER_2 5
#define INPUT_LASER_3 6
#define INPUT_LASER_4 7
#define RESET_PROP_BUTTON 16
#define NUM_LEDS 10
#define DATA_PIN INPUT_4
#define RELAY_PIN 1

#define INPUT_1 39 //39
#define INPUT_2 40 //40
#define INPUT_3 21 //021
#define INPUT_4 18 //18
#define INPUT_5 17 //16
#define INPUT_6 2 //2
#define INPUT_7 4 //4
#define INPUT_8 6 //6
#define INPUT_9 10 //10
#define INPUT_10 13 //13
#define INPUT_11 3 //3
#define INPUT_12 5//5

CRGB leds[NUM_LEDS];

volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
volatile bool toggle4 = false;
volatile bool buttonpressed = false;
bool relayState = false;

IPAddress staticIP(192, 168, 1, 203);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13808;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xBA}, serverPort);


void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
  ESP.restart();
}
void testLEDs()
{
  // Test all LEDs by lighting them up in sequence
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::White; // Set LED to white
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black; // Reset LED to black
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(INPUT_1, INPUT); 
  pinMode(INPUT_2, INPUT);
  pinMode(INPUT_3, INPUT);
  pinMode(INPUT_4, OUTPUT); //correggi qua
  
  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay);
  testLEDs(); // Test all LEDs at startup

}
bool state1, state2, state3 = false;
int count=0;

void updateLEDs()
{
   for (int i=0; i<NUM_LEDS ; i++){
  leds[i] = CRGB::Black;
 }
  // Spegni tutti i LED
 for (int i=0; i<count ; i++){
  leds[i] = CRGB::Green;
 }

  FastLED.show();
}

// RGY

void flashErrorLeds()
{
  //flash 3 times all the leds in red
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CRGB::Red; // Set all LEDs to red
    }
    FastLED.show();
    delay(300);
    for (int j = 0; j < NUM_LEDS; j++) {
      leds[j] = CRGB::Black; // Reset all LEDs to black
    }
    FastLED.show();
    delay(300);
  }
}

#define DEBOUNCE_DELAY 400 // Milliseconds for debounce delay
void loop()
{
  bool input1 = !digitalRead(INPUT_1);
  bool input2 = !digitalRead(INPUT_2);
  bool input3 = !digitalRead(INPUT_3);
  //se gualcosa va a 1 stampalo
  if (input1) {
    count++;
    Serial.print("Input 1 pressed, count: ");
    Serial.println(count);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input2) {
    count +=2;
    Serial.print("Input 2 pressed, count: ");
    Serial.println(count);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input3) {
    count +=7 ;
    Serial.print("Input 3 pressed, count: ");
    Serial.println(count);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  updateLEDs(); // Update the LEDs based on the current counts
  if (count == 10) {
    Serial.println("Right combination entered!");
    openRelay();
  } else if (count >10) {
    Serial.println("Wrong combination, resetting counts.");
    flashErrorLeds(); // Flash error LEDs
    count =0;
  }
  
  
  //delay(10); // Aggiungi un ritardo per evitare spam di messaggi seriali
  eth.loop(); // Assicurati di chiamare il loop dell'istanza EthernetConnection
}

//rosso verde giallo
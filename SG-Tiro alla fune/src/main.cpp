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
#define NUM_LEDS 15
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

IPAddress staticIP(192, 168, 1, 207);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAD}, serverPort);

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
  ESP.restart(); // Restart the ESP after opening the relay
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
  pinMode(INPUT_4, OUTPUT);
  
  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay);
  testLEDs(); // Test all LEDs at startup

}
bool state1, state2, state3 = false;
int count1 = 0, count2 = 0, count3 = 0; 
int rightCombination[] = {4,1,2}; // RGY 2 1 4 / oridne reale YGR 412

void updateLEDs()
{
  // Spegni tutti i LED
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }

  // Gruppo 1: LED 0-4
  for (int i = 0; i < count1 && i < 5; i++) {
    leds[i] = CRGB::Yellow;
  }

  // Gruppo 2: LED 5-9
  for (int i = 0; i < count2 && i < 5; i++) {
    leds[5 + i] = CRGB::Green;
  }

  // Gruppo 3: LED 10-14
  for (int i = 0; i < count3 && i < 5; i++) {
    leds[10 + i] = CRGB::Red;
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
    count1++;
    Serial.print("Input 1 pressed, count1: ");
    Serial.println(count1);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input2) {
    count2++;
    Serial.print("Input 2 pressed, count2: ");
    Serial.println(count2);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input3) {
    count3++;
    Serial.print("Input 3 pressed, count3: ");
    Serial.println(count3);
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (count1 == rightCombination[0] && count2 == rightCombination[1] && count3 == rightCombination[2]) {
    Serial.println("Right combination entered!");
    openRelay();
    count1 = 0;
    count2 = 0;
    count3 = 0;
  } else if (count1 > 5 || count2 > 5 || count3 > 5) {
    Serial.println("Wrong combination, resetting counts.");
    flashErrorLeds(); // Flash error LEDs
    count1 = 0;
    count2 = 0;
    count3 = 0;
  }
  updateLEDs(); // Update the LEDs based on the current counts
  
  //delay(10); // Aggiungi un ritardo per evitare spam di messaggi seriali
  eth.loop(); // Assicurati di chiamare il loop dell'istanza EthernetConnection
}

//rosso verde giallo
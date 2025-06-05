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
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAD}, serverPort);

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
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
  //eth.init(openRelay);

}
bool state1, state2, state3 = false;
int count1 = 0, count2 = 0, count3 = 0;
int rightCombination[] = {2,4,3};

void updateLEDs()
{
  //ho 3 gruppi da 5 led ogni gruppo ha un colore diverso gli indirizzi sono 0-4, 5-9, 10-14, accendi i led in base ai counter di ogni gruppo
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black; // Reset all LEDs to black
  }
  if (count1 > 0) {
    for (int i = 0; i < count1 && i < 5; i++) {
      leds[i] = CRGB::Red; // First group of LEDs (0-4) in red
    }
  }
  if (count2 > 0) {
    for (int i = 5; i < count2 && i < 10; i++) {
      leds[i] = CRGB::Green; // Second group of LEDs (5-9) in green
    }
  }
  if (count3 > 0) {
    for (int i = 10; i < count3 && i < 15; i++) {
      leds[i] = CRGB::Blue; // Third group of LEDs (10-14) in blue
    }
  }
  FastLED.show(); // Update the LEDs with the new colors
}

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

#define DEBOUNCE_DELAY 200 // Milliseconds for debounce delay
void loop()
{
  bool input1 = !digitalRead(INPUT_1);
  bool input2 = !digitalRead(INPUT_2);
  bool input3 = !digitalRead(INPUT_3);
  if (input1) {
    count1++;
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input2) {
    count2++;
    delay(DEBOUNCE_DELAY); // Debounce delay
  }
  if (input3) {
    count3++;
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
  
  delay(10); // Aggiungi un ritardo per evitare spam di messaggi seriali
  //eth.loop(); // Assicurati di chiamare il loop dell'istanza EthernetConnection
}

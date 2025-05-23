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
#define NUM_LEDS 1
#define DATA_PIN 16
#define RELAY_PIN 1

#define RELAY_PIN_1 39
#define RELAY_PIN_2 40
#define RELAY_PIN_3 1
#define RELAY_PIN_4 2

CRGB leds[NUM_LEDS];

volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
volatile bool toggle4 = false;
volatile bool buttonpressed = false;
bool relayState = false;

IPAddress staticIP(192, 168, 1, 5);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
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
  Serial.begin(115200);
  delay(1000);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  
  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay);
  //print

}

void loop()
{
  digitalWrite(RELAY_PIN_1, HIGH);
  delay(1000);
  digitalWrite(RELAY_PIN_2, HIGH);
  delay(1000);
  digitalWrite(RELAY_PIN_3, HIGH);
  delay(1000);
  digitalWrite(RELAY_PIN_4, HIGH);
  delay(1000);
  digitalWrite(RELAY_PIN_1, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN_2, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN_3, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN_4, LOW);
  delay(1000);
 
  Serial.println("Looping..."); 
  delay(10); // Aggiungi un piccolo delay per evitare di saturare il core
}

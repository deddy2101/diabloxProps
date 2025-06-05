#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>

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
#define INPUT_1 4

CRGB leds[NUM_LEDS];

bool relayState1 = false;
bool relayState2 = false;
bool relayState3 = false;
bool relayState4 = false;

IPAddress staticIP(192, 168, 1, 206);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13804;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAC}, serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
}
void resetRelays()
{
  digitalWrite(RELAY_PIN_1, LOW);
  digitalWrite(RELAY_PIN_2, LOW);
  digitalWrite(RELAY_PIN_3, LOW);
  digitalWrite(RELAY_PIN_4, LOW);  
  relayState1 = false;
  relayState2 = false;
  relayState3 = false;
  relayState4 = false;
  
  Serial.println("Relays reset.");
}
void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  pinMode(INPUT_1, INPUT_PULLUP);
  
  Serial.println("\033[1;33mStarting...\033[0m");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay, resetRelays, &relayState1, &relayState2, &relayState3, &relayState4);
  Serial.println("\033[1;33mCompleted\033[0m");
}

long manichinTurnOnTime = 0;
bool lastManichinoState = false;
long boxTurnOnTime = 0;
bool lastBoxState = false;

void loop()
{
  if (digitalRead(INPUT_1) == LOW) {
    relayState1 = true; // attiva il relay 1
  }
  //aggiorna i valori dei realy in base agli stati
  digitalWrite(RELAY_PIN_1, relayState1); //porta
  digitalWrite(RELAY_PIN_2, relayState2); //manichino
  digitalWrite(RELAY_PIN_3, relayState3); //box
  digitalWrite(RELAY_PIN_4, relayState4);
  lastManichinoState = relayState2; //aggiorna lo stato del manichino
  if (relayState2 && !lastManichinoState) {
    manichinTurnOnTime = millis(); // Salva il tempo di accensione del manichino
  }
  lastBoxState = relayState3; //aggiorna lo stato del box
  if (relayState3 && !lastBoxState) {
    boxTurnOnTime = millis(); // Salva il tempo di accensione del box
  }
  // Controlla se il manichino è acceso da più di 2 secondi
  if (relayState2 && (millis() - manichinTurnOnTime > 2000)) {
    Serial.println("\033[1;31mManichino acceso da più di 2 secondi, spegnimento...\033[0m");
    relayState2 = false; // Spegne il manichino
    Serial.println("\033[1;31mManichino spento.\033[0m");
  }
  // Controlla se il box è acceso da più di 2 secondi
  if (relayState3 && (millis() - boxTurnOnTime > 2000)) {
    Serial.println("\033[1;31mBox acceso da più di 2 secondi, spegnimento...\033[0m");
    relayState3 = false; // Spegne il box
    Serial.println("\033[1;31mBox spento.\033[0m");
  }
  eth.loop(); // Chiama il loop dell'istanza di EthernetConnection
 
}

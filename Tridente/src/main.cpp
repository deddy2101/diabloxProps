#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <Preferences.h> // Libreria per la NVS

#define RELAY_PIN 42
#define INT_1 41
#define RST_1 2
#define CS_1 1

#define INT_2 38
#define RST_2 39
#define CS_2 40

#define SWITCH_1 INT_1
#define SWITCH_2 CS_2
#define ETH_RESET_PIN 46

#define NUM_LEDS 1
#define DATA_PIN 48
CRGB leds[NUM_LEDS];

bool relayState = false;
IPAddress staticIP(192, 168, 1, 6);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, serverPort);

// Istanza per la gestione della NVS
Preferences preferences;

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
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup()
{
  pinMode(SWITCH_1, INPUT_PULLUP);
  pinMode(SWITCH_2, INPUT_PULLUP);

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  setLedColor(CRGB::Red);

  pinMode(RELAY_PIN, OUTPUT);

  // Inizializza NVS
  preferences.begin("relay-config", false);

  // Leggi lo stato salvato dalla NVS
  relayState = preferences.getBool("relayState", false);
  digitalWrite(RELAY_PIN, relayState);

  // Imposta colore LED in base allo stato del relè
  if (relayState)
    setLedColor(CRGB::Blue);
  else
    setLedColor(CRGB::Green);

  eth.init(&relayState);

  // Crea il task Ethernet sul core 0
  xTaskCreatePinnedToCore(
      ethernetTask,    // Nome della funzione
      "Ethernet Task", // Nome del task (per debug)
      10000,           // Stack size
      NULL,            // Parametro passato al task
      1,               // Priorità del task
      NULL,            // Handle del task (opzionale)
      0                // Core su cui eseguire il task (0 per Core 0)
  );
}

void loop()
{
  // Controlla lo stato degli interruttori
  if (!digitalRead(SWITCH_1) && !digitalRead(SWITCH_2) && !relayState)
  {
    relayState = true;

    // Salva lo stato del relè nella NVS
    

    printf("\033[1;32m[I] The relay is on\n\033[0m");
    digitalWrite(RELAY_PIN, relayState);
    eth.apiCall("{080ffce7-f73e-4932-a7e3-c09a62701323}"); // sends the API call to the server
  };
  preferences.putBool("relayState", relayState);

  // Aggiorna lo stato del relè
  digitalWrite(RELAY_PIN, relayState);

  // Cambia colore del LED in base allo stato del relè
  if (relayState)
  {
    setLedColor(CRGB::Blue);
  }
  else
  {
    setLedColor(CRGB::Green);
  }

  delay(100);
}

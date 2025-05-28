#include <Arduino.h>
#include <EthernetConnection.h>
#include <FastLED.h>
#include <TimerOne.h>
#include <PCF8575.h>

#define ETH_RESET_PIN 8

#define NUM_LEDS 1
#define RELAY_PIN 1

#define INPUT_1 39  // 39
#define INPUT_2 40  // 40
#define INPUT_3 21  // 021
#define INPUT_4 18  // 18
#define INPUT_5 17  // 16
#define INPUT_6 2   // 2
#define INPUT_7 4   // 4
#define INPUT_8 6   // 6
#define INPUT_9 10  // 10
#define INPUT_10 13 // 13
#define INPUT_11 3  // 3
#define INPUT_12 5  // 5

uint16_t getState();
void setOut(uint16_t value);

CRGB leds[NUM_LEDS];
unsigned long lastActivityTime = 0;
unsigned long lastRandomToggle = 0;
const unsigned long timeoutIdle = 120000;  // 2 minuti
const unsigned long intervalRandom = 5000; // 5 secondi

IPAddress staticIP(192, 168, 1, 205);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte, 6>{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xBB}, serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

PCF8575 EXT_IN(0x20);
PCF8575 EXT_OUT(0x21);
PCF8575 PCF[2] = {EXT_IN, EXT_OUT};

// lo stato di tutti i pulsanti
uint8_t buttonState[16] = {0};
uint8_t buttonStateOld[16] = {0};

// Definiamo N combinazioni possibili; qui ne metto 3 di esempio
const uint8_t NUM_WIN_PATTERNS = 1;
// Ciascuna riga è una combinazione: {gruppo0, gruppo1, gruppo2, gruppo3, gruppo4}
// dove il valore è 0,1 o 2 (il pulsante all’interno del gruppo)
const uint8_t winPatterns[NUM_WIN_PATTERNS][5] = {
    {0, 1, 2, 0, 1}, // es. combo A
};

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
}
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  delay(2000);
  Wire.begin(38, 37); // SDA, SCL
  Serial.println("PCF8575 Game Controller");
  Serial.println("=================================");
  Serial.println("Initializing Ethernet...");

  Serial.println("Starting...");
  pinMode(RELAY_PIN, OUTPUT);
  // eth.init(openRelay);
  digitalWrite(LED_BUILTIN, HIGH); // LED spento all'avvio
  for (int i = 0; i < 16; ++i)
  {
    EXT_IN.pinMode(i, INPUT);   // tutti gli ingressi come input con pull-up
    EXT_OUT.pinMode(i, OUTPUT); // tutti gli output come output
  }
  // inizializzo pcf8575
  if (!EXT_IN.begin() || !EXT_OUT.begin())
  {
    Serial.println("Error initializing PCF8575");
    while (1)
      delay(1000); // blocco se non riesco a inizializzare
  }
}

uint16_t lastState = 0;
uint16_t state = 0;
bool toggleState[16] = {0};  // stato logico memorizzato (acceso/spento)
bool prevRawState[16] = {0}; // stato fisico precedente (per rilevare fronte di salita)


void readAndUpdateStates()
{
  // 1) Leggo tutti i 16 bit dal PCF8575
  lastState = state;
  uint16_t raw = getState(); // funzione che legge lo stato dai pin del PCF8575
  state = raw;
  if (state != lastState)
  {
    lastActivityTime = millis();
  }

  // 2) Gestione del toggle per ogni pulsante
  for (int i = 0; i < 16; ++i)
  {
    bool currentRaw = !(raw >> i & 0x01); // active LOW (pulsante premuto)
    if (currentRaw && !prevRawState[i])
    {
      // Fronte di salita: premuto ora ma non nel ciclo precedente
      toggleState[i] = !toggleState[i]; // toggle dello stato
    }
    prevRawState[i] = currentRaw;    // aggiorno lo stato precedente
    buttonState[i] = toggleState[i]; // aggiorno lo stato logico
  }

  // 3) Per ogni gruppo di 3 pulsanti, tengo solo l’ultimo premuto
  const int GROUPS = 5;
  for (int g = 0; g < GROUPS; ++g)
  {
    int base = g * 3;
    int lastPressed = -1;

    for (int j = 0; j < 3; ++j)
    {
      int idx = base + j;
      if (buttonState[idx])
      {
        lastPressed = idx;
      }
    }

    for (int j = 0; j < 3; ++j)
    {
      int idx = base + j;
      if (idx != lastPressed)
      {
        buttonState[idx] = 0;
        toggleState[idx] = 0; // anche il toggle viene disattivato per coerenza
      }
    }
  }

  // 4) Ricompongo il word da scrivere su EXT_OUT
  uint16_t toWrite = 0;
  for (int i = 0; i < 16; ++i)
  {
    if (buttonState[i])
    {
      toWrite |= (1 << i);
    }
  }

  // 5) Scrivo il nuovo stato sui LED
  // EXT_OUT.write16(toWrite);
}

void verifyWin()
{
  // 1) Calcolo l'indice selezionato in ogni gruppo
  uint8_t current[5];
  for (int g = 0; g < 5; ++g)
  {
    current[g] = 255; // valore impossibile per identificare errori
    for (int j = 0; j < 3; ++j)
    {
      int idx = g * 3 + j;
      if (buttonState[idx])
      {
        current[g] = j;
        break; // esco alla prima 1 trovata (c’è sempre al massimo un 1)
      }
    }
  }

  // 2) Confronto con ciascuna pattern
  bool win = false;
  for (int p = 0; p < NUM_WIN_PATTERNS; ++p)
  {
    bool match = true;
    for (int g = 0; g < 5; ++g)
    {
      if (current[g] != winPatterns[p][g])
      {
        match = false;
        break;
      }
    }
    if (match)
    {
      win = true;
      break;
    }
  }

  // 3) Azione a seconda del risultato
  if (win)
  {
    // vincita: LED verde e, per esempio, invio un messaggio seriale

    Serial.println("=== WINNER! ===");
    openRelay();
    // attendi 4 secondi
    delay(4000);
    // calcola un nuovo stato random che non sia uguale a quello attuale
    uint16_t newState;
    do
    {
      newState = random(0, 0xFFFF);
    } while (newState == state);
    // scrivi il nuovo stato
    // EXT_OUT.write16(newState);
    // qui potresti anche inviare un pacchetto via ethernet,
    // oppure attivare un buzzer, ecc.
  }
  else
  {
    // non-vincita: LED rosso o spento
    Serial.println("=== NO WIN ===");
  }
}

void doLightGame()
{
  unsigned long now = millis();
  if (now - lastActivityTime > timeoutIdle)
  {
    if (now - lastRandomToggle > intervalRandom)
    {
      uint16_t rnd = random(0, 0xFFFF);
      // EXT_OUT.write16(rnd);
      Serial.println("=== RANDOM STATE ===");
      lastRandomToggle = now;
    }
  }
}

void loop()
{
  //readAndUpdateStates();
  //verifyWin();
  // eth.loop();
  //doLightGame();
  uint16_t currentState = getState();
  setOut(currentState); // aggiorno gli output in base allo stato corrente
  delay(10);
}



uint16_t getState()
{
  PCF8575::DigitalInput di = EXT_IN.digitalReadAll();
  uint16_t raw = 0;
  raw |= (di.p0 ? 0 : 1) << 0;
  raw |= (di.p1 ? 0 : 1) << 1;
  raw |= (di.p2 ? 0 : 1) << 2;
  raw |= (di.p3 ? 0 : 1) << 3;
  raw |= (di.p4 ? 0 : 1) << 4;
  raw |= (di.p5 ? 0 : 1) << 5;
  raw |= (di.p6 ? 0 : 1) << 6;
  raw |= (di.p7 ? 0 : 1) << 7;
  raw |= (di.p8 ? 0 : 1) << 8;
  raw |= (di.p9 ? 0 : 1) << 9;
  raw |= (di.p10 ? 0 : 1) << 10;
  raw |= (di.p11 ? 0 : 1) << 11;
  raw |= (di.p12 ? 0 : 1) << 12;
  raw |= (di.p13 ? 0 : 1) << 13;
  raw |= (di.p14 ? 0 : 1) << 14;
  raw |= (di.p15 ? 0 : 1) << 15;
  Serial.print("Raw value: ");
  for (int i = 15; i >= 0; --i)
  {
    Serial.print((raw >> i) & 0x01);
  }
  Serial.println();
  return raw;
}

void setOut(uint16_t value)
{
  for (int i = 0; i < 16; ++i)
  {
    if (value & (1 << i))
    {
      EXT_OUT.digitalWrite(i, HIGH); // accendo il pin
    }
    else
    {
      EXT_OUT.digitalWrite(i, LOW); // spengo il pin
    }
  }
  Serial.print("Set OUT value: ");
  Serial.println(value, BIN);
}
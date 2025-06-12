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
const unsigned long timeoutIdle = 120000;    // 2 minuti
const unsigned long intervalRandom = 5000; // 5 secondi

IPAddress staticIP(192, 168, 1, 205);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13806;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte, 6>{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xBB}, serverPort);

volatile unsigned long lastInterruptTime = 0;

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
    {1, 0, 2, 0, 2}, // es. combo A
};
uint16_t lastState = 0;
uint16_t state = 0;

bool toggleState[16] = {0};  // Stato logico attivo/disattivo
bool prevRawState[16] = {0}; // Stato fisico precedente per rilevare il fronte

unsigned long lastDebounceTime[16] = {0}; // timer per debounce di ogni pulsante
const unsigned long debounceDelay = 20;   // in millisecondi
bool stableRawState[16] = {0};            // stato ritenuto stabile (usato per toggle)


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
  eth.init(openRelay);
  digitalWrite(LED_BUILTIN, HIGH); // LED spento all'avvio
  for (int i = 0; i < 16; ++i)
  {
    EXT_IN.pinMode(i, INPUT);   // tutti gli ingressi come input
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

void readAndUpdateStates()
{
  bool prevLogicalState[16];
  memcpy(prevLogicalState, buttonState, sizeof(buttonState));

  // 1. Leggi i 16 bit fisici dall’expander
  lastState = state;
  uint16_t raw = getState();
  state = raw;

  if (state != lastState)
  {
    lastActivityTime = millis();
    Serial.print("State changed: ");
    for (int i = 15; i >= 0; --i)
    {
      Serial.print((state >> i) & 0x01);
      if (i % 3 == 0 && i != 0)
        Serial.print(" ");
    }
    Serial.println();
  }

  // 2. Gestione del toggle per fronte di salita
  for (int i = 0; i < 16; ++i)
  {
    bool reading = (raw >> i) & 0x01; // lettura fisica grezza

    if (reading != prevRawState[i])
    {
      lastDebounceTime[i] = millis(); // è cambiato → azzera il timer
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay)
    {
      // valore stabile: se è diverso da quello salvato, aggiornalo
      if (reading != stableRawState[i])
      {
        stableRawState[i] = reading;

        // Fronte di salita (premuto ora, non era prima)
        if (stableRawState[i] && !toggleState[i])
        {
          toggleState[i] = true;
          Serial.print("Button ");
          Serial.print(i);
          Serial.println(" pressed, new state: ON");
        }
      }
    }

    // aggiorno prevRawState e buttonState
    prevRawState[i] = reading;
    buttonState[i] = toggleState[i];
  }

  // 3. Gestione gruppi (1 attivo per gruppo)
  const int GROUPS = 5;
  const int GROUP_SIZE = 3;

  for (int g = 0; g < GROUPS; ++g)
  {
    int base = g * GROUP_SIZE;
    int lastPressed = -1;

    // Cerca il tasto logicamente attivo e appena cambiato
    for (int j = 0; j < GROUP_SIZE; ++j)
    {
      int idx = base + j;
      if (!prevLogicalState[idx] && buttonState[idx])
        lastPressed = idx;
    }

    // Se c'è un nuovo attivo nel gruppo, disattiva gli altri
    if (lastPressed != -1)
    {
      for (int j = 0; j < GROUP_SIZE; ++j)
      {
        int idx = base + j;
        if (idx != lastPressed)
        {
          buttonState[idx] = false;
          toggleState[idx] = false;
        }
      }
    }
  }

  // 4. Calcolo del nuovo stato binario da scrivere in output
  uint16_t newOutput = 0;
  uint16_t oldOutput = 0;

  for (int i = 0; i < 16; ++i)
  {
    if (buttonState[i])
      newOutput |= (1 << i);
    if (prevLogicalState[i])
      oldOutput |= (1 << i);
  }

  // 5. Scrivi il nuovo stato se c'è differenza
  if (newOutput != oldOutput)
  {
    Serial.print("Writing new state: ");
    for (int i = 15; i >= 0; --i)
    {
      Serial.print((newOutput >> i) & 0x01);
      if (i % 3 == 0 && i != 0)
        Serial.print(" ");
    }
    Serial.println();
    setOut(newOutput); // Uscita fisica (LED, relay, ecc.)
  }
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
    delay(5000);
    // calcola un nuovo stato random che non sia uguale a quello attuale
    uint16_t newState;
    do
    {
      newState = random(0, 0xFFFF);
    } while (newState == state);
    // aggiorna lo stato dei pulsanti
    for (int i = 0; i < 16; ++i)
    {
      toggleState[i] = (newState >> i) & 0x01;
      buttonState[i] = toggleState[i];
    }
    // aggiorna lo stato fisico
    state = newState;
    // scrivi il nuovo stato
    setOut(newState);
    // qui potresti anche inviare un pacchetto via ethernet,
    // oppure attivare un buzzer, ecc.
  }
  else
  {
    // non-vincita: LED rosso o spento
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
      setOut(rnd); // scrivo un nuovo stato random
      Serial.println("=== RANDOM STATE ===");
      lastRandomToggle = now;
      //resettiamo anch e lo stato di tutto a 0
      for (int i = 0; i < 16; ++i)
      {
        toggleState[i] = false;
        buttonState[i] = false;
        buttonStateOld[i] = false;
        stableRawState[i] = false;
      }
    }
  }
}

void loop()
{
  readAndUpdateStates();
  verifyWin();
  eth.loop();
  doLightGame();
}

uint16_t getState()
{
  PCF8575::DigitalInput di = EXT_IN.digitalReadAll();
  const uint8_t logicalToPhysical[16] = {
      7, 6, 5, 4, 3, 2, 1, 0,
      8, 9, 10, 11, 12, 13, 14, 15};

  uint16_t raw = 0;

  for (uint8_t i = 0; i < 16; ++i)
  {
    bool bitValue = false;
    switch (logicalToPhysical[i])
    {
    case 0:
      bitValue = di.p0;
      break;
    case 1:
      bitValue = di.p1;
      break;
    case 2:
      bitValue = di.p2;
      break;
    case 3:
      bitValue = di.p3;
      break;
    case 4:
      bitValue = di.p4;
      break;
    case 5:
      bitValue = di.p5;
      break;
    case 6:
      bitValue = di.p6;
      break;
    case 7:
      bitValue = di.p7;
      break;
    case 8:
      bitValue = di.p8;
      break;
    case 9:
      bitValue = di.p9;
      break;
    case 10:
      bitValue = di.p10;
      break;
    case 11:
      bitValue = di.p11;
      break;
    case 12:
      bitValue = di.p12;
      break;
    case 13:
      bitValue = di.p13;
      break;
    case 14:
      bitValue = di.p14;
      break;
    case 15:
      bitValue = di.p15;
      break;
    }
    raw |= (!bitValue << i);
  }

  // Serial.print("IN :");
  // for (int i = 15; i >= 0; --i)
  // {
  //  Serial.print((raw >> i) & 0x01);
  // }
  // Serial.println();
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
}
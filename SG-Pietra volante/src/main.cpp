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

IPAddress staticIP(192, 168, 1, 204);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13803;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAE}, serverPort);

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  ESP.restart();
}

void resetGame() {
  // -------------------------------------------------------------------
  // 1) Spegniamo subito entrambi i motori, per sicurezza
  // -------------------------------------------------------------------
  digitalWrite(INPUT_11, LOW);  // Motore 1 OFF
  digitalWrite(INPUT_12, LOW);  // Motore 2 OFF

  Serial.println("------ resetGame() inizio ------");

  // -------------------------------------------------------------------
  // 2) Leggiamo subito lo stato iniziale dei pin di “richiesta reset”
  //    Stampa lo stato di INPUT_1..INPUT_8 per capire chi richiede reset
  // -------------------------------------------------------------------
  Serial.printf("INPUT_1=%d, INPUT_2=%d, INPUT_3=%d, INPUT_4=%d\n",
                digitalRead(INPUT_1), digitalRead(INPUT_2),
                digitalRead(INPUT_3), digitalRead(INPUT_4));
  Serial.printf("INPUT_5=%d, INPUT_6=%d, INPUT_7=%d, INPUT_8=%d\n",
                digitalRead(INPUT_5), digitalRead(INPUT_6),
                digitalRead(INPUT_7), digitalRead(INPUT_8));

  // -------------------------------------------------------------------
  // 3) Determiniamo quali motori effettivamente serve resettare
  //    (se uno qualunque dei quattro pin dedicati al motore è LOW)
  // -------------------------------------------------------------------
  bool do1 = (!digitalRead(INPUT_1) ||
              !digitalRead(INPUT_2) ||
              !digitalRead(INPUT_3) ||
              !digitalRead(INPUT_4));

  bool do2 = (!digitalRead(INPUT_5) ||
              !digitalRead(INPUT_6) ||
              !digitalRead(INPUT_7) ||
              !digitalRead(INPUT_8));

  Serial.printf("Condizione reset: do1=%d, do2=%d\n", do1, do2);

  // -------------------------------------------------------------------
  // 4) Se serve, accendiamo immediatamente i relè dei motori
  // -------------------------------------------------------------------
  if (do1) {
    digitalWrite(INPUT_11, HIGH);
    Serial.println("[Motore 1] Accensione per RESET (INPUT_11=HIGH)");
  } else {
    Serial.println("[Motore 1] Reset NON richiesto");
  }

  if (do2) {
    digitalWrite(INPUT_12, HIGH);
    Serial.println("[Motore 2] Accensione per RESET (INPUT_12=HIGH)");
  } else {
    Serial.println("[Motore 2] Reset NON richiesto");
  }

  // -------------------------------------------------------------------
  // 5) Preparo le variabili per il polling dei finecorsa
  //    last9/last10 conterranno l’ultimo valore “stabile” del pin
  // -------------------------------------------------------------------
  bool last9  = digitalRead(INPUT_9);   // stato iniziale finecorsa mot1
  bool last10 = digitalRead(INPUT_10);  // stato iniziale finecorsa mot2

  unsigned long start    = millis();
  const unsigned long timeout = 5000;   // timeout massimo in ms

  Serial.println("Inizio gestione finecorsa:");

  // -------------------------------------------------------------------
  // 6) SE il finecorsa è già a LOW subito all’avvio, allora
  //    “aspettiamo” che torni a HIGH prima di considerare la caduta
  //    effettiva. In pratica, gestiamo il caso “già in posizione finale”.
  // -------------------------------------------------------------------
  if (do1 && last9 == LOW) {
    Serial.println("[Motore 1] Finecorsa iniziale (INPUT_9) già LOW!");
    Serial.println("           Attendo che INPUT_9 diventi HIGH prima di contare il reset...");
    // Aspetto che il pin torni HIGH, o timeout
    while (digitalRead(INPUT_9) == LOW && (millis() - start < timeout)) {
      delay(5);
    }
    if (digitalRead(INPUT_9) == HIGH) {
      Serial.printf("[Motore 1] INPUT_9 tornato HIGH dopo %lums, pronto a monitorare la caduta.\n",
                    (millis() - start));
      last9 = HIGH;
    } else {
      Serial.printf("[Motore 1] Timeout mentre attendevo INPUT_9=HIGH (t=%lums).\n",
                    (millis() - start));
    }
  }

  if (do2 && last10 == LOW) {
    Serial.println("[Motore 2] Finecorsa iniziale (INPUT_10) già LOW!");
    Serial.println("           Attendo che INPUT_10 diventi HIGH prima di contare il reset...");
    // Aspetto che il pin torni HIGH, o timeout
    while (digitalRead(INPUT_10) == LOW && (millis() - start < timeout)) {
      delay(5);
    }
    if (digitalRead(INPUT_10) == HIGH) {
      Serial.printf("[Motore 2] INPUT_10 tornato HIGH dopo %lums, pronto a monitorare la caduta.\n",
                    (millis() - start));
      last10 = HIGH;
    } else {
      Serial.printf("[Motore 2] Timeout mentre attendevo INPUT_10=HIGH (t=%lums).\n",
                    (millis() - start));
    }
  }

  // -------------------------------------------------------------------
  // 7) Ora entriamo nel polling principale, dove guardiamo la caduta
  //    HIGH → LOW (falling-edge) per spegnere i motori.
  // -------------------------------------------------------------------
  Serial.println("Polling per rilevare FALLING-EDGE dei finecorsa (max 5000 ms)...");

  // Per non inondare di messaggi, stampiamo lo stato solo ogni 200 ms
  unsigned long lastLog = 0;

  while ((do1 || do2) && (millis() - start < timeout)) {
    bool curr9  = digitalRead(INPUT_9);
    bool curr10 = digitalRead(INPUT_10);
    unsigned long elapsed = millis() - start;

    // Stampiamo lo stato dei finecorsa ogni ~200 ms
    if (millis() - lastLog >= 200) {
      Serial.printf(" t=%lums | finecorsa1(INPUT_9)=%d, finecorsa2(INPUT_10)=%d\n",
                    elapsed, curr9, curr10);
      lastLog = millis();
    }

    // -------------------------------
    // Motore 1: se stavo resettando e vedo FALLING-EDGE su INPUT_9
    // (last9 == HIGH && curr9 == LOW), allora spengo e segno do1=false
    // -------------------------------
    if (do1 && last9 == HIGH && curr9 == LOW) {
      digitalWrite(INPUT_11, LOW);
      do1 = false;
      Serial.printf("[Motore 1] FALLING-EDGE su INPUT_9 a t=%lums → spengo INPUT_11 e concludo reset.\n",
                    elapsed);
    }

    // -------------------------------
    // Motore 2: analogo a Motore 1, verso il pin INPUT_10
    // -------------------------------
    if (do2 && last10 == HIGH && curr10 == LOW) {
      digitalWrite(INPUT_12, LOW);
      do2 = false;
      Serial.printf("[Motore 2] FALLING-EDGE su INPUT_10 a t=%lums → spengo INPUT_12 e concludo reset.\n",
                    elapsed);
    }

    // Aggiorno i “last” per il prossimo giro
    last9  = curr9;
    last10 = curr10;
    delay(5);
  }

  // -------------------------------------------------------------------
  // 8) Verifichiamo se siamo usciti per timeout o perché entrambi i reset
  //    si sono completati.
  // -------------------------------------------------------------------
  unsigned long totalElapsed = millis() - start;
  if (do1 || do2) {
    // Uno (o entrambi) non hanno mai visto la caduta entro i 5 s
    Serial.printf("Attenzione: timeout dopo %lums. Stati finali: do1=%d, do2=%d\n",
                  totalElapsed, do1, do2);
  } else {
    // Entrambi hanno terminato correttamente
    Serial.printf("Reset completato correttamente in %lums.\n", totalElapsed);
  }

  // -------------------------------------------------------------------
  // 9) Per sicurezza, forziamo di nuovo entrambi i motori su OFF
  // -------------------------------------------------------------------
  digitalWrite(INPUT_11, LOW);
  digitalWrite(INPUT_12, LOW);
  Serial.println("[Sicurezza] INPUT_11 e INPUT_12 forzati LOW");
  Serial.println("------ resetGame() terminato ------\n");
}


void setup()
{
  Serial.begin(115200);
  delay(1000);
  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
  pinMode(INPUT_3, INPUT_PULLUP);
  pinMode(INPUT_4, INPUT_PULLUP);
  pinMode(INPUT_5, INPUT_PULLUP);
  pinMode(INPUT_6, INPUT_PULLUP);
  pinMode(INPUT_7, INPUT_PULLUP);
  pinMode(INPUT_8, INPUT_PULLUP);
  pinMode(INPUT_9, INPUT_PULLUP);
  pinMode(INPUT_10, INPUT_PULLUP);
  pinMode(INPUT_11, OUTPUT);
  pinMode(INPUT_12, OUTPUT);
  
  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay, resetGame);
  //print
  resetGame(); // Reset del gioco all'avvio
  Serial.println("Game reset complete.");

}

void loop()
{
  bool input1 = digitalRead(INPUT_1); //1
  bool input2 = digitalRead(INPUT_2); //2
  bool input3 = digitalRead(INPUT_3); //3
  bool input4 = digitalRead(INPUT_4); //4
  bool input5 = digitalRead(INPUT_5); //5
  bool input6 = digitalRead(INPUT_6); //6
  bool input7 = digitalRead(INPUT_7); //7
  bool input8 = digitalRead(INPUT_8); //8 
  //se sono tutti LOW
  if (!input1 && !input2 && !input3 && !input4 && !input5 && !input6 && !input7 && !input8) {
    Serial.println("All inputs are LOW, resetting game...");
    openRelay();
    delay(2000);
    resetGame();
  }
  //Serial.printf("Input 1: %d, Input 2: %d, Input 3: %d, Input 4: %d, Input 5: %d, Input 6: %d, Input 7: %d, Input 8: %d\n", input1, input2, input3, input4, input5, input6, input7, input8);
  eth.loop(); // Gestione della connessione Ethernet
  delay(10);
}

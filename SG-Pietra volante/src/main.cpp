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
IPAddress serverIP(192, 168, 1, 109);
int serverPort = 13802;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAE}, serverPort);

void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
}

void resetGame() {
  // Assicuriamoci che i due motori siano spenti
  digitalWrite(INPUT_11, LOW);  // Motore 1 OFF
  digitalWrite(INPUT_12, LOW);  // Motore 2 OFF

  // Verifico se devo resettare ciascun motore
  bool do1 = (!digitalRead(INPUT_1) ||
              !digitalRead(INPUT_2) ||
              !digitalRead(INPUT_3) ||
              !digitalRead(INPUT_4));

  bool do2 = (!digitalRead(INPUT_5) ||
              !digitalRead(INPUT_6) ||
              !digitalRead(INPUT_7) ||
              !digitalRead(INPUT_8));

  // Avvio i motori in parallelo se servono
  if (do1) digitalWrite(INPUT_11, HIGH);
  if (do2) digitalWrite(INPUT_12, HIGH);

  // Preparo il polling di fine corsa per entrambi
  bool last9  = HIGH;
  bool last10 = HIGH;
  unsigned long start = millis();
  const unsigned long timeout = 5000; // ms di sicurezza

  // Finché almeno uno dei due è “in lavoro” e non scade il timeout...
  while ((do1 || do2) && (millis() - start < timeout)) {
    bool curr9  = digitalRead(INPUT_9);
    bool curr10 = digitalRead(INPUT_10);

    // Motore 1: se stavo facendo reset e vedo falling edge su 9 → spengo
    if (do1 && last9 == HIGH && curr9 == LOW) {
      digitalWrite(INPUT_11, LOW);
      do1 = false;
    }

    // Motore 2: se stavo facendo reset e vedo falling edge su 10 → spengo
    if (do2 && last10 == HIGH && curr10 == LOW) {
      digitalWrite(INPUT_12, LOW);
      do2 = false;
    }

    last9  = curr9;
    last10 = curr10;
    delay(5);
  }

  // Sicurezza: spegniamoli entrambi
  digitalWrite(INPUT_11, LOW);
  digitalWrite(INPUT_12, LOW);

  Serial.println("resetGame(): sequenza parallela terminata");
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
  }


  Serial.printf("Input 1: %d, Input 2: %d, Input 3: %d, Input 4: %d, Input 5: %d, Input 6: %d, Input 7: %d, Input 8: %d\n", input1, input2, input3, input4, input5, input6, input7, input8);
  digitalWrite(RELAY_PIN, !input1 || !input2 || !input3 || !input4 || !input5 || !input6 || !input7 || !input8 );
 
  eth.loop(); // Gestione della connessione Ethernet
  delay(10); // Aggiungi un piccolo delay per evitare di saturare il core
}

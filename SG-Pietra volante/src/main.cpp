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
  //eth.init(openRelay);
  //print

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
  
  bool currentInput9 = digitalRead(INPUT_9);
  bool currentInput10 = digitalRead(INPUT_10);

  static bool lastInput9 = HIGH; // Stato precedente di INPUT_9
  static bool lastInput10 = HIGH; // Stato precedente di INPUT_10

  // MOTORE 1
  if (!input1 && !input2 && !input3 && !input4) {
    digitalWrite(INPUT_11, HIGH); // Attiva motore 1
  }
  // Rileva falling edge su INPUT_9
  if (lastInput9 == HIGH && currentInput9 == LOW) {
    digitalWrite(INPUT_11, LOW);  // Disattiva motore 1
  }

  // MOTORE 2
  if (!input5 && !input6 && !input7 && !input8) {
    digitalWrite(INPUT_12, HIGH); // Attiva motore 2
  }
  // Rileva falling edge su INPUT_10
  if (lastInput10 == HIGH && currentInput10 == LOW) {
    digitalWrite(INPUT_12, LOW);  // Disattiva motore 2
  }

  // Aggiorna i precedenti
  lastInput9 = currentInput9;
  lastInput10 = currentInput10;


  Serial.printf("Input 1: %d, Input 2: %d, Input 3: %d, Input 4: %d, Input 5: %d, Input 6: %d, Input 7: %d, Input 8: %d\n", input1, input2, input3, input4, input5, input6, input7, input8);
  digitalWrite(RELAY_PIN, !input1 || !input2 || !input3 || !input4 || !input5 || !input6 || !input7 || !input8 );
 
  
  delay(10); // Aggiungi un piccolo delay per evitare di saturare il core
}

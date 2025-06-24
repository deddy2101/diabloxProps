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
#define NUM_LEDS 5
#define DATA_PIN INPUT_6
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

IPAddress staticIP(192, 168, 1, 202);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13807;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte,6>{0xDE,0xAD,0xBE,0xEF,0xFE,0xAF}, serverPort);

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

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
  pinMode(INPUT_1, INPUT_PULLUP);
  pinMode(INPUT_2, INPUT_PULLUP);
  pinMode(INPUT_3, INPUT_PULLUP);
  pinMode(INPUT_4, INPUT_PULLUP);
  pinMode(INPUT_5, INPUT_PULLUP);
  pinMode(INPUT_6, OUTPUT);
  //pinMode(INPUT_6, INPUT_PULLUP);
  
  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay);
  //print

}
bool input1_state = false;
bool input2_state = false;
bool input3_state = false;
bool input4_state = false;
bool input5_state = false;
bool last_input1_state = false;
bool last_input2_state = false; 
bool last_input3_state = false;
bool last_input4_state = false;
bool last_input5_state = false;

const int inputs[] = {INPUT_1, INPUT_2, INPUT_3, INPUT_4, INPUT_5};
bool *input_states[] = {&input1_state, &input2_state, &input3_state, &input4_state, &input5_state};

void updateLed(){
  // Assumiamo NUM_LEDS == 5 e input_states[0] punta a input_1, ..., input_states[4] a input_5
  for (int i = 0; i < NUM_LEDS; ++i) {
    // Mappo led[i] all'input opposto: led[0] → input_states[4], led[1] → input_states[3], ...
    int sensorIndex = NUM_LEDS - 1 - i;
    bool state = *input_states[sensorIndex];

    if (state) {
      // se HIGH (true) → verde
      leds[i] = CRGB::Green;
    } else {
      // se LOW (false) → rosso
      leds[i] = CRGB::Red;
    }
  }
  FastLED.show();
}

void loop()
{
  for (int i = 0; i < 5; ++i)
  {
    bool input = digitalRead(inputs[i]);
    // if input is true skip
    if (input == HIGH)
    {
      continue; // Skip if input is HIGH
    }
    if (!input != *input_states[i])
    {
      *input_states[i] = !input;
      if (input == LOW)
      {
        Serial.print("Input ");
        Serial.print(i + 1);
        Serial.println(" ON");
      }
    }
  }

  updateLed();
  //if all are true 
  if (input1_state && input2_state && input3_state && input4_state && input5_state)
  {
    openRelay();
    delay(1000);
    ESP.restart();
  }

  if (last_input1_state != input1_state || last_input2_state != input2_state || last_input3_state != input3_state || last_input4_state != input4_state || last_input5_state != input5_state)
  {
    Serial.printf("Input states changed: %d %d %d %d %d\n", input1_state, input2_state, input3_state, input4_state, input5_state);
  }
  last_input1_state = input1_state;
  last_input2_state = input2_state;
  last_input3_state = input3_state;
  last_input4_state = input4_state;
  last_input5_state = input5_state;
  eth.loop();
}

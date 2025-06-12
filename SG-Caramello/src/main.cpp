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
#define NUM_LEDS 22
#define DATA_PIN 16
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

CRGB leds[NUM_LEDS];

volatile bool toggle1 = false;
volatile bool toggle2 = false;
volatile bool toggle3 = false;
volatile bool toggle4 = false;
volatile bool buttonpressed = false;
bool relayState = false;

IPAddress staticIP(192, 168, 1, 201);
IPAddress dnsServer(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnetMask(255, 255, 255, 0);
IPAddress serverIP(192, 168, 1, 9);
int serverPort = 13801;
EthernetConnection eth(staticIP, dnsServer, gateway, subnetMask, serverIP, std::array<byte, 6>{0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAB}, serverPort);
void openRelay()
{
  digitalWrite(RELAY_PIN, HIGH);
  delay(200);
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  ESP.restart();
}
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

void handleResetButtonPress()
{
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay)
  {
    relayState = !relayState;
    lastInterruptTime = currentTime;
    buttonpressed = true;
  }
}

bool input1_state = false;
bool input2_state = false;
bool input3_state = false;
bool input4_state = false;
bool input5_state = false;
bool input6_state = false;
bool input7_state = false;
bool input8_state = false;
bool input9_state = false;
bool input10_state = false;
bool input11_state = false;
bool input12_state = false;

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
  pinMode(INPUT_11, INPUT_PULLUP);
  pinMode(INPUT_12, OUTPUT);

  Serial.println("Starting...");
  FastLED.addLeds<WS2812B, INPUT_12, GRB>(leds, NUM_LEDS);
  eth.setLEDS(leds, NUM_LEDS);
  pinMode(RELAY_PIN, OUTPUT);
  eth.init(openRelay);
  //test all the leds
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i] = CRGB::Red;
  }
  FastLED.show();
  delay(1000);
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
  // print
}
const int inputs[] = {INPUT_1, INPUT_2, INPUT_3, INPUT_4, INPUT_5, INPUT_6, INPUT_7, INPUT_8, INPUT_9, INPUT_10, INPUT_11};
bool *input_states[] = {&input1_state, &input2_state, &input3_state, &input4_state, &input5_state, &input6_state, &input7_state, &input8_state, &input9_state, &input10_state, &input11_state};

void updateLed()
{
  // 1) Contiamo quanti input_states sono true
  int nled = 0;
  for (int i = 0; i < 11; ++i)
  {
    if (*input_states[i])
      nled++;
  }

  // 2) Ogni input true vale 2 LED, quindi:
  int nToLight = nled * 2;

  // 3) Non superare mai il numero totale di LED disponibile
  if (nToLight > NUM_LEDS) {
    nToLight = NUM_LEDS;
  }

  // 4) Accendiamo i primi nToLight LED, spegniamo gli altri
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    if (i < nToLight)
      leds[i] = CRGB::Blue;
    else
      leds[i] = CRGB::Black;
  }

  FastLED.show();
}


void blackAllLeds()
{
  for (int i = 0; i < NUM_LEDS; ++i)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}
void loop()
{

  for (int i = 0; i < 11; ++i)
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
  // if all are true
  if (input1_state && input2_state && input3_state && input4_state && input5_state && input6_state && input7_state && input8_state && input9_state && input10_state && input11_state)
  {
    Serial.println("All inputs are ON");
   // eth.apiCall("ALL_ON");
    openRelay();
    delay(5000);
    blackAllLeds();
    //reset all inputs
    for (int i = 0; i < 11; ++i)
    {
      *input_states[i] = false;
    }
    updateLed();
    Serial.println("All inputs reset to OFF");
    
  }
  else
  {
  }
  updateLed();
  eth.loop();
}

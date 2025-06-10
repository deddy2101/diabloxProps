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
#define NUM_LEDS 12
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
  pinMode(INPUT_5, OUTPUT);
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
const int inputs[] = {INPUT_1, INPUT_2, INPUT_3, INPUT_4};
bool *input_states[] = {&input1_state, &input2_state, &input3_state, &input4_state};
 
void updateLed(){
  //there are 3 led for each input 0, 1, 2 for inputState1, 3,4,5 for inputState2, 6,7,8 for inputState3, 9,10,11 for inputState4 turn on all 3 for each input that is HIGH
  for (int i = 0; i < 4; ++i)
  {
    if (*input_states[i]) // If the input state is HIGH
    {
      leds[i * 3] = CRGB::Green; // Turn on first LED
      leds[i * 3 + 1] = CRGB::Green; // Turn on second LED
      leds[i * 3 + 2] = CRGB::Green; // Turn on third LED
    }
    else // If the input state is LOW
    {
      leds[i * 3] = CRGB::Black; // Turn off first LED
      leds[i * 3 + 1] = CRGB::Black; // Turn off second LED
      leds[i * 3 + 2] = CRGB::Black; // Turn off third LED
    }
  }
  FastLED.show(); // Update the LEDs
}

void loop()
{
  for (int i = 0; i < 4; ++i)
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

  Serial.printf("Input 1: %d, Input 2: %d, Input 3: %d, Input 4: %d\n", input1_state, input2_state, input3_state, input4_state);
  eth.loop();
}

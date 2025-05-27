#include "EthernetConnection.h"

EthernetConnection::EthernetConnection(IPAddress ip,
                                       IPAddress dns,
                                       IPAddress gw,
                                       IPAddress mask,
                                       IPAddress serverIP,
                                       int serverPort) : server(80)
{
  /*
  mac[0] = 0xDE;
  mac[1] = 0xAD;
  mac[2] = 0xBE;
  mac[3] = 0xEF;
  mac[4] = 0xFE;
  mac[5] = 0xAE;
  */
  // Inizializza il MAC address con un valore di esempio
  mac = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAE}; // esempio di MAC address

  this->ip = ip;
  this->dns = dns;
  this->gw = gw;
  this->mask = mask;
  this->serverIP = serverIP;
  this->serverPort = serverPort;
}

EthernetConnection::EthernetConnection(IPAddress ip,
                                       IPAddress dns,
                                       IPAddress gw,
                                       IPAddress mask,
                                       IPAddress serverIP,
                                       const std::array<byte, 6> &macIn,
                                       int serverPort)
    : ip(ip), dns(dns), gw(gw), mask(mask), serverIP(serverIP), mac(macIn), serverPort(serverPort), server(80)
{
  // eventuale setup addizionale...
}

void EthernetConnection::init(void (*callback)(), void (*resetCallback)())
{
  this->callback = callback;
  this->resetCallback = resetCallback;
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(200);
  digitalWrite(ETH_RST, HIGH);
  // print the spi pin
  Serial.printf("\033[1;33mEterneth Init\033[0m\n");
  Serial.printf("\033[1;33mMISO : %d\033[0m\n", MISO);
  Serial.printf("\033[1;33mMOSI : %d\033[0m\n", MOSI);
  Serial.printf("\033[1;33mSCK : %d\033[0m\n", SCK);
  Serial.printf("\033[1;33mSS : %d\033[0m\n", SS);

  Serial.printf("\033[1;33mBegin Eterneth\033[0m\n");
  Serial.printf("\033[1;33mMAC Address : %02X:%02X:%02X:%02X:%02X:%02X\033[0m\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf("\033[1;33mIP Address : %d.%d.%d.%d\033[0m\n", ip[0], ip[1], ip[2], ip[3]);
  Serial.printf("\033[1;33mDNS Server : %d.%d.%d.%d\033[0m\n", dns[0], dns[1], dns[2], dns[3]);
  Serial.printf("\033[1;33mGateway : %d.%d.%d.%d\033[0m\n", gw[0], gw[1], gw[2], gw[3]);
  Serial.printf("\033[1;33mSubnet Mask : %d.%d.%d.%d\033[0m\n", mask[0], mask[1], mask[2], mask[3]);
  // spi pin
  Ethernet.init(12);
  // print the spi pin

  if (Ethernet.begin(mac.data()))
  { // Dynamic IP setup
    Serial.printf("\033[1;32mDHCP OK!\033[0m\n");
  }
  else
  {
    Serial.printf("\033[1;33mFailed to configure Ethernet using DHCP\033[0m\n");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.printf("\033[1;31m[E] Ethernet shield was not found.  Sorry, can't run without hardware. :(\033[0m\n");
      return;
      while (true)
      {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.printf("\033[1;31m[E] Ethernet cable is not connected.\033[0m\n");
    }
    Ethernet.begin(mac.data(), ip, dns, gw, mask);
    Serial.printf("\033[1;32mSTATIC OK!\033[0m\n");
  }
  initialized = true;
  Ethernet.begin(mac.data(), ip, dns, gw, mask);
  Serial.printf("\033[1;33mLocal IP : %s\033[0m\n", Ethernet.localIP().toString().c_str());
  Serial.printf("\033[1;33mSubnet Mask : %s\033[0m\n", Ethernet.subnetMask().toString().c_str());
  Serial.printf("\033[1;33mGateway IP : %s\033[0m\n", Ethernet.gatewayIP().toString().c_str());
  Serial.printf("\033[1;33mDNS Server : %s\033[0m\n", Ethernet.dnsServerIP().toString().c_str());
}

bool EthernetConnection::apiCall(String action)
{
  // the structure is @ + action + commandSplitter
  String message = "@" + action + commandSplitter;
  // check if client is connected
  if (!client.connected())
  {
    Serial.printf("Client not connected. Retry.");
    return false;
  }
  else
  {
    Serial.printf("Sending message: %s\n", message.c_str());
    client.print(message);
    return true;
  }
}
void EthernetConnection::loop()
{
  if (!initialized)
  {
    Serial.printf("Ethernet not initialized. Call init() first.\n");
    return;
  }
  // check if client is connected
  if (!client.connected())
  {
    for (int i = 0; i < this->numLEDs; i++)
    {
      this->leds[i] = CRGB::Yellow;
    }
    FastLED.show();
    client.stop();
    connect();
  }
  else
  {
    handleIncomingMessage();
  }
}

bool EthernetConnection::connect()
{
  if (client.connect(serverIP, serverPort))
  {
    for (int i = 0; i < this->numLEDs; i++)
    {
      this->leds[i] = CRGB::Green;
    }
    FastLED.show();
    client.flush();
    Serial.printf("Connected.\n");
    return true;
  }
  else
  {
    Serial.printf("Connection failed. Retry.\n");
    return false;
  }
}

void EthernetConnection::processIncomingMessage(String message)
{
  // rimuovo splitter
  int idx = message.indexOf(commandSplitter);
  if (idx != -1)
    message.remove(idx, commandSplitter.length());

  Serial.printf("Message received (cleaned): %s\n", message.c_str());

  // split action e (eventuale) numero
  String action;
  int relayNum = 0;
  int spacePos = message.indexOf(' ');
  if (spacePos == -1)
  {
    action = message; // es. "on" o "reset"
  }
  else
  {
    action = message.substring(0, spacePos);
    relayNum = message.substring(spacePos + 1).toInt();
  }

  // caso "on" senza numero → callback principale
  if (action == "on" && relayNum == 0)
  {
    if (callback)
      callback();
    Serial.println("Relay opened.");
    return;
  }

  // caso "reset"
  if (action == "reset")
  {
    if (resetCallback)
      resetCallback();
    Serial.println("Relay reset.");
    return;
  }

  // array di puntatori alle variabili di stato
  bool *relays[4] = {relaystate1, relaystate2, relaystate3, relaystate4};

  // on/off con indice 1..4
  if ((action == "on" || action == "off") && relayNum >= 1 && relayNum <= 4 && relays[relayNum - 1] != nullptr)
  {
    *relays[relayNum - 1] = (action == "on");
    Serial.printf("Relay %d %s.\n",
                  relayNum,
                  action == "on" ? "opened" : "closed");
    return;
  }

  Serial.printf("Unknown command: %s\n", message.c_str());
}

void EthernetConnection::handleIncomingMessage()
{
  if (client.available() > 0)
  {
    char thisChar = client.read();
    readLine += String(thisChar);
    if (readLine.endsWith(commandSplitter))
    {
      client.flush();
      Serial.printf("Line received from server:");
      processIncomingMessage(readLine);
      readLine = "";
    }
  }
}
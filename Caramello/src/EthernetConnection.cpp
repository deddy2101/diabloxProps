#include "EthernetConnection.h"

EthernetConnection::EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, int serverPort) : server(80)
{
  mac[0] = 0xDE;
  mac[1] = 0xAD;
  mac[2] = 0xBE;
  mac[3] = 0xEF;
  mac[4] = 0xFE;
  mac[5] = 0xEE;

  this->ip = ip;
  this->dns = dns;
  this->gw = gw;
  this->mask = mask;
  this->serverIP = serverIP;
  this->serverPort = serverPort;
}

void EthernetConnection::init(bool *relayState)
{
  this->relayState = relayState;
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(200);
  digitalWrite(ETH_RST, HIGH);
  //print the spi pin
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

  if (Ethernet.begin(mac))
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
    Ethernet.begin(mac,ip, dns, gw, mask);
    Serial.printf("\033[1;32mSTATIC OK!\033[0m\n");
  }
  Ethernet.begin(mac, ip, dns, gw, mask);
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

  int index = message.indexOf(commandSplitter);
  if (index != -1)
  {
    message.remove(index, commandSplitter.length());
  }

  // Stampa il messaggio ricevuto senza il commandSplitter
  Serial.printf("Message received (cleaned): %s\n", message.c_str());

  // Converti il messaggio in una costante per il switch
  const char *command = message.c_str();

  // Usa uno switch per gestire i comandi
  if (strcmp(command, "on") == 0)
  {
    if (relayState != nullptr)
    {
      *relayState = true;
      Serial.printf("Relay state set to ON\n");
    }
  }
  else if (strcmp(command, "reset") == 0)
  {
    if (relayState != nullptr)
    {
      *relayState = false;
      Serial.printf("Relay state set to OFF\n");
    }
  }
  else
  {
    Serial.printf("Unknown command: %s\n", command);
  }
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
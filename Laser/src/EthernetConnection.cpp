#include "EthernetConnection.h"

EthernetConnection::EthernetConnection() : server(80)
{
  mac[0] = 0xDE;
  mac[1] = 0xAD;
  mac[2] = 0xBE;
  mac[3] = 0xEF;
  mac[4] = 0xFE;
  mac[5] = 0xEE;

  ip[0] = 192;
  ip[1] = 168;
  ip[2] = 1;
  ip[3] = 150;

  dns[0] = 8;
  dns[1] = 8;
  dns[2] = 8;
  dns[3] = 8;

  gw[0] = 192;
  gw[1] = 168;
  gw[2] = 1;
  gw[3] = 1;

  mask[0] = 255;
  mask[1] = 255;
  mask[2] = 255;
  mask[3] = 0;

  serverIP[0] = 192;
  serverIP[1] = 168;
  serverIP[2] = 1;
  serverIP[3] = 109;
}

void EthernetConnection::init(bool *relayState)
{
  this->relayState = relayState;
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(200);
  digitalWrite(ETH_RST, HIGH);
  printf("\033[1;33mBegin Eterneth\033[0m\n");
  printf("\033[1;33mMAC Address : %02X:%02X:%02X:%02X:%02X:%02X\033[0m\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  printf("\033[1;33mIP Address : %d.%d.%d.%d\033[0m\n", ip[0], ip[1], ip[2], ip[3]);
  printf("\033[1;33mDNS Server : %d.%d.%d.%d\033[0m\n", dns[0], dns[1], dns[2], dns[3]);
  printf("\033[1;33mGateway : %d.%d.%d.%d\033[0m\n", gw[0], gw[1], gw[2], gw[3]);
  printf("\033[1;33mSubnet Mask : %d.%d.%d.%d\033[0m\n", mask[0], mask[1], mask[2], mask[3]);
  // spi pin
  Ethernet.init(10);
  // print the spi pin

  if (Ethernet.begin(mac))
  { // Dynamic IP setup
    printf("\033[1;32mDHCP OK!\033[0m\n");
  }
  else
  {
    printf("\033[1;33mFailed to configure Ethernet using DHCP\033[0m\n");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      printf("\033[1;31m[E] Ethernet shield was not found.  Sorry, can't run without hardware. :(\033[0m\n");
      return;
      while (true)
      {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
      printf("\033[1;31m[E] Ethernet cable is not connected.\033[0m\n");
    }
    Ethernet.begin(mac, ip, dns, gw, mask);
    printf("\033[1;32mSTATIC OK!\033[0m\n");
  }
  Ethernet.begin(mac, ip, dns, gw, mask);
  printf("\033[1;33mLocal IP : %s\033[0m\n", Ethernet.localIP().toString().c_str());
  printf("\033[1;33mSubnet Mask : %s\033[0m\n", Ethernet.subnetMask().toString().c_str());
  printf("\033[1;33mGateway IP : %s\033[0m\n", Ethernet.gatewayIP().toString().c_str());
  printf("\033[1;33mDNS Server : %s\033[0m\n", Ethernet.dnsServerIP().toString().c_str());
  delay(1000);
  initServer();
}

bool EthernetConnection::apiCall(String roomID, String action)
{
  // String roomID = ""; // Set the roomID if necessary
  // String action = "{846e92d0-299c-454b-a799-3b4227ddb862}"; // Set the action as needed
  String url = "http://" + serverIP.toString() + ":" + String(serverPort) + "/ersapi/runaction?roomID=" + roomID + "&action=" + action;
  printf("URL: %s\n", url.c_str());
  // Aprire una connessione al server
  if (client.connect(serverIP, serverPort))
  {
    printf("Connected to server\n");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + serverIP.toString() + "\r\n" +
                 "Connection: close\r\n\r\n");
    // Wait for server response
    while (client.connected() || client.available())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        Serial.println(line);
      }
    }
    client.stop();
    printf("Connection closed\n");
    return true;
  }
  else
  {
    printf("Connection failed\n");
    return false;
  }
  return false;
}

void EthernetConnection::initServer()
{
}

String readString = String(30);
void EthernetConnection::loop()
{
  // print thet is looping
  EthernetClient client = server.available();
  if (client)
  {
    boolean currentLineIsBlank = true;
    readString = "";
    boolean endofFirstLine = false;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();

        // first time encounter a line return say true
        if (c == '\n' && endofFirstLine == false)
          endofFirstLine = true;

        // MySerial.print(c); //print what server receives to serial monitor

        // read char by char HTTP request, limit to checking the first 30 and stop after first line
        if ((readString.length() < 30) && (endofFirstLine == false))
        {
          // store characters to string
          readString += c;
        }

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank)
        {

          // if readString starts with /s serve sensor data
          if (readString.indexOf("/r") > 0)
          {
            *relayState = false;

            printf("Relay is off");

            break;
          }
          else if (readString.indexOf("/o") > 0)
          {
            *relayState = true;

            printf("Relay is on");

            break;
          }
        }
        if (c == '\n')
        {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r')
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <Ethernet.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>  

#define ETH_RST 46
//this use the default SPI pins
class Base;
class EthernetConnection
{
public:
    EthernetConnection();
    void init(bool *relayState);
    EthernetClient client;
    void loop();
    IPAddress ip;
    bool apiCall(String url);


private:

    bool *relayState;
  
    byte mac[6]; // Provide a size for the mac array
    Base *base;
    IPAddress dns;
    IPAddress gw;
    IPAddress mask;
    IPAddress serverIP; // IP del server
    int serverPort = 8080; // Porta del server
    AsyncWebServer server;
    void initServer();
};

#endif // ETHERNETCONNECTION_H
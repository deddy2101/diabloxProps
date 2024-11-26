#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <EthernetESP32.h>

#define ETH_RST 46
//this use the default SPI pins
class Base;
class EthernetConnection
{
public:
    EthernetConnection();
    void init(bool *relayState);
    EthernetClient client;
    EthernetServer server;
    void loop();
    IPAddress ip;
    bool apiCall(String url);


private:
    void handleRoot();
    void handleText();
    bool *relayState;
  
    byte mac[6]; // Provide a size for the mac array
    Base *base;
    IPAddress dns;
    IPAddress gw;
    IPAddress mask;
    IPAddress serverIP; // IP del server
    int serverPort = 8080; // Porta del server
    void initServer();
};

#endif // ETHERNETCONNECTION_H
#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <EthernetESP32.h>

#define ETH_RST 46
//this use the default SPI pins
class EthernetConnection
{
public:
    EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, int serverPort);
    void init(bool *relayState);
    EthernetClient client;
    EthernetServer server;
    void loop();
    IPAddress ip;
    bool apiCall(String action);


private:
     bool *relayState;
  
    byte mac[6]; // Provide a size for the mac array
    IPAddress dns;
    IPAddress gw;
    IPAddress mask;
    IPAddress serverIP; // IP del server
    int serverPort; // Porta del server
    String commandSplitter = "_NEXT_ERSCOMMAND_";
    bool connect();
    void handleIncomingMessage();
    void processIncomingMessage(String message);
    String readLine = "";
};

#endif // ETHERNETCONNECTION_H
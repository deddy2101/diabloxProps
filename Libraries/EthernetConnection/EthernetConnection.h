#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <EthernetESP32.h>
#include <FastLED.h>

#define ETH_RST 46
//this use the default SPI pins
class EthernetConnection
{
public:
    EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, int serverPort);
    EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, const std::array<byte,6>& mac,int serverPort);
    void(*callback)();
    

    void init(void (*callback)(), void (*resetCallback)() = nullptr);
    void(*resetCallback)();
    EthernetClient client;
    EthernetServer server;
    void setLEDS(CRGB *leds, size_t numLEDs) 
    { 
        this->leds = leds; 
        this->numLEDs = numLEDs; 
    }
    void loop();
    IPAddress ip;
    bool apiCall(String action);


private:
     bool *relayState;

     CRGB *leds = nullptr; // Puntatore all'array di LED
    size_t numLEDs = 0;   // Numero di LED nell'array
    std::array<byte,6> mac;
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
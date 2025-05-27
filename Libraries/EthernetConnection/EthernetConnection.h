#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <EthernetESP32.h>
#include <FastLED.h>
#define ETH_RST 46
// this use the default SPI pins
class EthernetConnection
{
public:
    EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, int serverPort);
    EthernetConnection(IPAddress ip, IPAddress dns, IPAddress gw, IPAddress mask, IPAddress serverIP, const std::array<byte, 6> &mac, int serverPort);
    
    void init(void (*callback)(), void (*resetCallback)() = nullptr);
    void init(void (*callback)(), void (*resetCallback)(), bool *relayState1, bool *relayState2, bool *relayState3, bool *relayState4)
    {
        this->callback = callback;
        this->resetCallback = resetCallback;
        this->relaystate1 = relayState1;
        this->relaystate2 = relayState2;
        this->relaystate3 = relayState3;
        this->relaystate4 = relayState4;
        init(callback, resetCallback);
    }

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
    void (*resetCallback)() = nullptr; // Callback per il reset
    void (*callback)() = nullptr;      // Callback per l'apertura del relay
    bool *relayState;
    bool initialized = false; // Indica se la connessione è stata inizializzata

    CRGB *leds = nullptr; // Puntatore all'array di LED
    size_t numLEDs = 0;   // Numero di LED nell'array
    std::array<byte, 6> mac;
    IPAddress dns;
    IPAddress gw;
    IPAddress mask;
    IPAddress serverIP; // IP del server
    int serverPort;     // Porta del server
    String commandSplitter = "_NEXT_ERSCOMMAND_";
    bool connect();
    void handleIncomingMessage();
    void processIncomingMessage(String message);
    String readLine = "";

    bool *relaystate1 = nullptr; // Puntatore allo stato del relay 1
    bool *relaystate2 = nullptr; // Puntatore allo stato del relay 2
    bool *relaystate3 = nullptr; // Puntatore allo stato del relay 3
    bool *relaystate4 = nullptr; // Puntatore allo stato del relay 4
};

#endif // ETHERNETCONNECTION_H
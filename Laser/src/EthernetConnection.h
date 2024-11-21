#ifndef ETHERNETCONNECTION_H
#define ETHERNETCONNECTION_H

#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>  

#define ETH_RST 46
#define LED_ETH_OK 2
#define LED_SRV_OK 3
//this use the default SPI pins
class Base;
class EthernetConnection
{
public:
    EthernetConnection();
    void init();
    EthernetClient client;
    void loop();
    IPAddress ip;
    void connectTCPSocket();
    void sendTCPMessage(String message);
    void sendTagID(String tagID, long packetID);
    void setBase(Base *base) { this->base = base; };
    void sendSignedTag(String tagID); // Send the signed tag to the remote
    void sendResettedBase();
    void sendResettedCancello();
    void sendStopSigning();
    void sendRequestTime();
    void sendGateOpened();

private:
    void checkPHYConnection();
    void checkIncomingMessage();
    void handleMessage(long packetId, int packetType, byte data[6]);
    byte mac[6]; // Provide a size for the mac array
    Base *base;
    IPAddress dns;
    IPAddress gw;
    IPAddress mask;
    IPAddress serverIP; // IP del server
    int serverPort = 8080; // Porta del server
};

#endif // ETHERNETCONNECTION_H
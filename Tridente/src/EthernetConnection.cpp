#include "EthernetConnection.h"

EthernetConnection::EthernetConnection() :  server(80)
{
  mac[0] = 0xDE;
  mac[1] = 0xAD;
  mac[2] = 0xBE;
  mac[3] = 0xEF;
  mac[4] = 0xFE;
  mac[5] = 0xED;

  ip[0] = 10;
  ip[1] = 0;
  ip[2] = 152;
  ip[3] = 7;

  dns[0] = 8;
  dns[1] = 8;
  dns[2] = 8;
  dns[3] = 8;

  gw[0] = 10;
  gw[1] = 0;
  gw[2] = 152;
  gw[3] = 1;

  mask[0] = 255;
  mask[1] = 255;
  mask[2] = 255;
  mask[3] = 0;

  serverIP[0] = 10;
  serverIP[1] = 0;
  serverIP[2] = 100;
  serverIP[3] = 26;
  // 151.16.177.62
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
  //spi pin
  Ethernet.init(10);
  //print the spi pin 
  

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

bool EthernetConnection::apiCall(String url) {
  // Aprire una connessione al server
  if (client.connect(serverIP, serverPort)) {
    // Costruire la richiesta HTTP GET
    client.print("GET ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(serverIP.toString()); // Specifica l'host
    client.println("Connection: close");
    client.println(); // Fine dell'header HTTP

    // Attendi la risposta
    unsigned long timeout = millis();
    while (client.connected() && !client.available()) {
      if (millis() - timeout > 5000) {
        Serial.println("Timeout durante l'attesa della risposta");
        client.stop();
        return false; // Fallimento della chiamata
      }
    }

    // Leggi la risposta
    String response = "";
    while (client.available()) {
      char c = client.read();
      response += c;
    }

    // Stampa la risposta per il debug
    Serial.println("Risposta del server:");
    Serial.println(response);

    // Chiudi la connessione
    client.stop();

    // Puoi processare la risposta se necessario
    if (response.indexOf("200 OK") != -1) {
      return true; // Successo
    } else {
      return false; // Fallimento
    }
  } else {
    // Non è stato possibile connettersi al server
    Serial.println("Connessione al server fallita");
    return false;
  }
}


void EthernetConnection::initServer() {
    // Endpoint per la root "/"
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "ciao");
    });

    // Endpoint per "/reset"
    server.on("/reset", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "ok");
        *relayState = false;
        // Aggiungi la logica di reset
        Serial.println("Richiesta di reset ricevuta");
        // Puoi aggiungere un reset hardware o altre azioni qui
        digitalWrite(ETH_RST, LOW);
        delay(200);
        digitalWrite(ETH_RST, HIGH);
    });

    // Avvia il server
    server.begin();
    Serial.println("Server HTTP avviato");
}

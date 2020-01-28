/*
 * INCOMPLETE! Intended to connect to the arduino and send the "touch" events
 * to esp nodes over udp.
 *
 * Since its arduino mega, Serial2 could be used.
 *
 * Adapted from the esp32-arduino example.
 *
 * Idea was:
 *   ┌───────────┐                ┌──────────────────────────────────────┐
 *   │ ┌───────┐ │                │ Arduino Mega    ┌─────────────────┐  │
 *   │ │       │ │                │                 │                 │  │
 *   │ │  ESP  │ │                │                 │                 │  │
 *   │ │       │ │                │                 │      Cap. Touch │  │
 *   │ └───────┘ │     <->      ┌─┴──────┐          │                 │  │
 *   │           ├──────────────│Serial2 │          │                 │  │
 *   │           │              └─┬──────┘          └─────────────────┘  │
 *   │           │                │                                      │
 *   └────│──────┘                └──┬────────────────────────────────┬──┘
 *        │                          │     Lights                     │
 *   ┌────▽──────┐                   │                                │
 *   │ Broadcast │                   └────────────────────────────────┘
 *   │  to ESPs  │
 *   └───────────┘
 *
 */
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi network name and password:
const char * networkName = "NodeLinks";
const char * networkPswd = "nodelinker";

//IP address to send UDP data to:
// either use the ip address of the server or
// a network broadcast address
const char * udpAddress = "192.168.0.103";
const int udpPort = 21324;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

void setup(){
  Serial.begin(115200);
  Serial2.begin(115200);

  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void sendNotification() {
  // [!todo] Acually create the correct udp packet and send it. Should be added
  // to the loop.

  if(connected){
    //Send a packet
    udp.beginPacket(udpAddress, udpPort);
    udp.printf("Seconds since boot: %u", millis()/1000);
    udp.endPacket();
  }
}

void loop(){
  if (Serial2.available()) {
    // [!todo] If we read "E.TSTR" or "E.TEND" then send over udp notification
    // to turn on/off.

    int inByte = Serial2.read();
    Serial.println(inByte);
  }
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  // delete old config
  WiFi.disconnect(true);
  //register event handler
  WiFi.onEvent(WiFiEvent);

  //Initiate connection
  WiFi.begin(ssid, pwd);

  Serial.println("Waiting for WIFI connection...");
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());
          //initializes the UDP state
          //This initializes the transfer buffer
          udp.begin(WiFi.localIP(),udpPort);
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
    }
}

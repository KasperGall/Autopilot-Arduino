// Load Wi-Fi library

#include <WiFi.h>

#include <WebSocketsServer.h>




WebSocketsServer webSocket = WebSocketsServer(81);



// Replace with your network credentials

const char* ssid = "Vores_internet";

const char* password = "*************";

// Select LED output pin

#define LED_PIN  16



void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {

  const uint8_t* src = (const uint8_t*) mem;

  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);

  for (uint32_t i = 0; i < len; i++) {

    if (i % cols == 0) {

      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);

    }

    Serial.printf("%02X ", *src);

    src++;

  }

  Serial.printf("\n");

}



void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {



    switch(type) {

        case WStype_DISCONNECTED:

            Serial.printf("[%u] Disconnected!\n", num);

            break;

        case WStype_CONNECTED:

            {

                IPAddress ip = webSocket.remoteIP(num);

                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);



        // send message to client

        webSocket.sendTXT(num, "Connected");

            }

            break;

        case WStype_TEXT:

            Serial.printf("[%u] get Text: %s\n", num, payload);
            //const char* ettal = "1";
            if (strcmp((char*)payload, "1") == 0) {
              digitalWrite(LED_PIN,HIGH);
            } else {
              digitalWrite(LED_PIN,LOW);
            }

            // send message to client

            // webSocket.sendTXT(num, "message here");



            // send data to all connected clients

            // webSocket.broadcastTXT("message here");

            break;

        case WStype_BIN:

            Serial.printf("[%u] get binary length: %u\n", num, length);

            hexdump(payload, length);



            // send message to client

            // webSocket.sendBIN(num, payload, length);

            break;

    case WStype_ERROR:      

    case WStype_FRAGMENT_TEXT_START:

    case WStype_FRAGMENT_BIN_START:

    case WStype_FRAGMENT:

    case WStype_FRAGMENT_FIN:

      break;

    }



}



void setup() {

  Serial.begin(9600);



  // Connect to Wi-Fi network with SSID and password

  Serial.print("Connecting to ");

  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");

  }

  // Print local IP address

  Serial.println("");

  Serial.println("WiFi connected.");

  Serial.println("IP address: ");

  Serial.println(WiFi.localIP());



  // server address, port and URL

  webSocket.begin();



  // event handler

  webSocket.onEvent(webSocketEvent);

  //LED
  pinMode(LED_PIN,INPUT);

}



unsigned long timer = 0;

int counter = 0;



void loop() {

  webSocket.loop();

  


}

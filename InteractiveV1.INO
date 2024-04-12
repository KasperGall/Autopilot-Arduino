#include <WiFi.h>
#include <WebSocketsServer.h>
#include <Servo.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

//Opretter servomotrer, MPU og WebSocketServer
Servo servoMove;
Servo servoSteer;
Adafruit_MPU6050 mpu;
WebSocketsServer webSocket = WebSocketsServer(81);

//Variabler som holder styr på dele af programmet
char first_char; //Benyttes til at skive WebSocket-input ind i
int LEDto = 0; //Holder øje med timing af LED
bool LEDstate = false; //Holder øje med, hvordan LED skal være
int turn = 1; //Holder øje med position af roret (0, 1 eller 2)
int turnValue = 90; //Holder øje med værdi af ror således at den kan bevæge sig langsomt
int servoSteerWait; //Tæller millis for hvor hurtigt roret kan dreje
double compass; //Gemmer kompasretning - normalt fortæller MPU'en kun forskellen
int compassWait; //Tæller millis for hvor hurtigt heading skal sendes

//Info om nettet den skal oprette forbindelse til
const char* ssid = "";
const char* password = "";

//Definere ben komponenter sidder på
#define LED_PIN 16
#define servoMovePin 17
#define servoSteerPin 33

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    //Får WebSocket input og bruger switch-case alt efter type. Case'ne er enum typer.
    switch(type) {
        case WStype_DISCONNECTED:

            Serial.printf("[%u] Disconnected!\n", num);
            digitalWrite(LED_PIN,LOW);
            servoMove.write(90);
            servoSteer.write(90);
            break;

        case WStype_CONNECTED:

              IPAddress ip = webSocket.remoteIP(num);
              Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
              digitalWrite(LED_PIN,HIGH);
            break;

        case WStype_TEXT:
            first_char = (char)payload[0];
            if (first_char == 'f') {
              Serial.println("Moving forward");
              servoMove.write(110);
              turn = 1;
              LEDstate = true;
            } else if (first_char == 's') {
              Serial.println("Stopping");
              servoMove.write(90);
              turn = 1;
              LEDstate = false;
            } else if (first_char == 'b') {
              Serial.println("Backing up");
              servoMove.write(70);
              turn = 1;
              LEDstate = true;
            } else if (first_char == 'r') {
              Serial.println("Backing up");
              servoMove.write(110);
              turn = 0;
              LEDstate = true;
            } else if (first_char == 'l') {
              Serial.println("Backing up");
              servoMove.write(110);
              turn = 2;
              LEDstate = true;
            } else {
              Serial.println("ABORT");
              servoMove.write(90);
              turn = 1;
              LEDstate = false;
            }
            break;
    }
}

void setup() {
  Serial.begin(9600);

  //LED setup
  pinMode(LED_PIN,OUTPUT);

  //Servo setup
  servoMove.attach(servoMovePin);
  servoSteer.attach(servoSteerPin);
  servoMove.write(90);
  servoSteer.write(90);

  //Starter med at oprette fobindelse til WebSocket-internet
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //Skriver Serial prikker ind til forbindelsen er oprettet
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address
  Serial.println("\nWiFi connected - IP address: ");
  Serial.println(WiFi.localIP());

  //Opretter forbindelse til p5.js
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  //Opretter forbindelse til MPU
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void LEDController() {
//Styrer LED som blinker, hvis bilen kører, lyser konstant hvis der er forbindelse via WebSocket og ellers slukket
  if (LEDstate) {
    if (millis() >= LEDto) {
      LEDto = millis() + 2000;
    } else if (LEDto - millis() < 1000) {
      digitalWrite(LED_PIN,LOW);
    } else {
      digitalWrite(LED_PIN,HIGH);
    }
  } else {
    digitalWrite(LED_PIN,HIGH);
  }
}

void turnController() {
  //Blødgør at båden ror drejer langsomt og mindre voldsomt
  if (turn == 0 && turnValue > 30 && millis() >= servoSteerWait) {
    turnValue--;
    servoSteerWait = millis() + 20;
  } else if (turn == 1 && turnValue > 90 && millis() >= servoSteerWait) {
    turnValue--;
    servoSteerWait = millis() + 40;
  } else if (turn == 1 && turnValue < 90 && millis() >= servoSteerWait) {
    turnValue++;
    servoSteerWait = millis() + 20;
  } else if (turn == 2 && turnValue < 120 && millis() >= servoSteerWait) {
    turnValue++;
    servoSteerWait = millis() + 40;
  }
  servoSteer.write(turnValue);
}

void compassController() {
  if (millis() >= compassWait) {
    /* Get new sensor events with the readings */
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    compass = compass + g.gyro.z;
    String numberString = String(compass);
    webSocket.broadcastTXT(numberString);
    compassWait = millis() + 10;
  }
}

void loop() {
  //Modtag input hvert loop
  webSocket.loop();

  //Styrer LED
  LEDController();

  //Drejer samt blødgør drej
  turnController();

  //Sender compassinformationer
  compassController();
}

#include <WiFi.h>
#include <PubSubClient.h>
#include "mqtt_secrets.h"
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 4 
#define LEDPin 2

const char *ssid = "mati";
const char *password = "qwerty123";

const char MQTT_HOST[] = "mqtt3.thingspeak.com";
uint16_t MQTT_PORT = 1883;
char mqttUserName[] = SECRET_MQTT_USERNAME;            
char mqttPass[] = SECRET_MQTT_PASSWORD;
char clientID[] = SECRET_MQTT_CLIENT_ID;
long writeChannelID = 2366749;

int zap;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);



void connectWifi() {
  WiFi.begin(ssid, password);
  Serial.print("Łączenie z WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Połączono z siecią WiFi");
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LEDPin, HIGH);
}

void thingspeakConnect() {
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  while (!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    Serial.println("with clientID:" + String(mqttUserName));
    if (mqttClient.connect(clientID, mqttUserName, mqttPass)) {
      Serial.println("Connected with Client ID:  " + String(clientID));
    } else {
      Serial.print("failed, rc = ");
      Serial.println(mqttClient.state());
      Serial.println(" Will try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendToThingSpeak(int data) {
  String topicString = "channels/" + String(writeChannelID) + "/publish";
  String dataString = "&field1=" + String(data);

  if (!mqttClient.connected()) {
    thingspeakConnect();
  }
  mqttClient.publish(topicString.c_str(), dataString.c_str());
  Serial.println("Published to channel " + String(writeChannelID));
}

void loraReceive() {
  Serial.println("LoRa Receiver");

  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  delay(1000);

  while (1) {
    int packetSize = LoRa.parsePacket();
    int rssi = LoRa.packetRssi();
    if (packetSize) {
      while (LoRa.available()) {
        String LoRaData = LoRa.readStringUntil('\n');  
        Serial.println("Received LoRa data: " + LoRaData);
        zap = LoRaData.toInt();

        sendToThingSpeak(zap);

      }
    }
  }
}

void setup() {
  pinMode(LEDPin, OUTPUT);
  Serial.begin(115200);
  delay(1000);
  connectWifi();
  delay(1000);
  thingspeakConnect();
  delay(300);
  loraReceive();
  delay(100);
}

void loop() {

}

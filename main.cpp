#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WiFi.h"
#include <SPI.h>
#include <LoRa.h>

#define uS_TO_S_FACTOR 1000000ULL  
#define TIME_TO_SLEEP  28800

#define ss 26
#define rst 2
#define dio0 25
#define controlPin 0

const int trigPin = 12;
const int echoPin = 13;
const int trigPin2 = 14;
const int echoPin2 = 4;
const int LEDPin = 9;
const int maxDystans = 100;
const int alarmDystans = 10;
int zapelnienie2;
int zapelnienie1;
int zapelnienie;

int counter = 0;
 
RTC_DATA_ATTR int bootCount = 0;

 
void loraTranceive(){
  while (!Serial);
  Serial.println("LoRa Sender");

  
  LoRa.setPins(ss, rst, dio0);

  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  delay(1000);
}

void mierz_zapelnienie() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LEDPin, OUTPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(26, OUTPUT);

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long czas_trwania = pulseIn(echoPin, HIGH);
  float dystans = (czas_trwania * 34) / 1000 / 2;
  delay(1000);
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);
  long czas_trwania2 = pulseIn(echoPin2, HIGH);
  float dystans2 = (czas_trwania2 * 34) / 1000 / 2;
  delay(1000);
  if (dystans >= 500) dystans = 1;
  if (dystans2 >= 500) dystans2 = 1;
  if (dystans >= 0 && dystans <= maxDystans) {
    zapelnienie1 = map(dystans, 0, maxDystans, 100, 0);
  } else {
    zapelnienie1 = 0;  
  }

  if (dystans2 >= 0 && dystans2 <= maxDystans) {
    zapelnienie2 = map(dystans2, 0, maxDystans, 100, 0);
  } else {
    zapelnienie2 = 0;  
  }

  if (zapelnienie1 <= 0) zapelnienie1 = 0;
  if (zapelnienie2 <= 0) zapelnienie2 = 0;


  zapelnienie = (zapelnienie1 + zapelnienie2) / 2;

  if (zapelnienie >= 80 && zapelnienie < 95) {
    Serial.print("Zapelnienie: ");
    Serial.print(zapelnienie);
    Serial.print("% - Bliski zapelnienia!\n");
    digitalWrite(LEDPin, LOW);
  } else if (zapelnienie >= 95) {
    Serial.print("KONTENER ZAPELNIONY!\n");
    digitalWrite(LEDPin, HIGH);
  } else {
    Serial.print("Zapelnienie: ");
    Serial.print(zapelnienie);
    Serial.print("%\n");
    digitalWrite(LEDPin, LOW);
  }
}

void loraSendPacket() {
  Serial.print("Sending packet: ");
  Serial.println(counter);

  String dataToSend = String(zapelnienie) +
  "," + String(zapelnienie1) +
  "," + String(zapelnienie2);

  LoRa.beginPacket();
  LoRa.print(dataToSend);
  LoRa.endPacket();

  counter++;

  delay(2000);
}

void setup() {
  pinMode(controlPin, OUTPUT);
  digitalWrite(controlPin, HIGH);
  Serial.begin(115200);
  delay(500);
  ++bootCount;
  Serial.println("Wybudzenie nr: " + String(bootCount));
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  loraTranceive();
  delay(1000);
  mierz_zapelnienie();
  delay(1000);
  loraSendPacket();
  delay(1000);
  Serial.println("sleep...");
  digitalWrite(controlPin, LOW);
  esp_deep_sleep_start();
}
 
void loop() {

}
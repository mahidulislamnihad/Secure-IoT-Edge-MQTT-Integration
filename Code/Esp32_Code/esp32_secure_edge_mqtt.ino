#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include "credentials.h"
#include "ca_cert.h"  // Root CA certificate generated on Broker and shared with esp32 main.io



// Sensor Configuration
#define DHTPIN        4
#define DHTTYPE       DHT11
DHT dht(DHTPIN, DHTTYPE);

WiFiClientSecure espClient;
PubSubClient client(espClient);

//Connection Setup
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32Client", MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
    } else {
      Serial.print(" failed, rc=");
      Serial.print(client.state());
      Serial.println(", retry in 5s");
      delay(5000);
    }
  }
}

// Main Program 
void setup() {
  Serial.begin(9600);
  dht.begin();
  connectWiFi();

  // Load CA cert (generated on Raspberry Pi)
  espClient.setCACert(ca_cert);

  client.setServer(MQTT_HOST, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (!isnan(temp) && !isnan(hum)) {
    StaticJsonDocument<128> doc;
    doc["temperature"] = temp;
    doc["humidity"] = hum;

    char buffer[128];
    size_t n = serializeJson(doc, buffer);
    client.publish(MQTT_TOPIC, buffer, n);
    Serial.printf("Published: %s\n", buffer);
  }

  delay(5000);
}

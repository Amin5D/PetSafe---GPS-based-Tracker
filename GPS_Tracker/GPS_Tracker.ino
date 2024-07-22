#include <TinyGPS++.h>
#include <WiFi.h>
#include <PubSubClient.h>

// WiFi and MQTT broker credentials
const char* ssid = "Z 4 hotspot";
const char* password = "cvlk0930";
const char* mqtt_server = "192.168.155.113";


TinyGPSPlus gps;
WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial ss(1);

void setup() {
  Serial.begin(115200);
  ss.begin(9600, SERIAL_8N1, 16, 17); // RX, TX
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("gps/request");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  if (String(topic) == "gps/request") {
    publishGPSData();
  }
}

void publishGPSData() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      String latitude = String(gps.location.lat(), 6);
      String longitude = String(gps.location.lng(), 6);
      String payload = "{\"lat\":" + latitude + ",\"lon\":" + longitude + ",\"name\":\"Publisher1\",\"icon\":\"male\",\"color\":\"#000\"}";
      Serial.println(payload);

      if (!client.connected()) {
        reconnect();
      }
      client.loop();

      client.publish("gps/data", (char*) payload.c_str());
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

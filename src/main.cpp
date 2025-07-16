#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const int DHTPIN = 23;
const int DHTTYPE = DHT11;
const int moistureSensor = 36; // Capacitative Soil Moisture Sensor GPIO (ADC)
const char *ssid = "Jules";
const char *password = "aabbccdd";
const char *mqtt_server = "192.168.1.79";

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

struct PlantData
{
  int soilMoisture;
  float temperature;
  int humidity;
};

PlantData plant;

void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT Connection...");
    if (client.connect("ESP32PlantMonitor"))
    {
      Serial.println("Connected");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(9600); // Initialize serial communication (9600 baud rate)

  WiFi.begin(ssid, password); // Connected to WiFi
  while (WiFi.status() != WL_CONNECTED)
  { // Pause until connected
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWifi connected!");

  client.setServer(mqtt_server, 1883);

  dht.begin();
}
void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  plant.soilMoisture = analogRead(moistureSensor); // Set current values to variable
  plant.humidity = (int)dht.readHumidity();
  plant.temperature = dht.readTemperature();
  plant.temperature = (plant.temperature * 9.0/5.0) + 32.0;

  StaticJsonDocument<128> doc; // Create JSON doc
  doc["moisture"] = plant.soilMoisture;
  doc["humidity"] = plant.humidity; // Add sensor values to JSON doc
  doc["temperature"] = plant.temperature;
  char payload[128]; // Convert JSON to C string
  serializeJson(doc, payload);

  client.publish("plant/monitor", payload); // Send JSON string to plant/monitor MQTT
  delay(500);
}

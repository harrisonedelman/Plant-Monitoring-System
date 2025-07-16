import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point, WritePrecision
import json

INFLUXDB_URL = "http://localhost:8086"
INFLUXDB_TOKEN = "CzopsmDLeRTrsNF8wsc4kpdUoBqW03Sz1LLBhE-0jawilPi9qFSuWqtsDztfVKsmWAc92KOCeTvPs1imFm8ghA=="
INFLUXDB_ORG = "edelman"
INFLUXDB_BUCKET = "plantdata"

MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "plant/monitor"

influx_client = InfluxDBClient(
    url=INFLUXDB_URL,
    token=INFLUXDB_TOKEN,
    org=INFLUXDB_ORG
)

def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")
    try:
        data = json.loads(msg.payload.decode())
        point = (
            Point("plant_data")
            .field("moisture", int(data["moisture"]))
            .field("humidity", int(data["humidity"]))
            .field("temperature", float(data["temperature"]))
        )
        write_api = influx_client.write_api()
        write_api.write(bucket=INFLUXDB_BUCKET, org=INFLUXDB_ORG, record=point)
        print("Saved to InfluxDB!")
    except Exception as e:
        print("Error parsing or saving:", e)

mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message

mqtt_client.connect(MQTT_BROKER, MQTT_PORT)
mqtt_client.subscribe(MQTT_TOPIC)

print("Listening for MQTT messages...")
mqtt_client.loop_forever()

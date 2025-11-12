import paho.mqtt.client as mqtt
import ssl, json, time
from datetime import datetime

# CONFIG 
BROKER = "192.168.0.107"   # Raspberry Pi IP (Server PI)
PORT = 8883
TOPIC = "esp32/env_data"
USERNAME = "piuser"
PASSWORD = "mqtt_password"

CAFILE = "/etc/mosquitto/certs/ca.crt"

# MQTT CALLBACKS 
def on_connect(client, userdata, flags, rc, properties=None):
    print("Connected to broker")
    client.subscribe(TOPIC, qos=1)

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        temp = data.get("temperature", "N/A")
        hum = data.get("humidity", "N/A")
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Temp: {temp}°C | Humidity: {hum}%")
    except Exception as e:
        print("Parse error:", e)

#  TLS & RUN 
client = mqtt.Client()
client.username_pw_set(USERNAME, PASSWORD)

context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
context.verify_mode = ssl.CERT_REQUIRED
context.load_verify_locations(CAFILE)
context.set_ciphers("ECDHE-RSA-AES128-GCM-SHA256")

client.tls_set_context(context)

client.on_connect = on_connect
client.on_message = on_message

print("Connecting to broker…")
client.connect(BROKER, PORT, keepalive=60)
client.loop_forever()

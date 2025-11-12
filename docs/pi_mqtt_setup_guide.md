# MQTT over TLS Setup Guide (Raspberry Pi + ESP32)

This guide configures a secure **MQTT broker (Mosquitto)** on Raspberry Pi and connects an ESP32 client using **TLS encryption**.  
Each code block is directly runnable in your Pi terminal.

---

## 🧰 Prerequisites
- Raspberry Pi with Raspberry Pi OS  
- ESP32 board (Arduino core)  
- Packages: `mosquitto`, `mosquitto-clients`, `openssl`  
- Sudo access  
- Basic network connectivity  

---

### 1️⃣ Clean previous Mosquitto configuration
```bash
sudo systemctl stop mosquitto
sudo rm -rf /etc/mosquitto/conf.d/*
sudo rm -rf /etc/mosquitto/certs/
2️⃣ Install Mosquitto and OpenSSL
bash
Copy code
sudo apt update
sudo apt install -y mosquitto mosquitto-clients openssl
3️⃣ Generate TLS certificates (CA + Server)
Create working directory:

bash
Copy code
mkdir -p ~/mqtt_tls_auth
cd ~/mqtt_tls_auth
a) Create Certificate Authority (CA)
bash
Copy code
openssl genrsa -out ca.key 2048
openssl req -new -x509 -days 365 -key ca.key -out ca.crt
Common Name (CN): RaspberryPi-CA

b) Create Server key, CSR, and Certificate
bash
Copy code
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out server.crt -days 365 -sha256
Common Name (CN): your Pi’s IP or hostname.

4️⃣ Move certificates and set permissions
bash
Copy code
sudo mkdir -p /etc/mosquitto/certs
sudo cp ca.crt server.crt server.key /etc/mosquitto/certs/
sudo chown mosquitto:mosquitto /etc/mosquitto/certs/*
sudo chmod 644 /etc/mosquitto/certs/*.crt
sudo chmod 600 /etc/mosquitto/certs/server.key
5️⃣ Create MQTT username & password (optional)
bash
Copy code
sudo mosquitto_passwd -c /etc/mosquitto/passwd myuser
6️⃣ Create secure Mosquitto configuration
bash
Copy code
sudo nano /etc/mosquitto/conf.d/secure.conf
Paste this inside:

bash
Copy code
listener 8883
protocol mqtt

# TLS configuration
cafile /etc/mosquitto/certs/ca.crt
certfile /etc/mosquitto/certs/server.crt
keyfile /etc/mosquitto/certs/server.key
tls_version tlsv1.2
ciphers ECDHE-RSA-AES128-GCM-SHA256

# Authentication
allow_anonymous false
password_file /etc/mosquitto/passwd
Save & exit, then restart Mosquitto:

bash
Copy code
sudo systemctl restart mosquitto
sudo systemctl enable mosquitto
sudo systemctl status mosquitto
7️⃣ Generate ca_cert.h for ESP32
bash
Copy code
awk 'BEGIN{print "const char ca_cert[] PROGMEM = R\"EOF("} {print} END{print ")EOF\";"}' \
  /etc/mosquitto/certs/ca.crt > ca_cert.h
Copy ca_cert.h into your ESP32 project folder and include:

cpp
Copy code
#include "ca_cert.h"
8️⃣ ESP32 (Arduino) setup note
Use WiFiClientSecure, call espClient.setCACert(ca_cert);,
and publish like:

cpp
Copy code
client.publish("esp32/env_data", payload);
9️⃣ Subscribe securely from Raspberry Pi
bash
Copy code
mosquitto_sub -h localhost -p 8883 \
  -u "myuser" -P "your_password" \
  --cafile /etc/mosquitto/certs/ca.crt \
  -t "esp32/env_data" -v

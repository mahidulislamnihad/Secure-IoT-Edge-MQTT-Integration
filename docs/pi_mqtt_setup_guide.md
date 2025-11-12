# MQTT over TLS Setup Guide (Raspberry Pi + ESP32)

This guide explains how to configure a **secure MQTT broker** on a Raspberry Pi (Mosquitto) and connect an ESP32 client using **TLS encryption**.  
It is written as a terminal-ready, step-by-step reference — copy the `bash` blocks into your Pi shell.

---

## Prerequisites
- Raspberry Pi with Raspbian / Raspberry Pi OS
- ESP32 board (Arduino core)
- `mosquitto`, `mosquitto-clients`, and `openssl`
- Basic network connectivity
- Sudo access on the Pi

---

## 1) Clean previous Mosquitto configuration (optional but recommended)
```bash
sudo systemctl stop mosquitto
sudo rm -rf /etc/mosquitto/conf.d/*
sudo rm -rf /etc/mosquitto/certs/
2) Install Mosquitto and OpenSSL
sudo apt update
sudo apt install -y mosquitto mosquitto-clients openssl

3) Generate TLS certificates (CA and server)

Create working directory:

mkdir -p ~/mqtt_tls_auth
cd ~/mqtt_tls_auth

3.a Create a Certificate Authority (CA)
openssl genrsa -out ca.key 2048
openssl req -new -x509 -days 365 -key ca.key -out ca.crt


When prompted, enter information; the Common Name (CN) can be RaspberryPi-CA or similar.

3.b Create Server key, CSR and certificate
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr
# In the CSR prompts, set Common Name (CN) to your Pi IP or hostname
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
  -out server.crt -days 365 -sha256

4) Move certificates to Mosquitto directory and set permissions
sudo mkdir -p /etc/mosquitto/certs
sudo cp ca.crt server.crt server.key /etc/mosquitto/certs/


Set ownership & permissions (important!)

# Owner: mosquitto, secure permission for private key
sudo chown mosquitto:mosquitto /etc/mosquitto/certs/*
sudo chmod 644 /etc/mosquitto/certs/*.crt    # CA + server cert readable
sudo chmod 600 /etc/mosquitto/certs/server.key  # private key only readable by owner


chmod 600 on the server private key is critical to avoid Mosquitto refusing to load the key.

5) Create MQTT username/password (optional but recommended)
# Create or update password file and add a user
sudo mosquitto_passwd -c /etc/mosquitto/passwd myuser
# (enter the password when prompted)

6) Create secure Mosquitto config

Create a new file:

sudo nano /etc/mosquitto/conf.d/secure.conf


Paste the following content:

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


Save and exit (Ctrl+O, Enter, Ctrl+X).

Then restart Mosquitto:

sudo systemctl restart mosquitto
sudo systemctl enable mosquitto
sudo systemctl status mosquitto   # verify running

7) Generate ca_cert.h for ESP32 (convenient method)

This creates a header file containing the CA certificate as a C string (ESP32-friendly).

awk 'BEGIN{print "const char ca_cert[] PROGMEM = R\"EOF("} {print} END{print ")EOF\";"}' \
  /etc/mosquitto/certs/ca.crt > ca_cert.h


Copy ca_cert.h to your ESP32 project folder and include:

#include "ca_cert.h"


Alternatively, you can store ca.crt on the ESP32 filesystem (SPIFFS/LittleFS) and load it at runtime, but embedding the CA is simpler for demos.

8) Minimal ESP32 (Arduino) test notes

In your sketch, use WiFiClientSecure and call espClient.setCACert(ca_cert); before connecting.

Use your MQTT username & password created earlier (or configure certificate-based auth later).

Example publish command (on ESP32):

client.publish("esp32/env_data", payload);

9) Test subscribing from the Pi (secure)
mosquitto_sub -h localhost -p 8883 \
  -u "myuser" -P "your_password" \
  --cafile /etc/mosquitto/certs/ca.crt \
  -t "esp32/env_data" -v


should see JSON messages published by the ESP32

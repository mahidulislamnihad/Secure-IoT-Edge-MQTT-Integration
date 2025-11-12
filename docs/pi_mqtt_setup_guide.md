MQTT over TLS Setup Guide (Raspberry Pi + ESP32)

This guide explains how to configure a secure MQTT broker on a Raspberry Pi and connect an ESP32 client using TLS encryption.
It ensures encrypted communication between devices in a local edge IoT environment.

🧩 Prerequisites
Raspberry Pi (Raspbian OS)
ESP32 board
Mosquitto + OpenSSL
Arduino IDE


Basic network connectivity
1. Clean Existing Mosquitto Configuration
sudo systemctl stop mosquitto
sudo rm -rf /etc/mosquitto/conf.d/*
sudo rm -rf /etc/mosquitto/certs/

2. Install Mosquitto and OpenSSL
sudo apt update
sudo apt install -y mosquitto mosquitto-clients openssl

3. Generate TLS Certificates (CA + Server)
Create a secure directory:
mkdir -p ~/mqtt_tls_auth
cd ~/mqtt_tls_auth

--Create a Certificate Authority (CA)
openssl genrsa -out ca.key 2048
openssl req -new -x509 -days 365 -key ca.key -out ca.crt
The Common Name (CN) can be anything, e.g., RaspberryPi-CA.

--Create Server Key, CSR, and Certificate
openssl genrsa -out server.key 2048
openssl req -new -key server.key -out server.csr
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial \
-out server.crt -days 365 -sha256
Must Use your Raspberry Pi’s IP address as the Common Name (CN).

4. Move Certificates to Mosquitto Directory
sudo mkdir -p /etc/mosquitto/certs
sudo cp *.crt *.key /etc/mosquitto/certs/
sudo chown mosquitto: /etc/mosquitto/certs/*
sudo chmod 600 /etc/mosquitto/certs/server.key

5. Create MQTT Username and Password
sudo mosquitto_passwd -c /etc/mosquitto/passwd myuser
Replace myuser with your desired MQTT username.


6. Configure Mosquitto for Secure Communication
Create and edit:
sudo nano /etc/mosquitto/conf.d/secure.conf


Paste:

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


Save and exit, then restart Mosquitto: 

sudo systemctl restart mosquitto
sudo systemctl enable mosquitto

💡 7. Generate ca_cert.h for ESP32

Run on Raspberry Pi:
awk 'BEGIN{print "const char ca_cert[] PROGMEM = R\"EOF("} {print} END{print ")EOF\";"}' \
/etc/mosquitto/certs/ca.crt > ca_cert.h


Copy ca_cert.h to your ESP32 project folder and include it:

#include "ca_cert.h"

🚀 8. Test Secure MQTT Communication
On ESP32

Upload your secure MQTT firmware that uses the CA certificate.

(b) On Raspberry Pi

Subscribe to the topic securely:

mosquitto_sub -h localhost -p 8883 \
-u "myuser" -P "your_password" \
--cafile /etc/mosquitto/certs/ca.crt \
-t "esp32/env_data" -v


You should see live temperature and humidity readings from the ESP32.

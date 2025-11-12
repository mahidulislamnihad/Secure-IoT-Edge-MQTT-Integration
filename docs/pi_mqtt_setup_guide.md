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

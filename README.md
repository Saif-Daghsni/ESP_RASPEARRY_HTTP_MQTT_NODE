# ESP32 IoT Environmental Monitoring & Data Streaming System

## 📋 Overview
This project implements an IoT solution where an ESP32 reads temperature and humidity values from a DHT11 sensor and sends the data to a Raspberry Pi via MQTT.  
The Raspberry Pi acts as both **MQTT broker** and **gateway** to a Node-RED dashboard.  
The ESP32 also uploads the same sensor data to **ThingSpeak** using HTTP for cloud visualization and charting.

---

## 🚀 Features
- Real-time temperature & humidity monitoring (DHT11 → ESP32)
- MQTT data transmission from ESP32 to Raspberry Pi
- Raspberry Pi as **Mosquitto broker** + **Node-RED host**
- Node-RED dashboard for live sensor visualization
- Cloud data uploading to ThingSpeak through HTTP
- Dual data streaming: **MQTT + HTTP**
- Automatic reconnection and stable communication

---

## 🛠️ Hardware Requirements
- ESP32 development board  
- DHT11 temperature & humidity sensor  
- Raspberry Pi (with Mosquitto + Node-RED)  
- Jumper wires, breadboard  
- USB cable (ESP32 connected to PC)

---

## 📡 System Architecture

```
DHT11 → ESP32 → MQTT → Raspberry Pi (Broker + Gateway) → Node-RED Dashboard
      ↘
        HTTP → ThingSpeak Cloud → Charts & Analytics
```


---

## 🔄 Data Flow
1. DHT11 sensor is connected to the ESP32.  
2. ESP32 is powered and programmed from the PC.  
3. ESP32 sends temperature & humidity to the Raspberry Pi using MQTT (via Pi IP).  
4. Raspberry Pi receives the MQTT messages and exposes them to Node-RED.  
5. Node-RED displays real-time values on the dashboard.  
6. ESP32 also sends the same data to ThingSpeak using HTTP requests.  
7. Raspberry Pi is used through the university’s computer peripherals.

---



## 🖥️ Dashboard Features
- Real-time temperature and humidity gauges  
- Historical trend charts  
- LED control toggle button  

---

## 🎥 Démo  
https://drive.google.com/file/d/1Vlfj0a7QMISLZJAqD-XoTgDpcHZoZpPd/view?usp=drive_link







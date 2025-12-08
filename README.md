# ESP32 IoT Environmental Monitoring & Control System

## 📋 Project Overview
A comprehensive IoT system that monitors temperature and humidity using an ESP32 with DHT11 sensor, publishes data to an MQTT broker and ThingSpeak cloud, and enables remote LED control through a Node-RED dashboard.

---

## 🎯 Features
- **Real-time Monitoring**: Reads temperature and humidity from DHT11 sensor  
- **Dual Data Streaming**: Publishes to MQTT broker and ThingSpeak  
- **Remote Control**: MQTT-controlled LED via Node-RED dashboard  
- **Fault Tolerance**: Retry mechanisms for sensor readings  
- **Retained Messages**: MQTT retain flag for instant subscriber updates  
- **Visual Dashboard**: Node-RED interface for monitoring and control  

---

## 🛠️ Hardware Requirements
- ESP32 Development Board  
- DHT11 Temperature & Humidity Sensor  
- LED  
- Jumper wires, breadboard  
- USB cable
- 
---

## 📊 Software Architecture

ESP32 → WiFi → [MQTT Broker] → Node-RED Dashboard
      ↘
        [ThingSpeak Cloud] → Charts & Analytics

---

## 📈 Data Flow
- ESP32 reads the DHT11 sensor every 5 seconds  
- Publishes temperature and humidity to MQTT with the retain flag enabled  
- Sends the same data to ThingSpeak via HTTP POST requests  
- Node-RED subscribes to MQTT topics and displays real-time readings  
- LED is controlled through Node-RED → MQTT → ESP32  

---

## 🔒 Error Handling & Recovery
- Automatic WiFi reconnection on disconnect  
- DHT11 retry mechanism (2 attempts per reading)  
- MQTT auto-reconnection handling  
- Validation of sensor values before publishing  
- HTTP timeout protection (5 seconds) for ThingSpeak requests  

---

## 🖥️ Dashboard Features
- Real-time temperature and humidity gauges  
- Historical trend charts  
- LED control toggle button  
- Connection and sensor status indicators  
- Data logging and visualization tools  

---

## 🎥 Démo  



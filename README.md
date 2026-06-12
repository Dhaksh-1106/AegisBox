# 🛡️ AegisBox — Intelligent Cold-Chain Logistics Monitor

AegisBox is an end-to-end IoT and edge-computing solution designed to monitor high-value, sensitive cargo such as transplant organs, vaccines, and other temperature-sensitive medical assets during transit.

The system utilizes a defensive hybrid architecture that combines deterministic hardware interrupts for anti-tamper protection with asynchronous network pipelines for real-time telemetry streaming. Environmental conditions, physical impacts, and enclosure integrity are continuously monitored and transmitted to a live web dashboard through MQTT and WebSocket technologies.

---

# 🚀 Key Features

### 🔒 Defensive Edge Logic
Implements a non-blocking network state machine that ensures cloud outages, broker failures, or network interruptions never interfere with critical local sensor monitoring and processing.

### ⚡ Guaranteed Latch Security
Uses a hardware Interrupt Service Routine (ISR) connected to a Light Dependent Resistor (LDR) to detect enclosure breaches within microseconds, eliminating polling-delay vulnerabilities.

### 📡 Real-Time Data Pipeline
Streams lightweight serialized JSON telemetry through MQTT using the public broker:

`test.mosquitto.org`

### 🌐 Asynchronous Web Interface
A FastAPI-powered backend serves an event-driven dashboard that receives live updates through WebSockets.

### 📊 Dynamic Viability Index
Calculates a real-time health score based on:

- Temperature
- Humidity
- Kinetic Impact
- Chamber Seal Integrity

The score provides a quick assessment of cargo safety and viability.

---

# 🏗️ System Architecture

```text
Sensors
│
├── DHT11 (Temperature & Humidity)
├── MPU6050 (Motion / G-Force)
└── LDR + ISR (Tamper Detection)
│
▼
ESP32 Edge Controller
│
├── Local Processing
├── Viability Index Calculation
├── Alert Generation
└── MQTT Telemetry Publishing
│
▼
test.mosquitto.org Broker
│
▼
FastAPI Backend
│
▼
WebSocket Server
│
▼
Live Dashboard Interface
```

---

# 📂 Project Directory Structure

```text
AEGISBOX/                    
│
├── AegisBox_Firmware/
│   ├── include/
│   ├── lib/
│   ├── src/                     # ESP32 firmware source code
│   └── test/
│
├── AegisBox_WebStack/
│   ├── backend/
│   │   └── server.py            # FastAPI backend server
│   │
│   └── frontend/
│       └── index.html           # Live dashboard interface
│
├── platformio.ini               # Embedded dependency manager
└── requirements.txt             # Python dependencies
```

---

# 🔧 Technical Threshold Configuration

The system evaluates environmental and kinetic parameters using a defensive safety model.

For demonstrations and presentations, thresholds may be configured using the following optimized values:

| Parameter | Optimal Range (Green) | Warning Boundary (Yellow) | Critical Trigger (Red)|
|------------|----------------------|--------------------------|------------------------|
| Temperature | 24°C – 28°C | 18°C – 23°C or 29°C – 32°C | <18°C or >32°C |
| Humidity | 40% – 60% | 30% – 39% or 61% – 75% | <30% or >75% |
| G-Force | ≤ 1.2g | 1.21g – 1.8g | >1.8g |
| Chamber Seal | Sealed (Dark) | N/A | Light Detected |

---

# 📈 Viability Index Logic

The Viability Index is calculated dynamically using weighted environmental and physical parameters.

Factors affecting the score include:

- Temperature deviation
- Humidity deviation
- Excessive vibration
- High-impact events
- Tamper detection

### Example Interpretation

| Score | Status |
|---------|---------|
| 90–100 | Excellent |
| 70–89 | Stable |
| 50–69 | Warning |
| Below 50 | Critical |

---

# ⚡ Quick Start Guide

## 1️⃣ Install Dependencies

Open a terminal in the project root directory and run:

```bash
pip install -r requirements.txt
```

---

## 2️⃣ Launch the Backend Server

Navigate to the backend directory:

```bash
cd AegisBox_WebStack/backend
```

Start the FastAPI server:

```bash
uvicorn server:app --reload
```

---

## 3️⃣ Open the Dashboard

Navigate to:

```text
AegisBox_WebStack/frontend/
```

Open:

```text
index.html
```

The dashboard automatically establishes a WebSocket connection with the FastAPI backend and begins displaying incoming MQTT telemetry data in real time.

---

# 📡 Data Flow Overview

```text
ESP32 Sensors
      │
      ▼
Telemetry Packaging
(JSON)
      │
      ▼
MQTT Publisher
      │
      ▼
test.mosquitto.org
      │
      ▼
FastAPI Subscriber
      │
      ▼
WebSocket Broadcast
      │
      ▼
Live Dashboard
```

---

# 🛠️ Technologies Used

## Embedded Systems

- ESP32
- PlatformIO
- C++

## Sensors

- DHT11
- MPU6050
- LDR

## Communication

- MQTT
- JSON
- WebSockets

## Backend

- Python
- FastAPI
- Uvicorn

## Frontend

- HTML
- CSS
- JavaScript

---

# 👥 Contributors

| Member | Responsibility |
|----------|----------------|
| [Dhaksh S Kumar](https://github.com/Dhaksh-1106) | Embedded Firmware Engineering & Edge Node Architecture |
| [Deepesh Kumar](https://github.com/DeepSisphysus) | Frontend User Interface Design & Visual Layout |
| [Bharath Raghuraman](https://github.com/BurgeryJuice) | Backend Infrastructure & Parameter Compatibility Integration |

---

# 📜 License

This project is developed for academic and demonstration purposes.

---

### AegisBox
**Secure Monitoring. Real-Time Intelligence. Trusted Logistics.**

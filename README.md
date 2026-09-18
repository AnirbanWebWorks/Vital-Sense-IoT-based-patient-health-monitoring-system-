# 🩺 VITAL-SENSE — IoT-Based Patient Health Monitoring System

**Real-Time Physiological Monitoring, Remote Access & Emergency Alerts**

*Sep 2024 – Jan 2025 · Narula Institute of Technology (NIT)*

---

## 📌 Overview

**VITAL-SENSE** is an IoT-based intelligent patient health monitoring system built for real-time physiological data acquisition, remote monitoring, and automated emergency alert generation.

The system integrates **EMG sensors, heart rate sensors, temperature sensors, pulse oximeter modules, and an MPU6050 motion sensor** to continuously track a patient's vital signs and body movements. Sensor data is transmitted to a web server for live visualization and abnormality detection — enabling automatic emergency alerts to healthcare providers the moment a critical condition is detected. The MPU6050 module also supports gesture- and tilt-based command transmission across four directional axes, enabling smart patient interaction and motion-monitoring applications.

The project was developed by **Anirban Saha** and **Somshubhra Bose**, under the guidance of **Dr. Susmita Das**, Assistant Professor, Department of EIE, Narula Institute of Technology.

---

## 🚀 Key Features

- **Remote Patient Monitoring** — continuous health surveillance from any location
- **Real-Time Data Transmission** — up-to-date vitals via Wi-Fi/IoT connectivity
- **Automated Alerts** — notifications sent the instant abnormal health metrics are detected
- **Cloud-Based Data Management** — patient records stored securely and made accessible remotely
- **Motion & Gesture Tracking** — MPU6050 detects falls, tilt, and directional gesture commands
- **Cost-Effective & Scalable** — suited to hospitals, elder care facilities, and home settings

---

## 🎯 Objectives

- Continuously monitor and record patient vitals — heart rate, ECG, temperature, and motion
- Enable remote patient monitoring so caregivers aren't tied to in-person checkups
- Provide a cloud-based platform accessible to doctors, caregivers, and family members
- Implement an automated alert mechanism that flags abnormal readings for quick medical action

---

## 🔄 System Flow

![Connection / Flowchart Diagram](images/flowchart.jpeg)

Biomedical sensors continuously capture heart rate, ECG, body temperature, and motion data. The **NodeMCU ESP8266** microcontroller reads, filters, and prepares this data before wirelessly transmitting it to a cloud/web server. There, it's processed, compared against healthy baseline thresholds, and pushed to a real-time web/mobile dashboard. If any reading falls outside a safe range, the system automatically triggers an alert to the assigned doctor or family member for prompt intervention.

---

## 🔧 Components Used

| # | Component | Specification | Purpose |
|---|---|---|---|
| 1 | **ESP8266 (NodeMCU)** | 64 KB instr. / 96 KB data RAM, 3.3V, 17 GPIO pins, 15 µA (sleep) / 60 mA (active) | Main microcontroller |
| 2 | **MAX30102** | 5.6 × 3.3 × 1.55 mm optical module, ultra-low power | Heart rate & SpO₂ (pulse oximeter) |
| 3 | **AD8232** | 5V, 0.8W output, ~133 mA current output | ECG (electrocardiogram) monitoring |
| 4 | **MPU6050** | 3.3–5V, I2C, built-in DMP | Motion/gesture signals across axes |
| 5 | **DS18B20** | 3.3V, 1-Wire protocol, ±0.5°C accuracy (−10 to 85°C) | Body temperature measurement |
| 6 | **ON/OFF Switch** | 12–250V, 1–5A (varies by switch) | System power toggle |
| 7 | **9V Battery** | ~500 mAh (alkaline) / 200–300 mAh (NiMH) | Power for the transmitter unit |

---

## 🧠 Component Details

### ESP8266 (NodeMCU)
![ESP8266](images/esp8266.png)

A low-cost Wi-Fi microcontroller that reads sensor data and pushes it to the web server. Its large GPIO count makes it easy to interface with multiple biomedical sensors simultaneously.

### AD8232 — ECG Sensor
![AD8232](images/ad8232.jpeg)

A compact single-lead ECG module that amplifies and filters weak bioelectrical heart signals into a clear waveform via three electrode pads (RA, LA, RL), enabling real-time cardiac monitoring.

### MAX30102 — Pulse Oximeter & Heart Rate Sensor
![MAX30102](images/max30102.png)

Uses red/infrared LEDs and a photodetector to measure blood oxygen saturation (SpO₂) and heart rate via photoplethysmography — communicates over I2C.

### DS18B20 — Digital Temperature Sensor
![DS18B20](images/ds18b20.png)

A 1-Wire digital thermometer with ±0.5°C accuracy, used to continuously track body temperature for fever/abnormality detection.

### MPU6050 — 6-Axis Motion Sensor
![MPU6050](images/mpu6050.jpeg)

Combines a 3-axis accelerometer and 3-axis gyroscope with an onboard Digital Motion Processor (DMP), used here to detect falls, tilt, and gesture-based directional commands.

---

## 📊 Performance Considerations

- **Real-time transmission** — low-latency cloud updates; MQTT/WebSockets could further improve throughput
- **Sensor accuracy** — movement and ambient temperature can affect HR/SpO₂ readings; filtering improves consistency
- **Automated alerts** — flags abnormal vitals instantly, though adaptive thresholds help reduce false positives
- **Cloud data management** — designed with encryption and secure storage in mind for sensitive health data

---

## 🏆 Applications

- Hospitals & clinics — continuous remote ICU/post-surgery monitoring
- Elderly care facilities — round-the-clock health surveillance
- Home-based chronic illness management (hypertension, diabetes, cardiovascular conditions)
- Post-hospitalization recovery monitoring
- Wearable health devices (smartwatches, fitness bands)
- Telemedicine & remote healthcare consultations
- High-risk occupations (military, firefighting, industrial work)
- Sports & fitness performance monitoring

---

## 📈 Future Scope

- **AI-Powered Health Insights** — ML-based anomaly detection and predictive health risk forecasting
- **Wearable & Compact Design** — lightweight, battery-efficient continuous-monitoring form factor
- GPS integration for tracking patients with mobility challenges
- 5G/edge computing integration to reduce transmission latency

---

## 🧾 Conclusion

VITAL-SENSE is a real-time, scalable IoT-based patient health monitoring system aimed at improving healthcare accessibility and efficiency. By combining biomedical sensors, cloud computing, and automated alerts, it enables continuous tracking of heart rate, ECG, body temperature, and motion — reducing the need for repeated hospital visits while enabling faster medical response for patients with chronic conditions, the elderly, and those in post-hospitalization care.


---

## 👨‍💻 Authors

**Anirban Saha** & **Somshubhra Bose**
B.Tech, Electronics and Instrumentation Engineering (EIE)
Narula Institute of Technology

Guided by **Dr. Susmita Das**, Assistant Professor, Dept. of EIE, NIT

**Connect With Me**
- GitHub: [AnirbanWebWorks](https://github.com/AnirbanWebWorks)
- LinkedIn: *(Add your LinkedIn URL here)*

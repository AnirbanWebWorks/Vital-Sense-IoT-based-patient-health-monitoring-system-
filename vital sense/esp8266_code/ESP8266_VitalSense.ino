/*
  ============================================================================
  VITAL-SENSE — IoT Based Patient Health Monitoring System
  Firmware for ESP8266 (NodeMCU)
  ============================================================================
  Reads:
    - MAX30102  -> Heart Rate + SpO2        (I2C)
    - AD8232    -> ECG waveform             (Analog, A0)
    - DS18B20   -> Body Temperature         (1-Wire)
    - MPU6050   -> Motion / Gesture / Tilt  (I2C)

  Sends all readings as JSON to a web server via HTTP POST every SEND_INTERVAL ms.
  Triggers a local alert (buzzer/LED) if any vital crosses a critical threshold.

  Required Libraries (install via Arduino Library Manager):
    - ESP8266WiFi            (bundled with ESP8266 board package)
    - ESP8266HTTPClient      (bundled with ESP8266 board package)
    - ArduinoJson             by Benoit Blanchon
    - MAX30105                by SparkFun (works with MAX30102)
    - OneWire                 by Jim Studt / Paul Stoffregen
    - DallasTemperature       by Miles Burton
    - Adafruit MPU6050        by Adafruit
    - Adafruit Unified Sensor  (MPU6050 dependency)

  Wiring (NodeMCU):
    MAX30102  -> SDA=D2, SCL=D1, VIN=3.3V, GND=GND
    MPU6050   -> SDA=D2, SCL=D1 (shared I2C bus), VCC=3.3V, GND=GND
    AD8232    -> OUTPUT=A0, LO+=D5, LO-=D6, 3.3V, GND
    DS18B20   -> DATA=D4 (with 4.7k pull-up to 3.3V), VCC=3.3V, GND=GND
    Buzzer/LED (alert) -> D7
  ============================================================================
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "MAX30105.h"
#include "heartRate.h"

// ---------------------------------------------------------------------------
// USER CONFIG — EDIT THESE
// ---------------------------------------------------------------------------
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* SERVER_URL    = "http://your-server-address/api/vitals";   // web server endpoint
const char* PATIENT_ID    = "PATIENT_001";

const unsigned long SEND_INTERVAL = 5000;   // ms between server updates

// Critical thresholds for local alert
const float TEMP_HIGH_C   = 38.0;
const int   HR_LOW_BPM    = 40;
const int   HR_HIGH_BPM   = 130;
const int   SPO2_LOW_PCT  = 90;
const float FALL_THRESHOLD_G = 2.5;   // combined accel magnitude for fall detection

// ---------------------------------------------------------------------------
// PIN CONFIG
// ---------------------------------------------------------------------------
#define ONE_WIRE_BUS   D4     // DS18B20 data pin
#define ECG_PIN        A0     // AD8232 output
#define ECG_LO_PLUS    D5     // AD8232 leads-off detect +
#define ECG_LO_MINUS   D6     // AD8232 leads-off detect -
#define ALERT_PIN      D7     // Buzzer / LED

// ---------------------------------------------------------------------------
// GLOBAL OBJECTS
// ---------------------------------------------------------------------------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

MAX30105 particleSensor;
Adafruit_MPU6050 mpu;

unsigned long lastSendTime = 0;

// Heart rate calculation (MAX30102)
const byte RATE_SIZE = 4;
byte rateSpots[RATE_SIZE];
byte rateSpotIndex = 0;
long lastBeatTime = 0;
float currentBPM = 0;
int avgBPM = 0;

// ---------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(ECG_LO_PLUS, INPUT);
  pinMode(ECG_LO_MINUS, INPUT);
  pinMode(ALERT_PIN, OUTPUT);
  digitalWrite(ALERT_PIN, LOW);

  Wire.begin();  // default SDA=D2, SCL=D1 on NodeMCU

  connectWiFi();

  // --- DS18B20 ---
  tempSensor.begin();

  // --- MAX30102 ---
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found — check wiring");
  } else {
    particleSensor.setup();                    // default settings
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
  }

  // --- MPU6050 ---
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found — check wiring");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }

  Serial.println("VITAL-SENSE initialized.");
}

// ---------------------------------------------------------------------------
// MAIN LOOP
// ---------------------------------------------------------------------------
void loop() {
  float temperature = readTemperature();
  updateHeartRate();                 // updates currentBPM / avgBPM continuously
  float spo2 = estimateSpO2();       // simplified SpO2 estimate
  float ecgValue = readECG();
  MotionData motion = readMotion();

  bool alert = checkThresholds(temperature, avgBPM, (int)spo2, motion);
  digitalWrite(ALERT_PIN, alert ? HIGH : LOW);

  if (millis() - lastSendTime >= SEND_INTERVAL) {
    lastSendTime = millis();
    sendToServer(temperature, avgBPM, spo2, ecgValue, motion, alert);
  }
}

// ---------------------------------------------------------------------------
// WIFI
// ---------------------------------------------------------------------------
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(400);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWi-Fi connection failed — will retry in background.");
  }
}

// ---------------------------------------------------------------------------
// SENSOR READS
// ---------------------------------------------------------------------------
float readTemperature() {
  tempSensor.requestTemperatures();
  float t = tempSensor.getTempCByIndex(0);
  if (t == DEVICE_DISCONNECTED_C) {
    Serial.println("DS18B20 read error");
    return NAN;
  }
  return t;
}

// Detects individual heartbeats from the MAX30102 IR signal and maintains a
// rolling average BPM. Call this every loop iteration.
void updateHeartRate() {
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue)) {
    long delta = millis() - lastBeatTime;
    lastBeatTime = millis();

    currentBPM = 60000.0 / delta;

    if (currentBPM > 20 && currentBPM < 255) {
      rateSpots[rateSpotIndex++] = (byte)currentBPM;
      rateSpotIndex %= RATE_SIZE;

      int total = 0;
      for (byte i = 0; i < RATE_SIZE; i++) total += rateSpots[i];
      avgBPM = total / RATE_SIZE;
    }
  }
}

// Simplified SpO2 approximation from red/IR ratio.
// For clinical-grade SpO2, replace with SparkFun's full spo2_algorithm.h
float estimateSpO2() {
  long irValue = particleSensor.getIR();
  if (irValue < 5000) return NAN;   // no finger detected

  long redValue = particleSensor.getRed();
  if (redValue == 0) return NAN;

  float ratio = (float)redValue / (float)irValue;
  float spo2 = 110.0 - (25.0 * ratio);   // rough linear approximation
  spo2 = constrain(spo2, 70.0, 100.0);
  return spo2;
}

float readECG() {
  // Leads-off detection: AD8232 pulls LO+ / LO- HIGH if electrodes are not
  // making good contact with the skin.
  if (digitalRead(ECG_LO_PLUS) == 1 || digitalRead(ECG_LO_MINUS) == 1) {
    return -1;   // leads off, no valid reading
  }
  return analogRead(ECG_PIN);   // 0–1023 raw ADC value
}

struct MotionData {
  float accelMagnitude;
  float tiltX, tiltY, tiltZ;
  bool fallDetected;
};

MotionData readMotion() {
  MotionData m = {0, 0, 0, 0, false};

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  m.accelMagnitude = sqrt(ax * ax + ay * ay + az * az) / 9.81;  // in g
  m.tiltX = ax;
  m.tiltY = ay;
  m.tiltZ = az;
  m.fallDetected = (m.accelMagnitude > FALL_THRESHOLD_G);

  return m;
}

// ---------------------------------------------------------------------------
// THRESHOLD CHECK — returns true if any vital is in a critical range
// ---------------------------------------------------------------------------
bool checkThresholds(float temp, int bpm, int spo2, const MotionData &motion) {
  bool critical = false;

  if (!isnan(temp) && temp >= TEMP_HIGH_C) critical = true;
  if (bpm > 0 && (bpm < HR_LOW_BPM || bpm > HR_HIGH_BPM)) critical = true;
  if (spo2 > 0 && spo2 < SPO2_LOW_PCT) critical = true;
  if (motion.fallDetected) critical = true;

  return critical;
}

// ---------------------------------------------------------------------------
// SEND DATA TO SERVER
// ---------------------------------------------------------------------------
void sendToServer(float temp, int bpm, float spo2, float ecg,
                   const MotionData &motion, bool alert) {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    if (WiFi.status() != WL_CONNECTED) return;
  }

  WiFiClient client;
  HTTPClient http;

  StaticJsonDocument<384> doc;
  doc["patient_id"]   = PATIENT_ID;
  doc["temperature_c"] = isnan(temp) ? -1 : temp;
  doc["heart_rate_bpm"] = bpm;
  doc["spo2_pct"] = isnan(spo2) ? -1 : spo2;
  doc["ecg_raw"] = ecg;
  doc["accel_g"] = motion.accelMagnitude;
  doc["fall_detected"] = motion.fallDetected;
  doc["alert"] = alert;
  doc["uptime_ms"] = millis();

  String payload;
  serializeJson(doc, payload);

  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.printf("POST -> %d\n", httpCode);
  } else {
    Serial.printf("POST failed: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  Serial.println(payload);   // local debug log
}

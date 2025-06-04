#include <SoftwareSerial.h>

// Pin configuration
const int trigPin = 9;
const int echoPin = 10;

// Water level thresholds (in cm)
const int WARNING_LEVEL = 150;
const int DANGER_LEVEL = 180;

// Tank height from sensor to bottom (in cm)
const int TANK_HEIGHT = 250;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.begin(9600);
  
  Serial.println("Hydrological Monitoring System");
  Serial.println("Initializing...");
}

void loop() {
  // Measure water level
  float distance = getDistance();
  float waterLevel = TANK_HEIGHT - distance;
  
  // Determine status
  String status;
  if (waterLevel < WARNING_LEVEL) {
    status = "Aman";
  } else if (waterLevel < DANGER_LEVEL) {
    status = "Siaga";
  } else {
    status = "Bahaya";
  }
  
  // Get current time (simulated)
  String currentTime = getCurrentTime();
  
  // Prepare data in JSON format for transmission
  String data = "{\"waktu\":\"" + currentTime + "\",\"tinggi\":" + String(waterLevel, 1) + ",\"status\":\"" + status + "\",\"jarak\":" + String(distance, 1) + "}";
  
  // Send data via Serial USB
  Serial.println(data);
  
  // Wait for 5 minutes before next reading
  delay(300000); // 300000 ms = 5 minutes
}

float getDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  float distance = duration * 0.034 / 2; // Convert to cm
  
  return distance;
}

String getCurrentTime() {
  static int minutes = 0;
  static int hours = 0;
  
  minutes += 5;
  if (minutes >= 60) {
    minutes = 0;
    hours++;
    if (hours >= 24) {
      hours = 0;
    }
  }
  
  String timeStr = String(hours < 10 ? "0" + String(hours) : String(hours)) + ":" + 
                   String(minutes < 10 ? "0" + String(minutes) : String(minutes));
  
  return timeStr;
}
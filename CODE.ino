#include <OneWire.h>
#include <DallasTemperature.h>

#include "BluetoothSerial.h"

// Check if Bluetooth is available on the ESP32
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to enable it.
#endif 

// BluetoothSerial SerialBT;
// Pin definitions
const int LID_PIN    = 16;
const int BODY_PIN   = 4;
const int BOTTOM_PIN = 2;

const int BUZZER_PIN = 12;
const int RELAY_PIN  = 13;

// DallasTemperature setup
OneWire oneWireLid(LID_PIN);
OneWire oneWireBody(BODY_PIN);
OneWire oneWireBottom(BOTTOM_PIN);

DallasTemperature sensorLid(&oneWireLid);
DallasTemperature sensorBody(&oneWireBody);
DallasTemperature sensorBottom(&oneWireBottom);

// State machine
enum State {
  IDLE,
  HEATING,
  STABILIZING,
  TARGET_REACHED,
  OVERHEAT
};
State currentState = IDLE;

// Configurable thresholds
float TARGET_TEMP     = 85.0;   // Desired temp
float OVERHEAT_TEMP   = 100.0;  // Will be adjusted dynamically
const float STAB_MARGIN = 8.0; // ± range for stabilizing
const float TOLERANCE   = 0.75;  // ± range for target reached

// Tracks highest temperature across all sensors
float getMaxTemp(float t1, float t2, float t3) {
  return max(t1, max(t2, t3));
}

void setup() {
  Serial.begin(115200);
  Serial.println("System Booting...");
  Serial.println("ESP32 Bluetooth Heating Monitor");
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_PIN, LOW);
  SerialBT.begin("ESP32_Heater");
  Serial.println("Initializing DS18B20 sensors...");
  sensorLid.begin();
  sensorBody.begin();
  sensorBottom.begin();

  sensorLid.setResolution(12);
  sensorBody.setResolution(12);
  sensorBottom.setResolution(12);
}

void loop() {
   if (SerialBT.available()) {
    // Read the incoming message string
    String receivedMessage = SerialBT.readStringUntil('\n');
    receivedMessage.trim(); // Remove any whitespace

    // Check if the received message is a valid number
    if (receivedMessage.toFloat() > 0) {
      TARGET_TEMP = receivedMessage.toFloat();
      Serial.print("New target temperature received: ");
      Serial.println(TARGET_TEMP);
      
      // Send an acknowledgement back over Bluetooth
      SerialBT.print("Target set to: ");
      SerialBT.println(TARGET_TEMP);
    } else {
      // Handle non-numeric or invalid input
      SerialBT.println("Invalid input. Please send a number.");
    }
  }
  // Read all sensors
  sensorLid.requestTemperatures();
  sensorBody.requestTemperatures();
  sensorBottom.requestTemperatures();

  float tLid    = sensorLid.getTempCByIndex(0);
  float tBody   = sensorBody.getTempCByIndex(0);
  float tBottom = sensorBottom.getTempCByIndex(0);

  float maxTemp = getMaxTemp(tLid, tBody, tBottom);

  // Adjust overheat temp if target is high
  if (TARGET_TEMP > 100 && TARGET_TEMP < 120) {
    OVERHEAT_TEMP = TARGET_TEMP + 5;
  } else if (TARGET_TEMP >= 120) {
    Serial.println("ERROR: Target temp too high (>120°C)");
    TARGET_TEMP = 85.0; // Reset to safe default
  }

  // State machine logic
  switch (currentState) {
    case IDLE:
      if (maxTemp > TARGET_TEMP) {
        currentState = OVERHEAT;
      } else if (TARGET_TEMP > 30 && maxTemp < TARGET_TEMP) {
        currentState = HEATING;
      }
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      break;

    case HEATING:
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(BUZZER_PIN, LOW);
      if (maxTemp >= TARGET_TEMP - STAB_MARGIN && maxTemp <= TARGET_TEMP + STAB_MARGIN) {
        currentState = STABILIZING;
      } else if (maxTemp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;

    case STABILIZING:
      digitalWrite(RELAY_PIN, HIGH);
      if (maxTemp >= TARGET_TEMP - TOLERANCE && maxTemp <= TARGET_TEMP + TOLERANCE) {
        currentState = TARGET_REACHED;
      } else if (maxTemp < TARGET_TEMP - STAB_MARGIN) {
        currentState = HEATING;
      } else if (maxTemp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      }
      break;

    case TARGET_REACHED:
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);
      if (maxTemp >= OVERHEAT_TEMP) {
        currentState = OVERHEAT;
      } else if (maxTemp < TARGET_TEMP - 3) {
        currentState = HEATING;
      }
      break;

    case OVERHEAT:
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER_PIN, HIGH);
      if (maxTemp < TARGET_TEMP - 5 && maxTemp > 30) {
        digitalWrite(BUZZER_PIN, LOW);
        currentState = HEATING;
      } else if (maxTemp < 30) {
        currentState = IDLE;
      }
      break;
  }

  // Status output
  Serial.printf("State: %s | LID: %.2f°C | BODY: %.2f°C | BOTTOM: %.2f°C | Max: %.2f°C\n",
                stateToStr(currentState), tLid, tBody, tBottom, maxTemp);
  SerialBT.printf("State: %s | LID: %.2f°C | BODY: %.2f°C | BOTTOM: %.2f°C | Max: %.2f°C\n",
                stateToStr(currentState), tLid, tBody, tBottom, maxTemp);

  delay(5000); // 5-second cycle
}

const char* stateToStr(State s) {
  switch (s) {
    case IDLE: return "IDLE";
    case HEATING: return "HEATING";
    case STABILIZING: return "STABILIZING";
    case TARGET_REACHED: return "TARGET_REACHED";
    case OVERHEAT: return "OVERHEAT";
    default: return "UNKNOWN";
  }
}

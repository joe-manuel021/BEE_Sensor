#include "NetworkManager.h"
#include "SensorManager.h"
#include "EEPROM.h"
#include "SystemConfig.h"
#include <Adafruit_NeoPixel.h>

// NeoPixel setup
#define LED_PIN 14           // ESP32 pin connected to NeoPixel strip
#define NUM_LEDS 32          // Number of LEDs in the strip

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);


bool configTimeFlag = false;
uint32_t lastMQTTConnectionTime = 0;


void setup() {

  Serial.begin(115200);
  strip.begin();
  strip.show();
  delay(1000);

  pinMode(TIMER_DONE, OUTPUT);
  digitalWrite(TIMER_DONE, LOW);

  // Start the Network Monitor Task --> Replace with Self Healing
  startNetworkManagerTask();

  // Start the Sensor Manager Task
  startSensorManagerTask();

  delay(1000);

}

void loop() {
  if (networkInfo.wifiConnected == false) {
    glowAndDimWhite(200, 10);
  }

  // Check Server Connectivity -> Restart when disconnected for too long
  if (networkInfo.mqttConnected) {

    lastMQTTConnectionTime = millis();

    // LED Indicator ON -------------------------------->


  } else {

    // LED Indicator OFF -------------------------------->


    // Check if the ESP has been disconnected for too long
    if (millis() - lastMQTTConnectionTime > MQTT_DISCONNECT_TIMEOUT_MS) {
#ifdef DEBUG_MQTT
      Serial.println("MQTT disconnected for too long. Restarting ESP...");
#endif
      vTaskDelay(100);
      ESP.restart();  // Restart the ESP
    }
  }

  // Send data to MQTT
  if (sensorDataReady) {
    //     colorWipe(strip.Color(255, 0, 0), 50); // Red color wipe
    String payload = "";



    // Format MQTT Payload
    if (xSemaphoreTake(xSemaphore_Sensor, portMAX_DELAY) == pdTRUE) {

      payload = "{\"N\":\"" + String(sensorData.averaged_sound_level, 4) + "\"}";

      xSemaphoreGive(xSemaphore_Sensor);
    }


    // Send data to MQTT Server
    if (networkInfo.mqttConnected) {
#ifdef DEBUG_MQTT
      Serial.println("Sending Data to MQTT");
      Serial.println("Payload: " + payload);
#endif
      mqtt.publish(topicSensorData, payload.c_str());

      vTaskDelay(1000);
#ifdef DEBUG_SYSTEM
      Serial.println("Shutting down now");

#endif
      glowGreen(100, 10);
      vTaskDelay(100);
      digitalWrite(TIMER_DONE, HIGH);
      Serial.println("Shut down");


#ifdef DEBUG_MQTT
      Serial.println("Sent MQTT heartbeat message.");
#endif
    }
  }

  vTaskDelay(100);
}

void glowAndDimWhite(uint8_t maxBrightness, uint16_t duration) {
  static uint32_t previousMillis = 0;
  static int16_t brightness = 0;
  static int16_t step = 10; // 1 for increasing, -1 for decreasing brightness

  uint16_t interval = duration / (2 * maxBrightness); // Time per brightness step

  if (millis() - previousMillis >= interval) {
    previousMillis = millis();

    // Update brightness
    brightness += step;
    // Reverse direction at limits
    if (brightness >= maxBrightness) {
      step = -10; // Start decreasing brightness
    } else if (brightness <= 0) {
      step = 10; // Start increasing brightness
    }

    // Set LED color
    strip.fill(strip.Color(brightness, 0, 0)); // White color
    strip.show();
  }
}




void glowGreen(uint8_t maxBrightness, uint8_t delayMs) {
  // Gradually increase brightness
  for (uint8_t brightness = 0; brightness <= maxBrightness; brightness++) {
    strip.fill(strip.Color(0, brightness, 0)); // White color
    strip.show();
    Serial.println("Brightness: " + String(brightness));
    delay(delayMs);
  }
  strip.fill(strip.Color(0, 0, 0)); //
  strip.show();
}

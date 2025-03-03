#include <WiFi.h>
#include "SystemConfig.h"
#include "NetworkManager.h"

// Define modem type
//#define TINY_GSM_MODEM_SIM800
//#include <TinyGsmClient.h>

// Serial ports
#define SerialMon Serial
#define SerialAT Serial1

// Debugging
//#define TINY_GSM_DEBUG SerialMon

NetworkInfo_t networkInfo = { false, false, "", "", 0 };

// GSM credentials
const char apn[] = "Internet";
const char gprsUser[] = "";
const char gprsPass[] = "";


// GSM and MQTT clients
//TinyGsm modem(SerialAT);
//TinyGsmClient gsmClient(modem);
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);  // Initially set to WiFi client, will switch to GSM if needed

bool isWiFiConnected = false;
bool isGPRSConnected = false;

void mqttCallback(char* topic, byte* payload, unsigned int len) {
#ifdef DEBUG_MQTT_PAYLOAD
  SerialMon.printf("Message arrived [%s]: ", topic);
  SerialMon.write(payload, len);
  SerialMon.println();
#endif
}

bool connectWiFi() {
#ifdef DEBUG_WIFI
  SerialMon.println("Connecting to Wi-Fi...");
#endif
  WiFi.begin(wifiSSID, wifiPass);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
#ifdef DEBUG_WIFI
    SerialMon.print(".");
#endif
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
#ifdef DEBUG_WIFI
    SerialMon.println("\nConnected to Wi-Fi.");
    SerialMon.print("IP Address: ");
    SerialMon.println(WiFi.localIP());
#endif
    mqtt.setClient(wifiClient);
    return true;
  } else {
#ifdef DEBUG_WIFI
    SerialMon.println("\nFailed to connect to Wi-Fi.");
#endif
    return false;
  }
}

bool mqttConnect() {
#ifdef DEBUG_MQTT
  SerialMon.println("Connecting to MQTT broker...");
#endif
  if (mqtt.connect(deviceESN, mqttUser, mqttPassword)) {
#ifdef DEBUG_MQTT
    SerialMon.println("Connected to MQTT broker.");
#endif
    mqtt.publish(topicInit, "GSM/Wi-Fi MQTT Client Started");
    mqtt.subscribe(topicConfig);
    return true;
  }
#ifdef DEBUG_MQTT
  SerialMon.println("Failed to connect to MQTT broker.");
#endif
  return false;
}



void networkTask(void* pvParameters) {
  SerialMon.begin(115200);
  delay(10);

  //   if (!connectWiFi()) {

  // #ifdef DEBUG_SYSTEM
  //     SerialMon.println("No network connectivity. Task exiting...");
  // #endif
  //     vTaskDelete(NULL);
  //   }

  mqtt.setServer(mqttServer, mqttPort);
  mqtt.setCallback(mqttCallback);

  uint32_t lastReconnectAttempt = 0;
  uint32_t lastWiFiCheck = millis();

  while (true) {

    networkInfo.wifiConnected = isWiFiConnected;

    // Check Wi-Fi status every 5 seconds
    if (millis() - lastWiFiCheck > 5000) {
      lastWiFiCheck = millis();

      if (WiFi.status() == WL_CONNECTED) {
        if (!isWiFiConnected) {

// Wi-Fi just reconnected
#ifdef DEBUG_WIFI
          SerialMon.println("Wi-Fi reconnected");
#endif
          mqtt.setClient(wifiClient);  // Switch MQTT to Wi-Fi client
          isWiFiConnected = true;
        }
      } else {
        connectWiFi();
      }
    }


    if (!mqtt.connected()) {
      networkInfo.mqttConnected = false;
      uint32_t now = millis();
      if (now - lastReconnectAttempt > 10000) {
        lastReconnectAttempt = now;
        if (mqttConnect()) {
          lastReconnectAttempt = 0;
        }
      }
    } else {
      networkInfo.mqttConnected = true;
      mqtt.loop();
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);  // Run every 100ms
  }
}

void startNetworkManagerTask() {
  xTaskCreatePinnedToCore(networkTask, "Network MQTT Task", 8192, NULL, 1, NULL, 1);
}

#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// Debug flags
#define DEBUG_WIFI
#define DEBUG_MQTT
#define DEBUG_MQTT_PAYLOAD
#define DEBUG_GPRS
#define DEBUG_SENSORS
#define DEBUG_SYSTEM

// Watchdog timer timeout (in milliseconds)
#define WDT_TIME_MS 30000

#define MQTT_DISCONNECT_TIMEOUT_MS 120000

#define EEPROM_SIZE 512
#define EEPROM_SEND_INTERVAL_ADDR 0

// MQTT configuration
extern const char* mqttServer;
extern const int mqttPort;
extern const char* mqttUser;
extern const char* mqttPassword;
extern const char* deviceESN;

extern const char* topicInit;
extern const char* topicConfig;
extern const char* topicSensorData;


// Wi-Fi credentials
extern const char* wifiSSID;
extern const char* wifiPass;

#define TIMER_DONE 33

#endif // SYSTEM_CONFIG_H


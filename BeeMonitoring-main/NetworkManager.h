#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <PubSubClient.h>

void startNetworkManagerTask();



// Struct to hold network information
typedef struct {
    bool wifiConnected;       // Connection Status
    bool mqttConnected;       // Connection to MQTT Server
    String SSID;              // Current SSID
    String password;          // Password (optional, based on use case)
    int RSSI;                 // Signal strength (RSSI)
} NetworkInfo_t;

// Declare the global NetworkInfo object as extern
extern NetworkInfo_t networkInfo;


// Declare the global mqtt object as extern
extern PubSubClient mqtt;

#endif // NETWORK_MQTT_MANAGER_H

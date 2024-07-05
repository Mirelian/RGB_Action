#ifndef HEADER_H_
#define HEADER_H_

// Libraries
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <cstring>
#include "time.h"

// NeoPixel
#define PIN 38
extern Adafruit_NeoPixel pixels;
extern volatile float R, G, B;

// Actions
typedef struct action
{
    uint8_t R;
    uint8_t G;
    uint8_t B;
    bool mode;
    uint16_t duration;
} Action;
extern Action actions[16];
extern uint8_t current_com, com_size;

// Wifi, MQTT and NTP
extern const char *ssid;
extern const char *password;
extern const char *mqtt_server;

extern WiFiClient espClient;
extern PubSubClient client;

void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

// Flash
extern Preferences prefs;

void writeActionsToFlash(uint8_t id, byte *pay, unsigned int length);
void readActionsFromFlash(uint8_t id);

// Tasks
extern TaskHandle_t xHandleTaskMode0;
extern TaskHandle_t xHandleTaskMode1;

extern TaskHandle_t xHandleCheckTime;

void vTaskMode0(void *pvParameters);
void vTaskMode1(void *pvParameters);
void vCheckTime(void *pvParameters);
void stopTasks();
void startTasks();
void startTimeTask();

// NTP
extern const char *ntp_server;

extern const uint32_t gmtOffset_sec;
extern const uint16_t daylightOffset_sec;

void printLocalTime();

#endif

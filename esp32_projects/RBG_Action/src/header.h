#ifndef HEADER_H_
#define HEADER_H_

// Libraries
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <Update.h>
#include <string.h>
#include <time.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

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
    uint16_t duration; // ms
} Action;
extern Action actions[16];
extern uint8_t current_com, com_size;

// Wifi, MQTT
extern char ssid[32];
extern char password[32];
extern const char *mqtt_server;

extern char baseMacStr[18];

extern WiFiClient espClient;
extern PubSubClient client;

void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

// Flash
extern Preferences prefs;

void writeActionsToFlash(uint8_t id, byte *pay, unsigned int length);
void readActionsFromFlash(uint8_t id);

void writeID(uint8_t id);
void deleteID(uint8_t id);
void listIDs();
void listIDs(uint8_t nrIDs, uint8_t *IDs);
void showID(uint8_t id, Action *ac, uint8_t size);

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

// NTP and time
typedef struct schedule
{
    uint8_t id;
    char type;
    uint8_t hour;
    uint8_t minute;
    time_t lastOccurence; // only used for 'r' events
} Event;
extern Event events[10];
extern uint8_t nr_events;

extern const char *ntp_server;

extern const uint32_t gmtOffset_sec;
extern const uint16_t daylightOffset_sec;

void printLocalTime(struct tm timeinfo);
void addEvent(uint8_t id, byte *pay, unsigned int length);
double getSeconds(Event event);
void checkEvents(struct tm currentTime);

// HTTPClient
void updateFirmware(byte *pay, unsigned int length);

// BLE
#define SERVICE_UUID "a73e9e22-98b3-4b18-b01a-c73c154d8e97"
#define SSID_CHAR_UUID "0b298162-d28b-4c2e-98e1-2f1d084f2792"
#define PASS_CHAR_UUID "5c453f16-8d60-4b0c-a9d2-d32d7d0ffdc1"

void setup_BLE();

#endif

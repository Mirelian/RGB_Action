#include "header.h"

const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "fractalengineering.dev";

char baseMacStr[18];

WiFiClient espClient;
PubSubClient client;

void setup_wifi()
{
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected.");

    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);

    sprintf(baseMacStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            baseMac[0], baseMac[1], baseMac[2],
            baseMac[3], baseMac[4], baseMac[5]);

    Serial.print("Station MAC: ");
    Serial.println(baseMacStr);

    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if (strcmp(topic, (String(baseMacStr) + "/Action").c_str()) == 0)
    {
        unsigned int i = 0;
        uint8_t id = 0;

        while (payload[i] != ';')
        {
            id = id * 10 + (payload[i] - '0');
            i++;
        }
        i++;
        writeActionsToFlash(id, payload + i, length - i);
    }
    else if (strcmp(topic, (String(baseMacStr) + "/Trigger/Now").c_str()) == 0)
    {
        uint8_t id = 0;
        for (unsigned int i = 0; i < length; i++)
        {
            id = id * 10 + (payload[i] - '0');
        }
        Serial.println(id);
        readActionsFromFlash(id);
        startTasks();
    }
    else if (strcmp(topic, (String(baseMacStr) + "/Trigger/Time").c_str()) == 0)
    {
        unsigned int i = 0;
        uint8_t id = 0;

        while (payload[i] != ';')
        {
            id = id * 10 + (payload[i] - '0');
            i++;
        }
        i++;
        addEvent(id, payload + i, length - i);
    }
    else if (strcmp(topic, (String(baseMacStr) + "/Update").c_str()) == 0)
    {
        updateFirmware(payload, length);
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            client.publish((String(baseMacStr) + "/Status").c_str(), "just connected");
            client.subscribe((String(baseMacStr) + "/Action").c_str());
            client.subscribe((String(baseMacStr) + "/Trigger/Now").c_str());
            client.subscribe((String(baseMacStr) + "/Trigger/Time").c_str());
            client.subscribe((String(baseMacStr) + "/Update").c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
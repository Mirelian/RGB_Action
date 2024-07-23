#include "header.h"

const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "fractalengineering.dev";

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

    if (strcmp(topic, "Action") == 0)
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
    else if (strcmp(topic, "Trigger/Now") == 0)
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
    else if (strcmp(topic, "Trigger/Time") == 0)
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
            client.subscribe("Action");
            client.subscribe("Trigger/Now");
            client.subscribe("Trigger/Time");
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
#include "header.h"

char ssid[32] = "TP-Link_E008";
char password[32] = "31242547";
const char *mqtt_server = "fractalengineering.dev";

char baseMacStr[18];

WiFiClient espClient;
PubSubClient client;

void setup_wifi()
{
    delay(10);

    while (strlen(ssid) == 0 || strlen(password) == 0)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("Credentials recieved!");

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

    if (strcmp(topic, (String(baseMacStr) + "/Event/Add").c_str()) == 0)
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
    else if (strcmp(topic, (String(baseMacStr) + "/Scene/Add").c_str()) == 0)
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
    else if (strcmp(topic, (String(baseMacStr) + "/Scene/Delete").c_str()) == 0)
    {
        uint8_t id = 0;
        for (unsigned int i = 0; i < length; i++)
        {
            id = id * 10 + (payload[i] - '0');
        }
        deleteID(id);
    }
    else if (strcmp(topic, (String(baseMacStr) + "/Trigger/Color").c_str()) == 0)
    {
        stopTasks();
        unsigned int i = 0;
        uint8_t result = 0, j = 0;

        uint8_t color[3] = {0, 0, 0};
        while (i < length)
        {
            color[j] = color[j] * 10 + (payload[i] - '0');
            i++;
            if (payload[i] == ';')
            {
                j++;
                i++;
            }
        }
        R = color[0];
        G = color[1];
        B = color[2];
    }
    else if (strcmp(topic, (String(baseMacStr) + "/Trigger/Scene").c_str()) == 0)
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
    else if (strcmp(topic, (String(baseMacStr) + "/Update").c_str()) == 0)
    {
        updateFirmware(payload, length);
    }
}

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        if (client.connect((String(baseMacStr) + "-ESP32").c_str()))
        {
            Serial.println("connected");
            client.subscribe((String(baseMacStr) + "/Event/Add").c_str());
            client.subscribe((String(baseMacStr) + "/Scene/Add").c_str());
            client.subscribe((String(baseMacStr) + "/Scene/Delete").c_str());
            client.subscribe((String(baseMacStr) + "/Trigger/Color").c_str());
            client.subscribe((String(baseMacStr) + "/Trigger/Scene").c_str());
            client.subscribe((String(baseMacStr) + "/Update").c_str());
            listIDs();
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
#include <WiFi.h> // Include the WiFi library
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <cstring>

#define PIN 38

Adafruit_NeoPixel pixels(1, PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "86.121.175.88";

int Action[5] = {0, 0, 0, 0, 0};

volatile float R = 0, G = 0, B = 0;
float dR = 0, dG = 0, dB = 0;

WiFiClient espClient;
PubSubClient client(espClient);

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
    for (unsigned int i = 0, j = 0; j < 5 && i < length; i++, j++)
    {
      int result = 0;

      while (payload[i] != ',' && i < length)
      {
        result = result * 10 + (payload[i] - '0');
        i++;
      }

      Action[j] = result;
      Serial.println(result);
    }

    if (Action[4])
    {
      dR = (float)(Action[0] - R) / Action[3];
      dG = (float)(Action[1] - G) / Action[3];
      dB = (float)(Action[2] - B) / Action[3];
    }
    else
    {
      dR = (float)Action[0];
      dG = (float)Action[1];
      dB = (float)Action[2];
    }
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

void setup()
{
  // Start the Serial communication to see messages in the Serial Monitor
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pixels.begin();
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // if (Action[3] > 0 && !timerRunning)
  //   timerBegin1();

  pixels.clear();
  // Set the color of the LED (e.g., red)
  pixels.setPixelColor(0, pixels.Color((int)R, (int)G, (int)B));
  pixels.show();

  delay(10);
}
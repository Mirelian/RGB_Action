#include <WiFi.h> // Include the WiFi library
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <string.h>

#define PIN 38

Adafruit_NeoPixel pixels(1, PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "86.121.175.88";

int R = 0, G = 0, B = 0;

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
  int result = 0;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
    result = result * 10 + (payload[i] - '0');
  }
  Serial.println();

  if (strcmp(topic, "Leds/R") == 0)
  {
    R = result;
    Serial.println(result);
  }
  else if (strcmp(topic, "Leds/G") == 0)
    G = result;
  else if (strcmp(topic, "Leds/B") == 0)
    B = result;
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
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");

      client.subscribe("Leds/R");
      client.subscribe("Leds/G");
      client.subscribe("Leds/B");
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

  // Publish a message every 5 seconds
  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 5000)
  {
    lastMsg = now;
    String msg = "hello world";
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg.c_str());
  }

  pixels.clear();

  // Set the color of the LED (e.g., red)
  pixels.setPixelColor(0, pixels.Color(R, G, B));
  pixels.show();

  delay(1000);
}
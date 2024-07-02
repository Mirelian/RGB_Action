#include <WiFi.h> // Include the WiFi library
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <cstring>
#include <vector>

#define PIN 38

Adafruit_NeoPixel pixels(1, PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "86.121.175.88";

struct Action
{
  int R;
  int G;
  int B;
  int duration;
  bool mode;
};

volatile float R = 0, G = 0, B = 0;

WiFiClient espClient;
PubSubClient client(espClient);

bool running = false;
int mode = -1;

TaskHandle_t xHandleTaskMode0 = NULL;
TaskHandle_t xHandleTaskMode1 = NULL;

void vTaskMode0(void *pvParameters)
{
  running = true;
  for (;;)
  {
    vTaskDelay(Action[3] / portTICK_PERIOD_MS);
    R = Action[0];
    G = Action[1];
    B = Action[2];
    running = false;
    vTaskSuspend(NULL);                  // Suspend itself after completion
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  }
}

void vTaskMode1(void *pvParameters)
{
  running = true;
  for (;;)
  {
    float stepR = (Action[0] - R) / (Action[3] / 1.0);
    float stepG = (Action[1] - G) / (Action[3] / 1.0);
    float stepB = (Action[2] - B) / (Action[3] / 1.0);

    for (int i = 0; i < Action[3]; i++)
    {
      R += stepR;
      G += stepG;
      B += stepB;
      Serial.print("R:");
      Serial.print(R);
      Serial.print(" G:");
      Serial.print(G);
      Serial.print(" B:");
      Serial.println(B);
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    running = false;
    vTaskSuspend(NULL);                  // Suspend itself after completion
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  }
}

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
    mode = Action[4];
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

  xTaskCreatePinnedToCore(
      vTaskMode0,        // Function to be called
      "TaskMode0",       // Name of the task
      10000,             // Stack size (bytes)
      NULL,              // Parameter to pass
      1,                 // Task priority
      &xHandleTaskMode0, // Task handle
      0);                // Core where the task should run

  xTaskCreatePinnedToCore(
      vTaskMode1,        // Function to be called
      "TaskMode1",       // Name of the task
      10000,             // Stack size (bytes)
      NULL,              // Parameter to pass
      1,                 // Task priority
      &xHandleTaskMode1, // Task handle
      1);                // Core where the task should run

  // Immediately suspend both tasks
  vTaskSuspend(xHandleTaskMode0);
  vTaskSuspend(xHandleTaskMode1);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (!running)
  {
    if (mode == 0)
    {
      mode = -1;
      vTaskResume(xHandleTaskMode0);
    }
    else if (mode == 1)
    {
      mode = -1;
      vTaskResume(xHandleTaskMode1);
    }
  }

  pixels.clear();
  // Set the color of the LED (e.g., red)
  pixels.setPixelColor(0, pixels.Color((int)R, (int)G, (int)B));
  pixels.show();

  delay(10);
}
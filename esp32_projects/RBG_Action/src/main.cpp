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

int Action[16][5];

uint8_t com_size = 0, current_com = 0;

volatile float R = 0, G = 0, B = 0;

WiFiClient espClient;
PubSubClient client(espClient);

TaskHandle_t xHandleTaskMode0 = NULL;
TaskHandle_t xHandleTaskMode1 = NULL;

void vTaskMode0(void *pvParameters)
{
  vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  for (;;)
  {
    vTaskDelay((Action[current_com][3] - 10) / portTICK_PERIOD_MS);
    R = Action[current_com][0];
    G = Action[current_com][1];
    B = Action[current_com][2];
    current_com++;
    vTaskSuspend(NULL);                  // Suspend itself after completion
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  }
}

void vTaskMode1(void *pvParameters)
{
  for (;;)
  {
    float stepR = (Action[current_com][0] - R) / (Action[current_com][3] / 1.0);
    float stepG = (Action[current_com][1] - G) / (Action[current_com][3] / 1.0);
    float stepB = (Action[current_com][2] - B) / (Action[current_com][3] / 1.0);

    for (int i = 0; i < Action[current_com][3]; i++)
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
    current_com++;
    vTaskSuspend(NULL);                  // Suspend itself after completion
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  }
}

void stopTasks()
{
  if (xHandleTaskMode0 != NULL)
  {
    vTaskDelete(xHandleTaskMode0);
    xHandleTaskMode0 = NULL;
  }
  if (xHandleTaskMode1 != NULL)
  {
    vTaskDelete(xHandleTaskMode1);
    xHandleTaskMode1 = NULL;
  }
}

void startTasks()
{
  stopTasks();
  com_size = 0;
  current_com = 0;
  xTaskCreatePinnedToCore(
      vTaskMode0,        // Function to be called
      "TaskMode0",       // Name of the task
      10000,             // Stack size (bytes)
      NULL,              // Parameter to pass
      1,                 // Task priority
      &xHandleTaskMode0, // Task handle
      0);                // Core where the task should run

  vTaskSuspend(xHandleTaskMode0);

  xTaskCreatePinnedToCore(
      vTaskMode1,        // Function to be called
      "TaskMode1",       // Name of the task
      10000,             // Stack size (bytes)
      NULL,              // Parameter to pass
      1,                 // Task priority
      &xHandleTaskMode1, // Task handle
      1);                // Core where the task should run

  // Immediately suspend both tasks
  vTaskSuspend(xHandleTaskMode1);
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
    startTasks();
    unsigned i = 0;
    while (i < length)
    {
      for (unsigned int j = 0; j < 4; i++, j++)
      {
        int result = 0;

        while (payload[i] != ',')
        {
          result = result * 10 + (payload[i] - '0');
          i++;
        }

        Action[com_size][j] = result;
        Serial.println(result);
      }
      Action[com_size][4] = payload[i] - '0';
      com_size++;
      i += 2;
      Serial.print(com_size);
      Serial.print("");
      Serial.println(current_com);
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

  if (current_com < com_size)
  {
    if (Action[current_com][4] == 0)
    {
      vTaskResume(xHandleTaskMode0);
    }
    else if (Action[current_com][4] == 1)
    {
      vTaskResume(xHandleTaskMode1);
    }
  }

  pixels.clear();
  // Set the color of the LED (e.g., red)
  pixels.setPixelColor(0, pixels.Color((int)R, (int)G, (int)B));
  pixels.show();

  delay(10);
}
#include <WiFi.h> // Include the WiFi library
#include <PubSubClient.h>
#include <Adafruit_NeoPixel.h>
#include <Preferences.h>
#include <cstring>

#define PIN 38

Adafruit_NeoPixel pixels(1, PIN, NEO_GRB + NEO_KHZ800);

// Replace with your network credentials
const char *ssid = "TP-Link_4757";
const char *password = "28361473";
const char *mqtt_server = "86.121.175.88";

typedef struct action
{
  uint8_t R;
  uint8_t G;
  uint8_t B;
  bool mode;
  uint32_t duration;
} Action;

Action actions[16];

uint8_t current_com = 0, com_size = 0;

volatile float R = 0, G = 0, B = 0;

WiFiClient espClient;
PubSubClient client(espClient);

TaskHandle_t xHandleTaskMode0 = NULL;
TaskHandle_t xHandleTaskMode1 = NULL;

Preferences prefs;

void vTaskMode0(void *pvParameters)
{
  vTaskDelay(50 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  for (;;)
  {
    vTaskDelay((actions[current_com].duration - 50) / portTICK_PERIOD_MS);
    R = actions[current_com].R;
    G = actions[current_com].G;
    B = actions[current_com].B;
    current_com++;
    vTaskSuspend(NULL);                  // Suspend itself after completion
    vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
  }
}

void vTaskMode1(void *pvParameters)
{
  for (;;)
  {
    float stepR = (actions[current_com].R - R) / (actions[current_com].duration / 1.0);
    float stepG = (actions[current_com].G - G) / (actions[current_com].duration / 1.0);
    float stepB = (actions[current_com].B - B) / (actions[current_com].duration / 1.0);
    Serial.print(current_com);
    Serial.print(" ");
    Serial.print("stepR:");
    Serial.print(stepR);
    Serial.print(" stepG:");
    Serial.print(stepG);
    Serial.print(" stepB:");
    Serial.println(stepB);

    for (uint32_t i = 0; i < actions[current_com].duration; i++)
    {
      R += stepR;
      G += stepG;
      B += stepB;
      // Serial.print("R:");
      // Serial.print(R);
      // Serial.print(" G:");
      // Serial.print(G);
      // Serial.print(" B:");
      // Serial.println(B);
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

void writeActionsToFlash(uint8_t id, byte *pay, unsigned int length)
{
  char key[20];
  char b_id[10];
  sprintf(b_id, "%d", id);
  strcpy(key, b_id);
  strcat(key, "_action_");

  Action Aactions[16];

  uint8_t numActions = 0;
  unsigned int i = 0;

  while (i < length)
  {
    uint8_t *ptr = (uint8_t *)&Aactions[numActions];
    for (uint8_t j = 0; j < 4; j++)
    {
      uint8_t result = 0;

      while (pay[i] != ',')
      {
        result = result * 10 + (pay[i] - '0');
        i++;
      }

      *(ptr + j) = result;
      i++;
      // Serial.print(result);
      // Serial.print(" ");
    }
    Aactions[numActions].duration = 0;
    while (pay[i] != ';')
    {
      Aactions[numActions].duration = Aactions[numActions].duration * 10 + (pay[i] - '0');
      i++;
    }
    // Serial.println(actions[numActions].mode);
    // Serial.print(actions[numActions].R);
    // Serial.print(" ");
    // Serial.print(actions[numActions].G);
    // Serial.print(" ");
    // Serial.print(actions[numActions].B);
    // Serial.print(" ");
    // Serial.print(actions[numActions].duration);
    // Serial.print(" ");
    // Serial.println(actions[numActions].mode);
    numActions++;
    i += 2;
  }

  prefs.begin(key);

  prefs.putBytes(key, Aactions, sizeof(Action) * numActions);
  Serial.println("Actions written to flash memory.");
}

void readActionsFromFlash(uint8_t id)
{
  char key[20];
  char b_id[10];
  uint8_t numActions = 0;
  unsigned int i = 0;

  sprintf(b_id, "%d", id);
  strcpy(key, b_id);
  strcat(key, "_action_");
  size_t schLen = prefs.getBytesLength(key);
  Serial.println(schLen);
  memset(actions, 0, sizeof(Action) * 16);
  prefs.getBytes(key, actions, schLen);
  com_size = schLen / sizeof(Action);
  Serial.println(com_size);
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
  if (strcmp(topic, "Trigger") == 0)
  {
    uint8_t id = 0;
    for (unsigned int i = 0; i < length; i++)
    {
      id = id * 10 + (payload[i] - '0');
    }
    Serial.println(id);
    readActionsFromFlash(id);
    // Serial.print(actions[0].R);
    // Serial.print(" ");
    // Serial.print(actions[0].G);
    // Serial.print(" ");
    // Serial.print(actions[0].B);
    // Serial.print(" ");
    // Serial.println(actions[0].mode);
    // Serial.print(" ");
    // Serial.print(actions[0].duration);
    // Serial.print(actions[7].R);
    // Serial.print(" ");
    // Serial.print(actions[7].G);
    // Serial.print(" ");
    // Serial.print(actions[7].B);
    // Serial.print(" ");
    // Serial.println(actions[7].mode);
    // Serial.print(" ");
    // Serial.print(actions[7].duration);
    startTasks();
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
      client.subscribe("Trigger");
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
    if (actions[current_com].mode == 0)
    {
      vTaskResume(xHandleTaskMode0);
    }
    else if (actions[current_com].mode == 1)
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
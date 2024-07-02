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
  uint32_t duration;
  uint8_t mode;
} Action;

Action actions[16];
Action aux;

uint8_t com_size = 0, current_com = 0;

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

    for (int i = 0; i < actions[current_com].duration; i++)
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

void writeActionsToFlash(int id, byte *pay)
{
  char key[20];
  char b_id[2];
  sprintf(b_id, "%d", id);
  strcat(key, b_id);
  strcat(key, "_action_");
  prefs.putBytes(key, pay, sizeof(pay));
  Serial.println("Actions written to flash memory.");
}

// void writeNumActionsToFlash(int id, int numActions)
// {
//   char key[20];
//   char b_id[2];
//   sprintf(b_id, "%d", id);
//   strcat(key, b_id);
//   strcat(key, "_numActions_");
//   prefs.putBytes(key, &numActions, sizeof(numActions));
// }

void readActionsFromFlash(int id)
{
  char key[20];
  char b_id[2];
  int numActions = 0;
  unsigned int i = 0;

  sprintf(b_id, "%d", id);
  strcat(key, b_id);
  strcat(key, "_action_");
  size_t schLen = prefs.getBytesLength(key);
  char buffer[schLen];
  prefs.getBytes(key, buffer, schLen);

  while (i < schLen)
  {
    uint8_t *ptr = (uint8_t *)&actions[numActions];
    for (unsigned int j = 0; j < 4; i++, j++)
    {
      int result = 0;

      while (buffer[i] != ',')
      {
        result = result * 10 + (buffer[i] - '0');
        i++;
      }

      *(ptr + j) = result;
      Serial.println(result);
    }
    actions[numActions].mode = buffer[i] - '0';
    numActions++;
    i += 2;
  }
  com_size = numActions;
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
    unsigned int i = 0, j;
    int id = 0;

    while (payload[i] != ',')
    {
      id = id * 10 + (payload[i] - '0');
      i++;
    }
    i++;

    // while (i < length)
    // {
    //   // uint8_t *ptr = (uint8_t *)&aux;
    //   // for (unsigned int j = 0; j < 4; i++, j++)
    //   // {
    //   //   int result = 0;

    //   //   while (payload[i] != ',')
    //   //   {
    //   //     result = result * 10 + (payload[i] - '0');
    //   //     i++;
    //   //   }

    //   //   *(ptr + i) = result;
    //   //   Serial.println(result);
    //   // }
    //   // aux.mode = payload[i] - '0';
    //   // writeActionToFlash(id, numActions, aux);
    //   // numActions++;
    //   // i += 2;
    //   if (payload[i] == ';')
    //     numActions++;
    //   i++;
    // }
    writeActionsToFlash(id, payload + i);
  }
  if (strcmp(topic, "Trigger") == 0)
  {
    int id = 0;
    for (unsigned int i = 0; i < length; i++)
    {
      id = id * 10 + (payload[i] - '0');
    }
    readActionsFromFlash(id);
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
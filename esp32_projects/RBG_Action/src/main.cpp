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

hw_timer_t *timer = NULL;
volatile bool timerRunning = false;
volatile int count = 0;

void timerEnd1()
{
  if (timerRunning)
  {
    timerEnd(timer); // Deinitialize the timer
    timerRunning = false;
    Serial.println("Timer finished");
  }
}

void IRAM_ATTR onTimer()
{
  count++;

  // if (Action[4] == 1)
  // { // Mode 1: Increment/Decrement
  //   if (count <= Action[3])
  //   {
  //     R += dR;
  //     G += dG;
  //     B += dB;
  //   }
  //   else
  //   {
  //     timerEnd1(); // Stop the timer after the duration
  //   }
  // }
  // else
  // { // Mode 0: Immediate change
  //   if (count >= Action[3])
  //   {
  //     R = Action[0];
  //     G = Action[1];
  //     B = Action[2];
  //     timerEnd1(); // Stop the timer after the duration
  //   }
  // }
  if (count <= 250)
  {
    // R += 1;
    // G += 1;
    // B += 1;
  }
  else
  {
    timerEnd1();
  }
  Serial.println("aha");
}

void timerBegin1()
{
  if (timerRunning)
  {
    timerEnd1(); // Ensure previous timer is stopped before starting a new one
  }

  dR = (float)(Action[0] - R) / Action[3];
  dG = (float)(Action[1] - G) / Action[3];
  dB = (float)(Action[2] - B) / Action[3];

  count = 0;

  timer = timerBegin(0, 80, true);             // Use timer 0, prescaler 80 (1 us per tick)
  timerAttachInterrupt(timer, &onTimer, true); // Attach the interrupt
  timerAlarmWrite(timer, 1000, true);          // Set alarm to 1 ms
  timerAlarmEnable(timer);                     // Enable the alarm
  timerRunning = true;
  Serial.println("Timer started");
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

    // transition = (bool)Action[4];
    // timerDuration = Action[3];

    // if (Action[4])
    // {
    //   // dR = (float)(Action[0] - R) / Action[3];
    //   // dG = (float)(Action[1] - G) / Action[3];
    //   // dB = (float)(Action[2] - B) / Action[3];
    // }
    // else
    // {
    //   dR = (float)Action[0];
    //   dG = (float)Action[1];
    //   dB = (float)Action[2];
    // }
    timerBegin1();
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
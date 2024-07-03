#include "header.h"

void setup()
{
  // Start the Serial communication to see messages in the Serial Monitor
  Serial.begin(115200);

  setup_wifi();

  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pixels.updateLength(1);
  pixels.setPin(PIN);
  pixels.updateType(NEO_GRB + NEO_KHZ800);
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
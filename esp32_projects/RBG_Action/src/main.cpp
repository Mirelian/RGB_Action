#include "header.h"

void setup()
{
  Serial.begin(115200);

  setup_BLE();
  setup_wifi();

  client.setClient(espClient);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  configTime(gmtOffset_sec, daylightOffset_sec, ntp_server);

  pixels.updateLength(1);
  pixels.setPin(PIN);
  pixels.updateType(NEO_GRB + NEO_KHZ800);
  pixels.begin();

  startTimeTask();
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
  pixels.setPixelColor(0, pixels.Color((int)R, (int)G, (int)B));
  pixels.show();

  delay(10);
}
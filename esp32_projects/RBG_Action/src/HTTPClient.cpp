#include "header.h"

//"http://localhost:3000/firmware.bin";

void updateFirmware(byte *pay, unsigned int length)
{
  char *firmwareURL = new char[length + 1];

  for (unsigned int i = 0; i < length; i++)
  {
    firmwareURL[i] = (char)pay[i];
  }

  firmwareURL[length] = '\0';

  HTTPClient http;
  http.begin(firmwareURL);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    // HTTP header has been sent and server response header has been handled
    Serial.printf("HTTP GET code: %d\n", httpCode);

    // File found at server
    if (httpCode == HTTP_CODE_OK)
    {
      int contentLength = http.getSize();
      bool canBegin = Update.begin(contentLength);
      if (canBegin)
      {
        WiFiClient *client = http.getStreamPtr();
        size_t written = Update.writeStream(*client);
        if (written == contentLength)
        {
          Serial.print("Written : ");
          Serial.print(written);
          Serial.println(" successfully");
        }
        else
        {
          Serial.print("Written only : ");
          Serial.print(written);
          Serial.print("/");
          Serial.print(contentLength);
          Serial.println(". Retry?");
        }
        if (Update.end())
        {
          Serial.println("OTA done!");
          if (Update.isFinished())
          {
            Serial.println("Update successfully completed. Rebooting.");
            ESP.restart();
          }
          else
          {
            Serial.println("Update not finished? Something went wrong!");
          }
        }
        else
        {
          Serial.print("Error Occurred. Error #: ");
          Serial.println(Update.getError());
        }
      }
      else
      {
        Serial.println("Not enough space to begin OTA");
      }
    }
    else
    {
      Serial.print("HTTP GET failed with error code: ");
      Serial.println(httpCode);
    }
  }
  else
  {
    Serial.print("HTTP GET failed with error: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  http.end();
}
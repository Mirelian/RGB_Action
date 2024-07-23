#include "header.h"

void updateFirmware(byte *payload, unsigned int length)
{
    if (Update.begin(length))
    {
        size_t written = Update.write(payload, length);
        if (written == length)
        {
            Serial.println("Written : " + String(written) + " successfully");
        }
        else
        {
            Serial.println("Written only : " + String(written) + "/" + String(length) + ". Retry?");
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
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }
    }
    else
    {
        Serial.println("Not enough space to begin OTA");
    }
}
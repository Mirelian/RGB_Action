#include "header.h"

Preferences prefs;

void writeActionsToFlash(uint8_t id, byte *pay, unsigned int length)
{
    char key[20];
    char b_id[10];
    sprintf(b_id, "%d", id);
    strcpy(key, b_id);
    strcat(key, "_scene_");

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

    prefs.putBytes(key, Aactions, sizeof(Action) * numActions);
    Serial.println("Actions written to flash memory.");
}

void readActionsFromFlash(uint8_t id)
{
    char key[20];
    char b_id[10];
    uint8_t numActions = 0;

    sprintf(b_id, "%d", id);
    strcpy(key, b_id);
    strcat(key, "_scene_");
    size_t schLen = prefs.getBytesLength(key);
    memset(actions, 0, sizeof(Action) * 16);
    prefs.getBytes(key, actions, schLen);
    com_size = schLen / sizeof(Action);
}
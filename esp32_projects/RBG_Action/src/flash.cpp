#include "header.h"

Preferences prefs;

void writeID(uint8_t id)
{

    prefs.begin("actions", false);

    if (!prefs.isKey("nrIDs"))
    {
        prefs.putBytes("IDs", (void *)&id, sizeof(uint8_t));
        prefs.putBytes("nrIDs", (void *)1, sizeof(uint8_t));
    }
    else
    {
        uint8_t nrIDs;
        prefs.getBytes("nrIDs", (void *)&nrIDs, sizeof(uint8_t));

        uint8_t *IDs;
        IDs = new uint8_t[++nrIDs];
        prefs.getBytes("IDs", IDs, nrIDs * sizeof(uint8_t));

        bool isPresent = false;
        for (uint8_t i = 0; i < nrIDs - 1; i++)
        {
            if (IDs[i] == id)
            {
                isPresent = true;
                break;
            }
        }

        if (!isPresent)
        {
            IDs[nrIDs - 1] = id;

            for (uint8_t interval = nrIDs / 2; interval > 0; interval /= 2)
            {
                for (uint8_t i = interval; i < nrIDs; i += 1)
                {
                    uint8_t temp = IDs[i];
                    uint8_t j;
                    for (j = i; j >= interval && IDs[j - interval] > temp; j -= interval)
                    {
                        IDs[j] = IDs[j - interval];
                    }
                    IDs[j] = temp;
                }
            }
            prefs.putBytes("nrIDs", (void *)&nrIDs, sizeof(uint8_t));
            prefs.putBytes("IDs", IDs, nrIDs * sizeof(uint8_t));
        }

        listIDs(nrIDs, IDs);

        delete[] IDs;
    }

    prefs.end();
}

void deleteID(uint8_t id)
{
    prefs.begin("actions", false);

    prefs.remove((String(id) + "_scene_").c_str());

    uint8_t nrIDs;
    prefs.getBytes("nrIDs", (void *)&nrIDs, sizeof(uint8_t));

    uint8_t *IDs;
    IDs = new uint8_t[nrIDs];
    prefs.getBytes("IDs", IDs, nrIDs * sizeof(uint8_t));

    bool isPresent = false;

    for (uint8_t i = 0; i < nrIDs; i++)
    {
        if (IDs[i] == id)
        {
            IDs[i] = IDs[i] + IDs[nrIDs - 1];
            IDs[nrIDs - 1] = IDs[i] - IDs[nrIDs - 1];
            IDs[i] = IDs[i] - IDs[nrIDs - 1];
            isPresent = true;
            break;
        }
    }

    if (isPresent)
    {
        nrIDs--;

        for (uint8_t interval = nrIDs / 2; interval > 0; interval /= 2)
        {
            for (uint8_t i = interval; i < nrIDs; i += 1)
            {
                uint8_t temp = IDs[i];
                uint8_t j;
                for (j = i; j >= interval && IDs[j - interval] > temp; j -= interval)
                {
                    IDs[j] = IDs[j - interval];
                }
                IDs[j] = temp;
            }
        }
        prefs.putBytes("nrIDs", (void *)&nrIDs, sizeof(uint8_t));
        prefs.putBytes("IDs", IDs, nrIDs * sizeof(uint8_t));
    }
    listIDs(nrIDs, IDs);

    delete[] IDs;

    prefs.end();
}

void writeActionsToFlash(uint8_t id, byte *pay, unsigned int length)
{
    prefs.begin("actions", false);

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
        }
        Aactions[numActions].duration = 0;
        while (pay[i] != ';')
        {
            Aactions[numActions].duration = Aactions[numActions].duration * 10 + (pay[i] - '0');
            i++;
        }
        Serial.println(Aactions[numActions].mode);
        Serial.print(Aactions[numActions].R);
        Serial.print(" ");
        Serial.print(Aactions[numActions].G);
        Serial.print(" ");
        Serial.print(Aactions[numActions].B);
        Serial.print(" ");
        Serial.print(Aactions[numActions].duration);
        Serial.print(" ");
        Serial.println(Aactions[numActions].mode);
        numActions++;
        i++;
    }

    prefs.putBytes((String(id) + "_scene_").c_str(), Aactions, sizeof(Action) * numActions);
    Serial.println("Actions written to flash memory.");

    writeID(id);
    showID(id, Aactions, numActions);

    prefs.end();
}

void readActionsFromFlash(uint8_t id)
{
    prefs.begin("actions", true);

    size_t schLen = prefs.getBytesLength((String(id) + "_scene_").c_str());
    memset(actions, 0, sizeof(Action) * 16);
    prefs.getBytes((String(id) + "_scene_").c_str(), actions, schLen);
    com_size = schLen / sizeof(Action);

    writeID(id);
    showID(id, actions, com_size);

    prefs.end();
}
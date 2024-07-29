#include "header.h"

Event events[10];
uint8_t nr_events = 0;

const char *ntp_server = "pool.ntp.org";

const uint32_t gmtOffset_sec = 7200;
const uint16_t daylightOffset_sec = 3600;

void printLocalTime(struct tm timeinfo)
{
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");
}

void addEvent(uint8_t id, byte *pay, unsigned int length)
{
    if (nr_events > 9)
        return;

    events[nr_events].id = id;
    events[nr_events].type = (char)pay[0];

    events[nr_events].hour = (pay[2] - '0') * 10 + (pay[3] - '0');
    events[nr_events].minute = (pay[5] - '0') * 10 + (pay[6] - '0');
    if (events[nr_events].type == 'r')
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
        {
            Serial.println("Failed to obtain time");
            return;
        }
        events[nr_events].lastOccurence = mktime(&timeinfo);
    }
    Serial.println("Event added:");
    Serial.print(id);
    Serial.print(" ");
    Serial.print(events[nr_events].type);
    Serial.print(" ");
    Serial.print(events[nr_events].hour);
    Serial.print(" ");
    Serial.println(events[nr_events].minute);
    nr_events++;
}

double getSeconds(Event event)
{
    return (event.hour * 60 + event.minute) * 60;
}

void checkEvents(struct tm currentTime)
{
    bool ev = false;
    uint8_t i;
    for (i = 0; i < nr_events; i++)
    {
        if (events[i].type == 'd')
        {
            if (currentTime.tm_hour == events[i].hour && currentTime.tm_min == events[i].minute)
            {
                ev = true;
                break;
            }
        }
        else if (events[i].type == 'r')
        {
            time_t now = mktime(&currentTime);
            if (difftime(now, events[i].lastOccurence) >= getSeconds(events[i]))
            {
                ev = true;
                events[i].lastOccurence = now;
                break;
            }
        }
    }
    if (ev)
    {
        printLocalTime(currentTime);
        Serial.print("ID:");
        Serial.println(events[i].id);
        readActionsFromFlash(events[i].id);
        startTasks();
    }
}
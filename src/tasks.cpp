#include "header.h"

TaskHandle_t xHandleActionMode = NULL;
TaskHandle_t xHandleCheckTime = NULL;

void vActionMode(void *pvParameters)
{
    vTaskDelay(50 / portTICK_PERIOD_MS); // Small delay to prevent task triggeing at the beggining
    for (;;)
    {
        if (actions[current_com].mode == 0)
        {
            vTaskDelay((actions[current_com].duration) / portTICK_PERIOD_MS);
            R = actions[current_com].R;
            G = actions[current_com].G;
            B = actions[current_com].B;
            current_com++;
            vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
        }
        else
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

            for (uint16_t i = 0; i < actions[current_com].duration; i++)
            {
                R += stepR;
                G += stepG;
                B += stepB;
                vTaskDelay(1 / portTICK_PERIOD_MS);
            }
            current_com++;
        }
        if (current_com == com_size)
        {
            vTaskSuspend(NULL);                  // Suspend itself after completion
            vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
        }
    }
}

void vCheckTime(void *pvParameters)
{
    struct tm currentTime;
    for (;;)
    {
        if (!getLocalTime(&currentTime))
            continue;
        checkEvents(currentTime);

        vTaskDelay(60000 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
    }
}

void startTasks()
{
    xTaskCreatePinnedToCore(
        vActionMode,        // Function to be called
        "TaskMode0",        // Name of the task
        10000,              // Stack size (bytes)
        NULL,               // Parameter to pass
        1,                  // Task priority
        &xHandleActionMode, // Task handle
        0);                 // Core where the task should run

    vTaskSuspend(xHandleActionMode);

    xTaskCreatePinnedToCore(
        vCheckTime,        // Function to be called
        "CheckTime",       // Name of the task
        10000,             // Stack size (bytes)
        NULL,              // Parameter to pass
        1,                 // Task priority
        &xHandleCheckTime, // Task handle
        0);
}

void stopActionTask()
{
    vTaskDelete(xHandleActionMode);

    xTaskCreatePinnedToCore(
        vActionMode,        // Function to be called
        "TaskMode0",        // Name of the task
        10000,              // Stack size (bytes)
        NULL,               // Parameter to pass
        1,                  // Task priority
        &xHandleActionMode, // Task handle
        0);                 // Core where the task should run

    vTaskSuspend(xHandleActionMode);
}
#include "header.h"

TaskHandle_t xHandleTaskMode0 = NULL;
TaskHandle_t xHandleTaskMode1 = NULL;

TaskHandle_t xHandleCheckTime = NULL;

void vTaskMode0(void *pvParameters)
{
    vTaskDelay(50 / portTICK_PERIOD_MS); // Small delay to prevent task triggeing at the beggining
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
            // Serial.print("R:");
            // Serial.print(R);
            // Serial.print(" G:");
            // Serial.print(G);
            // Serial.print(" B:");
            // Serial.println(B);
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
        current_com++;
        vTaskSuspend(NULL);                  // Suspend itself after completion
        vTaskDelay(10 / portTICK_PERIOD_MS); // Small delay to prevent task hogging CPU
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

void startTimeTask()
{
    xTaskCreatePinnedToCore(
        vCheckTime,        // Function to be called
        "CheckTime",       // Name of the task
        10000,             // Stack size (bytes)
        NULL,              // Parameter to pass
        1,                 // Task priority
        &xHandleCheckTime, // Task handle
        0);
}
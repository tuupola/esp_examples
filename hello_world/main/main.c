#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void hello_world_task(void *pvParameters)
{
    while(1) {
        printf("Hello world!\n");
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void hello_moon_task(void *pvParameters)
{
    while(1) {
        printf("Hello moon!\n");
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}


// ESP32 application entry point
void app_main()
{
    printf("Starting...\n");
    xTaskCreatePinnedToCore(hello_world_task, "Hello world task", 1024, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(hello_moon_task, "Hello moon task", 1000, NULL, 1, NULL, 1);

    while(1);
}
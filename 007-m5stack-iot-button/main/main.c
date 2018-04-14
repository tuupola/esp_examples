#define BUTTON_A_PIN          39
#define BUTTON_ACTIVE_LEVEL   1

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "iot_button.h"
#include "esp_log.h"

static const char* TAG = "main";

static void dummy_task(void* arg)
{
    while (1) {
        ESP_LOGI(TAG, "My name is Mud.");
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    /* When line below is commented out "My name is Mud." is logged once per second. */
    button_handle_t btn_handle = iot_button_create(BUTTON_A_PIN, BUTTON_ACTIVE_LEVEL);
    xTaskCreatePinnedToCore(dummy_task, "Dummy", 2048, NULL, 1, NULL, 1);
}


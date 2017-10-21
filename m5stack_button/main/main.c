#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdio.h"

#include "sdkconfig.h"

static const char* TAG = "hello_world";

void button_a_task(void *pvParameters)
{
    while(1) {
        ESP_LOGD(TAG, "Button A value is %d", gpio_get_level(GPIO_NUM_39));
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void button_b_task(void *pvParameters)
{
    while(1) {
        ESP_LOGD(TAG, "Button B value is %d", gpio_get_level(GPIO_NUM_38));
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void button_c_task(void *pvParameters)
{
    while(1) {
        ESP_LOGD(TAG, "Button C value is %d", gpio_get_level(GPIO_NUM_37));
        vTaskDelay(250 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

// ESP32 application entry point
void app_main()
{
    ESP_LOGD(TAG, "Starting...");

    gpio_config_t button_config;
    button_config.pin_bit_mask = GPIO_SEL_37 | GPIO_SEL_38 | GPIO_SEL_39;
    button_config.mode         = GPIO_MODE_INPUT;
    button_config.pull_up_en   = GPIO_PULLUP_DISABLE;
    button_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    button_config.intr_type    = GPIO_INTR_DISABLE;
    gpio_config(&button_config);

    xTaskCreatePinnedToCore(button_a_task, "Button A task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(button_b_task, "Button B task", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(button_c_task, "Button C task", 2048, NULL, 1, NULL, 1);

    while(1);
}
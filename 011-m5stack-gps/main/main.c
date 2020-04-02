/*

SPDX-License-Identifier: MIT-0

MIT No Attribution

Copyright (c) 2018-2020 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <driver/uart.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "copepod.h"
#include "font8x8.h"
#include "minmea.h"
#include "sdkconfig.h"

#define UART_RX_BUF_SIZE    2048
#define UART_TX_BUF_SIZE    0
#define UART_QUEUE_SIZE     0
#define UART_QUEUE_HANDLE   NULL
#define UART_INTR_FLAGS     0

static const char* TAG = "main";

typedef struct {
    float latitude;
    float longitude;
    float speed;
} gnss_status_t;

static gnss_status_t gnss_status;
static SemaphoreHandle_t mutex;

void uart_init()
{
    uart_config_t uart_config = {
        //.baud_rate = 115200,
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        // .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        // .rx_flow_ctrl_thresh = 122,
    };

    ESP_ERROR_CHECK(
        uart_param_config(UART_NUM_2, &uart_config)
    );
    ESP_ERROR_CHECK(
        uart_driver_install(
            UART_NUM_2,
            UART_RX_BUF_SIZE,
            UART_TX_BUF_SIZE,
            UART_QUEUE_SIZE,
            UART_QUEUE_HANDLE,
            UART_INTR_FLAGS
        )
    );

    ESP_ERROR_CHECK(
        uart_set_pin(
            UART_NUM_2,
            GPIO_NUM_17, // M5Stack TX GPS module pins
            GPIO_NUM_16, // M5Stack RX GPS module pins
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );
}

char *uart_read_line(uart_port_t uart_port) {
    static char line[255];
    char *ptr = line;

    while (true) {

        int16_t bytes_read = uart_read_bytes(
            uart_port,
            (unsigned char *)ptr,
            1,
            portMAX_DELAY
        );

        if (1 == bytes_read) {
            if ('\n' == *ptr) {
                ptr++;
                *ptr = '\0';
                return line;
            }
            ptr++;
        }
    }
}

void uart_read_task(void *params)
{
    ESP_LOGI(TAG, "Reading uart...");
    while(1) {
        char *line = uart_read_line(UART_NUM_2);
        ESP_LOGV(TAG, "%s", line);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void uart_read_and_parse_task(void *params)
{
    ESP_LOGI(TAG, "Reading and parsing NMEA from uart...");
    while(1) {
        char *line = uart_read_line(UART_NUM_2);

        ESP_LOGD(TAG, "%s", line);

        switch (minmea_sentence_id(line, false)) {

            /* Only RMC and GLL are used for updating internal status. */

            /* RMC (Recommended Minimum: position, velocity, time) */
            case MINMEA_SENTENCE_RMC: {
                struct minmea_sentence_rmc frame;
                if (minmea_parse_rmc(&frame, line)) {
                    ESP_LOGV(
                        TAG,
                        "$xxRMC: coordinates and speed: (%f,%f) %f",
                        minmea_tocoord(&frame.latitude),
                        minmea_tocoord(&frame.longitude),
                        minmea_tofloat(&frame.speed)
                    );

                    /* Update internal status. */
                    gnss_status.latitude = minmea_tocoord(&frame.latitude);
                    gnss_status.longitude = minmea_tocoord(&frame.longitude);
                    gnss_status.speed = minmea_tofloat(&frame.speed);

                } else {
                    ESP_LOGV(TAG, "$xxRMC sentence was not parsed");
                }
            } break;

            /* GLL (Geographic Position: Latitude/Longitude) */
            case MINMEA_SENTENCE_GLL: {
                struct minmea_sentence_gll frame;
                if (minmea_parse_gll(&frame, line)) {
                    ESP_LOGV(
                        TAG,
                        "$xxGLL: coordinates, status and mode: (%f,%f), %d, %d",
                        minmea_tocoord(&frame.latitude),
                        minmea_tocoord(&frame.longitude),
                        frame.status,
                        frame.mode
                    );

                    /* Update internal status. */
                    gnss_status.latitude = minmea_tocoord(&frame.latitude);
                    gnss_status.longitude = minmea_tocoord(&frame.longitude);

                } else {
                    ESP_LOGV(TAG, "$xxGLL sentence is not parsed");
                }
            } break;

            /* Rest of the sentences are just for demo. Enable verbose */
            /* logging to see the output. */

            /* ZDA (Time & Date - UTC, day, month, year and local time zone) */
            case MINMEA_SENTENCE_ZDA: {
                struct minmea_sentence_zda frame;
                if (minmea_parse_zda(&frame, line)) {
                    ESP_LOGV(
                        TAG,
                        "$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d",
                        frame.time.hours,
                        frame.time.minutes,
                        frame.time.seconds,
                        frame.date.day,
                        frame.date.month,
                        frame.date.year,
                        frame.hour_offset,
                        frame.minute_offset
                    );
                } else {
                    ESP_LOGV(TAG, "$xxZDA sentence was not parsed");
                }
            } break;

            /* GGA (Fix Data) */
            case MINMEA_SENTENCE_GGA: {
                struct minmea_sentence_gga frame;
                if (minmea_parse_gga(&frame, line)) {
                    ESP_LOGV(TAG, "$xxGGA: fix quality: %d", frame.fix_quality);
                } else {
                    ESP_LOGV(TAG, "$xxGGA sentence is not parsed");
                }
            } break;

            /* VTG (Track made good and Ground speed) */
            case MINMEA_SENTENCE_VTG: {
                struct minmea_sentence_vtg frame;
                if (minmea_parse_vtg(&frame, line)) {
                        ESP_LOGV(
                        TAG,
                        "$xxVTG: degrees and speed: %f true, %f mag, %f knots, %f kph",
                        minmea_tofloat(&frame.true_track_degrees),
                        minmea_tofloat(&frame.magnetic_track_degrees),
                        minmea_tofloat(&frame.speed_knots),
                        minmea_tofloat(&frame.speed_kph)
                    );
                } else {
                    ESP_LOGV(TAG, "$xxVTG sentence is not parsed");
                }
            } break;

            /* GSV (Satellites in view) */
            case MINMEA_SENTENCE_GSV: {
                struct minmea_sentence_gsv frame;
                if (minmea_parse_gsv(&frame, line)) {
                    ESP_LOGV(
                        TAG,
                        "$xxGSV: message %d of %d",
                        frame.msg_nr,
                        frame.total_msgs
                    );
                    ESP_LOGV(TAG, "$xxGSV: satellites in view: %d", frame.total_sats);
                    for (int i = 0; i < 4; i++)
                        ESP_LOGV(
                            TAG,
                            "$xxGSV: #%d, elevation: %d, azimuth: %d, snr: %d dbm",
                            frame.sats[i].nr,
                            frame.sats[i].elevation,
                            frame.sats[i].azimuth,
                            frame.sats[i].snr
                        );
                } else {
                    ESP_LOGV(TAG, "$xxGSV sentence is not parsed");
                }
            } break;

            /* GST (Pseudorange Noise Statistics) */
            case MINMEA_SENTENCE_GST: {
                struct minmea_sentence_gst frame;
                if (minmea_parse_gst(&frame, line)) {
                    ESP_LOGV(TAG,
                    "$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
                    minmea_tofloat(&frame.latitude_error_deviation),
                    minmea_tofloat(&frame.longitude_error_deviation),
                    minmea_tofloat(&frame.altitude_error_deviation));
                } else {
                    ESP_LOGV(TAG, "$xxGST sentence is not parsed");
                }
            } break;

            /* GSA (DOP and active satellites) */
            case MINMEA_SENTENCE_GSA: {
                struct minmea_sentence_gsa frame;
                if (minmea_parse_gsa(&frame, line)) {
                    ESP_LOGV(
                        TAG,
                        "$xxGSA: mode, fix type (PDOP, HDOP, VDOP):  %d, %d (%f, %f %f)",
                        frame.mode,
                        frame.fix_type,
                        minmea_tofloat(&frame.pdop),
                        minmea_tofloat(&frame.hdop),
                        minmea_tofloat(&frame.vdop)
                    );
                } else {
                    ESP_LOGV(TAG, "$xxGSA sentence is not parsed");
                }
            } break;

            case MINMEA_UNKNOWN: {
                ESP_LOGE(TAG, "$xxxxx sentence is not valid");
            } break;

            case MINMEA_INVALID: {
                ESP_LOGE(TAG, "$xxxxx sentence is not valid");
            } break;
        }
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
    vTaskDelete(NULL);
}


void display_gnss_status_task(void *params)
{

    uint16_t color = RGB565(0, 0, 255);
    char message[64];

    ESP_LOGI(TAG, "Displaying GNSS status...");

    while(1) {
        ESP_LOGI(
            TAG,
            "GNSS: Coordinates (%f, %f), speed %f knots",
            gnss_status.latitude,
            gnss_status.longitude,
            gnss_status.speed
        );

        sprintf(
            message, "  Lat: %f",
            gnss_status.latitude
        );
        pod_puttext(message, 95, 100, color, font8x8_basic);
        sprintf(
            message, "  Lon: %f",
            gnss_status.longitude
        );
        pod_puttext(message, 95, 109, color, font8x8_basic);
        sprintf(
            message, "Speed: %.2f km/h",
            gnss_status.speed * 1.852
        );
        pod_puttext(message, 95, 118, color, font8x8_basic);



        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

void framebuffer_task(void *params)
{
    while (1) {
        xSemaphoreTake(mutex, portMAX_DELAY);
        pod_flush();
        xSemaphoreGive(mutex);
    }

    vTaskDelete(NULL);
}

void app_main()
{
    ESP_LOGI(TAG, "SDK version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Heap when starting: %d", esp_get_free_heap_size());

    uart_init();
    pod_init();

    ESP_LOGI(TAG, "Heap after init: %d", esp_get_free_heap_size());

    mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(framebuffer_task, "Framebuffer", 8192, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(uart_read_and_parse_task, "UART read and parse", 2048, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(display_gnss_status_task, "Display GNSS status", 2048, NULL, 5, NULL, 1);

}
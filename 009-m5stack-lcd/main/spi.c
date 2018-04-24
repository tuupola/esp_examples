
#include "spi.h"
#include "ili9341.h"
#include "sdkconfig.h"

void spi_master_init(spi_device_handle_t *spi)
{
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPI_MAX_TRANSFER_SIZE /* Max transfer size in bytes. */
    };
    spi_device_interface_config_t devcfg = {
#ifdef CONFIG_LCD_OVERCLOCK
        .clock_speed_hz = 48 * 1000 * 1000,
#else
        .clock_speed_hz = 10 * 1000 * 1000,
#endif
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 64,
        .pre_cb = ili9341_pre_callback, /* Handles D/C line. */
        .flags = SPI_DEVICE_NO_DUMMY
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &buscfg, 1));
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &devcfg, spi));
}


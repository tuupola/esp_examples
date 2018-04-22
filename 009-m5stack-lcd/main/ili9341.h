#include <stdint.h>
#include <driver/spi_master.h>

#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK  18
#define PIN_NUM_CS   14

#define PIN_NUM_DC   27
#define PIN_NUM_RST  33
#define PIN_NUM_BCKL 32

typedef struct {
    uint8_t cmd;
    uint8_t data[16];
    uint8_t databytes; //No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

void ili9431_bitmap(spi_device_handle_t spi, uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t *bitmap);
void ili9341_wait(spi_device_handle_t spi);
void ili9341_init(spi_device_handle_t *spi);
void ili9341_pre_callback(spi_transaction_t *transaction);

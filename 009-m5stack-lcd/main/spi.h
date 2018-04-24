#include "driver/spi_master.h"

#define SPI_MAX_TRANSFER_SIZE   (320 * 240 * 2 + 8)

void spi_master_init(spi_device_handle_t *spi);

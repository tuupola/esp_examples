// #include <string.h>
//
#include <stdint.h>
#include <esp_log.h>

esp_err_t sim8xx_init();
esp_err_t sim8xx_register_network();
esp_err_t sim8xx_enable_gprs();
esp_err_t sim8xx_tcp_connect();
esp_err_t sim8xx_tcp_write_bytes(const char *bytes, size_t length);


void sim8xx_reset();
void sim8xx_time();
void sim8xx_imei();
void sim8xx_power_off();
void sim8xx_power_on();
void sim8xx_power_rf_off();
void sim8xx_power_rf_on();
void sim8xx_disable_gprs();
void sim8xx_tcp_disconnect();

void sim8xx_udp_connect();
void sim8xx_udp_disconnect();
void sim8xx_udp_write_bytes();


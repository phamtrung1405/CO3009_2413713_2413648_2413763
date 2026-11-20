#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include <stdint.h>

#define DHT20_I2C_ADDR_7BIT  0x38U

uint8_t dht20_scan_i2c(uint8_t *found_addrs, uint8_t max_len);

uint8_t dht20_init(void);

void dht20_read_task(void);

float   dht20_get_temperature(void);
float   dht20_get_humidity(void);
uint8_t dht20_is_valid(void);

#endif /* INC_DHT20_H_ */
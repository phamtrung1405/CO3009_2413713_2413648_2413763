#ifndef INC_DHT20_H_
#define INC_DHT20_H_

#include <stdint.h>

#define DHT20_I2C_ADDR_7BIT  0x38U

/* Quet bus I2C1 (Buoc 3.4), tra ve so thiet bi tim thay (dia chi 7-bit) */
uint8_t dht20_scan_i2c(uint8_t *found_addrs, uint8_t max_len);

/* Khoi tao DHT20 (goi 1 lan truoc while(1)), tra ve 1 neu thanh cong */
uint8_t dht20_init(void);

/* Task non-blocking: dang ky chay moi 100 ms (10 tick) tren Scheduler */
void dht20_read_task(void);

float   dht20_get_temperature(void);
float   dht20_get_humidity(void);
uint8_t dht20_is_valid(void);

#endif /* INC_DHT20_H_ */
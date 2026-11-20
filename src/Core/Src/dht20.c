#include "dht20.h"
#include "main.h"
#include <stddef.h>

extern I2C_HandleTypeDef hi2c1;

#define DHT20_I2C_ADDR        ((uint16_t)(DHT20_I2C_ADDR_7BIT << 1))
#define DHT20_I2C_TIMEOUT_MS  20U

/* Task chay moi 100 ms -> 19 lan cho + 1 lan doc = chu ky do 2.0 s */
#define DHT20_IDLE_TICKS      19U
#define DHT20_MAX_BUSY_RETRY  3U

typedef enum {
    DHT20_STATE_IDLE = 0,
    DHT20_STATE_WAIT_DATA
} Dht20State_t;

static Dht20State_t dht20_state  = DHT20_STATE_IDLE;
static uint8_t      idle_counter = DHT20_IDLE_TICKS;
static uint8_t      busy_retry   = 0;

static float   cached_temp = 0.0f;
static float   cached_hum  = 0.0f;
static uint8_t data_valid  = 0;

static uint8_t dht20_calc_crc8(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0xFFU;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80U) {
                crc = (uint8_t)((crc << 1) ^ 0x31U);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static void dht20_reset_reg(uint8_t reg) {
    uint8_t cmd[3] = { reg, 0x00, 0x00 };
    uint8_t rx[3]  = { 0 };

    HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, cmd, 3, DHT20_I2C_TIMEOUT_MS);
    HAL_Delay(5);
    HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDR, rx, 3, DHT20_I2C_TIMEOUT_MS);
    HAL_Delay(10);

    cmd[0] = (uint8_t)(0xB0U | reg);
    cmd[1] = rx[1];
    cmd[2] = rx[2];
    HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, cmd, 3, DHT20_I2C_TIMEOUT_MS);
}

uint8_t dht20_scan_i2c(uint8_t *found_addrs, uint8_t max_len) {
    if (found_addrs == NULL) {
        return 0;
    }

    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(addr << 1), 1, 10) == HAL_OK) {
            if (count < max_len) {
                found_addrs[count++] = addr;
            }
        }
    }
    return count;
}

uint8_t dht20_init(void) {
    dht20_state  = DHT20_STATE_IDLE;
    idle_counter = DHT20_IDLE_TICKS;
    busy_retry   = 0;
    data_valid   = 0;

    HAL_Delay(100); /* Cho on dinh nguon >= 100 ms theo datasheet */

    uint8_t status_cmd = 0x71U;
    uint8_t status_val = 0x00U;

    if (HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, &status_cmd, 1, DHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        return 0;
    }
    if (HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDR, &status_val, 1, DHT20_I2C_TIMEOUT_MS) != HAL_OK) {
        return 0;
    }

    if ((status_val & 0x18U) != 0x18U) {
        dht20_reset_reg(0x1BU);
        dht20_reset_reg(0x1CU);
        dht20_reset_reg(0x1EU);
        HAL_Delay(10);
    }

    return 1;
}

void dht20_read_task(void) {
    switch (dht20_state) {
        case DHT20_STATE_IDLE:
            if (idle_counter >= DHT20_IDLE_TICKS) {
                uint8_t trigger_cmd[3] = { 0xACU, 0x33U, 0x00U };
                if (HAL_I2C_Master_Transmit(&hi2c1, DHT20_I2C_ADDR, trigger_cmd, 3, DHT20_I2C_TIMEOUT_MS) == HAL_OK) {
                    dht20_state = DHT20_STATE_WAIT_DATA;
                    busy_retry  = 0;
                }
                idle_counter = 0;
            } else {
                idle_counter++;
            }
            break;

        case DHT20_STATE_WAIT_DATA: {
            uint8_t buf[7] = { 0 };
            if (HAL_I2C_Master_Receive(&hi2c1, DHT20_I2C_ADDR, buf, 7, DHT20_I2C_TIMEOUT_MS) == HAL_OK) {
                if ((buf[0] & 0x80U) != 0U) {
                    busy_retry++;
                    if (busy_retry >= DHT20_MAX_BUSY_RETRY) {
                        dht20_state = DHT20_STATE_IDLE;
                    }
                    return;
                }

                if (dht20_calc_crc8(buf, 6) == buf[6]) {
                    uint32_t raw_hum = ((uint32_t)buf[1] << 12)
                                     | ((uint32_t)buf[2] << 4)
                                     | ((uint32_t)(buf[3] >> 4));

                    uint32_t raw_temp = (((uint32_t)(buf[3] & 0x0FU)) << 16)
                                      | ((uint32_t)buf[4] << 8)
                                      | ((uint32_t)buf[5]);

                    cached_hum  = ((float)raw_hum  / 1048576.0f) * 100.0f;
                    cached_temp = ((float)raw_temp / 1048576.0f) * 200.0f - 50.0f;
                    data_valid  = 1;
                }
            }
            dht20_state = DHT20_STATE_IDLE;
            break;
        }

        default:
            dht20_state = DHT20_STATE_IDLE;
            break;
    }
}

float dht20_get_temperature(void) {
    return cached_temp;
}

float dht20_get_humidity(void) {
    return cached_hum;
}

uint8_t dht20_is_valid(void) {
    return data_valid;
}
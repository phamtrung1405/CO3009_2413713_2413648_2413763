#include "lcd.h"
#include "main.h"
#include <stddef.h>

extern I2C_HandleTypeDef hi2c1;

/* Anh xa chan IC PCF8574 -> HD44780 (P0=RS, P1=RW, P2=EN, P3=BL, P4..7=D4..7) */
#define LCD_BIT_RS       0x01U
#define LCD_BIT_EN       0x04U
#define LCD_BIT_BL       0x08U
#define LCD_I2C_TIMEOUT  20U

static uint16_t lcd_addr_8bit = (uint16_t)(LCD_DEFAULT_ADDR_7BIT << 1);

static void lcd_write_nibble(uint8_t nibble_hi, uint8_t rs_mask) {
    uint8_t ctrl = rs_mask | LCD_BIT_BL;
    uint8_t pkt[2] = {
        (uint8_t)((nibble_hi & 0xF0U) | ctrl | LCD_BIT_EN),
        (uint8_t)((nibble_hi & 0xF0U) | ctrl)
    };
    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr_8bit, pkt, 2, LCD_I2C_TIMEOUT);
}

static void lcd_send_byte(uint8_t val, uint8_t rs_mask) {
    uint8_t hi   = (uint8_t)(val & 0xF0U);
    uint8_t lo   = (uint8_t)((val << 4) & 0xF0U);
    uint8_t ctrl = rs_mask | LCD_BIT_BL;

    uint8_t pkt[4] = {
        (uint8_t)(hi | ctrl | LCD_BIT_EN),
        (uint8_t)(hi | ctrl),
        (uint8_t)(lo | ctrl | LCD_BIT_EN),
        (uint8_t)(lo | ctrl)
    };
    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr_8bit, pkt, 4, LCD_I2C_TIMEOUT);
}

static void lcd_send_cmd(uint8_t cmd) {
    lcd_send_byte(cmd, 0x00U);
}

void lcd_init(uint8_t addr_7bit) {
    lcd_addr_8bit = (uint16_t)(addr_7bit << 1);

    /* Chuoi khoi tao che do 4-bit chuan HD44780 (chi goi 1 lan truoc while(1)) */
    HAL_Delay(50);
    lcd_write_nibble(0x30U, 0x00U);
    HAL_Delay(5);
    lcd_write_nibble(0x30U, 0x00U);
    HAL_Delay(1);
    lcd_write_nibble(0x30U, 0x00U);
    HAL_Delay(1);
    lcd_write_nibble(0x20U, 0x00U);
    HAL_Delay(1);

    lcd_send_cmd(0x28U); /* 4-bit, 2 dong, font 5x8 */
    HAL_Delay(1);
    lcd_send_cmd(0x08U); /* Tat hien thi */
    HAL_Delay(1);
    lcd_send_cmd(0x01U); /* Xoa DDRAM */
    HAL_Delay(2);
    lcd_send_cmd(0x06U); /* Tang con tro sang phai */
    HAL_Delay(1);
    lcd_send_cmd(0x0CU); /* Bat hien thi, tat con tro */
    HAL_Delay(1);
}

void lcd_clear(void) {
    lcd_print_line(0, "");
    lcd_print_line(1, "");
    lcd_set_cursor(0, 0);
}

void lcd_set_cursor(uint8_t row, uint8_t col) {
    if (row >= LCD_ROWS || col >= LCD_COLS) {
        return;
    }
    uint8_t addr = (row == 0U) ? (uint8_t)(0x80U + col) : (uint8_t)(0xC0U + col);
    lcd_send_cmd(addr);
}

void lcd_send_char(char ch) {
    lcd_send_byte((uint8_t)ch, LCD_BIT_RS);
}

void lcd_send_string(const char *str) {
    if (str == NULL) {
        return;
    }
    while (*str != '\0') {
        lcd_send_char(*str++);
    }
}

/* Gui gop 16 ky tu trong 1 giao dich I2C de giam overhead va tranh nhap nhay */
void lcd_print_line(uint8_t row, const char *str) {
    if (row >= LCD_ROWS) {
        return;
    }
    lcd_set_cursor(row, 0);

    uint8_t tx_buf[LCD_COLS * 4U];
    uint8_t ctrl  = LCD_BIT_RS | LCD_BIT_BL;
    uint8_t ended = (str == NULL) ? 1U : 0U;

    for (uint8_t col = 0; col < LCD_COLS; col++) {
        uint8_t ch = ' ';
        if (!ended) {
            if (str[col] == '\0') {
                ended = 1U;
            } else {
                ch = (uint8_t)str[col];
            }
        }

        uint8_t hi   = (uint8_t)(ch & 0xF0U);
        uint8_t lo   = (uint8_t)((ch << 4) & 0xF0U);
        uint8_t base = (uint8_t)(col * 4U);

        tx_buf[base + 0U] = (uint8_t)(hi | ctrl | LCD_BIT_EN);
        tx_buf[base + 1U] = (uint8_t)(hi | ctrl);
        tx_buf[base + 2U] = (uint8_t)(lo | ctrl | LCD_BIT_EN);
        tx_buf[base + 3U] = (uint8_t)(lo | ctrl);
    }

    HAL_I2C_Master_Transmit(&hi2c1, lcd_addr_8bit, tx_buf, sizeof(tx_buf), LCD_I2C_TIMEOUT);
}

void lcd_send_number(int32_t num) {
    char buf[12];
    int idx = 0;
    uint32_t uval;

    if (num < 0) {
        lcd_send_char('-');
        uval = (uint32_t)(-(num + 1)) + 1U;
    } else {
        uval = (uint32_t)num;
    }

    if (uval == 0U) {
        lcd_send_char('0');
        return;
    }

    while (uval > 0U && idx < 11) {
        buf[idx++] = (char)('0' + (uval % 10U));
        uval /= 10U;
    }
    while (idx > 0) {
        lcd_send_char(buf[--idx]);
    }
}

void lcd_send_float(float val, uint8_t decimals) {
    if (decimals > 4U) {
        decimals = 4U;
    }

    if (val < 0.0f) {
        lcd_send_char('-');
        val = -val;
    }

    uint32_t mult = 1U;
    for (uint8_t i = 0; i < decimals; i++) {
        mult *= 10U;
    }

    uint32_t scaled   = (uint32_t)(val * (float)mult + 0.5f);
    uint32_t int_part = scaled / mult;
    uint32_t frac     = scaled % mult;

    lcd_send_number((int32_t)int_part);

    if (decimals > 0U) {
        lcd_send_char('.');
        uint32_t div = mult / 10U;
        while (div > 0U) {
            lcd_send_char((char)('0' + ((frac / div) % 10U)));
            div /= 10U;
        }
    }
}
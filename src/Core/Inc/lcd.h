#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>

/* Dia chi 7-bit mac dinh cua module PCF8574 (cap nhat theo Buoc 3.4) */
#define LCD_DEFAULT_ADDR_7BIT 0x21
#define LCD_COLS               16U
#define LCD_ROWS               2U

void lcd_init(uint8_t addr_7bit);
void lcd_clear(void);
void lcd_set_cursor(uint8_t row, uint8_t col);
void lcd_send_char(char ch);
void lcd_send_string(const char *str);
void lcd_print_line(uint8_t row, const char *str);
void lcd_send_number(int32_t num);
void lcd_send_float(float val, uint8_t decimals);

#endif /* INC_LCD_H_ */
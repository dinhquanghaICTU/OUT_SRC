#ifndef MOCK_HAL_H
#define MOCK_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void mock_hal_reset(void);
void mock_hal_set_time_ms(uint64_t ms);
void mock_hal_advance_time_ms(uint64_t ms);
uint64_t mock_hal_get_time_ms(void);
uint64_t mock_hal_get_time_us(void);

#define MOCK_MAX_PINS 40
typedef enum { MOCK_GPIO_MODE_INPUT = 0, MOCK_GPIO_MODE_OUTPUT } mock_gpio_mode_t;
void mock_gpio_set_mode(int pin, mock_gpio_mode_t mode);
void mock_gpio_write(int pin, int level);
int  mock_gpio_read(int pin);
int  mock_gpio_get_output_level(int pin);
void mock_gpio_set_input_level(int pin, int level);

bool json_extract_string(const char *json, const char *key, char *out_val, size_t max_len);
bool json_extract_number(const char *json, const char *key, double *out_val);
bool json_extract_bool(const char *json, const char *key, bool *out_val);
bool json_has_key(const char *json, const char *key);

#ifdef __cplusplus
}
#endif

#endif

#include "mock_hal.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

static uint64_t s_virtual_time_ms = 1000;
static mock_gpio_mode_t s_pin_modes[MOCK_MAX_PINS];
static int s_pin_inputs[MOCK_MAX_PINS];
static int s_pin_outputs[MOCK_MAX_PINS];
static int s_adc_raw[MOCK_MAX_PINS];
static uint32_t s_adc_mv[MOCK_MAX_PINS];

void mock_hal_reset(void) {
    s_virtual_time_ms = 1000;
    memset(s_pin_modes, 0, sizeof(s_pin_modes));
    memset(s_pin_inputs, 0, sizeof(s_pin_inputs));
    memset(s_pin_outputs, 0, sizeof(s_pin_outputs));
    memset(s_adc_raw, 0, sizeof(s_adc_raw));
    memset(s_adc_mv, 0, sizeof(s_adc_mv));
}

void mock_hal_set_time_ms(uint64_t ms) {
    s_virtual_time_ms = ms;
}

void mock_hal_advance_time_ms(uint64_t ms) {
    s_virtual_time_ms += ms;
}

uint64_t mock_hal_get_time_ms(void) {
    return s_virtual_time_ms;
}

uint64_t mock_hal_get_time_us(void) {
    return s_virtual_time_ms * 1000ULL;
}

void mock_gpio_set_mode(int pin, mock_gpio_mode_t mode) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        s_pin_modes[pin] = mode;
    }
}

void mock_gpio_write(int pin, int level) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        s_pin_outputs[pin] = level ? 1 : 0;
    }
}

void mock_gpio_set_input_level(int pin, int level) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        s_pin_inputs[pin] = level ? 1 : 0;
    }
}

int mock_gpio_read(int pin) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        if (s_pin_modes[pin] == MOCK_GPIO_MODE_OUTPUT) {
            return s_pin_outputs[pin];
        }
        return s_pin_inputs[pin];
    }
    return 0;
}

int mock_gpio_get_output_level(int pin) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        return s_pin_outputs[pin];
    }
    return 0;
}

void mock_adc_set_raw(int pin, int raw_12bit) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        s_adc_raw[pin] = raw_12bit;
        s_adc_mv[pin] = (uint32_t)((raw_12bit * 3300ULL) / 4095ULL);
    }
}

void mock_adc_set_millivolts(int pin, uint32_t mv) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        s_adc_mv[pin] = mv;
        s_adc_raw[pin] = (int)((mv * 4095ULL) / 3300ULL);
    }
}

int mock_adc_read_raw(int pin) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        return s_adc_raw[pin];
    }
    return 0;
}

uint32_t mock_adc_read_millivolts(int pin) {
    if (pin >= 0 && pin < MOCK_MAX_PINS) {
        return s_adc_mv[pin];
    }
    return 0;
}

bool json_has_key(const char *json, const char *key) {
    if (!json || !key) return false;
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    return strstr(json, search) != NULL;
}

bool json_extract_string(const char *json, const char *key, char *out_val, size_t max_len) {
    if (!json || !key || !out_val || max_len == 0) return false;
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (*p != '"') return false;
    p++;
    size_t i = 0;
    while (*p && *p != '"' && i < max_len - 1) {
        out_val[i++] = *p++;
    }
    out_val[i] = '\0';
    return true;
}

bool json_extract_number(const char *json, const char *key, double *out_val) {
    if (!json || !key || !out_val) return false;
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    char *endptr = NULL;
    *out_val = strtod(p, &endptr);
    return endptr != p;
}

bool json_extract_bool(const char *json, const char *key, bool *out_val) {
    if (!json || !key || !out_val) return false;
    char search[128];
    snprintf(search, sizeof(search), "\"%s\"", key);
    const char *p = strstr(json, search);
    if (!p) return false;
    p += strlen(search);
    while (*p && (*p == ' ' || *p == ':' || *p == '\t')) p++;
    if (strncmp(p, "true", 4) == 0) {
        *out_val = true;
        return true;
    }
    if (strncmp(p, "false", 5) == 0) {
        *out_val = false;
        return true;
    }
    return false;
}

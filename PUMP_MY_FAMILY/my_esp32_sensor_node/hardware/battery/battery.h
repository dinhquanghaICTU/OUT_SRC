#ifndef __BATTERY_H__
#define __BATTERY_H__

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Khởi tạo kênh ADC đọc điện áp pin
 * 
 * @param adc_gpio_pin Chân GPIO kết nối cầu phân áp pin (VD: GPIO 34)
 * @param divider_ratio Tỉ lệ cầu phân áp (VD: R1=100k, R2=100k -> 2.0f)
 * @return esp_err_t ESP_OK nếu thành công
 */
esp_err_t battery_init(int adc_gpio_pin, float divider_ratio);

/**
 * @brief Đọc điện áp pin thực tế (Volt)
 * 
 * @param[out] out_voltage Con trỏ lưu giá trị điện áp pin (Volt)
 * @return esp_err_t ESP_OK nếu đọc thành công
 */
esp_err_t battery_read_voltage(float *out_voltage);

/**
 * @brief Lấy giá trị điện áp ADC thô tại chân GPIO (mV)
 * 
 * @param[out] out_mv Con trỏ lưu giá trị điện áp (mV)
 * @return esp_err_t ESP_OK nếu đọc thành công
 */
esp_err_t battery_read_raw_mv(int *out_mv);

#ifdef __cplusplus
}
#endif

#endif // __BATTERY_H__

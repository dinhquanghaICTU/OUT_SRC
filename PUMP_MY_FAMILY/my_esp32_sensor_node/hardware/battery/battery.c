#include "battery.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_rom_sys.h"

static const char *TAG = "BATTERY_ADC";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;
static bool s_has_cali = false;
static adc_channel_t s_channel = ADC_CHANNEL_6;
static adc_unit_t s_unit = ADC_UNIT_1;
static float s_divider_ratio = 2.0f;
static bool s_is_inited = false;

esp_err_t battery_init(int adc_gpio_pin, float divider_ratio) {
    s_divider_ratio = (divider_ratio > 0.1f) ? divider_ratio : 2.0f;

    esp_err_t err = adc_oneshot_io_to_channel(adc_gpio_pin, &s_unit, &s_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Chân GPIO %d không hỗ trợ ADC: %s", adc_gpio_pin, esp_err_to_name(err));
        return err;
    }

    // Khởi tạo ADC oneshot unit
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = s_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    err = adc_oneshot_new_unit(&init_config, &s_adc_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Không thể khởi tạo ADC Unit: %s", esp_err_to_name(err));
        return err;
    }

    // Cấu hình Channel: Độ phân giải 12-bit mặc định, suy hao 12dB (đo điện áp ngõ vào tới ~2.5V - 3.1V)
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    err = adc_oneshot_config_channel(s_adc_handle, s_channel, &chan_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Lỗi cấu hình kênh ADC: %s", esp_err_to_name(err));
        return err;
    }

    // Khởi tạo hiệu chuẩn Calibration (Line Fitting cho ESP32)
    s_has_cali = false;
#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = s_unit,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_config, &s_cali_handle) == ESP_OK) {
        s_has_cali = true;
        ESP_LOGI(TAG, "Hiệu chuẩn ADC Line Fitting: BẬT");
    } else {
        ESP_LOGW(TAG, "Không tìm thấy eFuse Calibration, dùng tính toán xấp xỉ");
    }
#endif

    s_is_inited = true;
    ESP_LOGI(TAG, "Khởi tạo đo pin thành công (GPIO %d -> Kênh ADC %d, Hệ số phân áp: %.2f)",
             adc_gpio_pin, (int)s_channel, s_divider_ratio);
    return ESP_OK;
}

esp_err_t battery_read_raw_mv(int *out_mv) {
    if (!s_is_inited || !s_adc_handle || !out_mv) {
        return ESP_ERR_INVALID_STATE;
    }

    // Lấy mẫu nhiều lần (16 mẫu) lọc nhiễu
    const int SAMPLES = 16;
    int sum_raw = 0;
    int success_cnt = 0;

    for (int i = 0; i < SAMPLES; i++) {
        int raw = 0;
        if (adc_oneshot_read(s_adc_handle, s_channel, &raw) == ESP_OK) {
            sum_raw += raw;
            success_cnt++;
        }
        esp_rom_delay_us(100);
    }

    if (success_cnt == 0) {
        return ESP_FAIL;
    }

    int avg_raw = sum_raw / success_cnt;
    int voltage_mv = 0;

    if (s_has_cali && s_cali_handle) {
        adc_cali_raw_to_voltage(s_cali_handle, avg_raw, &voltage_mv);
    } else {
        // Dự phòng khi chip không có eFuse calibration (suy hao 12dB: max ~2450mV)
        voltage_mv = (avg_raw * 2450) / 4095;
    }

    *out_mv = voltage_mv;
    return ESP_OK;
}

esp_err_t battery_read_voltage(float *out_voltage) {
    if (!out_voltage) {
        return ESP_ERR_INVALID_ARG;
    }

    int mv = 0;
    esp_err_t err = battery_read_raw_mv(&mv);
    if (err != ESP_OK) {
        return err;
    }

    // Điện áp pin (V) = (Điện áp tại chân ADC * hệ số cầu phân áp) / 1000
    float volt = ((float)mv / 1000.0f) * s_divider_ratio;
    if (volt < 0.0f) {
        volt = 0.0f;
    }

    *out_voltage = volt;
    return ESP_OK;
}

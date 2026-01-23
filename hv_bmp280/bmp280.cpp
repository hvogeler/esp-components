#include "bmp280.hpp"
#include "i2c.hpp"
#include <mutex>

esp_err_t Bmp280::read_calibration()
{
    uint8_t calib_data_raw[24];
    esp_err_t err = ESP_FAIL;

    auto &i2c = I2c::getInstance();
    // Retry up to 3 times if read fails
    for (int attempt = 0; attempt < 3; attempt++)
    {
        err = i2c.receive(dev_handle, BMP280_REG_CALIB_START, calib_data_raw, 24);
        if (err == ESP_OK)
        {
            break;
        }
        ESP_LOGW(TAG, "Calibration read attempt %d failed", attempt + 1);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read calibration after 3 attempts");
        return err;
    }

    // Parse calibration data (little-endian)
    calib_data.dig_T1 = (calib_data_raw[1] << 8) | calib_data_raw[0];
    calib_data.dig_T2 = (calib_data_raw[3] << 8) | calib_data_raw[2];
    calib_data.dig_T3 = (calib_data_raw[5] << 8) | calib_data_raw[4];
    calib_data.dig_P1 = (calib_data_raw[7] << 8) | calib_data_raw[6];
    calib_data.dig_P2 = (calib_data_raw[9] << 8) | calib_data_raw[8];
    calib_data.dig_P3 = (calib_data_raw[11] << 8) | calib_data_raw[10];
    calib_data.dig_P4 = (calib_data_raw[13] << 8) | calib_data_raw[12];
    calib_data.dig_P5 = (calib_data_raw[15] << 8) | calib_data_raw[14];
    calib_data.dig_P6 = (calib_data_raw[17] << 8) | calib_data_raw[16];
    calib_data.dig_P7 = (calib_data_raw[19] << 8) | calib_data_raw[18];
    calib_data.dig_P8 = (calib_data_raw[21] << 8) | calib_data_raw[20];
    calib_data.dig_P9 = (calib_data_raw[23] << 8) | calib_data_raw[22];

    return ESP_OK;
}

esp_err_t Bmp280::read_raw(int32_t *raw_temp, int32_t *raw_press)
{
    auto &i2c = I2c::getInstance();
    std::lock_guard<std::mutex> lock_i2c(i2c.getMutex());

    // Trigger forced mode measurement
    uint8_t ctrl_meas = build_ctrl_meas(0x01); // mode = forced
    esp_err_t err = i2c.transmit(dev_handle, BMP280_REG_CTRL_MEAS, ctrl_meas);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to trigger forced mode");
        return err;
    }

    // Poll status register until measurement is complete
    uint8_t status;
    int timeout = 100; // 100 attempts with 1ms delays = ~100ms max

    while (timeout > 0)
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        err = i2c.receive(dev_handle, BMP280_REG_STATUS, &status, 1);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to read status register");
            return err;
        }

        // Bit 3 is the 'measuring' bit - 0 means measurement complete
        if ((status & 0x08) == 0)
        {
            break;
        }
        timeout--;
    }

    if (timeout == 0)
    {
        ESP_LOGW(TAG, "Measurement timeout - status: 0x%02x", status);
        // Continue anyway - data might still be valid
    }

    // Read the measurement data
    uint8_t data[6];
    err = i2c.receive(dev_handle, BMP280_REG_PRESS_MSB, data, 6);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read measurement data");
        return err;
    }

    // Combine the 20-bit values
    *raw_press = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((uint32_t)data[2] >> 4);
    *raw_temp = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((uint32_t)data[5] >> 4);

    if (*raw_temp == 0 || *raw_temp == 0x80000 || *raw_press == 0 || *raw_press == 0x80000)
    {
        ESP_LOGW(TAG, "Invalid raw values - temp: 0x%lx, press: 0x%lx", *raw_temp, *raw_press);
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

void Bmp280::compensate_temp_press(int32_t raw_temp, int32_t raw_press,
                                   double *temperature, double *pressure)
{
    // Temperature compensation (from BMP280 datasheet)
    int32_t var1, var2, t_fine;
    var1 = ((((raw_temp >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * ((int32_t)calib_data.dig_T2)) >> 11;
    var2 = (((((raw_temp >> 4) - ((int32_t)calib_data.dig_T1)) *
              ((raw_temp >> 4) - ((int32_t)calib_data.dig_T1))) >>
             12) *
            ((int32_t)calib_data.dig_T3)) >>
           14;
    t_fine = var1 + var2;
    *temperature = (t_fine * 5 + 128) / 25600.0;

    // Pressure compensation (from BMP280 datasheet)
    int64_t var1_64, var2_64, p_64;
    var1_64 = ((int64_t)t_fine) - 128000;
    var2_64 = var1_64 * var1_64 * (int64_t)calib_data.dig_P6;
    var2_64 = var2_64 + ((var1_64 * (int64_t)calib_data.dig_P5) << 17);
    var2_64 = var2_64 + (((int64_t)calib_data.dig_P4) << 35);
    var1_64 = ((var1_64 * var1_64 * (int64_t)calib_data.dig_P3) >> 8) +
              ((var1_64 * (int64_t)calib_data.dig_P2) << 12);
    var1_64 = (((((int64_t)1) << 47) + var1_64)) * ((int64_t)calib_data.dig_P1) >> 33;

    if (var1_64 == 0)
    {
        *pressure = 0; // Avoid division by zero
        return;
    }

    p_64 = 1048576 - raw_press;
    p_64 = (((p_64 << 31) - var2_64) * 3125) / var1_64;
    var1_64 = (((int64_t)calib_data.dig_P9) * (p_64 >> 13) * (p_64 >> 13)) >> 25;
    var2_64 = (((int64_t)calib_data.dig_P8) * p_64) >> 19;
    p_64 = ((p_64 + var1_64 + var2_64) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
    *pressure = p_64 / 256.0;
}

esp_err_t Bmp280::read(double *temperature, double *pressure)
{
    int32_t raw_temp, raw_press;
    esp_err_t err = read_raw(&raw_temp, &raw_press);
    if (err != ESP_OK)
    {
        return err;
    }
    compensate_temp_press(raw_temp, raw_press, temperature, pressure);
    return ESP_OK;
}

esp_err_t Bmp280::init()
{
    esp_err_t err;
    uint8_t chip_id;
    bool found = false;

    i2c_device_config_t dev_config = {};
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = I2C_ADDRESS;
    dev_config.scl_speed_hz = I2C_CLOCK_SPEED;

    auto &i2c = I2c::getInstance();
    std::lock_guard<std::mutex> i2c_lock(i2c.getMutex());
    err = i2c.add_device(dev_config, dev_handle);
    if (err == ESP_OK)
    {
        err = i2c.receive(dev_handle, BMP280_REG_ID, &chip_id, 1);
        if (err == ESP_OK && (chip_id == BMP280_CHIP_ID || chip_id == BME280_CHIP_ID))
        {
            i2c_dev_addr = I2C_ADDRESS;
            found = true;
            ESP_LOGI(TAG, "Found %s at address 0x%02x (chip ID: 0x%02x)",
                     chip_id == BME280_CHIP_ID ? "BME280" : "BMP280",
                     i2c_dev_addr, chip_id);
        }
        else
        {
            i2c.rm_device(dev_handle);
        }
    }

    if (!found)
    {
        ESP_LOGE(TAG, "BMP280/BME280 not found at address 0x%02x", I2C_ADDRESS);
        return ESP_FAIL;
    }

    // Reset the sensor
    err = i2c.transmit(dev_handle, BMP280_REG_RESET, 0xB6);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to reset BMP280");
        return err;
    }

    // Wait for sensor to complete reset
    vTaskDelay(pdMS_TO_TICKS(100));

    // Verify sensor is responsive after reset
    uint8_t status;
    err = i2c.receive(dev_handle, BMP280_REG_STATUS, &status, 1);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Sensor not responding after reset");
        return err;
    }
    ESP_LOGI(TAG, "Sensor status after reset: 0x%02x", status);

    // Read calibration data
    err = read_calibration();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read calibration data");
        return err;
    }
    ESP_LOGI(TAG, "Calibration data read successfully");
    if (calib_data.dig_T1 == 0 || calib_data.dig_P1 == 0)
    {
        ESP_LOGE(TAG, "Invalid calibration data - T1: %u, P1: %u",
                 calib_data.dig_T1, calib_data.dig_P1);
        return ESP_ERR_INVALID_RESPONSE;
    }
    ESP_LOGI(TAG, "Calibration: T1=%u, T2=%d, T3=%d, P1=%u",
             calib_data.dig_T1, calib_data.dig_T2, calib_data.dig_T3, calib_data.dig_P1);

    // Configure sensor with settings from Kconfig
    uint8_t ctrl_meas = build_ctrl_meas(0x00); // mode = sleep initially
    err = i2c.transmit(dev_handle, BMP280_REG_CTRL_MEAS, ctrl_meas);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure CTRL_MEAS");
        return err;
    }

    uint8_t config = build_config();
    err = i2c.transmit(dev_handle, BMP280_REG_CONFIG, config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure CONFIG");
        return err;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "BMP280 initialization complete (addr=0x%02x, speed=%luHz, osrs_t=%d, osrs_p=%d, filter=%d)",
             I2C_ADDRESS, I2C_CLOCK_SPEED, TEMP_OVERSAMPLING, PRESS_OVERSAMPLING, IIR_FILTER);
    return ESP_OK;
}

#pragma once

#include <mutex>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"

// BMP280 register addresses
#define BMP280_REG_ID           0xD0
#define BMP280_REG_RESET        0xE0
#define BMP280_REG_STATUS       0xF3
#define BMP280_REG_CTRL_MEAS    0xF4
#define BMP280_REG_CONFIG       0xF5
#define BMP280_REG_PRESS_MSB    0xF7
#define BMP280_REG_TEMP_MSB     0xFA
#define BMP280_REG_CALIB_START  0x88

// BMP280 chip IDs
#define BMP280_CHIP_ID  0x58
#define BME280_CHIP_ID  0x60

// Calibration data structure
struct bmp280_calib_data
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
};

class Bmp280
{
    i2c_master_dev_handle_t dev_handle;
    uint8_t i2c_dev_addr;
    bmp280_calib_data calib_data;

    mutable std::mutex mutex_;

    static constexpr const char *TAG = "Bmp280";

    // Configuration from Kconfig
    static constexpr uint8_t I2C_ADDRESS = CONFIG_HV_BMP280_I2C_ADDRESS;
    static constexpr uint32_t I2C_CLOCK_SPEED = CONFIG_HV_BMP280_I2C_CLOCK_SPEED_HZ;
    static constexpr uint8_t TEMP_OVERSAMPLING = CONFIG_HV_BMP280_TEMP_OVERSAMPLING;
    static constexpr uint8_t PRESS_OVERSAMPLING = CONFIG_HV_BMP280_PRESS_OVERSAMPLING;
    static constexpr uint8_t IIR_FILTER = CONFIG_HV_BMP280_IIR_FILTER;

public:
    bool is_initialized = false;

public:
    // Delete copy constructor and assignment operator
    Bmp280(const Bmp280 &) = delete;
    Bmp280 &operator=(const Bmp280 &) = delete;

    // Static method to get the singleton instance
    static Bmp280 &getInstance()
    {
        static Bmp280 instance;
        return instance;
    }
    std::mutex &getMutex() { return mutex_; }

    esp_err_t init();
    esp_err_t read_raw(int32_t *raw_temp, int32_t *raw_press);
    void compensate_temp_press(int32_t raw_temp, int32_t raw_press,
                               double *temperature, double *pressure);

    // Convenience method to read compensated values directly
    esp_err_t read(double *temperature, double *pressure);

private:
    Bmp280() : dev_handle(nullptr), i2c_dev_addr(0)
    {
    }

    esp_err_t read_calibration();

    // Build CTRL_MEAS register value from oversampling settings
    static constexpr uint8_t build_ctrl_meas(uint8_t mode = 0x01)
    {
        // osrs_t[7:5], osrs_p[4:2], mode[1:0]
        return (TEMP_OVERSAMPLING << 5) | (PRESS_OVERSAMPLING << 2) | mode;
    }

    // Build CONFIG register value from filter setting
    static constexpr uint8_t build_config()
    {
        // t_sb[7:5], filter[4:2], spi3w_en[0]
        return (IIR_FILTER << 2);
    }
};

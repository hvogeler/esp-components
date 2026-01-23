#include "mcp23017.hpp"
#include "sdkconfig.h"

MCP23017 &MCP23017::getInstance()
{
    static MCP23017 instance;
    return instance;
}

std::optional<std::unique_lock<std::timed_mutex>> MCP23017::lock(std::chrono::milliseconds timeout)
{
    std::unique_lock<std::timed_mutex> lk(mutex_, std::defer_lock);
    if (lk.try_lock_for(timeout))
    {
        return lk;
    }
    return std::nullopt;
}

std::timed_mutex &MCP23017::getMutex()
{
    return mutex_;
}

MCP23017::MCP23017() : dev_handle_(nullptr), address_(CONFIG_MCP23017_I2C_ADDRESS), initialized_(false) {}

MCP23017::~MCP23017()
{
    if (initialized_ && dev_handle_)
    {
        I2c::getInstance().rm_device(dev_handle_);
    }
}

esp_err_t MCP23017::init()
{
    if (initialized_)
    {
        return ESP_ERR_INVALID_STATE;
    }

    i2c_device_config_t dev_config = {};
    dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_config.device_address = address_;
    dev_config.scl_speed_hz = CONFIG_MCP23017_I2C_CLOCK_FREQ;

    esp_err_t err = I2c::getInstance().add_device(dev_config, dev_handle_);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_, "Failed to add MCP23017 device: %s", esp_err_to_name(err));
        return err;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_LOGI(TAG_, "MCP23017 initialized at address 0x%02X", address_);
    initialized_ = true;

    err = writeRegister(Register::IODIRA, 0xFF);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_, "MCP23017 writeRegister failed, %02X", err);
        return ESP_ERR_INVALID_RESPONSE;
    }
    writeRegister(Register::IODIRB, 0xFF);

    uint8_t verify;
    err = readRegister(Register::IODIRA, &verify);
    if (err != ESP_OK || verify != 0xFF)
    {
        ESP_LOGE(TAG_, "MCP23017 communication verify failed");
        return ESP_ERR_INVALID_RESPONSE;
    }
    return ESP_OK;
}

esp_err_t MCP23017::writeRegister(Register reg, uint8_t value)
{
    if (!initialized_)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return I2c::getInstance().transmit(dev_handle_, static_cast<uint8_t>(reg), value);
}

esp_err_t MCP23017::readRegister(Register reg, uint8_t *value)
{
    if (!initialized_)
    {
        return ESP_ERR_INVALID_STATE;
    }
    return I2c::getInstance().receive(dev_handle_, static_cast<uint8_t>(reg), value, 1);
}

esp_err_t MCP23017::setPortADirection(uint8_t direction)
{
    return writeRegister(Register::IODIRA, direction);
}

esp_err_t MCP23017::setPortBDirection(uint8_t direction)
{
    return writeRegister(Register::IODIRB, direction);
}

esp_err_t MCP23017::writePortA(uint8_t value)
{
    return writeRegister(Register::GPIOA, value);
}

esp_err_t MCP23017::writePortB(uint8_t value)
{
    return writeRegister(Register::GPIOB, value);
}

esp_err_t MCP23017::readPortA(uint8_t &value)
{
    return readRegister(Register::GPIOA, &value);
}

esp_err_t MCP23017::readPortB(uint8_t &value)
{
    return readRegister(Register::GPIOB, &value);
}

esp_err_t MCP23017::setPullUpA(uint8_t pullup)
{
    return writeRegister(Register::GPPUA, pullup);
}

esp_err_t MCP23017::setPullUpB(uint8_t pullup)
{
    return writeRegister(Register::GPPUB, pullup);
}

void MCP23017::reset()
{
    idf::GPIO_Output reset_gpio_{idf::GPIONum(CONFIG_MCP23017_RESET_GPIO)};
    reset_gpio_.set_low();
    vTaskDelay(pdMS_TO_TICKS(100));
    reset_gpio_.set_high();
}

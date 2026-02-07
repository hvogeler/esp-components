#pragma once

#include "driver/i2c_master.h"
#include "esp_log.h"
#include "i2c.hpp"
#include "sdkconfig.h"
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include "freertos/FreeRTOS.h"
#include "gpio_cxx.hpp"

/**
 * @brief MCP23017 bank selection
 */
enum class McpBank : uint8_t
{
    GPA = 0,
    GPB = 1,
};

enum class PinLevel : uint8_t
{
    LOW = 0,
    HIGH = 1,
};

enum class PinDirection : uint8_t
{
    OUTPUT = 0,
    INPUT = 1,
};

class MCP23017
{
public:
    // MCP23017 Register addresses (IOCON.BANK = 0)
    enum class Register : uint8_t
    {
        IODIRA = 0x00,   // I/O direction register A
        IODIRB = 0x01,   // I/O direction register B
        IPOLA = 0x02,    // Input polarity register A
        IPOLB = 0x03,    // Input polarity register B
        GPINTENA = 0x04, // Interrupt-on-change enable A
        GPINTENB = 0x05, // Interrupt-on-change enable B
        DEFVALA = 0x06,  // Default compare register A
        DEFVALB = 0x07,  // Default compare register B
        INTCONA = 0x08,  // Interrupt control register A
        INTCONB = 0x09,  // Interrupt control register B
        IOCON = 0x0A,    // Configuration register
        GPPUA = 0x0C,    // Pull-up resistor register A
        GPPUB = 0x0D,    // Pull-up resistor register B
        INTFA = 0x0E,    // Interrupt flag register A
        INTFB = 0x0F,    // Interrupt flag register B
        INTCAPA = 0x10,  // Interrupt capture register A
        INTCAPB = 0x11,  // Interrupt capture register B
        GPIOA = 0x12,    // GPIO register A
        GPIOB = 0x13,    // GPIO register B
        OLATA = 0x14,    // Output latch register A
        OLATB = 0x15,    // Output latch register B
    };

    static MCP23017 &getInstance();

    std::optional<std::unique_lock<std::timed_mutex>> lock(std::chrono::milliseconds timeout);
    std::timed_mutex &getMutex();

    esp_err_t init();

    // each binary bit of direction addresses one pin
    // 0 - Output, 1 - Input
    // 0x01111110 means pin 7 and pin 0 are output.
    esp_err_t setPortADirection(uint8_t direction_pin_mask);
    esp_err_t setPortBDirection(uint8_t direction_pin_mask);
    esp_err_t readPortADirection(uint8_t &direction);
    esp_err_t readPortBDirection(uint8_t &direction);
    esp_err_t writePortA(uint8_t value);
    esp_err_t writePortB(uint8_t value);
    esp_err_t readPortA(uint8_t &value);
    esp_err_t readPortB(uint8_t &value);
    esp_err_t setPullUpA(uint8_t pullup);
    esp_err_t setPullUpB(uint8_t pullup);

    esp_err_t setPin(McpBank bank, uint8_t pin, PinLevel level);
    esp_err_t setPinDirection(McpBank bank, uint8_t pin, PinDirection direction);

    void reset();

    bool isInitialized() const { return initialized_; }

private:
    static constexpr const char *TAG_ = "MCP23017";

    MCP23017();
    ~MCP23017();

    MCP23017(const MCP23017 &) = delete;
    MCP23017 &operator=(const MCP23017 &) = delete;
    esp_err_t writeRegister(Register reg, uint8_t value);
    esp_err_t readRegister(Register reg, uint8_t *value);

    i2c_master_dev_handle_t dev_handle_;
    uint8_t address_;
    bool initialized_;
    mutable std::timed_mutex mutex_;
};

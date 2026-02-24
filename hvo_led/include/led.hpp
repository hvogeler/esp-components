#ifndef LED_HPP
#define LED_HPP

#include <stdint.h>

#include "driver/gpio.h"
#include "gpio_cxx.hpp"

namespace hvo {

typedef enum class LedState
{
    on,
    off,
} led_state_t;

class Led
{
    idf::GPIO_Output gpio_pin_;
    led_state_t led_state_;
    bool is_active_high_{true};

public:
    Led(uint32_t pin) : gpio_pin_(idf::GPIONum(pin)), led_state_(LedState::off)
    {
    }

    Led(uint32_t pin, bool is_active_high) : gpio_pin_(idf::GPIONum(pin)), led_state_(LedState::off)
    {
        is_active_high_ = is_active_high;
    }
    
    void turn_on()
    {
        if (is_active_high_) {
            gpio_pin_.set_high();
        } else {
            gpio_pin_.set_low();
        }
        led_state_ = LedState::on;
    }

    void turn_off()
    {
        if (is_active_high_) {
            gpio_pin_.set_low();
        } else {
            gpio_pin_.set_high();
        }
        led_state_ = LedState::off;
    }

    LedState state()
    {
        return led_state_;
    }

    const char *state_str()
    {
        switch (led_state_)
        {
        case LedState::on:
            return "ON";
        case LedState::off:
            return "OFF";
        default:
            return "UNKNOWN";
        }
    }

    void blink(uint32_t duration_ms = 500, uint32_t count = 1);
};

} // namespace hvo

#endif // LED_HPP

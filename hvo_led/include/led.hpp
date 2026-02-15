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

public:
    Led(uint32_t pin) : gpio_pin_(idf::GPIONum(pin)), led_state_(LedState::off)
    {
    }

    void turn_on()
    {
        gpio_pin_.set_high();
        led_state_ = LedState::on;
    }

    void turn_off()
    {
        gpio_pin_.set_low();
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

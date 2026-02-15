#include "led.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void hvo::Led::blink(uint32_t duration_ms, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        turn_on();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        turn_off();
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
    }
}

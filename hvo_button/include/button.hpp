#include "button_gpio.h"
#include "iot_button.h"
#include "esp_log.h"

namespace hvo {

class Button
{
private:
    button_handle_t handle;
    button_gpio_config_t gpio_config;
    button_config_t config;
    void *user_data_;
    static constexpr const char *TAG = "Button";

public:
    Button(const Button &) = delete;
    Button &operator=(const Button &) = delete;
    Button(Button &&) = delete;
    Button &operator=(Button &&) = delete;

    ~Button()
    {
        iot_button_delete(handle);
    }

    Button(int32_t gpio, int32_t active_level, button_event_t event_type, button_cb_t callback,
           void *user_data = nullptr, bool enable_power_save = true, bool disable_pull = false)
        : user_data_(user_data ? user_data : this)
    {
        gpio_config = {};
        gpio_config.active_level = active_level;
        gpio_config.gpio_num = gpio;
        gpio_config.enable_power_save = enable_power_save;
        gpio_config.disable_pull = disable_pull;

        config = {};
        esp_err_t err = iot_button_new_gpio_device(&config, &gpio_config, &handle);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Button initialization failed");
        }
        err = iot_button_register_cb(handle, event_type, NULL, callback, user_data_);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Button register callback failed");
        }
    }

    esp_err_t register_callback(button_event_t event_type, button_cb_t callback)
    {
        esp_err_t err = iot_button_register_cb(handle, event_type, NULL, callback, user_data_);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Button register callback failed");
        }
        return err;
    }

    button_event_t get_event()
    {
        return iot_button_get_event(handle);
    }

    void print_event()
    {
        iot_button_print_event(handle);
    }
};

} // namespace hvo

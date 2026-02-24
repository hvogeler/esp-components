# hvo/wifi

A singleton C++ wrapper around the ESP-IDF Wi-Fi station (STA) API. It handles connection setup, retry logic, optional hostname assignment, and NTP time synchronisation. Credentials are read automatically from the `config` NVS namespace (written there by a provisioning step).

## Features

- Singleton — one shared instance across all files (`Wifi::getInstance()`).
- Reads Wi-Fi SSID and password from NVS (`config` namespace, keys `wifi_ssid` / `wifi_password`) so credentials never need to be hard-coded.
- Optional custom hostname set before connecting (visible in the router's device list and in mDNS).
- Blocking `wifi_connect()` that waits until connected or until the retry limit is reached.
- `time_sync()` configures SNTP and blocks until the system clock is set.
- Thread-safe via an internal mutex (`getMutex()`).
- Configurable via `menuconfig` (timezone, NTP servers, max retries).

## Dependencies

This component depends on `hvo/nvs` (pulled in automatically) to read Wi-Fi credentials from NVS.
The `nvs_flash` partition must therefore be initialised once in `app_main` before calling `wifi_connect()` — see the [hvo/nvs README](../nvs/README.md).

## Adding to a project

Add to `main/idf_component.yml`:

```yaml
dependencies:
  hvo/wifi:
    git: https://github.com/hvogeler/esp-components.git
    path: wifi
    version: "1.0.25"
```

Add to `main/CMakeLists.txt`:

```cmake
idf_component_register(SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
      nvs_flash   # needed for nvs_flash_init() in app_main
      wifi        # this component
)
```

## Kconfig options

Run `idf.py menuconfig` and navigate to **WiFi Configuration**:

| Config symbol            | Default                          | Description                        |
|--------------------------|----------------------------------|------------------------------------|
| `HV_WIFI_TIMEZONE`       | `CET-1CEST,M3.5.0,M10.5.0/3`    | POSIX timezone string for SNTP     |
| `HV_WIFI_TIME_SERVER_0`  | `0.de.pool.ntp.org`              | Primary NTP server                 |
| `HV_WIFI_TIME_SERVER_1`  | `1.de.pool.ntp.org`              | Secondary NTP server               |
| `HV_WIFI_MAX_RETRY`      | `5` (range 1–20)                 | Max connection attempts before fail|

## API

```cpp
class Wifi {
public:
    static Wifi &getInstance();                      // singleton accessor

    void        set_hostname(const std::string &h);  // call before wifi_connect()
    esp_err_t   wifi_connect();                      // blocking: connects or fails after MAX_RETRY
    esp_err_t   time_sync();                         // blocking: syncs system clock via SNTP
    std::string get_wifi_ssid();                     // returns the SSID used to connect
    bool        get_is_connected();                  // current connection state
    std::mutex &getMutex();                          // for external locking if needed
};
```

### Credentials

`wifi_connect()` reads the SSID and password from the NVS `config` namespace:

| Namespace | Key             | Written by             |
|-----------|-----------------|------------------------|
| `config`  | `wifi_ssid`     | provisioning component |
| `config`  | `wifi_password` | provisioning component |

If the keys are absent (device not yet provisioned), `wifi_ssid` defaults to `"unset"` and the connection will fail.

## Usage example (heatsens)

The sequence in `heatsens/main/heatsens.cpp` shows the recommended call order:

```cpp
#include "nvs_flash.h"
#include "nvs.hpp"
#include "wifi.hpp"

extern "C" void app_main(void)
{
    // 1. Initialise NVS flash partition (required before any NVS or Wi-Fi call)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // ... other init (display, provisioning, I2C) ...

    // 2. Optionally set a custom hostname from NVS before connecting
    auto &wifi = Wifi::getInstance();
    {
        Nvs nvs_meta;
        std::string device_name;
        if (nvs_meta.open_namespace("meta") == ESP_OK &&
            nvs_meta.read("device_name", device_name) == ESP_OK)
        {
            wifi.set_hostname("heatsens_" + device_name);
        }
    }

    // 3. Connect (blocking — returns ESP_OK or an error code)
    ret = wifi.wifi_connect();
    if (ret != ESP_OK) {
        std::string init_error = std::string("Wifi Error\n") + wifi.get_wifi_ssid();
        // handle error, e.g. show error screen
    }

    // 4. Sync the system clock via SNTP (only after a successful connection)
    if (ret == ESP_OK) {
        ret = wifi.time_sync();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Could not sync time with time server");
        }
    }
}
```

### Checking connection state at runtime

```cpp
auto &wifi = Wifi::getInstance();
if (wifi.get_is_connected()) {
    // proceed with network operations
}
```

### Logging the SSID in error messages

```cpp
ret = wifi.wifi_connect();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to connect to: %s", wifi.get_wifi_ssid().c_str());
}
```

## Notes

- **Provisioning**: This component does not write credentials to NVS. Use a provisioning component (e.g. `webprov`) to store `wifi_ssid` and `wifi_password` in the `config` namespace before the first boot.
- **Blocking connect**: `wifi_connect()` blocks until a connection is established or `CONFIG_HV_WIFI_MAX_RETRY` attempts are exhausted. Do not call it from a time-critical context.
- **Blocking time sync**: `time_sync()` polls SNTP every 2 seconds for up to 10 retries (20 seconds max). It returns `ESP_FAIL` if the clock cannot be synchronised within that window, which can safely be treated as a warning.
- **Timezone**: The POSIX timezone string set via `HV_WIFI_TIMEZONE` is applied before SNTP starts, so `localtime_r()` returns correctly offset times immediately after `time_sync()` returns.
- **Mutex**: The internal mutex is held inside the Wi-Fi event handler. If you inspect connection state from multiple tasks, acquire `getMutex()` first.

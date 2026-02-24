# hvo/nvs

A lightweight C++ wrapper around the ESP-IDF NVS (Non-Volatile Storage) API. It simplifies reading and writing typed key-value pairs (strings, integers, doubles) to flash storage using namespaced handles.

## Features

- RAII-style handle management: the NVS handle is opened per `Nvs` instance and automatically closed in the destructor.
- Overloaded `read` / `write` for `int`, `double`, and `std::string`.
- Doubles are stored as scaled `int32_t` values (caller is responsible for scaling, e.g. multiply by 10 before writing and divide after reading).
- Non-copyable and non-movable — create one instance per namespace per scope.

## Adding to a project

Add to your `main/idf_component.yml`:

```yaml
dependencies:
  hvo/nvs:
    git: https://github.com/hvogeler/esp-components.git
    path: nvs
    version: "1.0.25"
```

Add to your `main/CMakeLists.txt` `REQUIRES` list:

```cmake
idf_component_register(SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES
      nvs_flash   # ESP-IDF built-in — must call nvs_flash_init() once at startup
      nvs         # this component
)
```

## Initialization (once per firmware startup)

The ESP-IDF NVS flash partition must be initialized once before any `Nvs` object is used. Do this in `app_main`:

```cpp
#include "nvs_flash.h"

esp_err_t ret = nvs_flash_init();
if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
}
ESP_ERROR_CHECK(ret);
```

## API

```cpp
class Nvs {
public:
    Nvs();                                          // default constructor
    esp_err_t open_namespace(std::string ns);       // open (or create) an NVS namespace

    esp_err_t write(std::string key, int v);        // write an integer
    esp_err_t write(std::string key, double v);     // write a double (stored as int32)
    esp_err_t write(std::string key, std::string v);// write a string (max ~64 chars)

    esp_err_t read(std::string key, int &v);        // read an integer
    esp_err_t read(std::string key, double &v);     // read a double
    esp_err_t read(std::string key, std::string &v);// read a string
};
```

`open_namespace` must be called before any read/write. Check its return value — if it fails (e.g. NVS partition not initialized), subsequent reads/writes will also fail.

## Usage examples

### Reading configuration at startup

From `heatsens/main/mqtt.cpp` — reading MQTT credentials stored by a provisioning step:

```cpp
#include "nvs.hpp"

esp_err_t Mqtt::connect() {
    Nvs nvs_creds;
    esp_err_t ret = nvs_creds.open_namespace("config");
    if (ret == ESP_OK) {
        nvs_creds.read("mqtt_broker",   mqtt_broker_);
        nvs_creds.read("mqtt_user",     mqtt_user_);
        nvs_creds.read("mqtt_password", mqtt_password_);
    } else {
        ESP_LOGE(TAG, "Reading MQTT credentials from NVS failed");
    }
    // ...
}
```

### Reading multiple values from the same namespace

From `heatsens/main/temp_model.cpp` — loading device configuration and persisted settings at startup:

```cpp
#include "nvs.hpp"

void TempModel::init() {
    // Read persisted target temperature (stored scaled by 10)
    Nvs nvs_heatsens;
    nvs_heatsens.open_namespace("heatsens");
    esp_err_t ret = nvs_heatsens.read("tgt_temp", tgt_temp_);
    tgt_temp_ /= 10.0;   // undo the scaling applied on write
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "No target temp found in NVS");
    }

    // Read device metadata from a separate namespace
    Nvs nvs_config;
    nvs_config.open_namespace("meta");
    ret = nvs_config.read("device_name", device_name_);
    if (ret != ESP_OK) {
        device_name_ = "undefined";
    }
    ret = nvs_config.read("heat_actuator", heat_actuator_);
    // Optional fields — only use the value if the read succeeded
    int night_start = 0;
    ret = nvs_config.read("night_start", night_start);
    if (ret == ESP_OK) {
        night_start_time_ = night_start;
    }
}
```

### Writing a value when it changes

From `heatsens/main/temp_model.cpp` — persisting a new target temperature:

```cpp
void TempModel::set_tgt_temp(double temp) {
    tgt_temp_ = temp;

    Nvs nvs;
    nvs.open_namespace("heatsens");
    // Doubles are stored as int32 — scale to preserve one decimal place
    esp_err_t ret = nvs.write("tgt_temp", temp * 10.0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write target temp to NVS");
    }
}
```

### Writing multiple values to the same namespace

From `heatsens/main/temp_model.cpp` — saving a full device metadata record:

```cpp
void TempModel::set_device_meta(std::string name, int heat_actuator, ...) {
    Nvs nvs;
    nvs.open_namespace("meta");

    esp_err_t ret = nvs.write("device_name",   name);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write device name to NVS");
    }
    ret = nvs.write("heat_actuator", heat_actuator);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write heat actuator to NVS");
    }
    // Doubles: multiply before writing, divide after reading
    ret = nvs.write("night_tgt_temp", night_temp.value() * 10.0);
    ret = nvs.write("night_start",    night_start.value());
    ret = nvs.write("night_end",      night_end.value());
}
```

### Reading NVS at startup to set a runtime parameter

From `heatsens/main/heatsens.cpp` — reading a device name to configure the Wi-Fi hostname:

```cpp
#include "nvs.hpp"
#include "wifi.hpp"

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
wifi.wifi_connect();
```

## Namespace conventions (heatsens example)

| Namespace  | Key              | Type     | Notes                                      |
|------------|------------------|----------|--------------------------------------------|
| `config`   | `wifi_ssid`      | string   | Written by web provisioning                |
| `config`   | `wifi_password`  | string   | Written by web provisioning                |
| `config`   | `mqtt_broker`    | string   | Written by web provisioning                |
| `config`   | `mqtt_user`      | string   | Written by web provisioning                |
| `config`   | `mqtt_password`  | string   | Written by web provisioning                |
| `meta`     | `device_name`    | string   | Human-readable name set during provisioning|
| `meta`     | `heat_actuator`  | int      | Actuator ID                                |
| `meta`     | `night_tgt_temp` | double×10| Night-mode setpoint, scaled by 10          |
| `meta`     | `night_start`    | int      | Hour (0–23) when night mode begins         |
| `meta`     | `night_end`      | int      | Hour (0–23) when night mode ends           |
| `heatsens` | `tgt_temp`       | double×10| Day-mode temperature setpoint, scaled by 10|

## Notes

- **Key length**: NVS keys are limited to 15 characters by the ESP-IDF.
- **String length**: The `read(string)` implementation uses a 65-byte buffer internally; strings longer than 64 characters will be truncated.
- **Double storage**: There is no native `double` NVS type. This component stores doubles as `int32_t`. The caller must apply a scaling factor (e.g. `× 10`) before writing and the inverse after reading.
- **Commit on write**: Every `write` call immediately calls `nvs_commit`, so data is persisted even without an explicit flush step.
- **One namespace per instance**: Each `Nvs` object holds one open handle. To access multiple namespaces, create multiple `Nvs` objects (they can be stack-allocated in the same scope).

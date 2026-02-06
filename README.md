# ESP-IDF Components

Personal collection of reusable ESP-IDF components.

## Components

| Component | Description |
|-----------|-------------|
| [bmp280](./bmp280/) | BMP280/BME280 temperature and pressure sensor driver |
| [i2c](./i2c/) | I2C master bus wrapper with singleton interface |
| [mcp23017](./mcp23017/) | MCP23017 16-bit I/O expander driver |
| [nvs](./nvs/) | NVS wrapper class for simplified key-value storage |
| [tdisplays3](./tdisplays3/) | LilyGO T-Display S3 board with ST7789 LCD and LVGL integration |
| [wifi](./wifi/) | WiFi wrapper with singleton interface and NTP time sync |

## Dependencies

| Component | ESP-IDF Components | Custom Components |
|-----------|-------------------|-------------------|
| bmp280 | driver | i2c |
| i2c | driver | - |
| mcp23017 | driver | i2c |
| nvs | nvs_flash | - |
| tdisplays3 | driver, esp_lcd, esp_timer | - |
| wifi | esp_wifi, esp_event, esp_netif, nvs_flash, esp_sntp | nvs |

### Kconfig Options

**wifi** component provides these configuration options (prefix `HV_WIFI_`):

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `HV_WIFI_TIMEZONE` | string | `CET-1CEST,M3.5.0,M10.5.0/3` | POSIX timezone string |
| `HV_WIFI_TIME_SERVER_0` | string | `0.de.pool.ntp.org` | Primary NTP server |
| `HV_WIFI_TIME_SERVER_1` | string | `1.de.pool.ntp.org` | Secondary NTP server |
| `HV_WIFI_MAX_RETRY` | int | `5` | Max WiFi connection retries |

## Usage

### Via IDF Component Manager (Recommended)

Add to your project's `main/idf_component.yml`:

```yaml
dependencies:
  bmp280:
    git: https://github.com/hvogeler/esp-components.git
    path: bmp280
    version: "*"
  i2c:
    git: https://github.com/hvogeler/esp-components.git
    path: i2c
    version: "*"
  mcp23017:
    git: https://github.com/hvogeler/esp-components.git
    path: mcp23017
    version: "*"
  nvs:
    git: https://github.com/hvogeler/esp-components.git
    path: nvs
    version: "*"
  tdisplays3:
    git: https://github.com/hvogeler/esp-components.git
    path: tdisplays3
    version: "*"
  wifi:
    git: https://github.com/hvogeler/esp-components.git
    path: wifi
    version: "*"
```

Then run `idf.py build` - components will be fetched automatically.

### Via EXTRA_COMPONENT_DIRS

Add to your project's top-level `CMakeLists.txt`:

```cmake
set(EXTRA_COMPONENT_DIRS "/path/to/esp-components")
```

## Requirements

- ESP-IDF v5.0 or later
- C++17 or later

## Hardware Notes

### I2C Bus
- Use 2.2k pull-up resistors (10k may not work reliably)
- Pull up to 3.3V only, even if peripherals use 5V supply

## License

MIT

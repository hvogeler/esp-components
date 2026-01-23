# ESP-IDF Components

Personal collection of reusable ESP-IDF components.

## Components

| Component | Description |
|-----------|-------------|
| [bmp280](./bmp280/) | BMP280/BME280 temperature and pressure sensor driver |
| [i2c](./i2c/) | I2C master bus wrapper with singleton interface |
| [mcp23017](./mcp23017/) | MCP23017 16-bit I/O expander driver |
| [tdisplays3](./tdisplays3/) | LilyGO T-Display S3 board with ST7789 LCD and LVGL integration |

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
  tdisplays3:
    git: https://github.com/hvogeler/esp-components.git
    path: tdisplays3
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

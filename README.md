# ESP-IDF Components

Personal collection of reusable ESP-IDF components.

## Components

| Component | Description |
|-----------|-------------|
| [i2c](./i2c/) | I2C master bus wrapper with singleton interface |
| [mcp23017](./mcp23017/) | MCP23017 16-bit I/O expander driver |

## Usage

### Via IDF Component Manager (Recommended)

Add to your project's `main/idf_component.yml`:

```yaml
dependencies:
  i2c:
    git: https://github.com/YOUR_USERNAME/esp-components.git
    path: i2c
    version: "*"
  mcp23017:
    git: https://github.com/YOUR_USERNAME/esp-components.git
    path: mcp23017
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

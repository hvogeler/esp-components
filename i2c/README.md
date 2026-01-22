# I2C Component

ESP-IDF I2C master bus wrapper providing a singleton interface for I2C communication.

## Configuration

Configure via `idf.py menuconfig` under **I2C Configuration**:

| Option | Description | Default | Range |
|--------|-------------|---------|-------|
| `I2C_SCL_GPIO` | GPIO pin for I2C clock line | 4 | 0-48 |
| `I2C_SDA_GPIO` | GPIO pin for I2C data line | 3 | 0-48 |
| `I2C_PROBE_ADDRESS` | Device address to probe at init (0 = disabled) | 0x00 | 0x00-0x7F |

## Usage

```cpp
#include "i2c.hpp"

// Get singleton instance and initialize
auto &i2c = I2c::getInstance();
i2c.init();

// Add a device to the bus
i2c_device_config_t dev_config = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = 0x20,
    .scl_speed_hz = 100000,
};
i2c_master_dev_handle_t dev_handle;
i2c.add_device(dev_config, dev_handle);

// Write to a register
i2c.transmit(dev_handle, reg_addr, value);

// Read from a register
uint8_t data;
i2c.receive(dev_handle, reg_addr, &data, 1);

// Remove device when done
i2c.rm_device(dev_handle);
```

## API

| Method | Description |
|--------|-------------|
| `getInstance()` | Returns singleton instance |
| `init()` | Initializes I2C master bus with configured pins |
| `add_device(config, handle)` | Adds device to bus, returns handle |
| `rm_device(handle)` | Removes device from bus |
| `transmit(handle, reg, value)` | Writes single byte to register |
| `receive(handle, reg, data, len)` | Reads bytes from register |
| `del_bus()` | Deletes the I2C bus |
| `getMutex()` | Returns mutex for external synchronization |

## Device Probe

When `I2C_PROBE_ADDRESS` is set to a non-zero value, the component probes for a device at that address during `init()`. This is useful for verifying hardware connections at startup.

Common probe addresses:
- `0x20-0x27` - MCP23017 GPIO expander
- `0x48-0x4F` - PCF8591 ADC/DAC
- `0x50-0x57` - AT24C EEPROM

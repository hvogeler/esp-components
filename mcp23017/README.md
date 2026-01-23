# MCP23017 Component

ESP-IDF component driver for the MCP23017 16-bit I/O expander.

## Features

- Thread-safe singleton design with timed mutex locking
- Configurable I2C address, clock frequency, and reset GPIO via Kconfig
- Support for both Port A and Port B (16 GPIO pins total)
- Hardware reset capability
- Pull-up resistor configuration

## Configuration

Run `idf.py menuconfig` and navigate to "MCP23017 Configuration":

| Parameter | Default | Range | Description |
|-----------|---------|-------|-------------|
| `CONFIG_HV_MCP23017_I2C_ADDRESS` | 0x20 | 0x20-0x27 | I2C device address (depends on A0-A2 pins) |
| `CONFIG_HV_MCP23017_I2C_CLOCK_FREQ` | 100000 | 10000-400000 | I2C clock frequency in Hz |
| `CONFIG_HV_MCP23017_RESET_GPIO` | 6 | 0-48 | GPIO pin connected to MCP23017 reset |

## Dependencies

- `i2c` component (provides `I2c` singleton class)
- `espressif__esp-idf-cxx` (for GPIO C++ wrapper)

## Usage

### Initialization

```cpp
#include "mcp23017.hpp"

// Initialize I2C bus first
I2c::getInstance().init();

// Get MCP23017 instance and initialize
auto &mcp = MCP23017::getInstance();
mcp.reset();  // Optional: hardware reset
mcp.init();
```

### Configuring Pin Direction

Each bit in the direction mask corresponds to a pin (0-7):
- `0` = Output
- `1` = Input

```cpp
// Set Port A pins 0-3 as outputs, pins 4-7 as inputs
mcp.setPortADirection(0xF0);

// Set all Port B pins as outputs
mcp.setPortBDirection(0x00);
```

### Reading and Writing

```cpp
// Write to Port B
mcp.writePortB(0x05);  // Set pins 0 and 2 high

// Read from Port A
uint8_t value;
mcp.readPortA(value);
```

### Pull-up Resistors

Enable internal pull-up resistors for input pins:

```cpp
// Enable pull-ups on Port A pins 4-7
mcp.setPullUpA(0xF0);
```

### Thread Safety

Use the `lock()` method for thread-safe access:

```cpp
auto &mcp = MCP23017::getInstance();
auto lock = mcp.lock(std::chrono::milliseconds(100));
if (lock) {
    // Safe to access MCP23017
    mcp.writePortB(0xFF);
} else {
    // Lock acquisition timed out
}
```

## API Reference

### `static MCP23017 &getInstance()`
Returns the singleton instance.

### `esp_err_t init()`
Initializes the MCP23017. Returns `ESP_OK` on success, `ESP_ERR_INVALID_STATE` if already initialized.

### `void reset()`
Performs a hardware reset using the configured reset GPIO.

### `esp_err_t setPortADirection(uint8_t direction_pin_mask)`
### `esp_err_t setPortBDirection(uint8_t direction_pin_mask)`
Sets the I/O direction for each pin. Bit = 0 for output, 1 for input.

### `esp_err_t writePortA(uint8_t value)`
### `esp_err_t writePortB(uint8_t value)`
Writes a value to the GPIO output latch.

### `esp_err_t readPortA(uint8_t &value)`
### `esp_err_t readPortB(uint8_t &value)`
Reads the current state of the GPIO pins.

### `esp_err_t setPullUpA(uint8_t pullup)`
### `esp_err_t setPullUpB(uint8_t pullup)`
Enables internal 100k pull-up resistors. Bit = 1 to enable pull-up.

### `std::optional<std::unique_lock<std::timed_mutex>> lock(std::chrono::milliseconds timeout)`
Acquires the mutex with a timeout. Returns `std::nullopt` if the lock could not be acquired.

### `std::timed_mutex &getMutex()`
Returns a reference to the internal mutex for advanced locking scenarios.

## Register Map

The component uses IOCON.BANK = 0 (default) register addressing:

| Register | Address | Description |
|----------|---------|-------------|
| IODIRA/B | 0x00/0x01 | I/O direction |
| IPOLA/B | 0x02/0x03 | Input polarity |
| GPINTENA/B | 0x04/0x05 | Interrupt-on-change enable |
| DEFVALA/B | 0x06/0x07 | Default compare value |
| INTCONA/B | 0x08/0x09 | Interrupt control |
| IOCON | 0x0A | Configuration |
| GPPUA/B | 0x0C/0x0D | Pull-up resistor |
| INTFA/B | 0x0E/0x0F | Interrupt flag |
| INTCAPA/B | 0x10/0x11 | Interrupt capture |
| GPIOA/B | 0x12/0x13 | GPIO port |
| OLATA/B | 0x14/0x15 | Output latch |

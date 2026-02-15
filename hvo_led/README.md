# Led Component

ESP-IDF component providing a simple C++ wrapper around GPIO output for driving LEDs.

## Dependencies

- `driver` (ESP-IDF GPIO driver)
- `espressif__esp-idf-cxx` (C++ GPIO wrappers)

## Usage

Add `hvo_led` to your `REQUIRES` in `CMakeLists.txt`:

```cmake
idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES hvo_led)
```

### Example

```cpp
#include "led.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Create an LED on GPIO 2
hvo::Led led(2);

// Turn on/off
led.turn_on();
led.turn_off();

// Check state
if (led.state() == hvo::LedState::on) {
    printf("LED is %s\n", led.state_str());
}

// Blink 3 times with 200ms interval
led.blink(200, 3);
```

## API

| Method | Description |
|---|---|
| `Led(uint32_t pin)` | Construct an LED on the given GPIO pin |
| `turn_on()` | Set GPIO high |
| `turn_off()` | Set GPIO low |
| `state()` | Returns `hvo::LedState::on` or `hvo::LedState::off` |
| `state_str()` | Returns `"ON"`, `"OFF"`, or `"UNKNOWN"` |
| `blink(duration_ms, count)` | Blink `count` times with `duration_ms` on/off interval (defaults: 500ms, 1 time) |

# BMP280 Component

ESP-IDF component for the BMP280/BME280 temperature and pressure sensor.

## Features

- Support for both BMP280 and BME280 sensors (auto-detected via chip ID)
- Configurable I2C address and clock speed via Kconfig
- Configurable temperature and pressure oversampling
- Configurable IIR filter coefficient
- Singleton pattern for easy global access
- Thread-safe with mutex protection

## Dependencies

- ESP-IDF >= 5.0.0
- [i2c](../i2c/) component (must be initialized before using this component)

## Kconfig Options

Configure via `idf.py menuconfig` under "BMP280 Configuration":

| Option | Default | Range | Description |
|--------|---------|-------|-------------|
| `BMP280_I2C_ADDRESS` | 0x76 | 0x76-0x77 | I2C address. 0x76 when SDO→GND, 0x77 when SDO→VCC |
| `BMP280_I2C_CLOCK_SPEED_HZ` | 100000 | 100000-400000 | I2C clock speed in Hz |
| `BMP280_TEMP_OVERSAMPLING` | x2 | skip, x1, x2, x4, x8, x16 | Temperature oversampling |
| `BMP280_PRESS_OVERSAMPLING` | x16 | skip, x1, x2, x4, x8, x16 | Pressure oversampling |
| `BMP280_IIR_FILTER` | Off | Off, 2, 4, 8, 16 | IIR filter coefficient |

### Oversampling Guide

Higher oversampling increases measurement accuracy but also increases measurement time and power consumption.

| Setting | Resolution | Typical Use Case |
|---------|------------|------------------|
| Skip | - | Disable measurement |
| x1 | 16 bit | Ultra-low power |
| x2 | 17 bit | Low power |
| x4 | 18 bit | Standard |
| x8 | 19 bit | High resolution |
| x16 | 20 bit | Ultra-high resolution |

### IIR Filter Guide

The IIR filter reduces noise from short-term fluctuations (e.g., door slamming, wind).

| Coefficient | Response Time | Use Case |
|-------------|---------------|----------|
| Off | Immediate | Fast response needed |
| 2 | ~2 samples | Light filtering |
| 4 | ~5 samples | Moderate filtering |
| 8 | ~11 samples | Strong filtering |
| 16 | ~22 samples | Maximum filtering |

## Usage

### Basic Example

```cpp
#include "i2c.hpp"
#include "bmp280.hpp"

void app_main()
{
    // Initialize I2C bus first
    auto &i2c = I2c::getInstance();
    i2c.init();

    // Initialize BMP280
    auto &bmp280 = Bmp280::getInstance();
    esp_err_t err = bmp280.init();
    if (err != ESP_OK) {
        ESP_LOGE("main", "Failed to initialize BMP280");
        return;
    }

    // Read temperature and pressure
    double temperature, pressure;
    err = bmp280.read(&temperature, &pressure);
    if (err == ESP_OK) {
        ESP_LOGI("main", "Temperature: %.2f C, Pressure: %.2f Pa",
                 temperature, pressure);
    }
}
```

### Reading Raw Values

```cpp
int32_t raw_temp, raw_press;
double temperature, pressure;

auto &bmp280 = Bmp280::getInstance();
esp_err_t err = bmp280.read_raw(&raw_temp, &raw_press);
if (err == ESP_OK) {
    bmp280.compensate_temp_press(raw_temp, raw_press, &temperature, &pressure);
    ESP_LOGI("main", "Temperature: %.2f C, Pressure: %.2f Pa",
             temperature, pressure);
}
```

## API Reference

### `Bmp280::getInstance()`

Returns the singleton instance of the BMP280 driver.

```cpp
static Bmp280 &getInstance();
```

### `init()`

Initializes the BMP280 sensor. Must be called after I2C bus initialization.

```cpp
esp_err_t init();
```

**Returns:**
- `ESP_OK` on success
- `ESP_FAIL` if sensor not found
- `ESP_ERR_INVALID_RESPONSE` if calibration data is invalid

### `read()`

Reads compensated temperature and pressure values.

```cpp
esp_err_t read(double *temperature, double *pressure);
```

**Parameters:**
- `temperature` - Pointer to store temperature in degrees Celsius
- `pressure` - Pointer to store pressure in Pascals

**Returns:**
- `ESP_OK` on success
- Error code on failure

### `read_raw()`

Reads raw ADC values from the sensor.

```cpp
esp_err_t read_raw(int32_t *raw_temp, int32_t *raw_press);
```

**Parameters:**
- `raw_temp` - Pointer to store raw temperature ADC value
- `raw_press` - Pointer to store raw pressure ADC value

**Returns:**
- `ESP_OK` on success
- `ESP_ERR_INVALID_RESPONSE` if values are invalid

### `compensate_temp_press()`

Applies calibration compensation to raw values.

```cpp
void compensate_temp_press(int32_t raw_temp, int32_t raw_press,
                           double *temperature, double *pressure);
```

**Parameters:**
- `raw_temp` - Raw temperature ADC value
- `raw_press` - Raw pressure ADC value
- `temperature` - Pointer to store compensated temperature (Celsius)
- `pressure` - Pointer to store compensated pressure (Pascals)

### `getMutex()`

Returns the mutex for thread-safe access.

```cpp
std::mutex &getMutex();
```

## Pressure Unit Conversion

The sensor returns pressure in Pascals (Pa). Common conversions:

| Unit | Conversion |
|------|------------|
| hPa (hectopascal) | Pa / 100 |
| mbar (millibar) | Pa / 100 |
| mmHg | Pa / 133.322 |
| inHg | Pa / 3386.39 |
| atm | Pa / 101325 |

## License

MIT

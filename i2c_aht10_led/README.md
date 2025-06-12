# AHT10 Temperature & Humidity Monitor with LED Control

## Project Overview

This project implements a temperature and humidity monitoring system using the AHT10 sensor on an STM32F411CEU6 BlackPill development board. The system provides visual feedback through RGB LEDs and outputs data via UART to a connected USB-TTL converter for external monitoring.

## Features

- **Real-time environmental monitoring** using AHT10 I2C sensor
- **Visual status indication** with RGB LEDs based on environmental thresholds
- **UART data output** in both JSON and human-readable formats
- **I2C bus scanning** for device detection and troubleshooting
- **Automatic sensor initialization** with calibration verification
- **Error handling** with status reporting
- **Configurable thresholds** for temperature and humidity alerts

## Hardware Requirements

### Components
- STM32F411CEU6 BlackPill development board
- AHT10 Temperature & Humidity sensor
- 3x LEDs (Red, Green, Blue) with current-limiting resistors (220Ω recommended)
- USB-TTL converter (CP2102, FT232RL, or similar)
- Breadboard and jumper wires
- 4.7kΩ pull-up resistors for I2C lines (if not present on sensor breakout)

### Pin Connections

#### AHT10 Sensor (I2C1)
| AHT10 Pin | BlackPill Pin | Description |
|-----------|---------------|-------------|
| VCC       | 3.3V          | Power supply |
| GND       | GND           | Ground |
| SDA       | PB7           | I2C Data line |
| SCL       | PB6           | I2C Clock line |

#### LED Connections
| LED Color | BlackPill Pin | Function |
|-----------|---------------|----------|
| Red       | PC13          | High temperature indicator (>25°C) |
| Green     | PA1           | High humidity indicator (>60%) |
| Blue      | PA0           | Low temperature indicator (<20°C) |

#### UART Connection (USB-TTL Converter)
| USB-TTL Pin | BlackPill Pin | Description |
|-------------|---------------|-------------|
| TX          | PA10 (RX1)    | Receive data from BlackPill |
| RX          | PA9 (TX1)     | Transmit data to BlackPill |
| GND         | GND           | Common ground |
| VCC         | 3.3V or 5V    | Power (optional, can power from USB) |

## Software Architecture

### Project Structure
```
i2c_aht10_led/
├── src/
│   └── main.c                 # Main application code
├── CMakeLists.txt            # Build configuration
├── prj.conf                  # Project configuration
├── blackpill_f411ce.overlay  # Device tree overlay
└── README.md                 # Project documentation
```

### Key Functions

#### Sensor Management
- `aht10_init()` - Initialize and calibrate the AHT10 sensor
- `aht10_read_data()` - Read temperature and humidity values
- `scan_i2c_bus()` - Scan I2C bus for connected devices

#### Hardware Control
- `init_leds()` - Configure GPIO pins for LED control
- `control_leds()` - Update LED states based on sensor readings
- `send_uart_data()` - Transmit sensor data via UART

### Operating Thresholds

| Parameter | Threshold | LED Indicator | Action |
|-----------|-----------|---------------|---------|
| Temperature | < 20°C | Blue LED ON | Low temperature alert |
| Temperature | > 25°C | Red LED ON | High temperature alert |
| Humidity | > 60% | Green LED ON | High humidity alert |
| Normal conditions | 20-25°C, <60% RH | All LEDs OFF | Normal operation |

## Data Output Formats

### UART Output (115200 baud, 8N1)

#### JSON Format
```json
{"temperature":23.45,"humidity":55.20,"timestamp":12345}
```

#### Human-Readable Format
```
TEMP: 23.45°C, HUMID: 55.20%, TIME: 12345ms
```

#### Console Debug Output
```
Temperature: 23.45°C
Humidity: 55.20%
LED Status: All OFF (Normal conditions)
------------------------
```

## Building and Flashing

### Prerequisites
- Zephyr RTOS development environment
- West build tool
- ARM GCC toolchain
- STM32 flash programming tool (st-flash, OpenOCD, or ST-Link Utility)

### Build Commands
```bash
# Navigate to project directory
cd i2c_aht10_led

# Build the project
west build -b blackpill_f411ce

# Flash to device
west flash
```

### Alternative Build Methods
```bash
# Clean build
west build -b blackpill_f411ce --pristine

# Build with specific configuration
west build -b blackpill_f411ce -- -DCONFIG_LOG_DEFAULT_LEVEL=4
```

## Configuration Options

### Modifiable Parameters in `main.c`
```c
// Temperature thresholds (°C)
#define TEMP_LOW_THRESHOLD      20.0f
#define TEMP_HIGH_THRESHOLD     25.0f

// Humidity threshold (%)
#define HUMIDITY_HIGH_THRESHOLD 60.0f

// Measurement interval (seconds)
#define MEASUREMENT_INTERVAL    2
```

### Project Configuration (`prj.conf`)
- I2C support: `CONFIG_I2C=y`
- GPIO support: `CONFIG_GPIO=y`
- UART console: `CONFIG_UART_CONSOLE=y`
- Floating-point printf: `CONFIG_NEWLIB_LIBC_FLOAT_PRINTF=y`

## Troubleshooting

### Common Issues and Solutions

#### 1. AHT10 Not Detected
**Symptoms:** I2C scan doesn't show device at 0x38
**Solutions:**
- Check wiring connections (SDA/SCL)
- Verify 3.3V power supply
- Add 4.7kΩ pull-up resistors to I2C lines
- Check sensor orientation and pin mapping

#### 2. LEDs Not Working
**Symptoms:** LEDs don't respond to sensor readings
**Solutions:**
- Verify LED polarity (anode to GPIO pin, cathode to GND)
- Check current-limiting resistors (220Ω recommended)
- Confirm GPIO pin assignments in device tree overlay
- Test LEDs with multimeter

#### 3. UART Output Issues
**Symptoms:** No data received on serial terminal
**Solutions:**
- Verify baud rate (115200)
- Check TX/RX wiring (cross-connected)
- Ensure common ground connection
- Test USB-TTL converter with loopback

#### 4. Build Errors
**Symptoms:** Compilation or linking failures
**Solutions:**
- Update Zephyr SDK to latest version
- Check device tree overlay syntax
- Verify project configuration options
- Clean and rebuild project

### Debug Commands
```bash
# Monitor I2C traffic
west build -t menuconfig  # Enable I2C debug logs

# View real-time logs
west attach

# Check device tree compilation
west build -t devicetree_files
```

## Performance Characteristics

### Timing Specifications
- **Sensor read cycle:** ~100ms (including I2C communication)
- **Measurement interval:** 2 seconds (configurable)
- **LED response time:** <10ms
- **UART transmission:** ~1ms per message

### Accuracy
- **Temperature:** ±0.3°C (typical)
- **Humidity:** ±2% RH (typical)
- **Resolution:** 0.01°C, 0.01% RH

### Power Consumption
- **Active mode:** ~20mA (estimated, including LEDs)
- **Standby:** <1mA (LEDs off)

## Extending the Project

### Possible Enhancements
1. **Data logging** to SD card or external memory
2. **Wireless connectivity** (WiFi, Bluetooth, LoRa)
3. **Web interface** for remote monitoring
4. **Alarm system** with buzzer or email notifications
5. **Multiple sensor support** for environmental mapping
6. **LCD display** for local data visualization
7. **Battery operation** with low-power modes

### Code Modification Examples

#### Adding a Buzzer Alert
```c
// Add to threshold checking
if (temperature > TEMP_CRITICAL_THRESHOLD) {
    gpio_pin_set_dt(&buzzer_pin, 1);
    k_msleep(100);
    gpio_pin_set_dt(&buzzer_pin, 0);
}
```

#### Implementing Data Averaging
```c
#define SAMPLE_COUNT 5
static float temp_samples[SAMPLE_COUNT];
static float humid_samples[SAMPLE_COUNT];
static int sample_index = 0;

// Calculate moving average for smoother readings
```

## License and Credits

This project is based on the Zephyr RTOS framework and is provided as educational material. The AHT10 sensor interface follows the manufacturer's specifications.

### References
- [Zephyr RTOS Documentation](https://docs.zephyrproject.org/)
- [STM32F411CEU6 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0383-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [AHT10 Datasheet](https://server4.eca.ir/eshop/AHT10/Aosong_AHT10_en_draft_0c.pdf)
- [BlackPill Development Board Guide](https://github.com/WeActTC/MiniSTM32F4x1)

## Support and Contributing

For issues, suggestions, or contributions:
1. Check existing documentation and troubleshooting section
2. Verify hardware connections and configuration
3. Review Zephyr RTOS documentation for platform-specific issues
4. Consider contributing improvements or bug fixes

---

**Last Updated:** June 2025  
**Version:** 1.0  
**Compatible Zephyr Version:** 3.6+

# AHT10 Temperature & Humidity Sensor with Zephyr RTOS

A complete example for reading temperature and humidity data from an AHT10 sensor using Zephyr RTOS on the STM32F411CEU6 BlackPill board.

## Overview

This project demonstrates how to:
- Initialize and communicate with an AHT10 temperature/humidity sensor via I2C
- Read calibrated temperature and humidity values
- Display sensor data via UART serial console
- Handle sensor initialization, timing, and error conditions
- Debug I2C communication with detailed logging

## Hardware Requirements

### Main Components
- **STM32F411CEU6 BlackPill** (WeAct Studio version)
- **AHT10 Temperature & Humidity Sensor**
- **USB-to-TTL Serial Converter** (for console output)
- **Breadboard and jumper wires**

### Wiring Diagram

```
STM32F411CEU6 BlackPill    AHT10 Sensor
=======================    ============
3.3V                   --> VCC
GND                    --> GND
PB6 (I2C1_SCL)        --> SCL
PB7 (I2C1_SDA)        --> SDA

Serial Console Connection:
=========================
PA9 (USART1_TX)        --> RX of USB-to-TTL converter
PA10 (USART1_RX)       --> TX of USB-to-TTL converter
GND                    --> GND of USB-to-TTL converter
```

### Pin Configuration
- **I2C1 SCL**: PB6
- **I2C1 SDA**: PB7
- **UART1 TX**: PA9 (for serial console output)
- **UART1 RX**: PA10 (for serial console input)

## Software Requirements

- **Zephyr SDK** (v0.16.0 or later)
- **West Build Tool**
- **Visual Studio Code** with Zephyr extension (recommended)
- **Serial Terminal** (PuTTY, minicom, or VS Code terminal)

## Project Structure

```
aht10_example/
├── CMakeLists.txt          # Build configuration
├── prj.conf               # Project configuration
├── src/
│   └── main.c             # Main application code
└── README.md              # This documentation
```

## Configuration Files

### CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aht10_reader)
target_sources(app PRIVATE src/main.c)
```

### prj.conf
```ini
# Enable I2C driver
CONFIG_I2C=y

# Enable UART console for output
CONFIG_UART_CONSOLE=y
CONFIG_CONSOLE=y
CONFIG_SERIAL=y
CONFIG_PRINTK=y

# Enable logging
CONFIG_LOG=y
CONFIG_LOG_DEFAULT_LEVEL=3
```

## How It Works

### 1. AHT10 Sensor Protocol

The AHT10 uses a specific I2C communication protocol:

**I2C Address**: `0x38`

**Initialization Sequence**:
1. Send soft reset command (`0xBA`)
2. Wait 20ms for reset completion
3. Send initialization command (`0xE1 0x08 0x00`)
4. Check calibration status

**Reading Data**:
1. Send measurement trigger (`0xAC 0x33 0x00`)
2. Wait ~80ms for measurement completion
3. Check busy status until ready
4. Read 6 bytes of measurement data
5. Extract and convert temperature/humidity values

### 2. Data Format

The AHT10 returns 6 bytes of data:
```
[Status][Humidity High][Humidity Mid][Humidity Low + Temp High][Temp Mid][Temp Low]
```

**Temperature Calculation**:
```c
raw_temp = ((data[3] & 0x0F) << 16) | (data[4] << 8) | data[5];
temperature = (raw_temp / 1048576.0) * 200.0 - 50.0;  // Range: -40°C to +85°C
```

**Humidity Calculation**:
```c
raw_humidity = (data[1] << 12) | (data[2] << 4) | (data[3] >> 4);
humidity = (raw_humidity / 1048576.0) * 100.0;  // Range: 0% to 100%
```

### 3. Program Flow

1. **System Initialization**
   - Initialize I2C peripheral
   - Verify I2C device is ready
   - Perform I2C bus scan to detect AHT10

2. **Sensor Setup**
   - Send soft reset to AHT10
   - Initialize sensor with calibration
   - Verify sensor is calibrated and ready

3. **Main Loop**
   - Trigger temperature/humidity measurement
   - Wait for measurement completion
   - Read and parse sensor data
   - Display formatted results
   - Wait 2 seconds before next reading

## Building and Flashing

### Prerequisites Setup
```bash
# Set up Zephyr environment (one-time setup)
west init ~/zephyrproject
cd ~/zephyrproject
west update
west zephyr-export
```

### Build Commands
```bash
# Navigate to project directory
cd aht10_example

# Build for STM32F411CEU6 BlackPill
west build -b blackpill_f411ce

# Flash to board
west flash

# Monitor serial output (adjust port as needed)
minicom -D /dev/ttyUSB0 -b 115200
# OR
screen /dev/ttyUSB0 115200
```

### Visual Studio Code
1. Open project folder in VS Code
2. Install Zephyr extension
3. Use Command Palette: "Zephyr: Build"
4. Use Command Palette: "Zephyr: Flash"
5. Open integrated terminal for serial monitoring

## Expected Output

### Successful Operation
```
*** Booting Zephyr OS build v4.1.0 ***
STM32F411CEU6 BlackPill AHT10 Temperature & Humidity Reader
==========================================================
I2C device ready
Starting I2C bus scan...
Scanning addresses 0x08 to 0x77
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- 38 -- -- -- -- -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --

Scan complete. Found 1 device(s).

Initializing AHT10 sensor...
AHT10 initialized successfully (status: 0x18)
Starting temperature and humidity readings...
============================================

Raw data: 1C 6B 2A 8C 51 33
Raw humidity: 437802, Raw temperature: 573747
Temperature: 23.45°C
Humidity: 65.23%
------------------------
Temperature: 23.67°C
Humidity: 64.89%
------------------------
```

## Troubleshooting

### Common Issues

**1. "I2C device not ready"**
- Check I2C pin connections (PB6, PB7)
- Verify 3.3V power supply to sensor
- Ensure proper pull-up resistors on I2C lines (usually built into sensor boards)

**2. "AHT10 not found in I2C scan"**
- Double-check wiring connections
- Verify sensor power (3.3V, not 5V)
- Try different I2C address (some AHT10 variants use 0x39)
- Check for loose connections

**3. "AHT10 not calibrated"**
- Try power cycling the sensor
- Increase initialization delay
- Check sensor datasheet for specific initialization requirements

**4. "No serial output"**
- Verify UART connections (PA9 to RX, PA10 to TX of USB-TTL)
- Check serial terminal settings (115200 baud, 8N1)
- Ensure USB-TTL converter drivers are installed

**5. "Temperature/Humidity values seem wrong"**
- Check raw data output for patterns
- Verify sensor isn't damaged by heat/moisture
- Compare with known temperature/humidity source
- Ensure adequate measurement timing

### Debug Features

The code includes several debugging features:

- **I2C Bus Scan**: Confirms sensor detection
- **Raw Data Display**: Shows actual bytes received
- **Status Checking**: Verifies sensor calibration and measurement completion
- **Error Codes**: Detailed error reporting for troubleshooting

## Specifications

### AHT10 Sensor Specs
- **Temperature Range**: -40°C to +85°C
- **Temperature Accuracy**: ±0.3°C (typical)
- **Humidity Range**: 0% to 100% RH
- **Humidity Accuracy**: ±2% RH (typical)
- **Operating Voltage**: 2.0V to 5.5V
- **I2C Interface**: Standard/Fast mode
- **Measurement Time**: ~75ms

### STM32F411CEU6 BlackPill
- **MCU**: ARM Cortex-M4 @ 100MHz
- **Flash**: 512KB
- **RAM**: 128KB
- **I2C Interfaces**: 3 (I2C1, I2C2, I2C3)
- **Operating Voltage**: 3.3V logic

## Code Explanation

### Key Functions

**`aht10_init()`**
- Performs sensor reset and initialization
- Verifies sensor calibration status
- Returns 0 on success, negative on error

**`aht10_read_data()`**
- Triggers measurement and reads data
- Extracts raw values from 6-byte response
- Converts to temperature (°C) and humidity (%)
- Returns 0 on success, negative on error

**`scan_i2c_bus()`**
- Scans I2C addresses 0x08-0x77
- Useful for detecting connected devices
- Inherited from I2C scanner example

### Error Handling
- All I2C operations include return code checking
- Sensor status verification before data reading
- Detailed error messages for troubleshooting
- Graceful degradation on communication failures

## Extensions and Modifications

### Easy Modifications

**1. Change Reading Frequency**
```c
// In main loop, change from 2 seconds to 5 seconds
k_sleep(K_SECONDS(5));
```

**2. Add Temperature Limits/Alerts**
```c
if (temperature > 30.0) {
    printk("WARNING: High temperature detected!\n");
}
```

**3. Different I2C Interface**
```c
// Use I2C2 instead of I2C1
i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
```

**4. Celsius to Fahrenheit Conversion**
```c
float temp_fahrenheit = (temperature * 9.0/5.0) + 32.0;
```

### Advanced Extensions

- **Data Logging**: Store readings to flash memory
- **WiFi Integration**: Send data to IoT platforms
- **Display Integration**: Show data on LCD/OLED
- **Multiple Sensors**: Read from sensor arrays
- **Alarm System**: Trigger actions based on thresholds

## Related Examples

This example builds upon and connects to:

1. **I2C Scanner** - Device detection and I2C setup
2. **GPIO Control** - Adding LED indicators
3. **UART Communication** - Enhanced serial protocols
4. **Multiple Sensors** - Integrating additional I2C devices
5. **Data Logging** - Storing sensor readings

## References

- [AHT10 Datasheet](http://www.aosong.com/userfiles/files/media/AHT10%20humidity%20and%20temperature%20sensor.pdf)
- [Zephyr I2C API Documentation](https://docs.zephyrproject.org/latest/hardware/peripherals/i2c.html)
- [STM32F411 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00119316-stm32f411xce-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [Zephyr Device Tree Guide](https://docs.zephyrproject.org/latest/build/dts/index.html)

## Contributing

Found an issue or have an improvement? Please:
1. Check existing issues in the repository
2. Create detailed bug reports with serial output
3. Submit pull requests with clear descriptions
4. Share your modifications and extensions

## License

This example is provided under the Apache 2.0 License - see the LICENSE file for details.

---

**Next in Series**: GPIO Control and LED Patterns - Learn digital I/O basics with visual feedback

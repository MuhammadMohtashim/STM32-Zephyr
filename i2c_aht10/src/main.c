// main.c - AHT10 Temperature & Humidity Reader for STM32F411CEU6 BlackPill
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(aht10_reader, LOG_LEVEL_INF);

// AHT10 I2C address
#define AHT10_I2C_ADDR 0x38

// AHT10 Commands
#define AHT10_CMD_INIT          0xE1
#define AHT10_CMD_TRIGGER       0xAC
#define AHT10_CMD_SOFT_RESET    0xBA
#define AHT10_STATUS_BUSY       0x80
#define AHT10_STATUS_CALIBRATED 0x08

// I2C device pointer
static const struct device *i2c_dev;

// Function to initialize AHT10 sensor
int aht10_init(void)
{
    uint8_t init_cmd[3] = {AHT10_CMD_INIT, 0x08, 0x00};
    uint8_t status;
    int ret;
    
    printk("Initializing AHT10 sensor...\n");
    
    // Send soft reset command first
    uint8_t reset_cmd = AHT10_CMD_SOFT_RESET;
    ret = i2c_write(i2c_dev, &reset_cmd, 1, AHT10_I2C_ADDR);
    if (ret != 0) {
        printk("ERROR: Failed to send reset command to AHT10\n");
        return ret;
    }
    
    // Wait for reset to complete
    k_msleep(20);
    
    // Send initialization command
    ret = i2c_write(i2c_dev, init_cmd, 3, AHT10_I2C_ADDR);
    if (ret != 0) {
        printk("ERROR: Failed to initialize AHT10\n");
        return ret;
    }
    
    // Wait for initialization
    k_msleep(10);
    
    // Check if sensor is calibrated
    ret = i2c_read(i2c_dev, &status, 1, AHT10_I2C_ADDR);
    if (ret != 0) {
        printk("ERROR: Failed to read AHT10 status\n");
        return ret;
    }
    
    if (!(status & AHT10_STATUS_CALIBRATED)) {
        printk("ERROR: AHT10 not calibrated (status: 0x%02X)\n", status);
        return -1;
    }
    
    printk("AHT10 initialized successfully (status: 0x%02X)\n", status);
    return 0;
}

// Function to read temperature and humidity from AHT10
int aht10_read_data(float *temperature, float *humidity)
{
    uint8_t trigger_cmd[3] = {AHT10_CMD_TRIGGER, 0x33, 0x00};
    uint8_t data[6];
    uint32_t raw_humidity, raw_temperature;
    int ret;
    
    // Send measurement trigger command
    ret = i2c_write(i2c_dev, trigger_cmd, 3, AHT10_I2C_ADDR);
    if (ret != 0) {
        printk("ERROR: Failed to trigger AHT10 measurement\n");
        return ret;
    }
    
    // Wait for measurement to complete (typical 75ms)
    k_msleep(80);
    
    // Check if measurement is ready
    do {
        ret = i2c_read(i2c_dev, data, 1, AHT10_I2C_ADDR);
        if (ret != 0) {
            printk("ERROR: Failed to read AHT10 status\n");
            return ret;
        }
        
        if (data[0] & AHT10_STATUS_BUSY) {
            k_msleep(10);  // Wait a bit more if still busy
        }
    } while (data[0] & AHT10_STATUS_BUSY);
    
    // Read the full 6-byte data
    ret = i2c_read(i2c_dev, data, 6, AHT10_I2C_ADDR);
    if (ret != 0) {
        printk("ERROR: Failed to read AHT10 data\n");
        return ret;
    }
    
    // Debug: Print raw data bytes
    printk("Raw data: %02X %02X %02X %02X %02X %02X\n", 
           data[0], data[1], data[2], data[3], data[4], data[5]);
    
    // Extract raw humidity (20-bit, bits 19:0 of data[1:3])
    raw_humidity = ((uint32_t)data[1] << 12) | 
                   ((uint32_t)data[2] << 4) | 
                   ((uint32_t)data[3] >> 4);
    
    // Extract raw temperature (20-bit, bits 19:0 of data[3:5])
    raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) | 
                      ((uint32_t)data[4] << 8) | 
                      (uint32_t)data[5];
    
    // Debug: Print raw values
    printk("Raw humidity: %u, Raw temperature: %u\n", raw_humidity, raw_temperature);
    
    // Convert to actual values
    *humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;
    *temperature = (((float)raw_temperature / 1048576.0f) * 200.0f) - 50.0f;
    
    return 0;
}

// Function to scan I2C bus (from your original code)
void scan_i2c_bus(void)
{
    uint8_t dummy_data = 0;
    int ret;
    int devices_found = 0;
    
    printk("Starting I2C bus scan...\n");
    printk("Scanning addresses 0x08 to 0x77\n");
    printk("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    
    for (int addr = 0; addr < 128; addr++) {
        // Print address row header
        if (addr % 16 == 0) {
            printk("%02x: ", addr);
        }
        
        // Skip reserved addresses
        if (addr < 0x08 || addr > 0x77) {
            printk("   ");
        } else {
            // Try to communicate with device at this address
            ret = i2c_write(i2c_dev, &dummy_data, 0, addr);
            
            if (ret == 0) {
                // Device found
                printk("%02x ", addr);
                devices_found++;
            } else {
                // No device at this address  
                printk("-- ");
            }
        }
        
        // Print newline at end of row
        if ((addr + 1) % 16 == 0) {
            printk("\n");
        }
        
        // Small delay between scans
        k_msleep(1);
    }
    
    printk("\nScan complete. Found %d device(s).\n\n", devices_found);
}

int main(void)
{
    float temperature, humidity;
    int ret;
    
    printk("STM32F411CEU6 BlackPill AHT10 Temperature & Humidity Reader\n");
    printk("==========================================================\n");
    
    // Get I2C device
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (!device_is_ready(i2c_dev)) {
        printk("ERROR: I2C device not ready\n");
        return -1;
    }
    
    printk("I2C device ready\n");
    
    // Scan I2C bus first to verify AHT10 is detected
    scan_i2c_bus();
    
    // Initialize AHT10 sensor
    ret = aht10_init();
    if (ret != 0) {
        printk("ERROR: Failed to initialize AHT10 sensor\n");
        return -1;
    }
    
    printk("Starting temperature and humidity readings...\n");
    printk("============================================\n\n");
    
    while (1) {
        // Read temperature and humidity
        ret = aht10_read_data(&temperature, &humidity);
        
        if (ret == 0) {
            // Convert to integers for display (multiply by 100 to keep 2 decimal places)
            int temp_int = (int)(temperature * 100);
            int humid_int = (int)(humidity * 100);
            
            printk("Temperature: %d.%02d°C\n", temp_int / 100, temp_int % 100);
            printk("Humidity: %d.%02d%%\n", humid_int / 100, humid_int % 100);
            printk("Raw temp: %d, Raw humid: %d\n", temp_int, humid_int);
            printk("------------------------\n");
        } else {
            printk("ERROR: Failed to read AHT10 data (error: %d)\n", ret);
        }
        
        // Wait 2 seconds before next reading
        k_sleep(K_SECONDS(2));
    }
    
    return 0;
}
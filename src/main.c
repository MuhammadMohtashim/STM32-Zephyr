// main.c - I2C Scanner for STM32F411CEU6 BlackPill
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(i2c_scanner, LOG_LEVEL_INF);

// I2C device pointer
static const struct device *i2c_dev;

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
    printk("STM32F411CEU6 BlackPill I2C Scanner\n");
    printk("===================================\n");
    
    // Get I2C device
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (!device_is_ready(i2c_dev)) {
        printk("ERROR: I2C device not ready\n");
        return -1;
    }
    
    printk("I2C device ready\n");
    
    while (1) {
        scan_i2c_bus();
        
        // Wait 5 seconds before next scan
        printk("Waiting 5 seconds before next scan...\n");
        k_sleep(K_SECONDS(5));
    }
    
    return 0;
}
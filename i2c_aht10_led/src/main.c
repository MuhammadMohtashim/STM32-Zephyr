// main.c - AHT10 Temperature & Humidity Reader with LED Control and UART Output
// for STM32F411CEU6 BlackPill
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <stdio.h>
#include <string.h>

LOG_MODULE_REGISTER(aht10_reader, LOG_LEVEL_INF);

// AHT10 I2C address
#define AHT10_I2C_ADDR 0x38

// AHT10 Commands
#define AHT10_CMD_INIT          0xE1
#define AHT10_CMD_TRIGGER       0xAC
#define AHT10_CMD_SOFT_RESET    0xBA
#define AHT10_STATUS_BUSY       0x80
#define AHT10_STATUS_CALIBRATED 0x08

// Temperature and humidity thresholds for LED control
#define TEMP_LOW_THRESHOLD      20.0f   // Below this: Blue LED
#define TEMP_HIGH_THRESHOLD     25.0f   // Above this: Red LED
#define HUMIDITY_HIGH_THRESHOLD 60.0f   // Above this: Green LED

// GPIO LED definitions (adjust pins according to your setup)
#define LED_RED_NODE    DT_ALIAS(led0)      // Red LED - High temperature
#define LED_GREEN_NODE  DT_ALIAS(led1)      // Green LED - High humidity  
#define LED_BLUE_NODE   DT_ALIAS(led2)      // Blue LED - Low temperature

// Device pointers
static const struct device *i2c_dev;
static const struct device *uart_dev;

// GPIO LED specifications
static struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(LED_RED_NODE, gpios);
static struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(LED_GREEN_NODE, gpios);
static struct gpio_dt_spec blue_led = GPIO_DT_SPEC_GET(LED_BLUE_NODE, gpios);

// Function to initialize GPIO LEDs
int init_leds(void)
{
    int ret;
    
    printk("Initializing LEDs...\n");
    
    // Configure red LED
    if (!gpio_is_ready_dt(&red_led)) {
        printk("ERROR: Red LED GPIO device not ready\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Failed to configure red LED GPIO\n");
        return ret;
    }
    
    // Configure green LED
    if (!gpio_is_ready_dt(&green_led)) {
        printk("ERROR: Green LED GPIO device not ready\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Failed to configure green LED GPIO\n");
        return ret;
    }
    
    // Configure blue LED
    if (!gpio_is_ready_dt(&blue_led)) {
        printk("ERROR: Blue LED GPIO device not ready\n");
        return -1;
    }
    ret = gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("ERROR: Failed to configure blue LED GPIO\n");
        return ret;
    }
    
    printk("LEDs initialized successfully\n");
    return 0;
}

// Function to control LEDs based on temperature and humidity
void control_leds(float temperature, float humidity)
{
    // Turn off all LEDs first
    gpio_pin_set_dt(&red_led, 0);
    gpio_pin_set_dt(&green_led, 0);
    gpio_pin_set_dt(&blue_led, 0);
    
    // Control LEDs based on sensor readings
    if (temperature > TEMP_HIGH_THRESHOLD) {
        gpio_pin_set_dt(&red_led, 1);  // Red LED for high temperature
        printk("LED Status: RED (High Temperature)\n");
    } else if (temperature < TEMP_LOW_THRESHOLD) {
        gpio_pin_set_dt(&blue_led, 1); // Blue LED for low temperature
        printk("LED Status: BLUE (Low Temperature)\n");
    }
    
    if (humidity > HUMIDITY_HIGH_THRESHOLD) {
        gpio_pin_set_dt(&green_led, 1); // Green LED for high humidity
        printk("LED Status: GREEN (High Humidity)\n");
    }
    
    // If no conditions met, all LEDs remain off
    if (temperature >= TEMP_LOW_THRESHOLD && temperature <= TEMP_HIGH_THRESHOLD && 
        humidity <= HUMIDITY_HIGH_THRESHOLD) {
        printk("LED Status: All OFF (Normal conditions)\n");
    }
}

// Function to send data via UART
void send_uart_data(float temperature, float humidity)
{
    char uart_buffer[128];
    int len;
    
    // Format the data as JSON for easier parsing
    len = snprintf(uart_buffer, sizeof(uart_buffer),
                   "{\"temperature\":%.2f,\"humidity\":%.2f,\"timestamp\":%lld}\r\n",
                   temperature, humidity, k_uptime_get());
    
    // Send via UART
    for (int i = 0; i < len; i++) {
        uart_poll_out(uart_dev, uart_buffer[i]);
    }
    
    // Also send human-readable format
    len = snprintf(uart_buffer, sizeof(uart_buffer),
                   "TEMP: %.2f°C, HUMID: %.2f%%, TIME: %lldms\r\n",
                   temperature, humidity, k_uptime_get());
    
    for (int i = 0; i < len; i++) {
        uart_poll_out(uart_dev, uart_buffer[i]);
    }
}

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
    
    // Extract raw humidity (20-bit, bits 19:0 of data[1:3])
    raw_humidity = ((uint32_t)data[1] << 12) | 
                   ((uint32_t)data[2] << 4) | 
                   ((uint32_t)data[3] >> 4);
    
    // Extract raw temperature (20-bit, bits 19:0 of data[3:5])
    raw_temperature = (((uint32_t)data[3] & 0x0F) << 16) | 
                      ((uint32_t)data[4] << 8) | 
                      (uint32_t)data[5];
    
    // Convert to actual values
    *humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;
    *temperature = (((float)raw_temperature / 1048576.0f) * 200.0f) - 50.0f;
    
    return 0;
}

// Function to scan I2C bus
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
    
    printk("STM32F411CEU6 BlackPill AHT10 with LED Control & UART Output\n");
    printk("============================================================\n");
    
    // Get I2C device
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (!device_is_ready(i2c_dev)) {
        printk("ERROR: I2C device not ready\n");
        return -1;
    }
    printk("I2C device ready\n");
    
    // Get UART device
    uart_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
    if (!device_is_ready(uart_dev)) {
        printk("ERROR: UART device not ready\n");
        return -1;
    }
    printk("UART device ready\n");
    
    // Initialize LEDs
    ret = init_leds();
    if (ret != 0) {
        printk("ERROR: Failed to initialize LEDs\n");
        return -1;
    }
    
    // Test LEDs briefly
    printk("Testing LEDs...\n");
    gpio_pin_set_dt(&red_led, 1);
    k_msleep(500);
    gpio_pin_set_dt(&red_led, 0);
    gpio_pin_set_dt(&green_led, 1);
    k_msleep(500);
    gpio_pin_set_dt(&green_led, 0);
    gpio_pin_set_dt(&blue_led, 1);
    k_msleep(500);
    gpio_pin_set_dt(&blue_led, 0);
    printk("LED test complete\n");
    
    // Scan I2C bus first to verify AHT10 is detected
    scan_i2c_bus();
    
    // Initialize AHT10 sensor
    ret = aht10_init();
    if (ret != 0) {
        printk("ERROR: Failed to initialize AHT10 sensor\n");
        return -1;
    }
    
    // Send startup message via UART
    char startup_msg[] = "AHT10 Temperature & Humidity Monitor Started\r\n";
    for (int i = 0; i < strlen(startup_msg); i++) {
        uart_poll_out(uart_dev, startup_msg[i]);
    }
    
    printk("Starting temperature and humidity readings...\n");
    printk("Thresholds: Low Temp: %.1f°C, High Temp: %.1f°C, High Humidity: %.1f%%\n",
           TEMP_LOW_THRESHOLD, TEMP_HIGH_THRESHOLD, HUMIDITY_HIGH_THRESHOLD);
    printk("============================================\n\n");
    
    while (1) {
        // Read temperature and humidity
        ret = aht10_read_data(&temperature, &humidity);
        
        if (ret == 0) {
            // Display on console
            printk("Temperature: %.2f°C\n", temperature);
            printk("Humidity: %.2f%%\n", humidity);
            
            // Control LEDs based on readings
            control_leds(temperature, humidity);
            
            // Send data via UART
            send_uart_data(temperature, humidity);
            
            printk("------------------------\n");
        } else {
            printk("ERROR: Failed to read AHT10 data (error: %d)\n", ret);
            
            // Send error via UART
            char error_msg[] = "ERROR: Sensor read failed\r\n";
            for (int i = 0; i < strlen(error_msg); i++) {
                uart_poll_out(uart_dev, error_msg[i]);
            }
        }
        
        // Wait 2 seconds before next reading
        k_sleep(K_SECONDS(2));
    }
    
    return 0;
}
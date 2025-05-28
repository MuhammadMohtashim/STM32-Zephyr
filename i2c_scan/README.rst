# STM32F411 BlackPill I2C Scanner - Your First Zephyr Adventure!

Welcome to the exciting world of Zephyr RTOS! This project is your friendly introduction to building embedded applications with Zephyr on the popular STM32F411CEU6 BlackPill board. Don't worry if you're new to Zephyr - we'll walk through everything step by step!

*Note: We assume you have some basic programming experience (that's how you found your way to Zephyr, right?) and have read through the Zephyr getting started guide. If not, no worries - just take a quick detour there first!*

## What Does This Cool Project Do?

Imagine you're a detective, but instead of solving mysteries, you're hunting for I2C devices! This I2C scanner is like having X-ray vision for your electronics - it systematically checks every possible I2C address (from 0x08 to 0x77) and tells you exactly which devices are connected to your board. It's like playing "Marco Polo" with electronic components!

Perfect for those "Is my sensor actually connected?" moments we've all had!

## What You'll Need (The Shopping List!)

- **STM32F411CEU6 BlackPill** - Your trusty microcontroller (the star of the show!)
- **TTL-to-USB converter** - Think of this as your translator between the BlackPill and your computer
- **Some I2C devices** to test (sensors, displays, anything I2C!) - Optional but more fun with them!
- **Breadboard and jumper wires** - For making those important connections
- **A computer with Visual Studio Code** - Your command center!

*Why do we need that TTL-to-USB converter?* Great question! Unlike some fancy dev boards, our BlackPill doesn't have a built-in USB-to-serial converter. Think of the TTL-to-USB converter as a helpful interpreter that lets your computer understand what the BlackPill is saying!

## Setting Up Your Development Environment (The Fun Begins!)

### Step 1: Create Your Project in Visual Studio Code
1. **Open Visual Studio Code** (your new best friend!)
2. **Create a new folder** for your project - let's call it `blackpill-i2c-scanner` (or whatever makes you happy!)
3. **Open this folder** in VS Code (File → Open Folder)

### Step 2: Import Into Zephyr Extension
1. **Install the Zephyr extension** if you haven't already (it's a game-changer!)
2. **Press Ctrl+Shift+P** (or Cmd+Shift+P on Mac) to open the command palette
3. **Type "Zephyr: Import Project"** and select it
4. **Navigate to your project folder** and select it
5. **Copy the project files** (CMakeLists.txt, prj.conf, and src/main.c) into your project folder

### Step 3: Configure Your Board (Tell Zephyr What You're Using!)
1. **In the Zephyr extension panel**, look for the board selection
2. **Select "blackpill_f411ce"** - this tells Zephyr exactly what hardware you're working with
3. **Choose your runner**: Select **OpenOCD** 
4. **Runner configuration**: Select the **ST-Link** option (this is how we'll flash your board!)

### Step 4: Build Your Project (The Magic Moment!)
1. **Click the "Build" button** in the Zephyr extension (it looks like a hammer)
2. **Watch the magic happen!** You'll see lots of text scrolling by - that's Zephyr compiling your code
3. **Success!** You should see "Build completed successfully" (time to do a little happy dance!)

### Step 5: Flash Your Code (Upload Time!)
1. **Connect your ST-Link** to your BlackPill board
2. **Click the "Flash" button** in the Zephyr extension
3. **Here's the secret sauce**: As soon as you see the flashing process start, **press the reset button on your BlackPill board once!** This helps ensure a clean flash
4. **Wait for completion** - you should see "Flash completed successfully"

### Step 6: See Your Code in Action! (The Exciting Part!)
1. **Connect your TTL-to-USB converter** to your BlackPill:
   - Connect GND to GND
   - Connect the BlackPill's TX pin to the converter's RX pin
   - Connect the BlackPill's RX pin to the converter's TX pin
2. **Open VS Code's integrated terminal** (Terminal → New Terminal)
3. **Find your serial port**:
   - On Windows: Usually COM3, COM4, etc.
   - On Mac/Linux: Usually /dev/ttyUSB0 or /dev/ttyACM0
4. **Open a serial connection**:
   ```bash
   # On Windows (replace COM3 with your port)
   python -m serial.tools.miniterm COM3 115200
   
   # On Mac/Linux (replace /dev/ttyUSB0 with your port)
   python -m serial.tools.miniterm /dev/ttyUSB0 115200
   ```
5. **Press the reset button** on your BlackPill and watch the magic happen!

## Understanding Your Code (The Detective Work!)

### Configuration Magic (`prj.conf`)
This file is like your project's recipe card - it tells Zephyr exactly what ingredients (features) you want to use:

```
CONFIG_I2C=y          # "Yes, I want I2C support please!"
CONFIG_UART_CONSOLE=y # "I want to talk to my computer!"
CONFIG_PRINTK=y       # "Let me print debug messages!"
```

### The Main Event (`main.c`)

**Getting Ready:**
```c
i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
```
This line is like asking Zephyr: "Hey, can you give me access to the I2C1 peripheral?" It's how we get our hands on the I2C hardware!

**The Scanning Magic:**
Our scanner is like a polite neighbor knocking on every door in the I2C neighborhood (addresses 0x08 to 0x77). For each address, we try to send a message. If someone answers (returns 0), we know there's a device there! If not, we mark it as empty and move on.

**Pretty Output:**
The results are displayed in a neat grid format, just like the famous Linux `i2cdetect` command. When you see a number like `3C`, that means "Hey! There's a device at address 0x3C!"

## Want to Use This on Other STM32 Boards? (Expanding Your Empire!)

Great news! This code is super portable. Here's what you need to change:

### 1. Board Selection (The Easy Part!)
Just change your board in the Zephyr extension:
```bash
# Instead of blackpill_f411ce, you might use:
nucleo_f401re        # For STM32F401 Nucleo
stm32f4_disco        # For STM32F407 Discovery  
nucleo_f446re        # For STM32F446 Nucleo
```

### 2. I2C Instance (If Needed)
Some boards might use I2C2 instead of I2C1:
```c
// Change this line if your board uses I2C2
i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c2));
```

### 3. Pin Connections (Check Your Board's Manual!)
Different boards have I2C pins in different places. Check your board's pinout diagram to find:
- **SDA pin** (data line)
- **SCL pin** (clock line)
- **UART TX/RX pins** (for console output)

### 4. Device Tree Tweaks (For Advanced Users)
If your board isn't officially supported, you might need to create a custom device tree overlay. But don't worry - start with officially supported boards first!

## What You'll Learn (The Treasure!)

This project is like a masterclass in embedded development fundamentals:

- **Device Tree Magic**: How Zephyr uses device trees to make hardware portable
- **Driver APIs**: Using standardized functions that work across different chips
- **Timing Control**: Using Zephyr's sleep functions (because even microcontrollers need naps!)
- **Communication**: Setting up UART for debugging (your lifeline to the outside world!)
- **Hardware Debugging**: Building tools to understand what's connected to your system

## What's Next?

Once you've got this scanner working, you're ready for bigger adventures! Try:
- **Reading sensor data** from the devices you discovered
- **Building a temperature monitor** with an I2C sensor
- **Creating a display controller** for an I2C OLED screen
- **Making a weather station** that logs data over I2C

Remember, every expert was once a beginner. You've got this!

Happy coding, and welcome to the wonderful world of Zephyr RTOS!

---
*Pro tip: Keep this scanner project handy - you'll find yourself using it again and again as you build more complex I2C projects. It's like having a Swiss Army knife for I2C debugging!*

## Project Structure

```
blackpill-i2c-scanner/
├── CMakeLists.txt     # Build configuration
├── prj.conf          # Project configuration
└── src/
    └── main.c         # Main application code
```

## Expected Output

When running, you should see output similar to this:

```
STM32F411CEU6 BlackPill I2C Scanner
===================================
I2C device ready
Starting I2C bus scan...
Scanning addresses 0x08 to 0x77
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:          -- -- -- -- -- -- -- -- -- -- -- -- -- 
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- -- 
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- 
70: -- -- -- -- -- -- -- -- 

Scan complete. Found 1 device(s).

Waiting 5 seconds before next scan...
```

In this example, a device was found at address 0x3C (which might be an OLED display).

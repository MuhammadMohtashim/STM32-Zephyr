# CMakeLists.txt - Enhanced AHT10 project with LED control and UART
cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(aht10_led_uart_controller)

target_sources(app PRIVATE src/main.c)

# Set the board if not specified
if(NOT DEFINED BOARD)
    set(BOARD blackpill_f411ce)
endif()

# Add any additional compile definitions if needed
target_compile_definitions(app PRIVATE
    # Add any project-specific definitions here
)
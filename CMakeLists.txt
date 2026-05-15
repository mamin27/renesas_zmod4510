cmake_minimum_required(VERSION 3.0)
project(zmod4510)
set(EXE_NAME no2_o3-example)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This project is only supported on Linux")
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
  message(FATAL_ERROR "This library is designed for ARM/AArch64 platforms like Raspberry Pi")
endif()

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Sources
set(COMMON_SOURCES
    src/sensors/zmod4xxx.c
    src/hal/zmod4xxx_hal.c
    src/hal/hal.c
    #src/hal/raspi/rpi.c
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES} src/sensor_interface.c)

# Global compiler definitions for the shared library target
# 1. ZMOD4510: Enables the correct conditional blocks inside Renesas headers (fixes the zmod4xxx_init warning)
# 2. Hardware configurations for the custom Linux HAL layer
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ZMOD4510
    ZMOD4510_I2C_BUS_NUMBER=0
    ZMOD4510_I2C_ADDR=0x33
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/algos
    src/sensors
    src/hal
)

# Link libraries directly
target_link_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib")
target_link_libraries(${PROJECT_NAME} PRIVATE
    _no2_o3.a
    _zmod4xxx_cleaning.a
    m)

# Add executable
add_executable(${EXE_NAME} src/main.c)
target_include_directories(${EXE_NAME} PRIVATE
    src/algos src/sensors)
target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

# Crucial: This tells scikit-build where to put the .so file
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION python/lib)

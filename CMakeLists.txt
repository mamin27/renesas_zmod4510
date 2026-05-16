cmake_minimum_required(VERSION 3.0)
project(zmod4510)
set(EXE_NAME no2_o3-example)

# ==============================================================================
# 1. SPRÁVNA CROSS-KOMPILÁCIA NA ORANGE PI / RASPBERRY PI
# ==============================================================================
# Použijeme 32-bitový cross-kompilátor, ktorý akceptuje ARMv7-A flagy
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc-6)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++-6)

# Odstránili sme problematické -m32, cross-kompilátor ho nepotrebuje a vie, že robí 32-bit ARMv7-A
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv7-a -mfloat-abi=hard -mfpu=vfpv3-d16")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=armv7-a -mfloat-abi=hard -mfpu=vfpv3-d16")

# ==============================================================================
# KONTROLA PLATFORMY
# ==============================================================================
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  message(FATAL_ERROR "This project is only supported on Linux")
endif()

if(NOT CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64|ARM")
  message(FATAL_ERROR "This library is designed for ARM platforms")
endif()

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ==============================================================================
# ZDROJOVÉ SÚBORY A TARGETY
# ==============================================================================
set(COMMON_SOURCES
    src/sensors/zmod4xxx.c
    src/hal/zmod4xxx_hal.c
    src/hal/hal.c
)

# Create shared library (.so)
add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES} src/sensor_interface.c)

# Global compiler definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ZMOD4510
    ZMOD4510_I2C_BUS_NUMBER=0  # Uprav na 1, ak si na Raspberry Pi, nechaj 0 pre Orange Pi
    ZMOD4510_I2C_ADDR=0x33
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/algos
    src/sensors
    src/hal
)

# ==============================================================================
# 2. DEFINÍCIA PRESNÝCH CIEST K RENESAS KNIŽNICIAM
# ==============================================================================
set(RENESAS_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/ARMv7-A/arm-linux-gnueabihf-gcc")

# Linkovanie knižníc cez plnú cestu
target_link_libraries(${PROJECT_NAME} PRIVATE
    "${RENESAS_LIB_DIR}/lib_no2_o3.a"
    "${RENESAS_LIB_DIR}/lib_zmod4xxx_cleaning.a"
    m
)

# ==============================================================================
# EXECUTABLE
# ==============================================================================
add_executable(${EXE_NAME} src/main.c)
target_include_directories(${EXE_NAME} PRIVATE
    src/algos src/sensors)
target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

# Crucial for scikit-build
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION python/lib)

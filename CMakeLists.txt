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
    src/hal/raspi/rpi.c
)

# Replace the proprietary static archives with the C reimplementation in
# src/reimpl (see doc/reimplementation.md).
option(USE_REIMPL "Build src/reimpl instead of linking lib/lib_*.a" OFF)

set(REIMPL_SOURCES
    src/reimpl/no2_o3.c
    src/reimpl/no2_o3_nnet.c
    src/reimpl/no2_o3_data.c
    src/reimpl/no2_o3_nnet_data.c
    src/reimpl/zmod4xxx_cleaning.c
)

if(USE_REIMPL)
  list(APPEND COMMON_SOURCES ${REIMPL_SOURCES})
  # Keep the evaluation order of the original: no extra FMA contraction.
  set_source_files_properties(${REIMPL_SOURCES}
      PROPERTIES COMPILE_FLAGS "-ffp-contract=off")
endif()

# Create shared library
add_library(${PROJECT_NAME} SHARED ${COMMON_SOURCES} src/sensor_interface.c)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src src/algos src/sensors src/hal src/reimpl)

# Link libraries directly
target_link_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/lib")
if(USE_REIMPL)
  target_link_libraries(${PROJECT_NAME} PRIVATE m)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE
      _no2_o3.a
      _zmod4xxx_cleaning.a
      m)
endif()

# Add executable
add_executable(${EXE_NAME} src/main.c)
target_include_directories(${EXE_NAME} PRIVATE
    src/algos src/sensors)
target_link_libraries(${EXE_NAME} ${PROJECT_NAME})

# Crucial: This tells scikit-build where to put the .so file
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION python/lib)

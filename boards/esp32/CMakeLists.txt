# ── ESP32-S3 board CMakeLists ────────────────────────────────────────────────
# This file is used when building with the ESP-IDF CMake system.
# Run:  idf.py build  (from the boards/esp32/ directory)
# OR:   cmake -B build -DHMIC_BOARD=esp32  (from repo root with IDF toolchain)

cmake_minimum_required(VERSION 3.20)

# Include ESP-IDF build system (must be done before project())
# Uncomment when using idf.py or the IDF cmake toolchain file:
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(hmic_esp32 C)

add_executable(hmic_esp32
    hal_esp32.c
)

target_link_libraries(hmic_esp32
    hmic_core
    hmic_app
    # ESP-IDF components:
    # idf::driver
    # idf::esp_timer
    # idf::freertos
    # idf::log
    #
    # Add LVGL + display/touch driver libraries here when ready.
)

target_include_directories(hmic_esp32 PRIVATE
    boards/esp32
    # ${CMAKE_SOURCE_DIR}/external/lvgl
)

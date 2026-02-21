include(FetchContent)

FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG        v9.1.0
)

# Let LVGL find our lv_conf.h
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/app/ui/lv_conf.h" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(lvgl)

pico_sdk_init()

add_executable(hmic_rp2040
    hal_rp2040.c
)

target_link_libraries(hmic_rp2040
    hmic_core
    hmic_app
    lvgl
    pico_stdlib
    hardware_uart
    hardware_timer
)

target_include_directories(hmic_rp2040 PRIVATE
    boards/rp2040
    # Add LVGL include path when ready:
    # ${CMAKE_SOURCE_DIR}/external/lvgl
)

pico_add_extra_outputs(hmic_rp2040)

# Use USB stdio instead of UART stdio for debug output (optional):
# pico_enable_stdio_usb(hmic_rp2040 1)
# pico_enable_stdio_uart(hmic_rp2040 0)

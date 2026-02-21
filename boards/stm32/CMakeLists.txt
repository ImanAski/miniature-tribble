# ── STM32 board CMakeLists ──────────────────────────────────────────────────
# This is a template for STM32 integration.
# It assumes you are using a toolchain file or passing include/library paths
# for the STM32 HAL and CMSIS.

add_executable(hmic_stm32
    hal_stm32.c
)

target_link_libraries(hmic_stm32
    hmic_core
    hmic_app
    # Add your STM32 HAL libraries here:
    # stm32_hal
    # stm32_cmsis
)

target_include_directories(hmic_stm32 PRIVATE
    .
    # Add your STM32 HAL/CMSIS include paths:
    # ${STM32_HAL_INCLUDE_DIR}
)

# Optional: Add post-build steps for .bin/.hex generation if using ARM GCC
# arm_generate_binary_images(hmic_stm32)

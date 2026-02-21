cmake_minimum_required(VERSION 3.20)

# ── Board selection ──────────────────────────────────────────────────────────
set(HMIC_BOARD "sim" CACHE STRING "Target board: rp2040 | esp32 | sim | stm32")
message(STATUS "hmic board target: ${HMIC_BOARD}")

# ── Project declaration ──────────────────────────────────────────────────────
project(hmic C CXX ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Board-specific setup ─────────────────────────────────────────────────────
# We process the board subdirectory BEFORE the app layer so the board
# can provide dependencies (like LVGL via FetchContent for simulation).
if(HMIC_BOARD STREQUAL "rp2040")
    if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH is not set.")
    endif()
    include(pico_sdk_import.cmake)
endif()

add_subdirectory(boards/${HMIC_BOARD})

# ── Core library (board-agnostic) ────────────────────────────────────────────
add_library(hmic_core STATIC
    core/crc16.c
    core/dm_parser.c
    core/dm_protocol.c
    core/dm_packet.c
    core/dm_core.c
)
target_include_directories(hmic_core PUBLIC core)

# ── App binder + UI layer ────────────────────────────────────────────────────
add_library(hmic_app STATIC
    app/dm_binder.c
    app/ui/ui_pages.c
)
target_include_directories(hmic_app PUBLIC app core)

# hmic_app needs LVGL. We assume the board subdirectory has either:
# 1. Created a target named 'lvgl' (e.g. via FetchContent)
# 2. Set up include paths/libraries globally
if(TARGET lvgl)
    target_link_libraries(hmic_app PUBLIC lvgl)
else()
    # If no 'lvgl' target exists, we assume the user provides it via include paths
    # or the board port handles it. In most embedded setups, the board layer
    # knows where the LVGL port is.
endif()

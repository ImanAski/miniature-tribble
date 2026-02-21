cmake_minimum_required(VERSION 3.20)

# ── Board selection ──────────────────────────────────────────────────────────
set(HMIC_BOARD "sim" CACHE STRING "Target board: rp2040 | esp32 | sim | stm32")
message(STATUS "hmic board target: ${HMIC_BOARD}")

# ── Board-specific pre-project setup ─────────────────────────────────────────
if(HMIC_BOARD STREQUAL "rp2040")
    if(NOT DEFINED ENV{PICO_SDK_PATH} AND NOT DEFINED PICO_SDK_PATH)
        message(FATAL_ERROR "PICO_SDK_PATH is not set.")
    endif()
    include(pico_sdk_import.cmake)
endif()

# ── Project declaration ──────────────────────────────────────────────────────
project(hmic C CXX ASM)

# For Pico SDK, init must happen after project()
if(HMIC_BOARD STREQUAL "rp2040")
    pico_sdk_init()
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ── LVGL Setup (Shared across boards) ────────────────────────────────────────
# We fetch LVGL here once to avoid redundant clones and use shallow cloning
# to speed up the process and prevent hangs.
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG        v9.1.0
  GIT_SHALLOW    ON
  GIT_PROGRESS   ON
)

# Let LVGL find our lv_conf.h
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/app/ui/lv_conf.h" CACHE STRING "" FORCE)

# Simulator specific LVGL optimizations (Safe to set globally as they are mostly CACHE)
set(LV_CONF_SKIP_POSIX_DIR_ITER_CHECK ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(lvgl)

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

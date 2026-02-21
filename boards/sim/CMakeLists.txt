# ── Simulator (POSIX/SDL) board CMakeLists ───────────────────────────────────

include(FetchContent)

# Automatically fetch LVGL for the simulator build to ensure it "just works"
FetchContent_Declare(
  lvgl
  GIT_REPOSITORY https://github.com/lvgl/lvgl.git
  GIT_TAG        v9.1.0 # Or your preferred stable version
)

# Optimize LVGL build for the simulator
set(LV_CONF_SKIP_POSIX_DIR_ITER_CHECK ON CACHE BOOL "" FORCE)
# Let LVGL find our lv_conf.h
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/app/ui/lv_conf.h" CACHE STRING "" FORCE)

# Disable Arm specific optimizations on non-Arm sim
set(LV_USE_DRAW_SW_HELIUM OFF CACHE BOOL "" FORCE)
set(LV_USE_DRAW_SW_NEON OFF CACHE BOOL "" FORCE)
set(LV_USE_DRAW_SW_ASM OFF CACHE BOOL "" FORCE)
set(LV_USE_HELIUM OFF CACHE BOOL "" FORCE)
set(LV_USE_NEON OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(lvgl)

add_executable(hmic_sim
    hal_sim.c
)

target_link_libraries(hmic_sim
    hmic_core
    hmic_app
    lvgl
    m
)

target_include_directories(hmic_sim PRIVATE
    .
)

# Export the lvgl target so hmic_app can find it
# (FetchContent_MakeAvailable already makes the 'lvgl' target available)

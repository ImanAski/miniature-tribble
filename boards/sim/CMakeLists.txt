# ── Simulator (POSIX/SDL) board CMakeLists ───────────────────────────────────

# Disable Arm specific optimizations on non-Arm sim

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

import math

def calculate_mcu_resources(
    rx,
    ry,
    bpp,
    buffered_lines,
    fps,
    pages,
    components_per_page,
    updates_per_sec,
    touch_rate,
    mcu_clock_mhz
):

    # --- SRAM ---
    if buffered_lines == 0:
        framebuffer = rx * ry * bpp
    else:
        framebuffer = rx * buffered_lines * bpp

    component_memory = components_per_page * 200
    margin = 32000

    sram_total = framebuffer + component_memory + margin

    # --- CPU ---
    cycles_per_pixel = 20
    pixel_rate = rx * ry * fps
    cpu_display = pixel_rate * cycles_per_pixel
    cpu_components = updates_per_sec * 1000
    cpu_touch = touch_rate * 500

    cpu_required = cpu_display + cpu_components + cpu_touch
    mcu_clock = mcu_clock_mhz * 1_000_000

    cpu_load = (cpu_required / mcu_clock) * 100

    # --- Flash ---
    firmware_base = 800_000
    flash_total = firmware_base * 1.2

    return {
        "SRAM_KB": round(sram_total / 1024, 2),
        "Flash_KB": round(flash_total / 1024, 2),
        "CPU_Load_%": round(cpu_load, 2)
    }


if __name__ == "__main__":
    rx = input("rX:")
    ry = input("rY:")
    bpp = input("Bytes Per Pixel:")
    buffered_lines = input("Buffered Lines:")
    fps = input("Frames Per Second (FPS):")
    pages = input("Pages:")
    components_per_page = input("Compenents Per Page:")
    updates_per_sec = input("Updates Per Second:")
    touch_rate = input("Touch rate:")
    mcu_clock_mhz = input("Touch rate:")
    result = calculate_mcu_resources(
        rx,
        ry,
        bpp,
        buffered_lines,
        fps,
        pages,
        components_per_page,
        updates_per_sec,
        touch_rate,
        mcu_clock_mhz
    )

    print("=== MCU Resource Estimate ===")
    for k, v in result.items():
        print(f"{k}: {v}")

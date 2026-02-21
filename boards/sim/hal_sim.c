/**
 * @file hal_sim.c
 * @brief POSIX/SDL desktop simulator HAL.
 *
 * Allows developing and testing the core library and UI on a PC without
 * any hardware.  Uses:
 *   - POSIX UART (serial port or pty) for protocol bytes.
 *   - LVGL SDL2 backend for display.
 *   - POSIX clock_gettime for the millisecond counter.
 *
 * Build:
 *   cmake -B build-sim -DHMIC_BOARD=sim
 *   cmake --build build-sim
 *   ./build-sim/hmic_sim --port /dev/pts/3
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include "../../core/dm_core.h"
#include "../../core/dm_platform.h"
#include "../../app/dm_binder.h"

/* LVGL + SDL backend – provided by the sim CMakeLists */
#include "lvgl/lvgl.h"

/* ── Config ──────────────────────────────────────────────────────────────── */

#define SIM_DISPLAY_WIDTH  800
#define SIM_DISPLAY_HEIGHT 480

/* ── Serial port state ────────────────────────────────────────────────────── */

static int s_serial_fd = -1;

static int open_serial(const char *port, int baud)
{
    int fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) return -1;

    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetspeed(&tty, baud);
    tty.c_cflag = CS8 | CREAD | CLOCAL;
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tty.c_cc[VMIN]  = 0;
    tty.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSANOW, &tty);
    return fd;
}

/* ── Platform implementations ─────────────────────────────────────────────── */

static void sim_write_bytes(const uint8_t *data, uint16_t len)
{
    if (s_serial_fd >= 0) {
        write(s_serial_fd, data, len);
    } else {
        /* Loopback when no serial port: print hex to stdout */
        printf("[TX] ");
        for (uint16_t i = 0; i < len; i++) printf("%02X ", data[i]);
        printf("\n");
    }
}

static uint32_t sim_millis(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

static void sim_log(const char *msg)
{
    printf("[DM] %s\n", msg);
}

/* ── Platform struct ─────────────────────────────────────────────────────── */

static dm_platform_t s_platform = {
    .write_bytes = sim_write_bytes,
    .millis      = sim_millis,
    .log         = sim_log,
};

/* ── Entry point ──────────────────────────────────────────────────────────── */

int main(int argc, char *argv[])
{
    const char *port = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--port") == 0) port = argv[i + 1];
    }

    if (port) {
        s_serial_fd = open_serial(port, B115200);
        if (s_serial_fd < 0) {
            fprintf(stderr, "Could not open serial port %s: %s\n",
                    port, strerror(errno));
        } else {
            printf("[SIM] Serial port: %s\n", port);
        }
    } else {
        printf("[SIM] No --port specified, running in loopback mode.\n");
    }

    /*
     * TODO: Initialise LVGL SDL2 backend here.
     * Reference: lvgl/examples/porting/lv_port_disp_template.c
     *
     *   lv_init();
     *   lv_sdl_window_create(SIM_DISPLAY_WIDTH, SIM_DISPLAY_HEIGHT);
     *   lv_sdl_mouse_create();
     */

    dm_init(&s_platform);
    dm_binder_init(&s_platform);

    printf("[SIM] hmic simulator running. Ctrl-C to quit.\n");

    while (1) {
        /* Read serial bytes */
        if (s_serial_fd >= 0) {
            uint8_t buf[64];
            ssize_t n = read(s_serial_fd, buf, sizeof(buf));
            for (ssize_t i = 0; i < n; i++) {
                dm_receive_byte(buf[i]);
            }
        }

        dm_process();
        /* lv_timer_handler(); */

        usleep(5000); /* ~200 Hz tick */
    }

    if (s_serial_fd >= 0) close(s_serial_fd);
    return 0;
}

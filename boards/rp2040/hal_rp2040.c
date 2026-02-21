/**
 * @file hal_rp2040.c
 * @brief RP2040 board HAL implementation.
 *
 * Provides the dm_platform_t instance for the Raspberry Pi Pico / RP2040.
 *
 * Wiring assumptions:
 *   - UART0 (pins 0/1) at DM_UART_BAUDRATE – used for RS485 or direct UART.
 *   - Adjust TX/RX pins and UART instance as needed for your hardware.
 *
 * LVGL display and touch drivers are initialised in dm_board_init().
 * Adapt the display section for your specific TFT + touch controller.
 */
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/timer.h"
#include "pico/stdio.h"

#include <stdio.h>
#include <string.h>

#include "../../core/dm_core.h"
#include "../../core/dm_platform.h"
#include "../../app/dm_binder.h"

/* ── Config ──────────────────────────────────────────────────────────────── */

#define DM_UART_INSTANCE uart0
#define DM_UART_TX_PIN   0
#define DM_UART_RX_PIN   1
#define DM_UART_BAUDRATE 115200

/* ── Platform function implementations ───────────────────────────────────── */

static void rp2040_write_bytes(const uint8_t *data, uint16_t len)
{
    uart_write_blocking(DM_UART_INSTANCE, data, len);
}

static uint32_t rp2040_millis(void)
{
    return (uint32_t)(time_us_64() / 1000ULL);
}

static void rp2040_log(const char *msg)
{
    /* Routed to stdio (USB or UART depending on CMake config) */
    puts(msg);
}

/* ── Platform struct ─────────────────────────────────────────────────────── */

static dm_platform_t s_platform = {
    .write_bytes = rp2040_write_bytes,
    .millis      = rp2040_millis,
    .log         = rp2040_log,
};

/* ── Board init ──────────────────────────────────────────────────────────── */

static void dm_board_init(void)
{
    stdio_init_all();

    /* UART init */
    uart_init(DM_UART_INSTANCE, DM_UART_BAUDRATE);
    gpio_set_function(DM_UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(DM_UART_RX_PIN, GPIO_FUNC_UART);

    /*
     * TODO: Initialise your TFT display + touch driver here.
     * Example (pseudo-code):
     *   lv_init();
     *   my_tft_init();
     *   my_touch_init();
     *   lv_display_t *disp = lv_display_create(...);
     *   lv_indev_t   *indev = lv_indev_create(...);
     */
}

/* ── Main entry ──────────────────────────────────────────────────────────── */

int main(void)
{
    dm_board_init();

    /* Initialise core and app binder */
    dm_init(&s_platform);
    dm_binder_init(&s_platform);

    /* Main loop */
    while (true) {
        /* Feed incoming UART bytes */
        while (uart_is_readable(DM_UART_INSTANCE)) {
            uint8_t b = uart_getc(DM_UART_INSTANCE);
            dm_receive_byte(b);
        }

        dm_process();

        /* lv_timer_handler drives LVGL animations and redraws */
        /* lv_timer_handler(); */  /* Uncomment when LVGL is initialised */
    }

    return 0;
}

/**
 * @file hal_stm32.c
 * @brief STM32 board HAL implementation.
 *
 * Provides the dm_platform_t instance for STM32 microcontrollers.
 * Uses STM32 HAL for UART communication and system time.
 *
 * Assumptions:
 *   - huart1 is initialized and configured for the host communication.
 *   - The board layer provides the standard STM32 HAL headers.
 */
#include "stm32f4xx_hal.h" // Replace with your specific family header
#include <stdio.h>
#include <string.h>

#include "../../core/dm_core.h"
#include "../../core/dm_platform.h"
#include "../../app/dm_binder.h"

/* ── Externs ─────────────────────────────────────────────────────────────── */

extern UART_HandleTypeDef huart1; // Communication UART

/* ── Platform function implementations ───────────────────────────────────── */

static void stm32_write_bytes(const uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
}

static uint32_t stm32_millis(void)
{
    return HAL_GetTick();
}

static void stm32_log(const char *msg)
{
    // Log to a secondary ITM or UART if available
    // For now, just a stub
}

/* ── Platform struct ─────────────────────────────────────────────────────── */

static dm_platform_t s_platform = {
    .write_bytes = stm32_write_bytes,
    .millis      = stm32_millis,
    .log         = stm32_log,
};

/* ── Board init stub ─────────────────────────────────────────────────────── */

static void dm_board_init(void)
{
    /*
     * TODO: Initialise your TFT display + touch driver here.
     * Ensure LVGL is initialized including display and input buffers.
     */
}

/* ── Main loop integration ───────────────────────────────────────────────── */

/**
 * @brief Call this from your main.c after HAL_Init() and MX_Init()
 */
void hmic_run(void)
{
    dm_board_init();
    dm_init(&s_platform);
    dm_binder_init(&s_platform);

    uint8_t rx_byte;

    while (1) {
        /*
         * Non-blocking UART receive example. 
         * In a production app, use DMA or Interrupts per dm_receive_byte.
         */
        if (HAL_UART_Receive(&huart1, &rx_byte, 1, 10) == HAL_OK) {
            dm_receive_byte(rx_byte);
        }

        dm_process();
        
        /* Drives LVGL timers */
        /* lv_timer_handler(); */ // Uncomment when LVGL is ready
    }
}

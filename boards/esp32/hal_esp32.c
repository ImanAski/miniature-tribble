/**
 * @file hal_esp32.c
 * @brief ESP32-S3 board HAL implementation (ESP-IDF).
 *
 * Provides the dm_platform_t instance for the Espressif ESP32-S3.
 *
 * Wiring assumptions:
 *   - UART1 for RS485 communication.
 *   - Adjust TX/RX pins below for your hardware.
 *   - Initialise display + touch drivers before calling dm_board_init().
 *
 * Build with ESP-IDF (idf.py build) or via cmake with the ESP-IDF toolchain.
 */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "../../core/dm_core.h"
#include "../../core/dm_platform.h"
#include "../../app/dm_binder.h"

/* ── Config ──────────────────────────────────────────────────────────────── */

#define DM_TAG            "hmic"
#define DM_UART_PORT      UART_NUM_1
#define DM_UART_TX_PIN    17
#define DM_UART_RX_PIN    18
#define DM_UART_BAUDRATE  115200
#define DM_UART_BUF_SIZE  256

/* ── Platform function implementations ───────────────────────────────────── */

static void esp32_write_bytes(const uint8_t *data, uint16_t len)
{
    uart_write_bytes(DM_UART_PORT, (const char *)data, len);
}

static uint32_t esp32_millis(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000LL);
}

static void esp32_log(const char *msg)
{
    ESP_LOGI(DM_TAG, "%s", msg);
}

/* ── Platform struct ─────────────────────────────────────────────────────── */

static dm_platform_t s_platform = {
    .write_bytes = esp32_write_bytes,
    .millis      = esp32_millis,
    .log         = esp32_log,
};

/* ── Board init ──────────────────────────────────────────────────────────── */

static void dm_board_init(void)
{
    uart_config_t uart_config = {
        .baud_rate  = DM_UART_BAUDRATE,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
    };
    uart_param_config(DM_UART_PORT, &uart_config);
    uart_set_pin(DM_UART_PORT, DM_UART_TX_PIN, DM_UART_RX_PIN,
                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(DM_UART_PORT, DM_UART_BUF_SIZE * 2, 0, 0, NULL, 0);

    /*
     * TODO: Initialise your TFT display + touch driver here.
     * Example (pseudo-code):
     *   lv_init();
     *   my_tft_init();
     *   my_touch_init();
     */
}

/* ── FreeRTOS task ───────────────────────────────────────────────────────── */

static void hmic_task(void *arg)
{
    (void)arg;
    uint8_t buf[64];

    dm_board_init();
    dm_init(&s_platform);
    dm_binder_init(&s_platform);

    while (1) {
        int len = uart_read_bytes(DM_UART_PORT, buf, sizeof(buf),
                                  pdMS_TO_TICKS(5));
        for (int i = 0; i < len; i++) {
            dm_receive_byte(buf[i]);
        }

        dm_process();
        /* lv_timer_handler(); */  /* Uncomment when LVGL is initialised */

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void app_main(void)
{
    xTaskCreate(hmic_task, "hmic", 8192, NULL, 5, NULL);
}

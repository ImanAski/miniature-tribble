/**
 * @file dm_core.h
 * @brief Public API of the Display Manager core library.
 *
 * Board firmware interacts ONLY through these three functions:
 *
 *   dm_init()         – one-time initialisation
 *   dm_receive_byte() – feed each incoming byte
 *   dm_process()      – call periodically in the main loop
 */
#ifndef DM_CORE_H
#define DM_CORE_H

#include <stdint.h>
#include "dm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialise the Display Manager.
 *
 * Must be called once before dm_receive_byte / dm_process.
 * Stores a reference to @p platform (must remain valid for the lifetime
 * of the application).
 *
 * @param platform  Pointer to the board's HAL vtable.
 */
void dm_init(dm_platform_t *platform);

/**
 * @brief Feed one received byte into the protocol parser.
 *
 * Call this from your UART/USB RX interrupt or polling loop.
 * This function is non-blocking and safe to call from an ISR if
 * dm_process() is called from the main loop.
 *
 * @param byte  Received byte.
 */
void dm_receive_byte(uint8_t byte);

/**
 * @brief Periodic processing tick.
 *
 * Call this as frequently as possible from the main loop (ideally every
 * 1–10 ms).  Drives LVGL timer handling via lv_timer_handler() and any
 * deferred protocol work.
 */
void dm_process(void);

#ifdef __cplusplus
}
#endif

#endif /* DM_CORE_H */

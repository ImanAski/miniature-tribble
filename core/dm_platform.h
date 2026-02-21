/**
 * @file dm_platform.h
 * @brief Platform Abstraction Layer – HAL vtable injected by the board port.
 *
 * The core library never calls hardware directly.  All I/O is routed
 * through this structure, which the board layer fills in and passes
 * to dm_init().
 *
 * Board ports MUST implement every function pointer (no NULLs).
 */
#ifndef DM_PLATFORM_H
#define DM_PLATFORM_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Platform interface vtable.
 *
 * Filled by the board layer, injected into the core via dm_init().
 * No function pointer may be NULL.
 */
typedef struct {
    /**
     * @brief Transmit bytes to the host (UART / USB).
     * @param data  Pointer to bytes to send.
     * @param len   Number of bytes.
     */
    void (*write_bytes)(const uint8_t *data, uint16_t len);

    /**
     * @brief Return a monotonically increasing millisecond counter.
     * @return Milliseconds since boot (may wrap).
     */
    uint32_t (*millis)(void);

    /**
     * @brief Emit a null-terminated debug string.
     * @param msg  Message to log (no newline required).
     */
    void (*log)(const char *msg);
} dm_platform_t;

#ifdef __cplusplus
}
#endif

#endif /* DM_PLATFORM_H */

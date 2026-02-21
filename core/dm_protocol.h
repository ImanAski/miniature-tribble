/**
 * @file dm_protocol.h
 * @brief Command/event IDs and protocol dispatcher interface.
 *
 * All command IDs are prefixed CMD_ (host→device).
 * All event IDs are prefixed EVT_ (device→host).
 *
 * The dispatcher (dm_protocol_dispatch) is called by the parser on every
 * valid frame and routes the command to the corresponding handler in the
 * application binder layer.
 */
#ifndef DM_PROTOCOL_H
#define DM_PROTOCOL_H

#include "dm_parser.h"
#include "dm_platform.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Command IDs (Host → Device)

/** System commands */
#define CMD_PING 0x01
#define CMD_GET_VERSION 0x02
#define CMD_RESET 0x03
#define CMD_ENTER_BOOTLOADER 0x04

/** Navigation */
#define CMD_SHOW_PAGE 0x10

/** UI manipulation */
#define CMD_SET_TEXT 0x20
#define CMD_SET_VALUE 0x21
#define CMD_SET_VISIBLE 0x22
#define CMD_SET_ENABLED 0x23

// Event IDs (Device → Host)

#define EVT_BUTTON_PRESSED 0x80
#define EVT_SLIDER_CHANGED 0x81
#define EVT_PAGE_CHANGED 0x82
#define EVT_TOUCH_EVENT 0x83

#define EVT_ACK 0xF0
#define EVT_NACK 0xF1

// Dispatcher

/**
 * @brief Initialise the protocol dispatcher (clears seq-id tracking).
 */
void dm_protocol_init(void);

/**
 * @brief Route a validated frame to the appropriate command handler.
 *
 * Called automatically by dm_parser_feed on a valid frame.
 * Unknown commands receive an automatic NACK.
 *
 * @param frame  Validated frame.
 * @param plat   Platform interface (write_bytes / log).
 */
void dm_protocol_dispatch(const dm_frame_t *frame, const dm_platform_t *plat);

/* ── Weak handler stubs (override in app/dm_binder.c) ───────────────────── */

/**
 * Each handler receives the raw payload and its length.
 * The binder layer overrides these weak symbols.
 */
void dm_handle_ping(uint8_t seq, const uint8_t *p, uint8_t len,
                    const dm_platform_t *plat);
void dm_handle_get_version(uint8_t seq, const uint8_t *p, uint8_t len,
                           const dm_platform_t *plat);
void dm_handle_reset(uint8_t seq, const uint8_t *p, uint8_t len,
                     const dm_platform_t *plat);
void dm_handle_enter_bootloader(uint8_t seq, const uint8_t *p, uint8_t len,
                                const dm_platform_t *plat);
void dm_handle_show_page(uint8_t seq, const uint8_t *p, uint8_t len,
                         const dm_platform_t *plat);
void dm_handle_set_text(uint8_t seq, const uint8_t *p, uint8_t len,
                        const dm_platform_t *plat);
void dm_handle_set_value(uint8_t seq, const uint8_t *p, uint8_t len,
                         const dm_platform_t *plat);
void dm_handle_set_visible(uint8_t seq, const uint8_t *p, uint8_t len,
                           const dm_platform_t *plat);
void dm_handle_set_enabled(uint8_t seq, const uint8_t *p, uint8_t len,
                           const dm_platform_t *plat);

#ifdef __cplusplus
}
#endif

#endif /* DM_PROTOCOL_H */

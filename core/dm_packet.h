/**
 * @file dm_packet.h
 * @brief Packet encoder – builds and transmits frames via the platform HAL.
 *
 * All outgoing frames follow the same wire format as incoming frames.
 * Convenience helpers are provided for ACK, NACK, and common events.
 */
#ifndef DM_PACKET_H
#define DM_PACKET_H

#include <stdint.h>
#include "dm_config.h"
#include "dm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Build and send a generic frame.
 *
 * @param cmd      Command/event ID byte.
 * @param seq      Sequence ID (echoed from the triggering request, or 0).
 * @param payload  Payload bytes (may be NULL if payload_len == 0).
 * @param payload_len  Number of payload bytes.
 * @param plat     Platform interface (write_bytes is called).
 */
void dm_packet_send(uint8_t cmd,
                    uint8_t seq,
                    const uint8_t *payload,
                    uint8_t payload_len,
                    const dm_platform_t *plat);

/**
 * @brief Send an ACK response (EVT_ACK) with optional payload.
 */
void dm_packet_send_ack(uint8_t seq,
                        const dm_platform_t *plat,
                        const uint8_t *payload,
                        uint8_t payload_len);

/**
 * @brief Send a NACK response (EVT_NACK, no payload).
 */
void dm_packet_send_nack(uint8_t seq, const dm_platform_t *plat);

/** @brief Send EVT_BUTTON_PRESSED. Payload: 1 byte widget_id index. */
void dm_packet_send_button_pressed(uint8_t widget_idx, const dm_platform_t *plat);

/** @brief Send EVT_SLIDER_CHANGED. Payload: 1 byte widget_idx + 2 byte int16 value. */
void dm_packet_send_slider_changed(uint8_t widget_idx, int16_t value, const dm_platform_t *plat);

/** @brief Send EVT_PAGE_CHANGED. Payload: 1 byte page_id. */
void dm_packet_send_page_changed(uint8_t page_id, const dm_platform_t *plat);

/** @brief Send EVT_TOUCH_EVENT. Payload: 2 × int16_t (x, y). */
void dm_packet_send_touch_event(int16_t x, int16_t y, const dm_platform_t *plat);

#ifdef __cplusplus
}
#endif

#endif /* DM_PACKET_H */

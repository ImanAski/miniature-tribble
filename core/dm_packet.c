/**
 * @file dm_packet.c
 * @brief Packet encoder implementation.
 *
 * Builds frames in a static stack-allocated buffer (no heap).
 * The buffer is sized to DM_MAX_FRAME_SIZE at compile time.
 */
#include "dm_packet.h"
#include "crc16.h"
#include "dm_protocol.h"

#include <string.h>

// Internal helpers

static uint8_t s_seq_counter =
    0; /* Auto-incremented for device-originated events */

void dm_packet_send(uint8_t cmd, uint8_t seq, const uint8_t *payload,
                    uint8_t payload_len, const dm_platform_t *plat) {
  if (!plat || !plat->write_bytes)
    return;

  /* Guard against oversized payloads */
  if (payload_len > DM_MAX_PAYLOAD)
    payload_len = DM_MAX_PAYLOAD;

  uint8_t frame[DM_MAX_FRAME_SIZE];
  uint16_t idx = 0;

  /* Start byte (not included in CRC) */
  frame[idx++] = DM_START_BYTE;

  /* Header fields (CRC starts here) */
  frame[idx++] = DM_PROTOCOL_VERSION;
  frame[idx++] = cmd;
  frame[idx++] = seq;
  frame[idx++] = payload_len;

  /* Payload */
  if (payload && payload_len > 0) {
    memcpy(&frame[idx], payload, payload_len);
    idx += payload_len;
  }

  /* CRC over VERSION..PAYLOAD (bytes 1..idx-1) */
  uint16_t crc = crc16_ccitt(&frame[1], idx - 1);
  frame[idx++] = (uint8_t)(crc >> 8);
  frame[idx++] = (uint8_t)(crc & 0xFF);

  plat->write_bytes(frame, idx);
}

// Convenience wrappers

void dm_packet_send_ack(uint8_t seq, const dm_platform_t *plat,
                        const uint8_t *payload, uint8_t payload_len) {
  dm_packet_send(EVT_ACK, seq, payload, payload_len, plat);
}

void dm_packet_send_nack(uint8_t seq, const dm_platform_t *plat) {
  dm_packet_send(EVT_NACK, seq, NULL, 0, plat);
}

void dm_packet_send_button_pressed(uint8_t widget_idx,
                                   const dm_platform_t *plat) {
  dm_packet_send(EVT_BUTTON_PRESSED, s_seq_counter++, &widget_idx, 1, plat);
}

void dm_packet_send_slider_changed(uint8_t widget_idx, int16_t value,
                                   const dm_platform_t *plat) {
  uint8_t buf[3];
  buf[0] = widget_idx;
  buf[1] = (uint8_t)((value >> 8) & 0xFF);
  buf[2] = (uint8_t)(value & 0xFF);
  dm_packet_send(EVT_SLIDER_CHANGED, s_seq_counter++, buf, sizeof(buf), plat);
}

void dm_packet_send_page_changed(uint8_t page_id, const dm_platform_t *plat) {
  dm_packet_send(EVT_PAGE_CHANGED, s_seq_counter++, &page_id, 1, plat);
}

void dm_packet_send_touch_event(int16_t x, int16_t y,
                                const dm_platform_t *plat) {
  uint8_t buf[4];
  buf[0] = (uint8_t)((x >> 8) & 0xFF);
  buf[1] = (uint8_t)(x & 0xFF);
  buf[2] = (uint8_t)((y >> 8) & 0xFF);
  buf[3] = (uint8_t)(y & 0xFF);
  dm_packet_send(EVT_TOUCH_EVENT, s_seq_counter++, buf, sizeof(buf), plat);
}

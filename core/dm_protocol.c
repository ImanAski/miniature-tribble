/**
 * @file dm_protocol.c
 * @brief Protocol dispatcher – routes validated frames to handler functions.
 *
 * Handlers are defined as __attribute__((weak)) here so the application
 * binder (app/dm_binder.c) can override individual ones without touching
 * the core.  Handlers not overridden send an automatic NACK.
 */
#include "dm_protocol.h"
#include "dm_packet.h"

#include <string.h>

/* ── Dispatcher ──────────────────────────────────────────────────────────── */

void dm_protocol_init(void)
{
    /* Nothing to initialise yet; reserved for future seq-id tracking. */
}

void dm_protocol_dispatch(const dm_frame_t *frame, const dm_platform_t *plat)
{
    switch (frame->command) {
    case CMD_PING:
        dm_handle_ping(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_GET_VERSION:
        dm_handle_get_version(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_RESET:
        dm_handle_reset(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_ENTER_BOOTLOADER:
        dm_handle_enter_bootloader(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_SHOW_PAGE:
        dm_handle_show_page(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_SET_TEXT:
        dm_handle_set_text(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_SET_VALUE:
        dm_handle_set_value(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_SET_VISIBLE:
        dm_handle_set_visible(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    case CMD_SET_ENABLED:
        dm_handle_set_enabled(frame->seq_id, frame->payload, frame->payload_len, plat);
        break;
    default:
#if DM_DEBUG_LOG
        if (plat && plat->log) plat->log("DM: unknown command – sending NACK");
#endif
        dm_packet_send_nack(frame->seq_id, plat);
        break;
    }
}

/* ── Default (weak) handler implementations ─────────────────────────────── */

__attribute__((weak))
void dm_handle_ping(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    /* Default: reflect PONG as ACK with no payload */
    dm_packet_send_ack(seq, plat, NULL, 0);
}

__attribute__((weak))
void dm_handle_get_version(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    uint8_t ver[3] = { DM_PROTOCOL_VERSION, 0x00, 0x00 }; /* major.minor.patch */
    dm_packet_send_ack(seq, plat, ver, sizeof(ver));
}

__attribute__((weak))
void dm_handle_reset(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_ack(seq, plat, NULL, 0);
    /* Board layer should override this to trigger an actual reset. */
}

__attribute__((weak))
void dm_handle_enter_bootloader(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);   /* Not supported by default */
}

__attribute__((weak))
void dm_handle_show_page(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);   /* Override in dm_binder.c */
}

__attribute__((weak))
void dm_handle_set_text(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);
}

__attribute__((weak))
void dm_handle_set_value(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);
}

__attribute__((weak))
void dm_handle_set_visible(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);
}

__attribute__((weak))
void dm_handle_set_enabled(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    (void)p; (void)len;
    dm_packet_send_nack(seq, plat);
}

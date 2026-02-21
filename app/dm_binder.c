/**
 * @file dm_binder.c
 * @brief Application Binder implementation.
 *
 * Overrides the weak command handler stubs from dm_protocol.c.
 * Calls into ui_pages for actual LVGL operations.
 *
 * Payload conventions (host → device):
 *
 *   CMD_SHOW_PAGE    [1 byte]  page_id
 *   CMD_SET_TEXT     [1 byte widget_idx] [N bytes null-terminated string]
 *   CMD_SET_VALUE    [1 byte widget_idx] [2 bytes int16 big-endian]
 *   CMD_SET_VISIBLE  [1 byte widget_idx] [1 byte 0=hide 1=show]
 *   CMD_SET_ENABLED  [1 byte widget_idx] [1 byte 0=disable 1=enable]
 */
#include "dm_binder.h"
#include "dm_protocol.h"
#include "dm_packet.h"
#include "ui/ui_pages.h"
#include "dm_config.h"

#include <string.h>

static dm_platform_t *s_plat = NULL;

/* ── Init ───────────────────────────────────────────────────────────────── */

void dm_binder_init(dm_platform_t *plat)
{
    s_plat = plat;
    ui_pages_init();
}

/* ── Handler overrides ──────────────────────────────────────────────────── */

void dm_handle_show_page(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    if (len < 1) { dm_packet_send_nack(seq, plat); return; }
    uint8_t page_id = p[0];
    if (ui_pages_show(page_id)) {
        dm_packet_send_ack(seq, plat, NULL, 0);
        dm_packet_send_page_changed(page_id, plat);
    } else {
        dm_packet_send_nack(seq, plat);
    }
}

void dm_handle_set_text(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    if (len < 2) { dm_packet_send_nack(seq, plat); return; }
    uint8_t widget_idx = p[0];

    /* Copy text with guaranteed null-termination */
    char text[DM_MAX_TEXT_LEN];
    uint8_t text_len = len - 1;
    if (text_len >= DM_MAX_TEXT_LEN) text_len = DM_MAX_TEXT_LEN - 1;
    memcpy(text, p + 1, text_len);
    text[text_len] = '\0';

    if (ui_pages_set_text(widget_idx, text)) {
        dm_packet_send_ack(seq, plat, NULL, 0);
    } else {
        dm_packet_send_nack(seq, plat);
    }
}

void dm_handle_set_value(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    if (len < 3) { dm_packet_send_nack(seq, plat); return; }
    uint8_t  widget_idx = p[0];
    int16_t  value      = (int16_t)(((uint16_t)p[1] << 8) | p[2]);

    if (ui_pages_set_value(widget_idx, value)) {
        dm_packet_send_ack(seq, plat, NULL, 0);
    } else {
        dm_packet_send_nack(seq, plat);
    }
}

void dm_handle_set_visible(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    if (len < 2) { dm_packet_send_nack(seq, plat); return; }
    ui_pages_set_visible(p[0], p[1] != 0);
    dm_packet_send_ack(seq, plat, NULL, 0);
}

void dm_handle_set_enabled(uint8_t seq, const uint8_t *p, uint8_t len, const dm_platform_t *plat)
{
    if (len < 2) { dm_packet_send_nack(seq, plat); return; }
    ui_pages_set_enabled(p[0], p[1] != 0);
    dm_packet_send_ack(seq, plat, NULL, 0);
}

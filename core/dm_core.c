/**
 * @file dm_core.c
 * @brief Display Manager core – ties parser, protocol, and platform together.
 */
#include "dm_core.h"
#include "dm_parser.h"
#include "dm_protocol.h"

#include <stddef.h>

/* ── Module-private state ─────────────────────────────────────────────────── */

static dm_platform_t *s_platform = NULL;
static dm_parser_t    s_parser;

/* ── Public API ───────────────────────────────────────────────────────────── */

void dm_init(dm_platform_t *platform)
{
    s_platform = platform;
    dm_parser_init(&s_parser);
    dm_protocol_init();

#if DM_DEBUG_LOG
    if (s_platform && s_platform->log) {
        s_platform->log("DM: initialised");
    }
#endif
}

void dm_receive_byte(uint8_t byte)
{
    dm_parser_feed(&s_parser, byte, s_platform);
}

void dm_process(void)
{
    /*
     * Future: add timeout-based frame expiry, watchdog feed stub, etc.
     * LVGL's lv_timer_handler() is called by the board layer after dm_process().
     */
    (void)s_platform;
}

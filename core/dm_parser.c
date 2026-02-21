/**
 * @file dm_parser.c
 * @brief Byte-at-a-time frame parser with automatic re-synchronisation.
 *
 * CRC is computed over bytes VERSION..PAYLOAD (everything between the start
 * byte and the two CRC bytes).  On CRC failure the parser resets to
 * PARSE_WAIT_START with no frame delivery.
 */
#include "dm_parser.h"
#include "dm_protocol.h"
#include "crc16.h"

#include <string.h>

/* Forward declaration: implemented in dm_protocol.c */
void dm_protocol_dispatch(const dm_frame_t *frame, const dm_platform_t *plat);

/* ── helpers ──────────────────────────────────────────────────────────────── */

static void parser_reset(dm_parser_t *p)
{
    p->state         = PARSE_WAIT_START;
    p->payload_index = 0;
    p->running_crc   = 0xFFFFU;
    p->crc_high      = 0;
}

/* ── public API ───────────────────────────────────────────────────────────── */

void dm_parser_init(dm_parser_t *p)
{
    memset(p, 0, sizeof(*p));
    parser_reset(p);
}

void dm_parser_feed(dm_parser_t *p, uint8_t byte, const dm_platform_t *plat)
{
    switch (p->state) {

    /* ── Wait for start byte ──────────────────────────────────────────── */
    case PARSE_WAIT_START:
        if (byte == DM_START_BYTE) {
            parser_reset(p);          /* fresh CRC accumulation */
            p->state = PARSE_VERSION;
        }
        /* Any non-start byte is silently discarded (resync). */
        break;

    /* ── Header bytes – accumulate CRC ───────────────────────────────── */
    case PARSE_VERSION:
        p->frame.version = byte;
        p->running_crc   = crc16_update(p->running_crc, byte);
        p->state         = PARSE_COMMAND;
        break;

    case PARSE_COMMAND:
        p->frame.command = byte;
        p->running_crc   = crc16_update(p->running_crc, byte);
        p->state         = PARSE_SEQ_ID;
        break;

    case PARSE_SEQ_ID:
        p->frame.seq_id = byte;
        p->running_crc  = crc16_update(p->running_crc, byte);
        p->state        = PARSE_LENGTH;
        break;

    case PARSE_LENGTH:
        if (byte > DM_MAX_PAYLOAD) {
            /* Payload larger than our buffer – discard and resync. */
            p->frames_len_err++;
#if DM_DEBUG_LOG
            if (plat && plat->log) plat->log("DM: frame length overflow, resyncing");
#endif
            parser_reset(p);
            break;
        }
        p->frame.payload_len = byte;
        p->running_crc       = crc16_update(p->running_crc, byte);
        p->payload_index     = 0;

        if (byte == 0) {
            p->state = PARSE_CRC_HIGH;   /* zero-length payload */
        } else {
            p->state = PARSE_PAYLOAD;
        }
        break;

    /* ── Payload bytes ────────────────────────────────────────────────── */
    case PARSE_PAYLOAD:
        p->frame.payload[p->payload_index++] = byte;
        p->running_crc = crc16_update(p->running_crc, byte);

        if (p->payload_index >= p->frame.payload_len) {
            p->state = PARSE_CRC_HIGH;
        }
        break;

    /* ── CRC bytes ────────────────────────────────────────────────────── */
    case PARSE_CRC_HIGH:
        p->crc_high = byte;
        p->state    = PARSE_CRC_LOW;
        break;

    case PARSE_CRC_LOW: {
        uint16_t received_crc = ((uint16_t)p->crc_high << 8) | byte;

        if (received_crc == p->running_crc) {
            p->frames_ok++;
            dm_protocol_dispatch(&p->frame, plat);
        } else {
            p->frames_crc_err++;
#if DM_DEBUG_LOG
            if (plat && plat->log) plat->log("DM: CRC mismatch, frame dropped");
#endif
        }
        parser_reset(p);
        break;
    }

    default:
        parser_reset(p);
        break;
    }
}

/**
 * @file dm_parser.h
 * @brief Frame state-machine parser.
 *
 * Processes one byte at a time (dm_parser_feed).
 * Performs CRC validation and calls dm_protocol_dispatch on a valid frame.
 * Handles re-synchronisation on corrupted/truncated frames.
 *
 * Frame layout:
 *   [0]    START  (0xAA)
 *   [1]    VERSION
 *   [2]    COMMAND
 *   [3]    SEQUENCE_ID
 *   [4]    PAYLOAD_LENGTH
 *   [5..N] PAYLOAD
 *   [N+1]  CRC_HIGH
 *   [N+2]  CRC_LOW
 */
#ifndef DM_PARSER_H
#define DM_PARSER_H

#include <stdint.h>
#include "dm_config.h"
#include "dm_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Parsed frame handed to the dispatcher. */
typedef struct {
    uint8_t  version;
    uint8_t  command;
    uint8_t  seq_id;
    uint8_t  payload_len;
    uint8_t  payload[DM_MAX_PAYLOAD];
} dm_frame_t;

/** Parser states (internal – exposed for unit-testing only). */
typedef enum {
    PARSE_WAIT_START = 0,
    PARSE_VERSION,
    PARSE_COMMAND,
    PARSE_SEQ_ID,
    PARSE_LENGTH,
    PARSE_PAYLOAD,
    PARSE_CRC_HIGH,
    PARSE_CRC_LOW,
} dm_parse_state_t;

/** Parser context – one instance per interface. */
typedef struct {
    dm_parse_state_t state;
    dm_frame_t       frame;
    uint16_t         payload_index;
    uint16_t         running_crc;   /**< CRC accumulated over VERSION..PAYLOAD */
    uint8_t          crc_high;      /**< Received CRC MSB */

    /* Statistics (read-only for host) */
    uint32_t frames_ok;
    uint32_t frames_crc_err;
    uint32_t frames_len_err;
} dm_parser_t;

/**
 * @brief Initialise a parser context.
 * @param p  Pointer to parser instance.
 */
void dm_parser_init(dm_parser_t *p);

/**
 * @brief Feed one byte into the parser.
 *
 * When a complete, valid frame is received, dm_protocol_dispatch() is called.
 *
 * @param p     Parser instance.
 * @param byte  Received byte.
 * @param plat  Platform interface (for logging).
 */
void dm_parser_feed(dm_parser_t *p, uint8_t byte, const dm_platform_t *plat);

#ifdef __cplusplus
}
#endif

#endif /* DM_PARSER_H */

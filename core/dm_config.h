/**
 * @file dm_config.h
 * @brief Compile-time configuration knobs for the Display Manager.
 *
 * Override any of these via CMake defines:
 *   target_compile_definitions(hmic PRIVATE DM_MAX_PAYLOAD=256)
 */
#ifndef DM_CONFIG_H
#define DM_CONFIG_H

/** Maximum payload bytes in a single frame (excludes header + CRC). */
#ifndef DM_MAX_PAYLOAD
#define DM_MAX_PAYLOAD 128
#endif

/** Protocol version reported in every outgoing frame. */
#ifndef DM_PROTOCOL_VERSION
#define DM_PROTOCOL_VERSION 0x01
#endif

/** Start-of-frame magic byte. */
#define DM_START_BYTE 0xAA

/** Header size: START(1) + VERSION(1) + CMD(1) + SEQ(1) + LEN(1) = 5 */
#define DM_HEADER_SIZE 5

/** CRC size in bytes. */
#define DM_CRC_SIZE 2

/** Total maximum frame size. */
#define DM_MAX_FRAME_SIZE (DM_HEADER_SIZE + DM_MAX_PAYLOAD + DM_CRC_SIZE)

/** Maximum length of a widget ID string (null-terminated). */
#ifndef DM_MAX_WIDGET_ID
#define DM_MAX_WIDGET_ID 32
#endif

/** Maximum length of a text payload string (null-terminated). */
#ifndef DM_MAX_TEXT_LEN
#define DM_MAX_TEXT_LEN 64
#endif

/** Number of pages the UI binder can manage. */
#ifndef DM_MAX_PAGES
#define DM_MAX_PAGES 8
#endif

/** Enable (1) or disable (0) debug logging via platform->log. */
#ifndef DM_DEBUG_LOG
#define DM_DEBUG_LOG 1
#endif

#endif /* DM_CONFIG_H */

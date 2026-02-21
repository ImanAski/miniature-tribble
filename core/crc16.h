/**
 * @file crc16.h
 * @brief CRC16-CCITT (polynomial 0x1021, seed 0xFFFF, no XOR out).
 */
#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Compute CRC16-CCITT over a buffer.
 * @param data    Pointer to data.
 * @param length  Number of bytes.
 * @return        16-bit CRC.
 */
uint16_t crc16_ccitt(const uint8_t *data, size_t length);

/**
 * @brief Update a running CRC16 with a single byte.
 * @param crc   Current CRC value (start with 0xFFFF).
 * @param byte  New byte to process.
 * @return      Updated CRC value.
 */
uint16_t crc16_update(uint16_t crc, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif /* CRC16_H */

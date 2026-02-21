/**
 * @file crc16.c
 * @brief CRC16-CCITT implementation (polynomial 0x1021, seed 0xFFFF).
 *
 * Uses a byte-at-a-time table-less algorithm for minimal ROM footprint.
 * No XOR of the final value (CRC-CCITT "false" variant).
 */
#include "crc16.h"

#define CRC16_POLY 0x1021U

uint16_t crc16_update(uint16_t crc, uint8_t byte)
{
    crc ^= (uint16_t)byte << 8;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x8000U) {
            crc = (crc << 1) ^ CRC16_POLY;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

uint16_t crc16_ccitt(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;
    for (size_t i = 0; i < length; i++) {
        crc = crc16_update(crc, data[i]);
    }
    return crc;
}

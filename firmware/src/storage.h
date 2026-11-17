#ifndef STORAGE_H
#define STORAGE_H
#include <stdint.h>
#include "config.h"
#define RECORD_BYTES 3U
#define COMMITTED 0xa5U
#define EEPROM_BYTES (CHANNELS * RECORD_BYTES)

/* EEPROM layout, starting at address 0: [value, value XOR 0xff, marker] per RGB.
 * No value (including 255) is reserved as invalid. */
uint8_t storage_read_byte(uint8_t address);
void storage_write_byte(uint8_t address, uint8_t value);
uint8_t storage_load(uint8_t levels[CHANNELS]); /* bit mask of valid channels */
/* 0 = unchanged; 1 = committed and verified; 2 = verification failed. */
uint8_t storage_store(const uint8_t levels[CHANNELS]);
#endif

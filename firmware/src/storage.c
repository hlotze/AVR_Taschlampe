#include "storage.h"

static uint8_t read_channel(uint8_t channel, uint8_t *value)
{
    uint8_t address = channel * RECORD_BYTES;
    *value = storage_read_byte(address);
    uint8_t inverse = storage_read_byte(address + 1);
    return ((uint8_t)(*value ^ inverse) == 0xffU) &&
           storage_read_byte(address + 2) == COMMITTED;
}

uint8_t storage_load(uint8_t *levels)
{
    uint8_t valid = 0;
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        if (read_channel(c, &levels[c]))
            valid |= (1U << c);
        else
            levels[c] = DEFAULT_LEVEL;
    }
    return valid;
}

uint8_t storage_store(const uint8_t *levels)
{
    uint8_t changed = 0;
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        uint8_t old;
        if (read_channel(c, &old) && old == levels[c])
            continue;
        uint8_t address = c * RECORD_BYTES;
        /* Separate byte writes, deliberately ordered: invalidate FIRST,
         * commit LAST. Do not replace this by eeprom_update_block(). */
        storage_write_byte(address + 2, 0);
        storage_write_byte(address, levels[c]);
        storage_write_byte(address + 1, (uint8_t)~levels[c]);
        storage_write_byte(address + 2, COMMITTED);
        uint8_t verified;
        if (!read_channel(c, &verified) || verified != levels[c])
            return 2;
        changed = 1;
    }
    return changed;
}

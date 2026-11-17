#include <assert.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include "control.h"
#include "storage.h"

static uint8_t memory[EEPROM_BYTES];
static unsigned writes;
static int budget = -1;
static int torn_value = -1;
static int drop_address = -1;
static jmp_buf power_cut;

uint8_t storage_read_byte(uint8_t address)
{
    assert(address < EEPROM_BYTES);
    return memory[address];
}

void storage_write_byte(uint8_t address, uint8_t value)
{
    assert(address < EEPROM_BYTES);
    if (memory[address] == value)
        return;
    if ((int)writes == budget) {
        if (torn_value >= 0)
            memory[address] = (uint8_t)torn_value;
        longjmp(power_cut, 1);
    }
    ++writes;
    if (address != drop_address)
        memory[address] = value;
}

static void prepare_storage(const uint8_t values[CHANNELS])
{
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        memory[c * RECORD_BYTES] = values[c];
        memory[c * RECORD_BYTES + 1] = (uint8_t)~values[c];
        memory[c * RECORD_BYTES + 2] = COMMITTED;
    }
    writes = 0;
    budget = torn_value = drop_address = -1;
}

static unsigned ticks(Control *s, uint8_t raw, unsigned count)
{
    unsigned events = 0;
    while (count--)
        events += control_tick(s, raw) != 0;
    return events;
}

static Control fresh_control(void)
{
    Control s = {.level = {127, 127, 127}};
    control_reset_buttons(&s);
    assert(ticks(&s, 0, DEBOUNCE_TICKS) == 0);
    return s;
}

static void test_buttons(void)
{
    Control s = {.level = {127, 127, 127}};
    control_reset_buttons(&s);
    /* All keys held at startup: no action, even after the repeat interval. */
    assert(ticks(&s, BUTTON_MASK, HOLD_TICKS + REPEAT_TICKS) == 0);
    assert(s.level[0] == 127 && s.level[1] == 127 && s.level[2] == 127);
    ticks(&s, 0, DEBOUNCE_TICKS);
    /* Bounce never lasts long enough to be accepted. */
    for (unsigned i = 0; i < 10; ++i) {
        ticks(&s, 1, DEBOUNCE_TICKS - 1);
        ticks(&s, 0, DEBOUNCE_TICKS - 1);
    }
    assert(s.level[0] == 127);
    ticks(&s, 1, DEBOUNCE_TICKS);
    assert(s.level[0] == 131);
    ticks(&s, 1, HOLD_TICKS - 1);
    assert(s.level[0] == 131);
    ticks(&s, 1, 1);
    assert(s.level[0] == 135);
    ticks(&s, 1, REPEAT_TICKS);
    assert(s.level[0] == 139);
    ticks(&s, 0, DEBOUNCE_TICKS);
    ticks(&s, 0, HOLD_TICKS);
    assert(s.level[0] == 139);

    s = fresh_control();
    s.level[0] = 253;
    ticks(&s, 1, DEBOUNCE_TICKS);
    assert(s.level[0] == 255);
    s = fresh_control();
    s.level[0] = 2;
    ticks(&s, 2, DEBOUNCE_TICKS);
    assert(s.level[0] == 0);

    s = fresh_control();
    /* Both red keys suppress red, while green INC and blue DEC still work. */
    ticks(&s, 3 | 4 | 32, DEBOUNCE_TICKS + HOLD_TICKS);
    assert(s.level[0] == 127 && s.level[1] == 135 && s.level[2] == 119);
    /* A held STORE fires once; simultaneous change is included in the snapshot. */
    s = fresh_control();
    assert(ticks(&s, STORE_MASK | 1, DEBOUNCE_TICKS) == 1);
    assert(s.level[0] == 131);
    assert(ticks(&s, STORE_MASK, HOLD_TICKS) == 0);
    ticks(&s, 0, DEBOUNCE_TICKS);
    assert(ticks(&s, STORE_MASK, DEBOUNCE_TICKS) == 1);

    /* Resetting buttons after confirmation must preserve levels and inhibit holds. */
    control_reset_buttons(&s);
    uint8_t old = s.level[0];
    assert(ticks(&s, STORE_MASK | 1, HOLD_TICKS * 2) == 0);
    assert(s.level[0] == old);
    ticks(&s, 0, DEBOUNCE_TICKS);
    ticks(&s, 1, DEBOUNCE_TICKS);
    assert(s.level[0] == (uint8_t)(old + 4));
}

static void test_level_limits(void)
{
    /* Every starting value, every channel and both directions: no wrapping,
     * saturating final step, no effect on the other channels. */
    for (unsigned initial = 0; initial <= 255; ++initial) {
        for (uint8_t c = 0; c < CHANNELS; ++c) {
            for (uint8_t decreasing = 0; decreasing < 2; ++decreasing) {
                Control s = fresh_control();
                s.level[c] = (uint8_t)initial;
                uint8_t key = (uint8_t)(1U << (2 * c + decreasing));
                uint8_t expected = (uint8_t)initial;
                if (!decreasing)
                    expected = initial > 251 ? 255 : (uint8_t)(initial + 4);
                else
                    expected = initial < 4 ? 0 : (uint8_t)(initial - 4);
                ticks(&s, key, DEBOUNCE_TICKS);
                assert(s.level[c] == expected);
                /* Enough repeats to reach either limit from any initial value. */
                ticks(&s, key, HOLD_TICKS + 65 * REPEAT_TICKS);
                uint8_t limit = decreasing ? 0 : 255;
                assert(s.level[c] == limit);
                ticks(&s, key, 3 * REPEAT_TICKS);
                assert(s.level[c] == limit);
                ticks(&s, 0, DEBOUNCE_TICKS);
                ticks(&s, key, DEBOUNCE_TICKS);
                assert(s.level[c] == limit); /* Same key again still has no effect. */
                ticks(&s, 0, DEBOUNCE_TICKS);
                uint8_t inverse_key = (uint8_t)(1U << (2 * c + !decreasing));
                ticks(&s, inverse_key, DEBOUNCE_TICKS);
                assert(s.level[c] == (uint8_t)(decreasing ? limit + 4 : limit - 4));
                for (uint8_t other = 0; other < CHANNELS; ++other)
                    if (other != c)
                        assert(s.level[other] == 127);
            }
        }
    }
}

static void test_solo_display(void)
{
    Control s = fresh_control();
    uint8_t visible[3];
    const uint8_t start[3] = {50, 100, 150};
    memcpy(s.level, start, 3);
    control_display(&s, 1, visible);
    assert(memcmp(visible, start, 3) == 0); /* Jumper alone does not change RGB. */
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        for (uint8_t dec = 0; dec < 2; ++dec) {
            s = fresh_control();
            memcpy(s.level, start, 3);
            ticks(&s, (uint8_t)(1U << (2 * c + dec)), DEBOUNCE_TICKS);
            ticks(&s, (uint8_t)(1U << (2 * c + dec)), STEP_DARK_TICKS);
            uint8_t saved[3];
            memcpy(saved, s.level, 3);
            control_display(&s, 0, visible); /* Jumper open, full mixture. */
            assert(memcmp(visible, saved, 3) == 0);
            control_display(&s, 1, visible); /* Live change to isolated preview. */
            for (uint8_t k = 0; k < CHANNELS; ++k)
                assert(visible[k] == (k == c ? saved[k] : 0));
            assert(memcmp(s.level, saved, 3) == 0);
            ticks(&s, 0, DEBOUNCE_TICKS);
            control_display(&s, 1, visible);
            assert(memcmp(visible, saved, 3) == 0); /* Release restores mixture. */
        }
    }
    s = fresh_control();
    ticks(&s, 1 | 4, DEBOUNCE_TICKS + STEP_DARK_TICKS); /* Red and green together. */
    control_display(&s, 1, visible);
    assert(visible[0] == 131 && visible[1] == 131 && visible[2] == 0);
    ticks(&s, 4, DEBOUNCE_TICKS); /* Red released, green still held. */
    control_display(&s, 1, visible);
    assert(visible[0] == 0 && visible[1] == s.level[1] && visible[2] == 0);
    s = fresh_control();
    ticks(&s, 3, DEBOUNCE_TICKS); /* INC+DEC: visible but unchanged. */
    control_display(&s, 1, visible);
    assert(visible[0] == 127 && visible[1] == 0 && visible[2] == 0);
    control_reset_buttons(&s);
    ticks(&s, 1, HOLD_TICKS); /* Unreleased startup/confirmation key ignored. */
    control_display(&s, 1, visible);
    assert(memcmp(visible, s.level, 3) == 0);
    ticks(&s, 0, DEBOUNCE_TICKS);
    s.level[0] = 255;
    ticks(&s, 1, DEBOUNCE_TICKS); /* Isolation also works at a boundary stop. */
    control_display(&s, 1, visible);
    assert(visible[0] == 255 && visible[1] == 0 && visible[2] == 0);
    ticks(&s, STORE_MASK | 1, DEBOUNCE_TICKS);
    const uint8_t old[3] = {127, 127, 127};
    prepare_storage(old);
    assert(storage_store(s.level) == 1); /* Save actual values, not visible zeros. */
    uint8_t loaded[3];
    assert(storage_load(loaded) == 7 && memcmp(loaded, s.level, 3) == 0);
    control_reset_buttons(&s);
    control_display(&s, 1, visible);
    assert(memcmp(visible, s.level, 3) == 0);
}

static void test_step_feedback(void)
{
    uint8_t visible[3];
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        for (uint8_t dec = 0; dec < 2; ++dec) {
            Control s = fresh_control();
            uint8_t key = (uint8_t)(1U << (2 * c + dec));
            ticks(&s, key, DEBOUNCE_TICKS);
            assert(s.level[c] == (dec ? 123 : 131));
            for (unsigned elapsed = 0; elapsed < STEP_DARK_TICKS; ++elapsed) {
                control_display(&s, 0, visible);
                for (uint8_t k = 0; k < CHANNELS; ++k)
                    assert(visible[k] == (k == c ? 0 : 127));
                control_display(&s, 1, visible);
                assert(visible[0] == 0 && visible[1] == 0 && visible[2] == 0);
                ticks(&s, key, 1);
            }
            control_display(&s, 0, visible);
            assert(memcmp(visible, s.level, 3) == 0);
            ticks(&s, key, HOLD_TICKS - STEP_DARK_TICKS);
            assert(s.dark[c] == STEP_DARK_TICKS); /* Repeated change also pulses. */
            assert(s.level[c] == (dec ? 119 : 135));
            control_reset_buttons(&s);
            assert(s.dark[c] == 0); /* Confirmation/rearm cancels pending feedback. */
        }
    }
    Control s = fresh_control();
    ticks(&s, 1, DEBOUNCE_TICKS);
    /* Another channel is still processed while the feedback timer progresses. */
    ticks(&s, 1 | 4, DEBOUNCE_TICKS);
    assert(s.level[0] == 131 && s.level[1] == 131 && s.dark[1]);
    const uint8_t old[3] = {127,127,127};
    prepare_storage(old);
    assert(storage_store(s.level) == 1);
    uint8_t stored[3];
    assert(storage_load(stored) == 7 && memcmp(stored, s.level, 3) == 0);
    for (uint8_t dec = 0; dec < 2; ++dec) {
        s = fresh_control();
        s.level[0] = dec ? 0 : 255;
        ticks(&s, dec ? 2 : 1, DEBOUNCE_TICKS + HOLD_TICKS);
        assert(s.dark[0] == 0); /* No change at the limit means no pulse. */
    }
}

static void test_storage(void)
{
    const uint8_t original[3] = {127, 127, 127};
    const uint8_t changed[3] = {0, 255, 42};
    uint8_t loaded[3];
    prepare_storage(original);
    assert(storage_load(loaded) == 7 && memcmp(loaded, original, 3) == 0);
    assert(storage_store(original) == 0 && writes == 0);
    assert(storage_store(changed) == 1 && writes == 12);
    assert(storage_load(loaded) == 7 && memcmp(loaded, changed, 3) == 0);
    assert(storage_store(changed) == 0 && writes == 12);
    /* Exactly one channel changes; the other records are untouched. */
    prepare_storage(original);
    const uint8_t red_only[3] = {131, 127, 127};
    assert(storage_store(red_only) == 1 && writes == 4);

    memset(memory, 255, sizeof memory);
    writes = 0;
    assert(storage_load(loaded) == 0);
    assert(memcmp(loaded, original, 3) == 0 && writes == 0);
    /* Invalid EEPROM must be repaired on STORE, even at fallback level 127. */
    assert(storage_store(original) == 1);
    assert(storage_load(loaded) == 7);

    /* Every single-bit error in each stored record must invalidate its channel. */
    for (unsigned address = 0; address < EEPROM_BYTES; ++address) {
        for (unsigned bit = 0; bit < 8; ++bit) {
            prepare_storage(changed);
            memory[address] ^= (1U << bit);
            uint8_t mask = storage_load(loaded);
            assert(mask == (uint8_t)(7U ^ (1U << (address / RECORD_BYTES))));
            for (unsigned c = 0; c < CHANNELS; ++c)
                assert(loaded[c] == (c == address / RECORD_BYTES ? 127 : changed[c]));
        }
    }
    prepare_storage(original);
    drop_address = 0;
    assert(storage_store(changed) == 2); /* No successful-store confirmation. */
}

static void test_interrupted_writes(void)
{
    const uint8_t original[3] = {127, 127, 127};
    const uint8_t changed[3] = {0, 255, 42};
    /* Interrupt before every byte and with EVERY possible torn-byte result. */
    for (int cut = 0; cut < 12; ++cut) {
        for (int partial = -1; partial < 256; ++partial) {
            prepare_storage(original);
            budget = cut;
            torn_value = partial;
            if (setjmp(power_cut) == 0) {
                storage_store(changed);
                assert(!"Expected simulated power loss");
            }
            budget = -1;
            uint8_t loaded[3];
            uint8_t valid = storage_load(loaded);
            for (unsigned c = 0; c < CHANNELS; ++c) {
                if (valid & (1U << c))
                    assert(loaded[c] == original[c] || loaded[c] == changed[c]);
                else
                    assert(loaded[c] == DEFAULT_LEVEL);
            }
            writes = 0;
            assert(storage_store(changed) <= 1); /* Last torn commit may already be valid. */
            assert(storage_load(loaded) == 7 && memcmp(loaded, changed, 3) == 0);
        }
    }
}

int main(void)
{
    test_buttons();
    test_level_limits();
    test_solo_display();
    test_step_feedback();
    test_storage();
    test_interrupted_writes();
    puts("OK: debounce, startup/rearm, repeat, boundary stops/reversal, simultaneous keys, SOLO preview, step feedback, STORE,");
    puts("EEPROM defaults/integrity/write suppression and 3084 power-loss scenarios.");
    return 0;
}

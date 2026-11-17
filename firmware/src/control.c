#include "control.h"

void control_reset_buttons(Control *s)
{
    s->stable = BUTTON_MASK;
    s->armed = 0;
    for (uint8_t i = 0; i < BUTTONS; ++i)
        s->debounce[i] = 0;
    for (uint8_t i = 0; i < CHANNELS; ++i) {
        s->direction[i] = 0;
        s->repeat[i] = 0;
        s->dark[i] = 0;
    }
}

uint8_t control_tick(Control *s, uint8_t raw)
{
    for (uint8_t c = 0; c < CHANNELS; ++c)
        if (s->dark[c])
            --s->dark[c];
    uint8_t previous = s->stable;
    for (uint8_t i = 0, bit = 1; i < BUTTONS; ++i, bit <<= 1) {
        if ((raw ^ s->stable) & bit) {
            if (++s->debounce[i] >= DEBOUNCE_TICKS) {
                s->stable ^= bit;
                s->debounce[i] = 0;
            }
        } else {
            s->debounce[i] = 0;
        }
    }
    s->armed |= (uint8_t)~s->stable & BUTTON_MASK;
    uint8_t usable = s->stable & s->armed;
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        /* Both physical buttons suppress a channel even if one is not armed. */
        uint8_t pair = (s->stable >> (2 * c)) & 3;
        uint8_t direction = (usable >> (2 * c)) & 3;
        if (pair == 3 || direction == 0) {
            s->direction[c] = 0;
            continue;
        }
        uint8_t change = 0;
        if (direction != s->direction[c]) {
            s->direction[c] = direction;
            s->repeat[c] = HOLD_TICKS;
            change = 1;
        } else if (--s->repeat[c] == 0) {
            s->repeat[c] = REPEAT_TICKS;
            change = 1;
        }
        /* Saturate the last step at 0/255, then stay there until reversed. */
        if (change) {
            uint8_t old = s->level[c];
            if (direction == 1) {
                s->level[c] = s->level[c] > 255U - LEVEL_STEP
                    ? 255U : s->level[c] + LEVEL_STEP;
            } else {
                s->level[c] = s->level[c] < LEVEL_STEP
                    ? 0U : s->level[c] - LEVEL_STEP;
            }
            if (s->level[c] != old)
                s->dark[c] = STEP_DARK_TICKS;
        }
    }
    return usable & (uint8_t)~previous & STORE_MASK;
}

void control_display(const Control *s, uint8_t solo_mode, uint8_t visible[CHANNELS])
{
    uint8_t active = s->stable & s->armed & (STORE_MASK - 1U);
    for (uint8_t c = 0; c < CHANNELS; ++c) {
        uint8_t selected = (active >> (2 * c)) & 3U;
        visible[c] = (!s->dark[c] && (!solo_mode || !active || selected)) ? s->level[c] : 0;
    }
}

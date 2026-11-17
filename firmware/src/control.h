#ifndef CONTROL_H
#define CONTROL_H
#include <stdint.h>
#include "config.h"

typedef struct {
    uint8_t level[CHANNELS];
    uint8_t debounce[BUTTONS];
    uint8_t stable;
    uint8_t armed;
    uint8_t direction[CHANNELS];
    uint8_t repeat[CHANNELS];
    uint8_t dark[CHANNELS];
} Control;

/* Preserve levels; require a debounced release before accepting each button. */
void control_reset_buttons(Control *state);
/* Call once per polling tick. raw_pressed: bit 0..5 = INC/DEC pairs; bit 6 = STORE.
 * Returns one STORE press event, after applying simultaneous level changes. */
uint8_t control_tick(Control *state, uint8_t raw_pressed);
/* Derive visible RGB without changing the actual levels stored by STORE.
 * solo_mode: show only channels with accepted, debounced INC/DEC holds. */
void control_display(const Control *state, uint8_t solo_mode, uint8_t visible[CHANNELS]);
#endif

#ifndef CONFIG_H
#define CONFIG_H

#ifndef F_CPU
#define F_CPU 1000000UL
#endif
/* Oscillator fuse must select the internal 8 MHz RC oscillator. */
#if F_CPU != 1000000UL && F_CPU != 8000000UL
#error "Supported CPU clocks: 1 MHz and 8 MHz (internal 8 MHz RC oscillator)"
#endif
#ifndef PWM_PRESCALER
#define PWM_PRESCALER 1UL
#endif
#if PWM_PRESCALER != 1 && PWM_PRESCALER != 8
#error "PWM_PRESCALER must be 1 or 8"
#endif
/* Phase-correct PWM: overflow every 510 timer clocks. Poll near 5 ms. */
#define PWM_OVERFLOW_CYCLES (510UL * PWM_PRESCALER)
#define POLL_DIVIDER ((F_CPU + PWM_OVERFLOW_CYCLES * 100UL) / (PWM_OVERFLOW_CYCLES * 200UL))
#if POLL_DIVIDER < 1 || POLL_DIVIDER > 255
#error "Polling divider out of range"
#endif
#define TICKS_FOR_MS(ms) (((ms) * (F_CPU / 1000UL) + POLL_DIVIDER * PWM_OVERFLOW_CYCLES / 2UL) / (POLL_DIVIDER * PWM_OVERFLOW_CYCLES))
#define DEBOUNCE_TICKS TICKS_FOR_MS(20UL)
#define HOLD_TICKS TICKS_FOR_MS(500UL)
#define REPEAT_TICKS TICKS_FOR_MS(100UL)
#define BLINK_TICKS TICKS_FOR_MS(200UL)
#ifndef STEP_DARK_MS
#define STEP_DARK_MS 15UL
#endif
#define STEP_DARK_TICKS TICKS_FOR_MS(STEP_DARK_MS)
#if STEP_DARK_TICKS < 1 || STEP_DARK_TICKS > 255
#error "Step feedback duration out of range"
#endif
#if DEBOUNCE_TICKS < 1 || HOLD_TICKS > 255 || REPEAT_TICKS < 1 || BLINK_TICKS > 255
#error "UI timing does not fit counters"
#endif
#define CHANNELS 3
#define BUTTONS 7
#define BUTTON_MASK 0x7fU
#define STORE_MASK 0x40U
#define DEFAULT_LEVEL 127U
#define LEVEL_STEP 4U
#endif

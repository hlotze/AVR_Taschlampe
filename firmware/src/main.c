#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "control.h"
#include "storage.h"

#define SOLO_PIN PB0
#define PWM_PINS (_BV(PB2) | _BV(PB3) | _BV(PB4))
/* The .eep output programs this data separately from Flash. */
uint8_t EEMEM initial_levels[EEPROM_BYTES] = {
    127, 128, COMMITTED, 127, 128, COMMITTED, 127, 128, COMMITTED
};
static Control control;
static volatile uint8_t tick_ready;
static volatile uint8_t overflow_count;

/* Keep UI timing tied to hardware, even while the main loop computes. */
ISR(TIMER0_OVF_vect)
{
    if (++overflow_count >= POLL_DIVIDER) {
        overflow_count = 0;
        tick_ready = 1;
    }
}

static void restart_ticks(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        tick_ready = 0;
        overflow_count = 0;
        TIFR = _BV(TOV0);
    }
}

/* AVR-libc recommended early watchdog shutdown; .init3 falls through. */
void early_watchdog_off(void) __attribute__((naked, used, section(".init3")));
void early_watchdog_off(void)
{
    MCUSR = 0;
    wdt_disable();
}

uint8_t storage_read_byte(uint8_t address)
{
    return eeprom_read_byte(&initial_levels[address]);
}

void storage_write_byte(uint8_t address, uint8_t value)
{
    eeprom_update_byte(&initial_levels[address], value);
    eeprom_busy_wait(); /* A write returns before the physical burn completes. */
}

static void pwm_set(uint8_t red, uint8_t green, uint8_t blue)
{
    OCR0A = blue;
    OCR1A = green;
    OCR1B = red;
}

static void show_current_levels(void)
{
    uint8_t visible[CHANNELS];
    control_display(&control, !(PINB & _BV(SOLO_PIN)), visible);
    pwm_set(visible[0], visible[1], visible[2]);
}

static void pwm_init(void)
{
    /* NPN stages are active HIGH. Keep all outputs LOW until levels are loaded. */
    PORTB &= (uint8_t)~PWM_PINS;
    DDRB |= PWM_PINS;
    TCCR0A = _BV(WGM00); /* Phase correct, TOP = 255; 0/255 are fully OFF/ON. */
    TCCR0B = 0;
    TCCR1A = _BV(WGM10); /* Timer 1 mode 1: phase correct 8 bit. */
    TCCR1B = 0;
    TCNT0 = 0;
    TCNT1 = 0;
    pwm_set(control.level[0], control.level[1], control.level[2]);
    TCCR0A |= _BV(COM0A1);
    TCCR1A |= _BV(COM1A1) | _BV(COM1B1);
#if PWM_PRESCALER == 1
    TCCR0B = _BV(CS00);
    TCCR1B = _BV(CS10);
#else
    TCCR0B = _BV(CS01);
    TCCR1B = _BV(CS11);
#endif
    TIFR = _BV(TOV0);
    TIMSK = _BV(TOIE0);
}

static void wait_tick(void)
{
    while (!tick_ready) {}
    tick_ready = 0;
}

static void wait_blink_phase(void)
{
    for (uint8_t i = 0; i < BLINK_TICKS; ++i)
        wait_tick();
}

static void confirm_store(void)
{
    restart_ticks(); /* Discard ticks accumulated during EEPROM writes. */
    for (uint8_t i = 0; i < 2; ++i) {
        pwm_set(127, 127, 127);
        wait_blink_phase();
        pwm_set(0, 0, 0);
        wait_blink_phase();
    }
    pwm_set(control.level[0], control.level[1], control.level[2]);
}

int main(void)
{
    /* Assumes oscillator fuse selects internal 8 MHz RC, not a board crystal. */
#if F_CPU == 1000000UL
    clock_prescale_set(clock_div_8);
#else
    clock_prescale_set(clock_div_1);
#endif
    PORTB &= (uint8_t)~PWM_PINS;
    DDRB |= PWM_PINS;
    DDRB &= (uint8_t)~_BV(SOLO_PIN);
    PORTB |= _BV(SOLO_PIN); /* JP1: PB0 to GND enables isolated channel preview. */
    DDRD &= (uint8_t)~BUTTON_MASK;
    PORTD |= BUTTON_MASK; /* PD0..PD6: switches to GND, internal pull-ups. */
    storage_load(control.level);
    control_reset_buttons(&control);
    pwm_init();
    sei();
    for (;;) {
        wait_tick();
        uint8_t store = control_tick(&control, (uint8_t)~PIND & BUTTON_MASK);
        show_current_levels();
        if (store) {
            /* STORE takes priority: do not stretch a 15-ms impulse while writing. */
            for (uint8_t c = 0; c < CHANNELS; ++c)
                control.dark[c] = 0;
            pwm_set(control.level[0], control.level[1], control.level[2]);
            uint8_t result = storage_store(control.level);
            if (result == 1)
                confirm_store();
            if (result != 0) {
                /* Ignore all keys until a fresh debounced release after STORE. */
                control_reset_buttons(&control);
                restart_ticks();
            }
        }
    }
}

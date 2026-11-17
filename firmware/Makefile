MCU := attiny2313
F_CPU ?= 1000000UL
PWM_PRESCALER ?= 1
# Prefer PATH, otherwise use an existing PlatformIO toolchain.
AVR_PREFIX ?= $(if $(shell command -v avr-gcc 2>/dev/null),avr-,$(if $(wildcard $(HOME)/.platformio/packages/toolchain-atmelavr/bin/avr-gcc),$(HOME)/.platformio/packages/toolchain-atmelavr/bin/avr-,avr-))
HOST_CC ?= cc
BUILD := build
TARGET := $(BUILD)/taschenlampe
SOURCES := src/main.c src/control.c src/storage.c
CFLAGS := -std=c11 -Os -Wall -Wextra -Werror -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DPWM_PRESCALER=$(PWM_PRESCALER) -ffunction-sections -fdata-sections -flto

.PHONY: all clean test size FORCE
all: $(TARGET).hex $(TARGET).eep size

$(BUILD):
	mkdir -p $@

# Rebuild when command-line clock/PWM configuration changes, too.
$(TARGET).elf: $(SOURCES) $(wildcard src/*.h) Makefile FORCE | $(BUILD)
	$(AVR_PREFIX)gcc $(CFLAGS) $(SOURCES) -Wl,--gc-sections,-Map,$(TARGET).map -o $@

$(TARGET).hex: $(TARGET).elf FORCE
	$(AVR_PREFIX)objcopy -O ihex -R .eeprom $< $@

$(TARGET).eep: $(TARGET).elf FORCE
	$(AVR_PREFIX)objcopy -O ihex -j .eeprom --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0 $< $@

size: $(TARGET).elf
	$(AVR_PREFIX)size -C --mcu=$(MCU) $<

# Host tests use the production control/storage code and a simulated EEPROM.
test: | $(BUILD)
	$(HOST_CC) -std=c11 -O1 -g -Wall -Wextra -Werror -fsanitize=address,undefined -DF_CPU=$(F_CPU) -DPWM_PRESCALER=$(PWM_PRESCALER) -Isrc tests/test_firmware.c src/control.c src/storage.c -o $(BUILD)/test_firmware
	./$(BUILD)/test_firmware

clean:
	rm -rf $(BUILD)

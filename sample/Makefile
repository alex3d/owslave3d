MCU=atmega8

CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump

# optimize for size:
CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os -fwhole-program -I../owslave -I.
#CFLAGS=-g -mmcu=$(MCU) -Wall -Wstrict-prototypes -Os --combine -fwhole-program -ffunction-sections -fdata-sections -Wl,--relax,--gc-sections -mcall-prologues

all_:
	mkdir build 2>/dev/null ; true
	@make DEVNAME=main all 
	@grep "__vector_9>:" build/main.lss -A50 | grep eor -m1 -A50 | grep -v eor | grep push -B50 -v -m1 ; true
	@grep "__vector_1>:" build/main.lss -A50 | grep eor -m1 -A50 | grep -v eor | grep push -B50 -v -m1 ; true
	@echo "\033[1m" ; $(CC) $(CFLAGS) -DPRINT_TIMINGS -E main.c 2>&1 | sed  's/^exec bc \(.*\)/\1/;tx;d;:x' | bc ; echo -n "\033[0m"
	@echo "\033[1m" ; du -b build/main.bin ; echo "\033[0m"

all: build/$(DEVNAME).hex build/$(DEVNAME).lss build/$(DEVNAME).bin

#-------------------
build/%.o : %.c Makefile $(wildcard *.h) $(wildcard ../owslave/*.h)
	$(CC) $(CFLAGS) -E $< -o $(patsubst %.c, build/%.c, $<)
	$(CC) $(CFLAGS) -c $< -o $(patsubst %.c, build/%.o, $<)

build/$(DEVNAME).out : build/$(DEVNAME).o
	$(CC) $(CFLAGS) -o $@ -Wl,-Map,build/$(DEVNAME).map,--cref $^
build/$(DEVNAME).hex : build/$(DEVNAME).out 
	$(OBJCOPY) -R .eeprom -O ihex $< $@
build/$(DEVNAME).lss : build/$(DEVNAME).out 
	$(OBJDUMP) -h -S $< > $@
build/$(DEVNAME).bin : build/$(DEVNAME).out 
	$(OBJCOPY) -O binary $< $@

#------------------
clean:
	rm -rf build
#-------------------


SPINCMP=openspin
CC=propeller-elf-gcc
OBJCOPY=propeller-elf-objcopy
RM=rm -rf

CFLAGS=-Wall -Os -mcmm

OBJS = \
main.o \
quadkeyboard.o \
quadkeyboard_driver.o \
quadvga.o \
quadvga_fw.o \
monsta.o

all:	monsta.binary

monsta.binary:	monsta.elf
	propeller-load -s $<
    
monsta.elf:	$(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.dat: %.spin
	$(SPINCMP) -c -o $@ $<

%_fw.o: %.dat
	$(OBJCOPY) -I binary -B propeller -O $(CC) $< $@
	
run:	monsta.binary
	proploader -s $< -t
	
clean:
	$(RM) *.o *.dat *.elf *.binary

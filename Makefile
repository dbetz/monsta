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
monsta.o \
glyphs.o

all:	monsta.binary mktiles

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
	
glyphs.c:	glyphs.txt mktiles
	./mktiles <$< >$@

TOOLCFLAGS=-Wall

mktiles:	mktiles.c
	cc $(TOOLCFLAGS) -o mktiles mktiles.c
	
run:	monsta.binary
	proploader -s $< -t
	
clean:
	$(RM) *.o *.dat *.elf *.binary

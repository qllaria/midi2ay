# Makefile for midi2ay
# Works with MinGW

CC	= gcc
CFLAGS	= -W -Wall -O3 -mtune=pentium2
LIBS	= -lm
Z80ASM	= pasmo

OBJS	= midi2ay.o midi.o converter.o output.o
BINS	= zx/program_tap.bin zx/program_ay.bin

all:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o midi2ay

clean:
	rm -f $(OBJS)
	rm -f headers.c
	rm -f $(BINS)
	rm -f z80_bin_generator.exe

%.o:	%.c midi2ay.h
	$(CC) $(CFLAGS) -c $<

output.o:	output.c midi2ay.h headers.c
	$(CC) $(CFLAGS) -c $<

headers.c:	z80_bin_generator.exe zx/program_tap.asm $(BINS)
	./z80_bin_generator.exe

z80_bin_generator.exe:	z80_bin_generator.c
	$(CC) $(CFLAGS) -o z80_bin_generator.exe z80_bin_generator.c

%.bin:	%.asm
	$(Z80ASM) $< $@

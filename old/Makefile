# Makefile para midi2ay
# Funciona con MinGW

CC	= gcc
CFLAGS	= -W -Wall -O3 -mtune=pentium2
LIBS	= -lm
Z80ASM	= pasmo

OBJS	= midi2ay.o midi.o conversión.o salida.o
BINS	= zx/programa_tap.bin zx/programa_ay.bin

all:	$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIBS) -o midi2ay

clean:
	rm -f $(OBJS)
	rm -f cabeceras.c
	rm -f $(BINS)
	rm -f chapucilla.exe

%.o:	%.c midi2ay.h
	$(CC) $(CFLAGS) -c $<

salida.o:	salida.c midi2ay.h cabeceras.c
	$(CC) $(CFLAGS) -c $<

# Aquí viene lo bueno...

cabeceras.c:	chapucilla.exe zx/programa_tap.asm $(BINS)
	./chapucilla.exe

chapucilla.exe:	chapucilla.c
	$(CC) $(CFLAGS) -o chapucilla.exe chapucilla.c

%.bin:	%.asm
	$(Z80ASM) $< $@

# midi2ay
Adapts MIDI files for playing in the ZX Spectrum AY sound chip.
## Introduction
This is a quick-and-dirty program I wrote back in 2005. It takes a MIDI file and converts it to a format playable by a ZX Spectrum 128:
- AY
- TAP
- assembly source
- OUT (a custom format)

There are two versions of the source code: the one in [old](old) folder (the original source code, in Spanish) and the one in [src](src) folder (the same but with identifiers, comments, etc. translated into English).

The original binary and readme are included in the [release](release) folder.
## Building
This was originally built with some old version of MinGW; it should be pretty easy to adapt it to your favourite compiler.

The [Makefile](src/Makefile) file builds the program in two steps:
- The first builds and runs the z80_bin_generator executable, which generates the file [headers.c](src/headers.c) from the assembly sources in [zx](src/zx).
- The second builds the midi2ay executable.

Since a pre-built [headers.c](src/headers.c) file is already included the first step can be omitted, unless you change the asm files in [zx](src/zx). In this case you'll need the [pasmo](https://pasmo.speccy.org) Z80 assembler available in the build path.
## Usage
```
Usage: midi2ay [options] <MIDI file> [output file]

[options]:
  -tap  create ZX Spectrum tape (default)
  -asm  create assembly source code
  -out  create list of OUTs
```

OUT files are a list of groups of 4 bytes, each one containing the following fields:

 - time (2 bytes, Intel byte order): 1/50th seconds elapsed since previous write
 - register (1 byte): AY register to write in
 - value (1 byte): value to write into the register
## The algorithm
The program tries to play a MIDI (which can have up to 2048 simultaneous notes playing) in an AY chip with only three channels. In order to achieve this, every 1/50th of a second it checks the notes playing at that moment, assigns to each one a weight and selects the three notes with higher weight, discarding the others. If there's a rythm track, it's ignored.

The weight is calculated by multiplying the MIDI note and velocity values. Thus louder notes will get higher weight and higher-pitched notes will too (favouring melody lines over bass lines, hopefully).
## Possible improvements
Just in case anyone wants to use this program as a starting point for something more sophisticated:
- Use arpeggio to multiply the number of channels available.
- A way of manually selecting the priority of tracks, overriding the weight calculation. Maybe a GUI with a list of the tracks where you can order them by priority and mute unwanted ones.
- Play the rythm track. A table of rythm instruments adapted for the AY chip would be necessary.
- Make the program less "Spectrum-centric" and output formats for other computers that used AY-compatible chips.
## License
You can use this software and its source code in any way you want.

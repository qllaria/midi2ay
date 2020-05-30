midi2ay 0.1
(c) 2005 Quique Llaría <qllaria@yahoo.es>

First version!
This software is a piece of buggy shit, use it at your own risk.

Usage: midi2ay [options] <MIDI file> [output file]

[options]:
  -tap  create ZX Spectrum tape (default)
  -asm  create assembly source code
  -out  create list of OUTs


Format of .out files:

They're a list of groups of 4 bytes, each one containing the following fields:

 - time (2 bytes, Intel byte order): 1/50th seconds elapsed since previous write
 - register (1 byte): AY register to write in
 - value (1 byte): value to write into the register

#ifndef __MIDI2AY_H__
#define __MIDI2AY_H__

// midi.c

extern char         channels[16][128];
extern int          used_channel[16];
extern short        tone[16];
extern double       delta_freq;
extern unsigned int delta_abs;

int initMidi(char *midi, int size);
int readMidi(void);

// converter.c

extern char ay_state[3][3];
extern char ay_new[3][3];

void initConversion(void);
void conversion(void);

// output.c

typedef enum{ AY, OUT, TAP, ASM } t_output_format;

extern t_output_format output_format;

void initOutput(char *name, char *title);
void output(void);
void endOutput(void);

#endif

#ifndef __MIDI2AY_H__
#define __MIDI2AY_H__

// midi.c

extern char         canales[16][128];
extern int          canal_usado[16];
extern short        tono[16];
extern double       frec_delta;
extern unsigned int delta_abs;

int iniciaMidi(char *midi, int tamanho);
int leeMidi(void);

// conversión.c

extern char estado_ay[3][3];
extern char nuevo_ay[3][3];

void iniciaConversion(void);
void conversion(void);

// salida.c

typedef enum{ AY, OUT, TAP, ASM } t_formato_salida;

extern t_formato_salida formato_salida;

void iniciaSalida(char *nombre, char *titulo);
void salida(void);
void terminaSalida(void);

#endif

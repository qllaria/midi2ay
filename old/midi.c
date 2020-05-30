#include <stdlib.h>
#include <string.h>
#include "midi2ay.h"

// Variables públicas:
// contienen el estado del sintetizador en un instante dado.

char         canales[16][128]; // "rollo de pianola"
int          canal_usado[16];  // para acelerar las búsquedas de notas
short        tono[16];         // rueda de tono para cada canal
double       frec_delta;       // tempo (en pulsos por segundo)
unsigned int delta_abs;        // pulsos desde la última lectura

// Asquerosos macros para leer datos en formato extremista mayor
#define LEE2(p) ((*((unsigned char *)(p))<<8)+*((unsigned char *)(p)+1))
#define LEE3(p) ((LEE2(p)<<8)+*((unsigned char *)(p)+2))
#define LEE4(p) ((LEE2(p)<<16)+LEE2((unsigned char *)(p)+2))

typedef struct {
        char         *p;
        unsigned int  tamanho;
        unsigned int  delta;
        char          c_ant;
} pista_t;

static pista_t *pistas;
static double   delta_q;
static int      n_pistas;

static int leeLongVar(int pista)
{
        unsigned int  result = 0;
        char         *p = pistas[pista].p;

        do
                result=(result<<7)+(*p & 0x7f);
        while(*p++ & 0x80);
        pistas[pista].p=p;
        return result;
}

static void procesaComandoMidi(int pista)
{
        char c;

        // Lee el comando MIDI, que puede no aparecer
        if(!(*pistas[pista].p & 0x80))
                c=pistas[pista].c_ant;
        else
                pistas[pista].c_ant=c=*pistas[pista].p++;

        // Vamos a ver el tipo de comando:
        switch(c & 0xf0) {

        // Apagar nota
        case 0x80:
                canales[c & 0x0f][(int)*pistas[pista].p]=-1;
                pistas[pista].p+=2;
                break;

        // Encender nota
        case 0x90:
                canales[c & 0x0f][(int)pistas[pista].p[0]]=pistas[pista].p[1];
                canal_usado[c & 0x0f]=1;
                pistas[pista].p+=2;
                break;

        // Cambiar el volumen de una nota que está sonando.
        // Como muchos reproductores no hacen caso de este comando,
        // pues nosotros tampoco.
        case 0xa0:

        // Cambio de control. Creo que sirve para enviar datos para
        // dispositivos raros (con pedales, teléfonos móviles, etc.)
        case 0xb0:
                pistas[pista].p+=2;
                break;

        // Cambio de instrumento. Nosotros sólo tenemos uno (Pitido Cojonero).
        case 0xc0:

        // Cambiar el volumen para todo el canal. O algo así.
        case 0xd0:
                pistas[pista].p++;
                break;

        // Cambiar el tono de un canal
        case 0xe0:
        	tono[c & 0x0f]=pistas[pista].p[0]+(pistas[pista].p[1]<<7);
                pistas[pista].p+=2;
                break;

        // Eventos meta y sysex:
        case 0xf0:

                // Meta-eventos
                if(c==(char)0xff) {
			int l;

                        c=*pistas[pista].p++;
                        l=leeLongVar(pista);
                        switch(c) {

                        // Fin de pista
                        case 0x2f:
                                pistas[pista].delta=0xffffffff;
                                return;

                        // Fijar tempo
                        case 0x51:
                                if(delta_q!=-1)
                                        frec_delta=1000000*delta_q/
                                                (double)LEE3(pistas[pista].p);
                                break;

                        }
                        pistas[pista].p+=l;
                }

                // Eventos exclusivos del sistema:
                // ¿es así como se saltan?
                else
                        pistas[pista].p+=leeLongVar(pista);

                break;
        }

        // Leemos el delta del siguiente comando
        pistas[pista].delta=leeLongVar(pista);

}

int iniciaMidi(char *midi, int tamanho)
{
        int i, l;

        // Lee la cabecera del MIDI
        if((tamanho<14) || (LEE4(midi)!=0x4d546864)) // "MThd"
                return -1;
        if((i=LEE2(midi+12)) & 0x8000) { // SMPTE
                delta_q=-1.0;
                frec_delta=(double)(-(i>>8)*(i & 0xff));
        }
        else { // PPQN
                delta_q=(double)i;
                frec_delta=2.0*delta_q; // tempo=120 por defecto
        }
        n_pistas=LEE2(midi+10);
        midi+=14;
        tamanho-=14;

        // Va leyendo las pistas y rellenando la estructura pistas
        if((pistas=(pista_t *)malloc(sizeof(pista_t)*n_pistas))==NULL)
                return -2;
        i=0;
        while(i<n_pistas) {
                if((tamanho-=8)<0)
                        break;
                if((l=LEE4(midi+4))>tamanho)
                        l=tamanho;
                if(LEE4(midi)==0x4d54726b) { // "MTrk"
                        pistas[i].p       = midi+8;
                        pistas[i].tamanho = l;
                        pistas[i].delta   = leeLongVar(i);
                        pistas[i].c_ant   = 0;
                        i++;
                }
                tamanho-=l;
                midi+=l+8;
        }
        n_pistas=i;

        // Inicia variables diversas
        delta_abs=0;
        memset(canales, -1, sizeof(canales));
        for(i=0; i<16; i++) {
		canal_usado[i]=0;
                tono[i]=0x2000;
        }

        return 0;
}

int leeMidi(void)
{
        int i, pista;

        // Busca la pista más atrasada
        pista=0;
        delta_abs=pistas[0].delta;
        for(i=1; i<n_pistas; i++)
                if(pistas[i].delta<delta_abs) {
                        pista=i;
                        delta_abs=pistas[i].delta;
                }
        if(delta_abs==0xffffffff)
                return -1;

        // Procesa todos los comandos que se producen en ese instante
        for(i=0; i<n_pistas; i++)
                if(pistas[i].delta!=0xffffffff) {
                        pistas[i].delta-=delta_abs;
                        while(pistas[i].delta==0)
                                procesaComandoMidi(i);
	        }

        return 0;
}

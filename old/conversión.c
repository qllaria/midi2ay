#include <string.h>
#include "midi2ay.h"

// Variables públicas:
// contienen el estado previo y actual del chip AY

char estado_ay[3][3]; // canal, nota, volumen
char nuevo_ay[3][3];

static int losmejores[3];

static void inserta(int pos, char canal, char nota, char volumen, int puntuacion)
{
        int i;

        for(i=2; i>pos; i--) {
                nuevo_ay[i][0]=nuevo_ay[i-1][0];
                nuevo_ay[i][1]=nuevo_ay[i-1][1];
                nuevo_ay[i][2]=nuevo_ay[i-1][2];
                losmejores[i]=losmejores[i-1];
        }
        nuevo_ay[pos][0]=canal;
        nuevo_ay[pos][1]=nota;
        nuevo_ay[pos][2]=volumen;
        losmejores[pos]=puntuacion;
}

void iniciaConversion(void)
{
        memset(estado_ay, 0, sizeof(estado_ay));
}

void conversion(void)
{
        int canal, nota, puntuacion;

        losmejores[0]=losmejores[1]=losmejores[2]=0;
        memcpy(nuevo_ay, estado_ay, sizeof(nuevo_ay));
        for(canal=0; canal<16; canal++)
                if((canal!=9) && (canal_usado[canal]))
	                for(nota=0; nota<128; nota++)
                                if(canales[canal][nota]!=-1) {
                                        puntuacion=canales[canal][nota]*nota;
                                        if(losmejores[0]<puntuacion) {
                                                inserta(0, canal, nota,
                                                        canales[canal][nota],
                                                        puntuacion);
                                                continue;
                                        }
                                        if(losmejores[1]<puntuacion) {
                                                inserta(1, canal, nota,
                                                        canales[canal][nota],
                                                        puntuacion);
                                                continue;
                                        }
                                        if(losmejores[2]<puntuacion) {
                                                inserta(2, canal, nota,
                                                        canales[canal][nota],
                                                        puntuacion);
                                                continue;
                                        }
	                        }
        for(canal=0; canal<3; canal++)
                if(losmejores[canal]==0)
                        nuevo_ay[canal][2]=0;
}

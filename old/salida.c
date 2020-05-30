#include <stdio.h>
#include <string.h>
#include <math.h>
#include "midi2ay.h"

#include "cabeceras.c"

// Variables públicas:

t_formato_salida formato_salida;

static char cabecera_tap[] = {
        0x13, 0x00, 0x00, 0x03, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFF, 0xFF,
        0x00, 0x80, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xff
};

static const char cabecera_ay[] = {
        0x5a, 0x58, 0x41, 0x59, 0x45, 0x4d, 0x55, 0x4c,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
        0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0x00, 0x00, // SongLength
        0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00,
        0x80, 0x00, 0x80, 0x01, 0x80, 0x00, 0xFF, 0xFF, // Length
        0x00, 0x04, 0x00, 0x00
};

static const char reinicio_ay[] = {
        0x00, 0x07, 0x38, 0x00, 0x08, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x10, 0x00
};

static short         frecuencias[128];
static unsigned int  delta_salida, t_total, tam_partitura;

static FILE         *f;
static char          vertedero[1024];
static int           pos_vertedero;
static char          xor;

static char calculaXor(const char *datos, int tamanho)
{
        char result = 0;

        while(tamanho--)
                result^=*datos++;
        return result;
}

static void sacaByte(char c)
{
        if(formato_salida==ASM) {
                if(pos_vertedero==9) {
                        vertedero[strlen(vertedero)-2]=0;
                        fprintf(f, "%s\n", vertedero);
                        sprintf(vertedero, "\tdb\t0%02xh, ",
                                (unsigned char)c);
                        pos_vertedero=1;
	        }
	        else {
                        sprintf(vertedero+strlen(vertedero),
                                "0%02xh, ", (unsigned char)c);
                        pos_vertedero++;
                }
                return;
        }

        vertedero[pos_vertedero++]=c;
        if(pos_vertedero==sizeof(vertedero)) {
                fwrite(vertedero, 1, sizeof(vertedero), f);
                pos_vertedero=0;
        }
        xor^=c;
        tam_partitura++;

}

static void sacaOut(unsigned int delta, int puerto, int valor)
{

        // En el formato OUT los deltas son de 16 bits
        if(formato_salida==OUT) {
                sacaByte(delta);
                sacaByte(delta>>8);
        }

        // En los otros son de 8 bits, así que hay que hacer un apaño
        else {

                // Cuidado, que la partitura no nos pete la memoria
                if((tam_partitura>0x7e00) && (puerto!=0xff))
                        return;

                // El apaño en sí mismo
                while(delta>255) {
                        sacaByte(255);
                        sacaByte(0xfe);
                        sacaByte(0x00);
                        delta-=255;
                }
                sacaByte(delta);

        }

	sacaByte(puerto);
	sacaByte(valor);

}

void iniciaSalida(char *nombre, char *titulo)
{
        int    octava, nota, i;
        double frec, semitono = pow(2.0, 1.0/12.0);

        // Crea la tabla de frecuencias
        i=-3;
        for(octava=0; octava<11; octava++) {
                frec=13.75*(double)(1<<octava); // 27.5 Hz = La, octava 0
                for(nota=0; nota<12; nota++) {
                        if((i>=0) && (i<=127))
                                frecuencias[i]=(short)(110830.0/frec+0.5);
                        i++;
                        frec*=semitono;
                }
        }

        f=fopen(nombre, formato_salida==ASM ? "wt" : "wb");
        switch(formato_salida) {

        case ASM:
                fprintf(f, programa_asm);
                strcpy(vertedero, "\tdb\t");
                tam_partitura=sizeof(programa_tap);
                break;

        case AY:
                fwrite(cabecera_ay, 1, sizeof(cabecera_ay), f);
                fwrite(programa_ay, 1, sizeof(programa_ay), f);
                tam_partitura=sizeof(programa_ay);
                break;

        case OUT:
                break;

        case TAP:
                for(i=0; ; i++) {
                        if((i==10) || (titulo[i]==0))
                                break;
                        if(titulo[i] & 0x80)
                                titulo[i]='_';
                }
                memcpy(cabecera_tap+4, titulo, i);
                fwrite(cargador_tap, 1, sizeof(cargador_tap), f);
                fwrite(cabecera_tap, 1, sizeof(cabecera_tap), f);
                fwrite(programa_tap, 1, sizeof(programa_tap), f);
                xor=0xff ^ calculaXor(programa_tap, sizeof(programa_tap));
                tam_partitura=sizeof(programa_tap);
                break;

        }

        delta_salida=t_total=pos_vertedero=0;

        for(i=0; i<(int)sizeof(reinicio_ay); i+=3)
                sacaOut(reinicio_ay[i], reinicio_ay[i+1], reinicio_ay[i+2]);

}

void salida(void)
{
        unsigned int delta;
        int          canal, nota, nota2;

        // Convierte el delta a interrupciones
        delta_salida+=delta_abs;
        delta=(unsigned int)(50.0*(double)(delta_salida)/frec_delta+0.5);
        t_total+=delta;

        // Si algún registro AY ha cambiado de estado, sacamos el/los OUT
        for(canal=0; canal<3; canal++) {
                if(estado_ay[canal][1]!=nuevo_ay[canal][1]) {
                        nota=frecuencias[(int)nuevo_ay[canal][1]];
                        nota2=frecuencias[(int)estado_ay[canal][1]];
                        sacaOut(delta, canal*2, nota);
                        if((nota^nota2) & 0xff00)
                                sacaOut(0, canal*2+1, nota>>8);
                        delta=delta_salida=0;
                }
                if(estado_ay[canal][2]!=nuevo_ay[canal][2]) {
                        sacaOut(delta, 8+canal, nuevo_ay[canal][2]>>3);
                        delta=delta_salida=0;
                }
        }

}

void terminaSalida(void)
{

        // "Mensaje" fin de partitura
        if(formato_salida!=OUT)
                sacaOut(0, 0xff, 0x00);

        // Vacía el vertedero
        if(formato_salida==ASM) {
                vertedero[strlen(vertedero)-2]=0;
                fprintf(f, "%s\n\nend\tinicio\t", vertedero);
	}
	else
                if(pos_vertedero)
                        fwrite(vertedero, 1, pos_vertedero, f);

        // Rellena algunos campos tipo "longitud", "checksum", etc.
        switch(formato_salida) {

        case AY: {
                short s;

                s=(t_total<<8)+(unsigned char)(t_total>>8);
                fseek(f, 28, SEEK_SET);
                fwrite(&s, 1, 2, f);
                s=(tam_partitura<<8)+(unsigned char)(tam_partitura>>8);
                fseek(f, 46, SEEK_SET);
                fwrite(&s, 1, 2, f);
                break;
        }

        case TAP:
                fseek(f, 0, SEEK_END);
                fputc(xor, f);
                fseek(f, sizeof(cargador_tap)+14, SEEK_SET);
                fwrite(&tam_partitura, 1, 2, f);
                xor=calculaXor(cabecera_tap+2, 18);
                xor^=tam_partitura^(tam_partitura>>8);
                fseek(f, sizeof(cargador_tap)+20, SEEK_SET);
                fputc(xor, f);
                tam_partitura+=2;
                fwrite(&tam_partitura, 1, 2, f);
                break;

        default:
                break;

        }

        // Cierra el fichero
        fclose(f);

}

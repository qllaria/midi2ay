#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midi2ay.h"

static const char *opciones[4] = {
        "ay", "out", "tap", "asm"
};

static char *sacaNombre(char *nombre)
{
        char *result, *busca;

        busca=nombre+strlen(nombre)-1;
        while((*busca!='\\') && (*busca!='/') && (*busca!=':') &&
                (busca!=nombre))
                busca--;
        if(busca!=nombre)
                busca++;
        result=(char *)malloc(strlen(busca)+1);
        strcpy(result, busca);
        if((busca=strrchr(result, '.'))!=NULL)
                *busca=0;
        return result;
}

static char *cambiaExtension(char *nombre, const char *extension)
{
        char *result, *busca;

        result=(char *)malloc(strlen(nombre)+strlen(extension)+1);
        strcpy(result, nombre);
        busca=result+strlen(result)-1;
        while((*busca!='\\') && (*busca!='/') && (*busca!=':') &&
                (*busca!='.') && (busca!=result))
                busca--;
        if(*busca=='.')
                strcpy(busca+1, extension);
        else {
                strcat(result, ".");
                strcat(result, extension);
        }
        return result;
}

static void imprimeAyuda(void)
{
        printf("Usage: midi2ay [options] <MIDI file> [output file]\n"
               "\n"
               "[options]:\n"
               "  -tap  create ZX Spectrum tape (default)\n"
               "  -asm  create assembly source code\n"
               "  -out  create list of OUTs\n");
        exit(-1);
}

int main(int argc, char *argv[])
{
        FILE *f;
        char *midi;
        int   tamanho;
        char *nombre_salida;

        printf("midi2ay 0.1 --- (c) 2005 Quique Llar\241a <qllaria@yahoo.es>\n\n");

        if((argc<2) || (argc>4))
                imprimeAyuda();
        formato_salida=TAP;
        if(argv[1][0]=='-') {
		for(tamanho=0; tamanho<4; tamanho++)
                        if(!strcmp(argv[1]+1, opciones[tamanho])) {
                                formato_salida=tamanho;
                                argc--;
                                argv++;
                                break;
                        }
                if(tamanho==4)
                        imprimeAyuda();
        }
        if((argc!=2) && (argc!=3))
                imprimeAyuda();
        if(argc==3)
                nombre_salida=argv[2];
        else
                nombre_salida=cambiaExtension(argv[1],
                        opciones[formato_salida]);

        if((f=fopen(argv[1], "rb"))==NULL) {
                fprintf(stderr, "Error: no se pudo abrir %s\n", argv[1]);
                return -1;
        }
        fseek(f, 0, SEEK_END);
        tamanho=ftell(f);
        fseek(f, 0, SEEK_SET);
        midi=(char *)malloc(tamanho);
        fread(midi, 1, tamanho, f);
        fclose(f);

        if(iniciaMidi(midi, tamanho)) {
                fprintf(stderr, "Error: %s no es un fichero MIDI\n", argv[1]);
                return -1;
        }
        iniciaConversion();
        iniciaSalida(nombre_salida, sacaNombre(argv[1]));
        while(!leeMidi()) {
                conversion();
                salida();
                memcpy(estado_ay, nuevo_ay, sizeof(estado_ay));
        }
        terminaSalida();
        printf("%s created\n", nombre_salida);

        return 0;
}

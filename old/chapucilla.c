#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char nombre_asm[] = "zx/programa_tap.asm";
char nombre_car[] = "zx/carga.tap";
char nombre_tap[] = "zx/programa_tap.bin";
char nombre_ay[]  = "zx/programa_ay.bin";
char nombre_c[]   = "cabeceras.c";

char saludo[] = "// Generado automáticamente por 'chapucilla'\n\n";

unsigned char *p;
unsigned int   tam;

void carga(char *nombre)
{
        FILE *f;

        if((f=fopen(nombre, "rb"))==NULL) {
                fprintf(stderr, "Error: no se pudo abrir %s\n", nombre);
                exit(-1);
        }
        fseek(f, 0, SEEK_END);
        tam=ftell(f);
        fseek(f, 0, SEEK_SET);
        p=(unsigned char *)malloc(tam+1);
        fread(p, 1, tam, f);
        p[tam]=0;
        fclose(f);
}

void vuelca(FILE *f)
{
        unsigned char *tmp = p;
        int            i = 0;

        while(--tam) {
                fprintf(f, "0x%02x, ", *tmp++);
                if((++i % 8)==0)
                        fprintf(f, "\n        ");
        }
        fprintf(f, "0x%02x\n};\n\n", *tmp);
}

int main(void)
{
        FILE          *f;
        unsigned char *cuerpo;

        if((f=fopen(nombre_c, "wt"))==NULL) {
                fprintf(stderr, "Error: no se puede escribir en %s\n",
                        nombre_c);
                return -1;
        }
        fprintf(f, saludo);

        carga(nombre_asm);
        if((cuerpo=strstr(p, "; CUERPO"))==NULL) {
                fprintf(stderr, "Error: falta \"CUERPO\" en %s\n",
                        nombre_asm);
                return -1;
        }
        *cuerpo=0;
        tam=strlen(p)+1;
        fprintf(f, "static const char programa_asm[] = {\n        ");
        vuelca(f);
        free(p);

        carga(nombre_car);
        fprintf(f, "static const char cargador_tap[] = {\n        ");
        vuelca(f);
        free(p);

        carga(nombre_tap);
        fprintf(f, "static const char programa_tap[] = {\n        ");
        vuelca(f);
        free(p);

        carga(nombre_ay);
        fprintf(f, "static const char programa_ay[] = {\n        ");
        vuelca(f);
        free(p);

        fclose(f);
        return 0;
}

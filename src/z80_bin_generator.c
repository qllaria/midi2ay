#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char name_asm[] = "zx/program_tap.asm";
char name_loader[] = "zx/loader.tap";
char name_tap[] = "zx/program_tap.bin";
char name_ay[]  = "zx/program_ay.bin";
char name_c[]   = "headers.c";

char greeting[] = "// Automatically generated by 'z80_bin_generator'\n\n";

unsigned char *p;
unsigned int   size;

void load(char *name)
{
        FILE *f;

        if((f=fopen(name, "rb"))==NULL) {
                fprintf(stderr, "Error: could not open %s\n", name);
                exit(-1);
        }
        fseek(f, 0, SEEK_END);
        size=ftell(f);
        fseek(f, 0, SEEK_SET);
        p=(unsigned char *)malloc(size+1);
        fread(p, 1, size, f);
        p[size]=0;
        fclose(f);
}

void dump(FILE *f)
{
        unsigned char *tmp = p;
        int            i = 0;

        while(--size) {
                fprintf(f, "0x%02x, ", *tmp++);
                if((++i % 8)==0)
                        fprintf(f, "\n        ");
        }
        fprintf(f, "0x%02x\n};\n\n", *tmp);
}

int main(void)
{
        FILE          *f;
        unsigned char *body;

        if((f=fopen(name_c, "wt"))==NULL) {
                fprintf(stderr, "Error: could not write in %s\n", name_c);
                return -1;
        }
        fprintf(f, greeting);

        load(name_asm);
        if((body=strstr(p, "; BODY"))==NULL) {
                fprintf(stderr, "Error: \"BODY\" missing in %s\n", name_asm);
                return -1;
        }
        *body=0;
        size=strlen(p)+1;
        fprintf(f, "static const char program_asm[] = {\n        ");
        dump(f);
        free(p);

        load(name_loader);
        fprintf(f, "static const char loader_tap[] = {\n        ");
        dump(f);
        free(p);

        load(name_tap);
        fprintf(f, "static const char program_tap[] = {\n        ");
        dump(f);
        free(p);

        load(name_ay);
        fprintf(f, "static const char program_ay[] = {\n        ");
        dump(f);
        free(p);

        fclose(f);
        return 0;
}

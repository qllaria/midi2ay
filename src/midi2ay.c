#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "midi2ay.h"

static const char *options[4] = {
        "ay", "out", "tap", "asm"
};

static char *extractName(char *name)
{
        char *pName, *pSearch;

        pSearch=name+strlen(name)-1;
        while((*pSearch!='\\') && (*pSearch!='/') && (*pSearch!=':') && (pSearch!=name))
                pSearch--;
        if(pSearch!=name)
                pSearch++;
        pName=(char *)malloc(strlen(pSearch)+1);
        strcpy(pName, pSearch);
        if((pSearch=strrchr(pName, '.'))!=NULL)
                *pSearch=0;
        return pName;
}

static char *changeExtension(char *name, const char *extension)
{
        char *result, *pSearch;

        result=(char *)malloc(strlen(name)+strlen(extension)+1);
        strcpy(result, name);
        pSearch=result+strlen(result)-1;
        while((*pSearch!='\\') && (*pSearch!='/') && (*pSearch!=':') && (*pSearch!='.') && (pSearch!=result))
                pSearch--;
        if(*pSearch=='.')
                strcpy(pSearch+1, extension);
        else {
                strcat(result, ".");
                strcat(result, extension);
        }
        return result;
}

static void displayHelp(void)
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
        int   size;
        char *output_name;

        printf("midi2ay 0.1 --- (c) 2005 Quique Llar\241a <qllaria@yahoo.es>\n\n");

        if((argc<2) || (argc>4))
                displayHelp();
        output_format=TAP;
        if(argv[1][0]=='-') {
		for(size=0; size<4; size++)
                        if(!strcmp(argv[1]+1, options[size])) {
                                output_format=size;
                                argc--;
                                argv++;
                                break;
                        }
                if(size==4)
                        displayHelp();
        }
        if((argc!=2) && (argc!=3))
                displayHelp();
        if(argc==3)
                output_name=argv[2];
        else
                output_name=changeExtension(argv[1],
                        options[output_format]);

        if((f=fopen(argv[1], "rb"))==NULL) {
                fprintf(stderr, "Error: couldn't open %s\n", argv[1]);
                return -1;
        }
        fseek(f, 0, SEEK_END);
        size=ftell(f);
        fseek(f, 0, SEEK_SET);
        midi=(char *)malloc(size);
        fread(midi, 1, size, f);
        fclose(f);

        if(initMidi(midi, size)) {
                fprintf(stderr, "Error: %s is not a MIDI file\n", argv[1]);
                return -1;
        }
        initConversion();
        initOutput(output_name, extractName(argv[1]));
        while(!readMidi()) {
                conversion();
                output();
                memcpy(ay_state, ay_new, sizeof(ay_state));
        }
        endOutput();
        printf("%s created\n", output_name);

        return 0;
}

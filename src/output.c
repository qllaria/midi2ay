#include <stdio.h>
#include <string.h>
#include <math.h>
#include "midi2ay.h"

#include "headers.c"

// Public vars:

t_output_format output_format;

static char tap_header[] = {
        0x13, 0x00, 0x00, 0x03, 0x20, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFF, 0xFF,
        0x00, 0x80, 0x00, 0x80, 0xFF, 0xFF, 0xFF, 0xff
};

static const char ay_header[] = {
        0x5a, 0x58, 0x41, 0x59, 0x45, 0x4d, 0x55, 0x4c,
        0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
        0x00, 0x01, 0x02, 0x03, 0xFF, 0xFF, 0x00, 0x00, // SongLength
        0x00, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00,
        0x80, 0x00, 0x80, 0x01, 0x80, 0x00, 0xFF, 0xFF, // Length
        0x00, 0x04, 0x00, 0x00
};

static const char ay_reset[] = {
        0x00, 0x07, 0x38, 0x00, 0x08, 0x00, 0x00, 0x09,
        0x00, 0x00, 0x10, 0x00
};

static short         frequencies[128];
static unsigned int  output_delta, total_t, score_size;

static FILE         *f;
static char          buffer[1024];
static int           buffer_pos;
static char          xor;

static char calculateXor(const char *datos, int tamanho)
{
        char result = 0;

        while(tamanho--)
                result^=*datos++;
        return result;
}

static void outputByte(char c)
{
        if(output_format==ASM) {
                if(buffer_pos==9) {
                        buffer[strlen(buffer)-2]=0;
                        fprintf(f, "%s\n", buffer);
                        sprintf(buffer, "\tdb\t0%02xh, ",
                                (unsigned char)c);
                        buffer_pos=1;
	        }
	        else {
                        sprintf(buffer+strlen(buffer),
                                "0%02xh, ", (unsigned char)c);
                        buffer_pos++;
                }
                return;
        }

        buffer[buffer_pos++]=c;
        if(buffer_pos==sizeof(buffer)) {
                fwrite(buffer, 1, sizeof(buffer), f);
                buffer_pos=0;
        }
        xor^=c;
        score_size++;

}

static void outputOut(unsigned int delta, int port, int value)
{

        // With OUT format deltas are 16-bit
        if(output_format==OUT) {
                outputByte(delta);
                outputByte(delta>>8);
        }

        // With the others they are 8-bit, so we have to use a trick
        else {

                // Check if the score runs out of memory
                if((score_size>0x7e00) && (port!=0xff))
                        return;

                // The trick itself
                while(delta>255) {
                        outputByte(255);
                        outputByte(0xfe);
                        outputByte(0x00);
                        delta-=255;
                }
                outputByte(delta);

        }

	outputByte(port);
	outputByte(value);

}

void initOutput(char *name, char *title)
{
        int    octave, note, i;
        double freq, semitone = pow(2.0, 1.0/12.0);

        // Create freq table
        i=-3;
        for(octave=0; octave<11; octave++) {
                freq=13.75*(double)(1<<octave); // 27.5 Hz = A, octave 0
                for(note=0; note<12; note++) {
                        if((i>=0) && (i<=127))
                                frequencies[i]=(short)(110830.0/freq+0.5);
                        i++;
                        freq*=semitone;
                }
        }

        f=fopen(name, output_format==ASM ? "wt" : "wb");
        switch(output_format) {

        case ASM:
                fprintf(f, program_asm);
                strcpy(buffer, "\tdb\t");
                score_size=sizeof(program_tap);
                break;

        case AY:
                fwrite(ay_header, 1, sizeof(ay_header), f);
                fwrite(program_ay, 1, sizeof(program_ay), f);
                score_size=sizeof(program_ay);
                break;

        case OUT:
                break;

        case TAP:
                for(i=0; ; i++) {
                        if((i==10) || (title[i]==0))
                                break;
                        if(title[i] & 0x80)
                                title[i]='_';
                }
                memcpy(tap_header+4, title, i);
                fwrite(loader_tap, 1, sizeof(loader_tap), f);
                fwrite(tap_header, 1, sizeof(tap_header), f);
                fwrite(program_tap, 1, sizeof(program_tap), f);
                xor=0xff ^ calculateXor(program_tap, sizeof(program_tap));
                score_size=sizeof(program_tap);
                break;

        }

        output_delta=total_t=buffer_pos=0;

        for(i=0; i<(int)sizeof(ay_reset); i+=3)
                outputOut(ay_reset[i], ay_reset[i+1], ay_reset[i+2]);

}

void output(void)
{
        unsigned int delta;
        int          channel, note, note2;

        // Convert delta to interrupts
        output_delta+=delta_abs;
        delta=(unsigned int)(50.0*(double)(output_delta)/delta_freq+0.5);
        total_t+=delta;

        // If any AY register has changed, output the OUT(s)
        for(channel=0; channel<3; channel++) {
                if(ay_state[channel][1]!=ay_new[channel][1]) {
                        note=frequencies[(int)ay_new[channel][1]];
                        note2=frequencies[(int)ay_state[channel][1]];
                        outputOut(delta, channel*2, note);
                        if((note^note2) & 0xff00)
                                outputOut(0, channel*2+1, note>>8);
                        delta=output_delta=0;
                }
                if(ay_state[channel][2]!=ay_new[channel][2]) {
                        outputOut(delta, 8+channel, ay_new[channel][2]>>3);
                        delta=output_delta=0;
                }
        }

}

void endOutput(void)
{

        // End of score "message"
        if(output_format!=OUT)
                outputOut(0, 0xff, 0x00);

        // Empty buffer
        if(output_format==ASM) {
                buffer[strlen(buffer)-2]=0;
                fprintf(f, "%s\n\nend\tinit\t", buffer);
	}
	else
                if(buffer_pos)
                        fwrite(buffer, 1, buffer_pos, f);

        // Fill some fields like "length", "checksum", etc.
        switch(output_format) {

        case AY: {
                short s;

                s=(total_t<<8)+(unsigned char)(total_t>>8);
                fseek(f, 28, SEEK_SET);
                fwrite(&s, 1, 2, f);
                s=(score_size<<8)+(unsigned char)(score_size>>8);
                fseek(f, 46, SEEK_SET);
                fwrite(&s, 1, 2, f);
                break;
        }

        case TAP:
                fseek(f, 0, SEEK_END);
                fputc(xor, f);
                fseek(f, sizeof(loader_tap)+14, SEEK_SET);
                fwrite(&score_size, 1, 2, f);
                xor=calculateXor(tap_header+2, 18);
                xor^=score_size^(score_size>>8);
                fseek(f, sizeof(loader_tap)+20, SEEK_SET);
                fputc(xor, f);
                score_size+=2;
                fwrite(&score_size, 1, 2, f);
                break;

        default:
                break;

        }

        // Close the file
        fclose(f);

}

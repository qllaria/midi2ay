#include <stdlib.h>
#include <string.h>
#include "midi2ay.h"

// Public vars:
// hold the status of the synth at a given time.

char         channels[16][128]; // "piano roll"
int          used_channel[16];  // for speeding up note searchs
short        tone[16];         // pitch wheel for each channel
double       delta_freq;       // tempo (in beats per second)
unsigned int delta_abs;        // beats since last read

// Macros for reading big-endian data
#define READ2(p) ((*((unsigned char *)(p))<<8)+*((unsigned char *)(p)+1))
#define READ3(p) ((READ2(p)<<8)+*((unsigned char *)(p)+2))
#define READ4(p) ((READ2(p)<<16)+READ2((unsigned char *)(p)+2))

typedef struct {
        char         *p;
        unsigned int  size;
        unsigned int  delta;
        char          prev_c;
} track_t;

static track_t *tracks;
static double   delta_q;
static int      n_tracks;

static int readLongVar(int track)
{
        unsigned int  result = 0;
        char         *p = tracks[track].p;

        do
                result=(result<<7)+(*p & 0x7f);
        while(*p++ & 0x80);
        tracks[track].p=p;
        return result;
}

static void processMidiCommand(int track)
{
        char c;

        // Reads MIDI command, which may be omitted
        if(!(*tracks[track].p & 0x80))
                c=tracks[track].prev_c;
        else
                tracks[track].prev_c=c=*tracks[track].p++;

        // Check type of command:
        switch(c & 0xf0) {

        // Note off
        case 0x80:
                channels[c & 0x0f][(int)*tracks[track].p]=-1;
                tracks[track].p+=2;
                break;

        // Note on
        case 0x90:
                channels[c & 0x0f][(int)tracks[track].p[0]]=tracks[track].p[1];
                used_channel[c & 0x0f]=1;
                tracks[track].p+=2;
                break;

        // Change the volume of a note that is already playing.
        // Since many players ignore this command, so we do.
        case 0xa0:

        // Control change. I think it's for sending custom data to devices
        // (pedals, mobiles, etc.)
        case 0xb0:
                tracks[track].p+=2;
                break;

        // Instrument change. We only have one (Annoying Beep).
        case 0xc0:

        // Change the volume of the whole channel. Or something like that.
        case 0xd0:
                tracks[track].p++;
                break;

        // Change the pitch of a channel.
        case 0xe0:
        	tone[c & 0x0f]=tracks[track].p[0]+(tracks[track].p[1]<<7);
                tracks[track].p+=2;
                break;

        // Sysex and meta-events:
        case 0xf0:

                // Meta-events
                if(c==(char)0xff) {
			int l;

                        c=*tracks[track].p++;
                        l=readLongVar(track);
                        switch(c) {

                        // End of track
                        case 0x2f:
                                tracks[track].delta=0xffffffff;
                                return;

                        // Set tempo
                        case 0x51:
                                if(delta_q!=-1)
                                        delta_freq=1000000*delta_q/(double)READ3(tracks[track].p);
                                break;

                        }
                        tracks[track].p+=l;
                }

                // System exclusive events:
                // is this the way to skip them?
                else
                        tracks[track].p+=readLongVar(track);

                break;
        }

        // Read delta of next command
        tracks[track].delta=readLongVar(track);

}

int initMidi(char *midi, int size)
{
        int i, l;

        // Read MIDI header
        if((size<14) || (READ4(midi)!=0x4d546864)) // "MThd"
                return -1;
        if((i=READ2(midi+12)) & 0x8000) { // SMPTE
                delta_q=-1.0;
                delta_freq=(double)(-(i>>8)*(i & 0xff));
        }
        else { // PPQN
                delta_q=(double)i;
                delta_freq=2.0*delta_q; // tempo=120 by default
        }
        n_tracks=READ2(midi+10);
        midi+=14;
        size-=14;

        // Read tracks and fill track_t struct for each
        if((tracks=(track_t *)malloc(sizeof(track_t)*n_tracks))==NULL)
                return -2;
        i=0;
        while(i<n_tracks) {
                if((size-=8)<0)
                        break;
                if((l=READ4(midi+4))>size)
                        l=size;
                if(READ4(midi)==0x4d54726b) { // "MTrk"
                        tracks[i].p      = midi+8;
                        tracks[i].size   = l;
                        tracks[i].delta  = readLongVar(i);
                        tracks[i].prev_c = 0;
                        i++;
                }
                size-=l;
                midi+=l+8;
        }
        n_tracks=i;

        // Init vars
        delta_abs=0;
        memset(channels, -1, sizeof(channels));
        for(i=0; i<16; i++) {
		used_channel[i]=0;
                tone[i]=0x2000;
        }

        return 0;
}

int readMidi(void)
{
        int i, track;

        // Look for the track that needs to be updated the earliest
        track=0;
        delta_abs=tracks[0].delta;
        for(i=1; i<n_tracks; i++)
                if(tracks[i].delta<delta_abs) {
                        track=i;
                        delta_abs=tracks[i].delta;
                }
        if(delta_abs==0xffffffff)
                return -1;

        // Process all the commands happening at this time
        for(i=0; i<n_tracks; i++)
                if(tracks[i].delta!=0xffffffff) {
                        tracks[i].delta-=delta_abs;
                        while(tracks[i].delta==0)
                                processMidiCommand(i);
	        }

        return 0;
}

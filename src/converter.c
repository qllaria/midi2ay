#include <string.h>
#include "midi2ay.h"

// Public vars:
// contain previous and current state of AY chip

char ay_state[3][3]; // channel, note, volume
char ay_new[3][3];

static int best[3];

static void insert(int pos, char channel, char note, char volume, int score)
{
        int i;

        for(i=2; i>pos; i--) {
                ay_new[i][0]=ay_new[i-1][0];
                ay_new[i][1]=ay_new[i-1][1];
                ay_new[i][2]=ay_new[i-1][2];
                best[i]=best[i-1];
        }
        ay_new[pos][0]=channel;
        ay_new[pos][1]=note;
        ay_new[pos][2]=volume;
        best[pos]=score;
}

void initConversion(void)
{
        memset(ay_state, 0, sizeof(ay_state));
}

void conversion(void)
{
        int channel, note, score;

        best[0]=best[1]=best[2]=0;
        memcpy(ay_new, ay_state, sizeof(ay_new));
        for(channel=0; channel<16; channel++)
                if((channel!=9) && (used_channel[channel]))
	                for(note=0; note<128; note++)
                                if(channels[channel][note]!=-1) {
                                        score=channels[channel][note]*note;
                                        if(best[0]<score) {
                                                insert(0, channel, note,
                                                        channels[channel][note],
                                                        score);
                                                continue;
                                        }
                                        if(best[1]<score) {
                                                insert(1, channel, note,
                                                        channels[channel][note],
                                                        score);
                                                continue;
                                        }
                                        if(best[2]<score) {
                                                insert(2, channel, note,
                                                        channels[channel][note],
                                                        score);
                                                continue;
                                        }
	                        }
        for(channel=0; channel<3; channel++)
                if(best[channel]==0)
                        ay_new[channel][2]=0;
}

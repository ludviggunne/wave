/* Speeds audio up by 2x */
/* Compile with `gcc -lm -o example example.c wave.c` */
/* Run with `cat input.wav | ./example > output.wav` */

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include "wave.h"

const char *waverrstr(int c)
{
    switch (c)
    {
    case WAVOK:
        return "WAVOK";
    case WAVINVCID:
        return "WAVINVCID";
    case WAVUSFMT:
        return "WAVUSFMT";
    case WAVINVFMT:
        return "WAVINVFMT";
    case WAVEOF:
        return "WAVEOF";
    case WAVOOM:
        return "WAVOOM";
    default:
        return "unknown error";
    }
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    struct wave w;

    int res = wavread(stdin, &w);
    if (res)
    {
        fprintf(stderr, "error: failed to read WAVE file: %s\n",
                waverrstr(res));
        return 1;
    }

    for (int i = 0; i < w.scount / 2; i++)
    {
        w.data.i16mono[i] = w.data.i16mono[2 * i];
    }
    w.scount /= 2;

    res = wavwrite(stdout, &w);
    if (res)
    {
        fprintf(stderr, "error: failed to write WAVE file: %s\n",
                waverrstr(res));
        return 1;
    }

    return 0;
}

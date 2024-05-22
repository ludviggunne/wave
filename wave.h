// spec: https://mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html
#ifndef WAVE_H
#define WAVE_H

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/* Waveform format codes.
   Only WAVE_FORMAT_PCM is supported for now */
#define WAVE_FORMAT_PCM        0x0001
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#define WAVE_FORMAT_ALAW       0x0006
#define WAVE_FORMAT_MULAW      0x0007
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE

/* wavread & wavwrite error codes */
enum waverr
{
    WAVOK = 0,                  /* OK */
    WAVINVCID,                  /* Invalid chunk ID */
    WAVUSFMT,                   /* Unsupported format */
    WAVINVFMT,                  /* Invalid fmt chunk */
    WAVEOF,                     /* End of stream */
    WAVOOM,                     /* Out of memory */
};

struct wave
{
    uint16_t fmt;               /* Format code */
    uint16_t nchs;              /* Number of channels */
    uint32_t srate;             /* Sample rate */
    uint16_t bps;               /* Bits per sample */
    uint32_t scount;            /* Number of samples */
    union
    {
        int16_t *i16mono;
        struct
        {
            int16_t l, r;
        } *i16stereo;
        void *raw;
        /* ... */
    } data;                     /* Samples */
};

/* Read a WAVE file
   Discards any metadata */
int wavread(FILE * f, struct wave *w);

/* Write a WAVE file */
int wavwrite(FILE * f, struct wave *w);

/* Free data loaded from file */
void wavdestr(struct wave *w);

/* Get size of waveform data in bytes */
size_t wavdatsz(struct wave *w);

#endif

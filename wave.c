#include <stdlib.h>
#include <string.h>
#include "wave.h"

#define CHKNOEOF(x) if (x) return WAVEOF

static int _fwriteall(FILE *f, void *str, size_t len)
{
    return fwrite(str, 1, len, f) < len;
}

static int _fwriteu16le(FILE *f, uint16_t v)
{
    uint8_t buf[2];
    buf[0] = v & 0xff;
    buf[1] = (v >> 8) & 0xff;
    return _fwriteall(f, buf, 2);
}

static int _fwriteu32le(FILE *f, uint32_t v)
{
    uint8_t buf[4];
    buf[0] = v & 0xff;
    buf[1] = (v >> 8) & 0xff;
    buf[2] = (v >> 16) & 0xff;
    buf[3] = (v >> 24) & 0xff;
    return _fwriteall(f, buf, 4);
}

static int _freadall(FILE *f, void *buf, size_t len)
{
    return fread(buf, 1, len, f) < len;
}

static int _freadu16le(FILE *f, uint16_t *v)
{
    uint8_t buf[2];
    if (_freadall(f, buf, 2)) return 1;
    *v = buf[0] | (buf[1] << 8);
    return 0;
}

static int _freadu32le(FILE *f, uint32_t *v)
{
    uint8_t buf[4];
    if (_freadall(f, buf, 4)) return 1;
    *v = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
    return 0;
}

size_t wavdatsz(struct wave *w)
{
    return w->scount * w->nchs * w->bps / 8;
}

int wavread(FILE *f, struct wave *w)
{
    uint32_t datsz, fmtsz, srate, _discard32;
    uint16_t fmt, nchs, bps, _discard16;

    char idbuf[4];
    /*
     * File header
     */
    CHKNOEOF(_freadall(f, idbuf, 4));
    if (strncmp(idbuf, "RIFF", 4)) return WAVINVCID;

    /* File size - don't care */
    CHKNOEOF(_freadu32le(f, &_discard32));

    /* WAVE ID */
    CHKNOEOF(_freadall(f, idbuf, 4));
    if (strncmp(idbuf, "WAVE", 4)) return WAVINVCID;

    /*
     * Format chunk
     */
    CHKNOEOF(_freadall(f, idbuf, 4));
    if (strncmp(idbuf, "fmt ", 4)) return WAVINVCID;

    /* Chunk size */
    CHKNOEOF(_freadu32le(f, &fmtsz));

    /* Format code */
    CHKNOEOF(_freadu16le(f, &fmt));
    switch (fmt)
    {
    case WAVE_FORMAT_PCM:
        if (fmtsz != 16) return WAVINVFMT;
        break;
    default:
        return WAVUSFMT;
    }

    /* Number of channels */
    CHKNOEOF(_freadu16le(f, &nchs));

    /* Sampling rate */
    CHKNOEOF(_freadu32le(f, &srate));

    /* Data rate - don't care */
    CHKNOEOF(_freadu32le(f, &_discard32));

    /* Data block size - don't care */
    CHKNOEOF(_freadu16le(f, &_discard16));

    /* Bits per sample */
    CHKNOEOF(_freadu16le(f, &bps));

    /* Skip metadata */
    for (;;)
    {
        uint32_t cksz;
        CHKNOEOF(_freadall(f, idbuf, 4));
        if (strncmp(idbuf, "data", 4) == 0) break;
        CHKNOEOF(_freadu32le(f, &cksz));
        while (cksz--) (void) fgetc(f);
    }

    /*
     * Data chunk
     */
    /* Chunk ID already parsed */
    // CHKNOEOF(_freadall(f, idbuf, 4));
    // if (strncmp(idbuf, "data", 4)) return WAVINVCID;

    /* Data size */
    CHKNOEOF(_freadu32le(f, &datsz));

    /* Samples */
    char *data = malloc(datsz);
    if (data == NULL) return WAVOOM;
    CHKNOEOF(_freadall(f, data, datsz));

    w->fmt = fmt;
    w->nchs = nchs;
    w->srate = srate;
    w->bps = bps;
    w->scount = datsz / nchs / (bps / 8);
    w->data.raw = data;

    return WAVOK;
}

int wavwrite(FILE *f, struct wave *w)
{
    unsigned datsz, cksize, fmtsz, byps, blkalign;
    switch (w->fmt)
    {
        /* TODO: support more formats */
    case WAVE_FORMAT_PCM:
        fmtsz = 16;
        break;
    default:
        return WAVINVFMT;
    }

    /*
     * File header
     */
    CHKNOEOF(_fwriteall(f, "RIFF", 4));
    datsz = wavdatsz(w);
    cksize = 12 + fmtsz + datsz;
    CHKNOEOF(_fwriteu32le(f, cksize));
    CHKNOEOF(_fwriteall(f, "WAVE", 4));

    /*
     * Format chunk
     */
    CHKNOEOF(_fwriteall(f, "fmt ", 4));

    /* Chunk size */
    CHKNOEOF(_fwriteu32le(f, fmtsz));

    /* Format code */
    CHKNOEOF(_fwriteu16le(f, w->fmt));

    /* Number of channels */
    CHKNOEOF(_fwriteu16le(f, w->nchs));

    /* Sampling rate */
    CHKNOEOF(_fwriteu32le(f, w->srate));

    /* Data rate */
    byps = w->srate * w->bps * w->nchs / 8;
    CHKNOEOF(_fwriteu32le(f, byps));

    /* Data block size */
    blkalign = w->bps * w->nchs / 8;
    CHKNOEOF(_fwriteu16le(f, blkalign));

    /* Bits per sample */
    CHKNOEOF(_fwriteu16le(f, w->bps));

    /*
     * Data chunk
     */
    CHKNOEOF(_fwriteall(f, "data", 4));

    /* Data size */
    CHKNOEOF(_fwriteu32le(f, datsz));

    /* Samples */
    CHKNOEOF(_fwriteall(f, w->data.raw, datsz));

    /* Padding */
    if (datsz & 1 && fputc(0, f) == EOF) return WAVEOF;

    return WAVOK;
}

void wavdestr(struct wave *w)
{
    free(w->data.raw);
    memset(w, 0, sizeof (*w));
}

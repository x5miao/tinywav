/**
 * Copyright (c) 2015-2017, Martin Roth (mhroth@gmail.com)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */



#include <assert.h>
#include <string.h>
#if _WIN32
#include <winsock2.h>
#include <malloc.h>
#else
#include <alloca.h>
#include <netinet/in.h>
#endif
#include "tinywav.h"


 // http://soundfile.sapp.org/doc/WaveFormat/
#if _WIN32
#pragma pack(2)
#else
__attribute__((aligned(2)))
#endif
typedef struct TinyWavHeader {
    uint32_t ChunkID;
    uint32_t ChunkSize;
    uint32_t Format;
    uint32_t Subchunk1ID;
    uint32_t Subchunk1Size;
    uint16_t AudioFormat;
    uint16_t NumChannels;
    uint32_t SampleRate;
    uint32_t ByteRate;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
    uint16_t extSize;
    uint32_t Subchunk2ID;
    uint32_t Subchunk2Size;
} TinyWavHeader;
#if _WIN32
#pragma pack()
#else
__attribute__((packed))
#endif

int tinywav_open_read(TinyWav *tw, const char *path)
{
  tw->f = fopen(path, "rb");
  if (!tw->f) {
      return -1;
  }
  
  printf("sizeof(TinyWavHeader)=%d\n", sizeof(TinyWavHeader));
  tw->h = (TinyWavHeader *)malloc(sizeof(TinyWavHeader));
  if (!tw->h) {
      fclose(tw->f);
      return -1;
  }

  size_t ret = fread(tw->h, sizeof(TinyWavHeader), 1, tw->f);
  if (ret != 1) {
      fclose(tw->f);
      free(tw->h);
      return -1;
  }

  // ChunkID must be "RIFF", Format must be "WAVE", subchunkID must be "fmt\0"
  if (tw->h->ChunkID != htonl(0x52494646) || tw->h->Format != htonl(0x57415645) ||
      tw->h->Subchunk1ID != htonl(0x666d7420)) {
      fclose(tw->f);
      free(tw->h);
      return -1;
  }
  
  // skip over any other chunks before the "data" chunk
  while (tw->h->Subchunk2ID != htonl(0x64617461)) { 
    fseek(tw->f, 4, SEEK_CUR);
    ret = fread(&tw->h->Subchunk2ID, 4, 2, tw->f);
    if (ret != 2) {
        fclose(tw->f);
        free(tw->h);
        return -1;
    }
  }

  //must find the "data" section 
  if (tw->h->Subchunk2ID != htonl(0x64617461)) {
      fclose(tw->f);
      free(tw->h);
      return -1;
  }

  tw->totalDataBytes = tw->h->Subchunk2Size;
  tw->numChannels = tw->h->NumChannels;
  tw->bytesPerSample = tw->h->BitsPerSample / 8;
  tw->samplerate = tw->h->SampleRate;
  tw->rpos = 0;
  return 0;
}


// returns number of samples read
int tinywav_read_f(TinyWav *tw, void *data, int len)
{
    if (tw->bytesPerSample == 2) {
        len = len & (~0x00000001);
    }
    else if (tw->bytesPerSample == 4) {
        len = len & (~0x3);
    }
    else {
        fprintf(stderr, "unpsupported bytesPerSample %d", tw->bytesPerSample);
        return -1;
    }

    size_t bytes_read = fread(data, 1, len, tw->f);
    tw->rpos += bytes_read;

    if (tw->rpos > tw->totalDataBytes) {
        size_t valid_bytes = bytes_read - (tw->rpos - tw->totalDataBytes);
        return valid_bytes;
    }
    return bytes_read;
}


void tinywav_close_read(TinyWav *tw)
{
  fclose(tw->f);
  free(tw->h);
  tw->f = NULL;
}


int tinywav_open_write(TinyWav *tw, const char* path,
    int numChannels, int samplerate, int bytesPerSample) 
{
    if ((numChannels < 1) && (numChannels > 7)) {
        fprintf(stderr, "unsupported channels %d (only support 1 to 7 channels)", numChannels);
        return -1;
    }
    if ((samplerate != 16000) && (samplerate != 32000) && (samplerate != 48000)) {
        fprintf(stderr, "unsupported samplerate %d (only support 16000, 32000, 48000)", samplerate);
        return -1;
    }
    if ((bytesPerSample != 2 && bytesPerSample != 4) && (bytesPerSample != 3)) {
        fprintf(stderr, "unsupported bitsPerSample %d (only support 2, 3, 4)", bytesPerSample);
        return -1;
    }


    tw->f = fopen(path, "wb");
    if (!tw->f) {
        return -1;
    }

    tw->numChannels = numChannels;
    tw->bytesPerSample = bytesPerSample;
    tw->samplerate = samplerate;
    tw->totalDataBytes = 0;

    // prepare WAV header
    TinyWavHeader *h=(TinyWavHeader *)malloc(sizeof(TinyWavHeader));
    h->ChunkID = htonl(0x52494646); // "RIFF"
    h->ChunkSize = 0; // fill this in on file-close
    h->Format = htonl(0x57415645); // "WAVE"
    h->Subchunk1ID = htonl(0x666d7420); // "fmt "
    h->Subchunk1Size = 18; // PCM
    h->AudioFormat = (bytesPerSample == 2) ? 1 : 3; // 1 PCM, 3 IEEE float
    h->NumChannels = numChannels;
    h->SampleRate = samplerate;
    h->ByteRate = samplerate * numChannels * bytesPerSample;
    h->BlockAlign = numChannels * bytesPerSample;
    h->BitsPerSample = bytesPerSample * 8;
    h->extSize = 0;
    h->Subchunk2ID = htonl(0x64617461); // "data"
    h->Subchunk2Size = 0; // fill this in on file-close

    // write WAV header
    fwrite(h, sizeof(TinyWavHeader), 1, tw->f);
    //fflush(tw->f);
    tw->h = h;

    return 0;
}

size_t tinywav_write_f(TinyWav *tw, const void *data, int len)
{
    size_t ret = fwrite(data, 1, len, tw->f);
    //fflush(tw->f);
    tw->totalDataBytes += len;
    return ret;
}


void tinywav_close_write(TinyWav *tw)
{
  uint32_t data_len = tw->totalDataBytes;

  // set length of data
  fseek(tw->f, 4, SEEK_SET);
  uint32_t chunkSize_len = 38 + data_len;
  fwrite(&chunkSize_len, sizeof(uint32_t), 1, tw->f);

  fseek(tw->f, 42, SEEK_SET);
  fwrite(&data_len, sizeof(uint32_t), 1, tw->f);
  fclose(tw->f);
  free(tw->h);
  tw->f = NULL;
}


bool tinywav_isOpen(const TinyWav *tw) 
{
  return (tw->f != NULL);
}

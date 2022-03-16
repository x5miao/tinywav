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



#ifndef _TINY_WAV_
#define _TINY_WAV_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TinyWavHeader TinyWavHeader;

typedef struct TinyWav {
  FILE *f;
  TinyWavHeader *h;
  int numChannels;
  int samplerate;
  int bytesPerSample;
  size_t totalDataBytes;
  size_t rpos;
} TinyWav;


/**
 * Open a file for reading.
 *
 * @param sampFmt  The sample format (e.g. 16-bit integer or 32-bit float)
 *                 that the file should be converted to.
 * @param chanFmt  The channel format (how the channel data is layed out in memory) when read.
 * @param path     The path of the file to read.
 *
 * @return  The error code. Zero if no error.
 */
int tinywav_open_read(TinyWav *tw, const char *path);

/**
 * Read sample data from the file.
 *
 * @param data  A pointer to the data structure to read to. This data has the same layout
                with the file, user need to convert it to the self-defined format.
 * @param len   The bytes of data.
 * return value    the read samples from the wav file
 */
int tinywav_read_f(TinyWav *tw, void *data, int len);


/** Stop reading the file. The Tinywav struct is now invalid. */
void tinywav_close_read(TinyWav *tw);

/**
* Open a file for writing.
*
* @param numChannels  The number of channels to write.
* @param samplerate   The sample rate of the audio.
* @param sampFmt      The sample format (e.g. 16-bit integer or 32-bit float).
* @param chanFmt      The channel format (how the channel data is layed out in memory)
* @param path         The path of the file to write to. The file will be overwritten.
*
* @return  The error code. Zero if no error.
*/
int tinywav_open_write(TinyWav *tw, const char *path,
    int numChannels, int samplerate, int bytesPerSample);
/**
 * Write sample data to file.
 *
 * @param tw   The TinyWav structure which has already been prepared.
 * @param f    A pointer to the sample data to write.
 * @param len  The number of frames to write.
 *
 * @return The total number of samples written to file.
 */
size_t tinywav_write_f(TinyWav *tw, const void *data, int len);

/** Stop writing to the file. The Tinywav struct is now invalid. */
void tinywav_close_write(TinyWav *tw);

/** Returns true if the Tinywav struct is available to write or write. False otherwise. */
bool tinywav_isOpen(const TinyWav *tw);

#ifdef __cplusplus
}
#endif

#endif // _TINY_WAV_

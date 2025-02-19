/**
 * Copyright (c) 2015, Martin Roth (mhroth@gmail.com)
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

#include "tinywav.h"
#include <stdint.h>

#define NUM_CHANNELS 1
#define SAMPLE_RATE 48000
#define BLOCK_SIZE 512
#define NUM_ITERATIONS 3

int main(int argc, char *argv[])
{
  if (argc < 2){
	  return -1;
  }
  
  const char *outputPath = argv[1];
  TinyWav tw;
  tinywav_open_write(&tw, outputPath, NUM_CHANNELS, SAMPLE_RATE, 2);
  int16_t buffer[BLOCK_SIZE];
	  
  for (int i = 0; i < NUM_ITERATIONS; ++i) {
      for (int j = 0; j < BLOCK_SIZE; j++) {
          buffer[j] = j;
      }
      tinywav_write_f(&tw, buffer, BLOCK_SIZE * sizeof(int16_t));
  }

   tinywav_close_write(&tw);
   return 0;
}

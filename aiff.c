/******************************************************************************
 * $Id: aiff.c,v 1.4 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "error.h"
#include "aiff.h"

WAVE_FILE *aiff_open(FILE *fp, WAVE_INFO *info)
{
  char file_id[5], form_type[5];
  VINT file_data_size, file_position;
  WAVE_FILE *file;
  VBOOL is_aifc;

  /* common chunk */
  VBOOL got_common_chunk = FALSE;
  VSHORT num_channels = 0, sample_size = 0;
  char compression_type[5];
  VUINT num_sample_frames = 0;
  VREAL sample_rate = 0;

  /* sound data chunk */
  VUINT offset, block_size;
  VBOOL got_sound_data_chunk = FALSE;
  long sound_data_offset = 0;

  fread(file_id, 4, 1, fp);
  file_id[4] = '\0';
  if (strcmp(file_id, "FORM") != 0)
    return NULL;
  
  file_data_size = wave_read_int_big(fp) + 8;
  
  fread(form_type, 4, 1, fp);
  form_type[4] = '\0';
  if (strcmp(form_type, "AIFF") != 0 && strcmp(form_type, "AIFC") != 0)
    return NULL;
  is_aifc = strcmp(form_type, "AIFC") == 0;

  file_position = 12;
  while (file_position < file_data_size)
    {
      char chunk_id[5];
      VINT chunk_data_size;

      if (fread(chunk_id, 4, 1, fp) < 1) {
        if (feof(fp))
          error_display("aiff_open: bad format: EOF encountered where chunk expected");
        else if (ferror(fp))
          error_display("aiff_open: bad format: error encountered where chunk expected: %s",
                  strerror(errno));
      }
      chunk_id[4] = '\0';

      chunk_data_size = wave_read_int_big(fp);

      if (strcmp(chunk_id, "COMM") == 0)
        /* Common chunk */
        {
          num_channels = wave_read_short_big(fp);
          num_sample_frames = wave_read_int_big(fp);
          sample_size = wave_read_short_big(fp);
          sample_rate = wave_read_extended(fp);
          if (is_aifc) {
            fread(compression_type, 4, 1, fp);
            if (feof(fp))
              error_display("aiff_open: bad format: EOF encountered in common chunk");
            compression_type[4] = '\0';
          } else {
            strcpy(compression_type, "NONE");
          }
          got_common_chunk = TRUE;
        }
      else if (strcmp(chunk_id, "SSND") == 0)
        /* Sound Data chunk */
        {
          offset = wave_read_int_big(fp);
          block_size = wave_read_int_big(fp);
          sound_data_offset = ftell(fp) + offset;
          got_sound_data_chunk = TRUE;
        }

      file_position += chunk_data_size + 8;
      fseek(fp, file_position, SEEK_SET);
    }

  if (!got_common_chunk)
    error_display("aiff_open: bad format: did not find common chunk");
  
  if (!got_sound_data_chunk)
    error_display("aiff_open: bad format: did not find sound data chunk");

  if (strcmp(compression_type, "NONE") != 0)
    error_display("aiff_open: bad format: compressed AIFF-C files not supported");

  fseek(fp, sound_data_offset, SEEK_SET);

  file = error_malloc(sizeof(WAVE_FILE));
  file->is_big_endian = 1;
  file->sample_offset = 0;

  info->rate = (VINT)sample_rate;
  info->bits = (VBYTE)sample_size;
  info->channels = (VBYTE)num_channels;
  info->length = num_sample_frames;

  return file;
}

WAVE_FILE *aiff_create(FILE *fp, WAVE_INFO *info)
{
  VBYTE bytes_per_sample;
  WAVE_FILE *file;

  bytes_per_sample = (info->bits + 7) / 8;
  
  fwrite("FORM", 4, 1, fp);
  wave_write_int_big(0xDEADBEAF, fp);
  fwrite("AIFF", 4, 1, fp);

  fwrite("COMM", 4, 1, fp);
  wave_write_int_big(18, fp); /* ckDataSize */
  wave_write_short_big(info->channels, fp); /* numChannels */
  wave_write_int_big(0xDEADBEAF, fp); /* numSampleFrames */
  wave_write_short_big(info->bits, fp); /* sampleSize */
  wave_write_extended(info->rate, fp); /* sampleRate */

  fwrite("SSND", 4, 1, fp);
  wave_write_int_big(0xDEADBEAF, fp);
  wave_write_int_big(0, fp);
  wave_write_int_big(0, fp);

  file = error_malloc(sizeof(WAVE_FILE));
  file->is_big_endian = TRUE;
  file->sample_offset = 0;
  return file;
}

void aiff_close(WAVE_FILE *file)
{
  if (file->open_mode == WAVE_WRITE_MODE)
    {
      if (file->length % 2 == 1)
        putc(0, file->fp);
      
      fseek(file->fp, 4, SEEK_SET);
      wave_write_int_big(4 + 8 + 18 + 8 + 8 +
                         file->length * ((file->bits + 7) / 8), file->fp);
      fseek(file->fp, 12 + 10, SEEK_SET);
      wave_write_int_big(file->length, file->fp);
      fseek(file->fp, 12 + 18 + 8 + 4, SEEK_SET);
      wave_write_int_big(file->length * ((file->bits + 7) / 8) + 8, file->fp);
    }
}

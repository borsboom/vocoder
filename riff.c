/******************************************************************************
 * $Id: riff.c,v 1.5 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1996-1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "config.h"
#include "error.h"
#include "wave.h"
#include "riff.h"

WAVE_FILE *riff_open(FILE *fp, WAVE_INFO *info)
{
  char file_id[5], form_type[5];
  VINT file_data_size, file_position;
  VSHORT format_tag = 0, channels = 0, block_align, bits_per_sample = 0;
  VINT samples_per_sec = 0, avg_bytes_per_sec;
  VINT data_size = 0, length;
  long data_offset = 0;
  VBOOL got_format_chunk = FALSE, got_data_chunk = FALSE;
  WAVE_FILE *file;
  
  fread(file_id, 4, 1, fp);
  file_id[4] = '\0';
  if (strcmp(file_id, "RIFF") != 0)
    return NULL;
  
  file_data_size = wave_read_int_little(fp) + 8;
  
  fread(form_type, 4, 1, fp);
  form_type[4] = '\0';
  if (strcmp(form_type, "WAVE") != 0)
    return NULL;

  file_position = 12;
  while (file_position < file_data_size)
    {
      char chunk_id[5];
      VINT chunk_data_size;

      if (fread(chunk_id, 4, 1, fp) < 1)
        {
          if (feof(fp))
            error_display("riff_open: bad format: EOF encountered where chunk expected");
          else if (ferror(fp))
            error_display("riff_open: bad format: error encountered where chunk expected: %s",
                  strerror(errno));
        }
      chunk_id[4] = '\0';

      chunk_data_size = wave_read_int_little(fp);

      if (strcmp(chunk_id, "fmt ") == 0)
        /* Common chunk */
        {
          format_tag = wave_read_short_little(fp);
          channels = wave_read_short_little(fp);
          samples_per_sec = wave_read_int_little(fp);
          avg_bytes_per_sec = wave_read_int_little(fp);
          block_align = wave_read_short_little(fp);
          if (format_tag == 1)
            {
              bits_per_sample = wave_read_short_little(fp);
            }
          got_format_chunk = TRUE;
        }
      else if (strcmp(chunk_id, "data") == 0)
        /* Sound Data chunk */
        {
          data_size = chunk_data_size;
          data_offset = ftell(fp);
          got_data_chunk = TRUE;
        }

      file_position += chunk_data_size + 8;
      fseek(fp, file_position, SEEK_SET);
    }

  if (!got_format_chunk)
    error_display("riff_open: bad format: format chunk not found");
  if (!got_data_chunk)
    error_display("riff_open: bad format: data chunk not found");
  if (format_tag != 1)
    error_display("riff_open: bad format: only PCM data is supported");

  length = data_size / ((bits_per_sample + 7) / 8);

  fseek(fp, data_offset, SEEK_SET);

  file = error_malloc(sizeof(WAVE_FILE));
  file->is_big_endian = FALSE;
  file->sample_offset = (bits_per_sample <= 8) ? 128 : 0;

  info->rate = samples_per_sec;
  info->bits = (VBYTE)bits_per_sample;
  info->channels = (VBYTE)channels;
  info->length = length;

  return file;
}

WAVE_FILE *riff_create(FILE *fp, WAVE_INFO *info)
{
  WAVE_FILE *file;
  
  fwrite("RIFF", 4, 1, fp);
  wave_write_int_little(0xDEADBEEF, fp);
  fwrite("WAVE", 4, 1, fp);

  fwrite("fmt ", 4, 1, fp);
  wave_write_int_little(16, fp);
  wave_write_short_little(1, fp);
  wave_write_short_little(info->channels, fp);
  wave_write_int_little(info->rate, fp);
  wave_write_int_little(info->channels * info->rate * ((info->bits + 7) / 8), fp);
  wave_write_short_little(info->channels * ((info->bits + 7) / 8), fp);
  wave_write_short_little(info->bits, fp);

  fwrite("data", 4, 1, fp);
  wave_write_int_little(0xDEADBEEF, fp);

  file = error_malloc(sizeof(WAVE_FILE));
  file->is_big_endian = FALSE;
  file->sample_offset = (info->bits <= 8) ? 128 : 0;
  return file;
}

void riff_close(WAVE_FILE *file)
{
  if (file->open_mode == WAVE_WRITE_MODE)
    {
      fseek(file->fp, 4, SEEK_SET);
      wave_write_int_little(file->length * ((file->bits + 7) / 8) + 4 + 8 + 16 + 8, file->fp);
      fseek(file->fp, 12 + 8 + 16 + 4, SEEK_SET);
      wave_write_int_little(file->length * ((file->bits + 7) / 8), file->fp);
    }
}

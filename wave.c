/******************************************************************************
 * $Id: wave.c,v 1.4 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "config.h"
#include "wave.h"
#include "extended.h"
#include "error.h"

#include "aiff.h"
#include "riff.h"

static struct {
  WAVE_FILE *(*open_func)(FILE *fp, WAVE_INFO *info);
  WAVE_FILE *(*create_func)(FILE *fp, WAVE_INFO *info);
  void (*close_func)(WAVE_FILE *file);
} wave_formats[] = 
{
  {aiff_open, aiff_create, aiff_close},
  {riff_open, riff_create, riff_close}
};

WAVE_FILE *wave_open(char *filename, WAVE_INFO *info)
{
  FILE *fp;
  int i;

  fp = error_fopen(filename, "rb");

  for (i = 0; i < WAVE_NUM_FORMATS; ++i) {
    if (wave_formats[i].open_func != NULL)
      {
        WAVE_FILE *file;
        info->format = (WAVE_FORMAT) i;
        file = wave_formats[i].open_func(fp, info);
        if (file != NULL)
          {
            file->bits = info->bits;
            file->format = info->format;
            file->length = info->length;
            file->position = 0;
            file->open_mode = WAVE_READ_MODE;
            file->fp = fp;
            file->data_offset = ftell(fp);
            return file;
          }
        fseek(fp, 0, SEEK_SET);
      }
  }

  fclose(fp);
  error_display("wave_open %s: unknown format", filename);
  return NULL;
}

WAVE_FILE *wave_open_specific(char *filename, WAVE_INFO *info)
{
  FILE *fp;
  WAVE_FILE *file;
  
  fp = error_fopen(filename, "rb");

  file = wave_formats[info->format].open_func(fp, info);
  if (file == NULL) return NULL;
  file->bits = info->bits;
  file->format = info->format;
  file->length = info->length;
  file->position = 0;
  file->open_mode = WAVE_READ_MODE;
  file->fp = fp;
  file->data_offset = ftell(fp);
  return file;
}

WAVE_FILE *wave_create(char *filename, WAVE_INFO *info)
{
  FILE *fp;
  WAVE_FILE *file;

  fp = error_fopen(filename, "wb");

  file = wave_formats[info->format].create_func(fp, info);
  if (file == NULL) return NULL;
  file->bits = info->bits;
  file->format = info->format;
  file->open_mode = WAVE_WRITE_MODE;
  file->length = 0;
  file->position = 0;
  file->fp = fp;
  file->data_offset = ftell(fp);
  return file;
}

void wave_seek(WAVE_FILE *file, VINT position)
{
  if (file->open_mode == WAVE_WRITE_MODE)
    error_display("wave_seek: seek only supported for files opened in read mode");

  if (position > file->length)
    error_display("wave_seek: attempting to position out of file");

  fseek(file->fp, file->data_offset + position * ((file->bits + 7) / 8),
        SEEK_SET);
  file->position = position;
}

void wave_close(WAVE_FILE *file)
{
  if (wave_formats[file->format].close_func != NULL)
    wave_formats[file->format].close_func(file);
  fclose(file->fp);
  free(file);
}

size_t wave_read(WAVE_FILE *file, SAMPLE *buffer, size_t length)
{
  size_t count = 0;

  if (feof(file->fp))
    return 0;

  if ((size_t)(file->length - file->position) < length)
    length = file->length - file->position;

  if (file->bits == 8)
    {
      for (count = 0; count < length; ++count)
        buffer[count] = (VBYTE)(getc(file->fp) - file->sample_offset);
    }
  else if (file->bits == 16)
    { 
      if (file->is_big_endian)
        {
          for (count = 0; count < length; ++count)
            buffer[count] = (VSHORT)(wave_read_short_big(file->fp)
                                    - file->sample_offset);
        }
      else
        {
          for (count = 0; count < length; ++count)
            buffer[count] = (VSHORT)(wave_read_short_little(file->fp)
                                    - file->sample_offset);
        }
    }
  else
    {
      error_display("wave_read: only 8-bit and 16-bit audio supported");
    }

  if (ferror(file->fp))
    error_display("wave_read: read error: %s", strerror(errno));

  file->position += count;
  return count;
}

void wave_write(WAVE_FILE *file, SAMPLE *buffer, size_t length)
{
  size_t count = 0;

  if (file->bits == 8)
    {
      for (count = 0; count < length; ++count)
        putc(buffer[count] + file->sample_offset, file->fp);
    }
  else if (file->bits == 16)
    {
      if (file->is_big_endian)
        {
          for (count = 0; count < length; ++count)
            wave_write_short_big(buffer[count] + file->sample_offset, file->fp);
        }
      else
        {
          for (count = 0; count < length; ++count)
            wave_write_short_little(buffer[count] + file->sample_offset, file->fp);
        }
    }
  else
    {
      error_display("wave_write: only 8-bit and 16-bit audio supported");
    }

  if (ferror(file->fp))
    error_display("wave_write: write error: %s", strerror(errno));

  file->length += count;
  file->position += count;
}

VINT wave_read_int_big(FILE *fp)
{
  int a, b, c, d;
  a = getc(fp); b = getc(fp); c = getc(fp); d = getc(fp);
  return (a << 24) | (b << 16) | (c << 8) | d;
}

void wave_write_int_big(VINT i, FILE *fp)
{
  int a, b, c, d;
  a = (i >> 24) & 0xff;
  b = (i >> 16) & 0xff;
  c = (i >> 8) & 0xff;
  d = i & 0xff;
  putc(a, fp); putc(b, fp); putc(c, fp); putc(d, fp);
}

VINT wave_read_int_little(FILE *fp)
{
  int a, b, c, d;
  a = getc(fp); b = getc(fp); c = getc(fp); d = getc(fp);
  return (d << 24) | (c << 16) | (b << 8) | a;
}

void wave_write_int_little(VINT i, FILE *fp)
{
  int a, b, c, d;
  a = (i >> 24) & 0xff;
  b = (i >> 16) & 0xff;
  c = (i >> 8) & 0xff;
  d = i & 0xff;
  putc(d, fp); putc(c, fp); putc(b, fp); putc(a, fp);
}

VSHORT wave_read_short_big(FILE *fp)
{
  int a, b;
  a = getc(fp); b = getc(fp);
  return (a << 8) | b;
}

void wave_write_short_big(VSHORT i, FILE *fp)
{
  int a, b;
  a = (i >> 8) & 0xff;
  b = i & 0xff;
  putc(a, fp); putc(b, fp);
}

VSHORT wave_read_short_little(FILE *fp)
{
  int a, b;
  a = getc(fp); b = getc(fp);
  return (b << 8) | a;
}

void wave_write_short_little(VSHORT i, FILE *fp)
{
  int a, b;
  a = (i >> 8) & 0xff;
  b = i & 0xff;
  putc(b, fp); putc(a, fp);
}

VREAL wave_read_extended(FILE *fp)
{
  unsigned char bytes[10];
  fread(bytes, 10, 1, fp);
  return ConvertFromIeeeExtended(bytes);
}

void wave_write_extended(VREAL e, FILE *fp)
{
  unsigned char bytes[10];
  ConvertToIeeeExtended(e, bytes);
  fwrite(bytes, 10, 1, fp);
}

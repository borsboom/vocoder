/******************************************************************************
 * $Id: wave.h,v 1.2 1998/09/13 00:21:18 emanuel Exp $
 * Copyright (C) 1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#ifndef WAVE_H_INCLUDED
#define WAVE_H_INCLUDED

#include <stdio.h>
#include "config.h"

typedef enum {AIFF_FORMAT, RIFF_FORMAT,
	      WAVE_NUM_FORMATS} WAVE_FORMAT;

typedef enum {WAVE_READ_MODE, WAVE_WRITE_MODE} WAVE_OPEN_MODE;

typedef SHORT SAMPLE;

struct tag_WAVE_FILE
{
  FILE *fp;
  BYTE bits;
  BOOL is_big_endian;
  SAMPLE sample_offset;
  INT length, position;
  WAVE_OPEN_MODE open_mode;
  WAVE_FORMAT format;
  long data_offset;
};
typedef struct tag_WAVE_FILE WAVE_FILE;

struct tag_WAVE_INFO
{
  INT rate;
  BYTE bits;
  BYTE channels;
  INT length;
  WAVE_FORMAT format;
};
typedef struct tag_WAVE_INFO WAVE_INFO;

WAVE_FILE *wave_open(char *filename, WAVE_INFO *info);
WAVE_FILE *wave_open_specific(char *filename, WAVE_INFO *info);
WAVE_FILE *wave_create(char *filename, WAVE_INFO *info);
void wave_close(WAVE_FILE *file);
size_t wave_read(WAVE_FILE *file, SAMPLE *buffer, size_t length);
void wave_write(WAVE_FILE *file, SAMPLE *buffer, size_t length);
void wave_seek(WAVE_FILE *file, INT position);

INT wave_read_int_big(FILE *fp);
void wave_write_int_big(INT i, FILE *fp);
INT wave_read_int_little(FILE *fp);
void wave_write_int_little(INT i, FILE *fp);
SHORT wave_read_short_big(FILE *fp);
void wave_write_short_big(SHORT i, FILE *fp);
SHORT wave_read_short_little(FILE *fp);
void wave_write_short_little(SHORT i, FILE *fp);
REAL wave_read_extended(FILE *fp);
void wave_write_extended(REAL e, FILE *fp);

#endif /* WAVE_H_INCLUDED */

/******************************************************************************
 * $Id: wave.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998,2002 Emanuel Borsboom <em@nuel.ca>
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

typedef VSHORT SAMPLE;

struct tag_WAVE_FILE
{
  FILE *fp;
  VBYTE bits;
  VBOOL is_big_endian;
  SAMPLE sample_offset;
  VINT length, position;
  WAVE_OPEN_MODE open_mode;
  WAVE_FORMAT format;
  long data_offset;
};
typedef struct tag_WAVE_FILE WAVE_FILE;

struct tag_WAVE_INFO
{
  VINT rate;
  VBYTE bits;
  VBYTE channels;
  VINT length;
  WAVE_FORMAT format;
};
typedef struct tag_WAVE_INFO WAVE_INFO;

WAVE_FILE *wave_open(char *filename, WAVE_INFO *info);
WAVE_FILE *wave_open_specific(char *filename, WAVE_INFO *info);
WAVE_FILE *wave_create(char *filename, WAVE_INFO *info);
void wave_close(WAVE_FILE *file);
size_t wave_read(WAVE_FILE *file, SAMPLE *buffer, size_t length);
void wave_write(WAVE_FILE *file, SAMPLE *buffer, size_t length);
void wave_seek(WAVE_FILE *file, VINT position);

VINT wave_read_int_big(FILE *fp);
void wave_write_int_big(VINT i, FILE *fp);
VINT wave_read_int_little(FILE *fp);
void wave_write_int_little(VINT i, FILE *fp);
VSHORT wave_read_short_big(FILE *fp);
void wave_write_short_big(VSHORT i, FILE *fp);
VSHORT wave_read_short_little(FILE *fp);
void wave_write_short_little(VSHORT i, FILE *fp);
VREAL wave_read_extended(FILE *fp);
void wave_write_extended(VREAL e, FILE *fp);

#endif /* WAVE_H_INCLUDED */

/******************************************************************************
 * $Id: error.c,v 1.5 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error.h"

#define ERROR_BUF_SIZE 256

ERROR_DISPLAY_CB error_display_cb;

static char _error_buf[ERROR_BUF_SIZE];

void error_display(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vsprintf(_error_buf, fmt, ap);
  strcat(_error_buf, "\n");
  va_end(ap);
  error_display_cb(_error_buf);
}

void *error_malloc(size_t size)
{
  void *p = malloc(size);
  if (p == NULL) error_display("malloc: out of memory");
  return p;
}

void *error_calloc(size_t number, size_t size)
{
  void *p = calloc(number, size);
  if (p == NULL) error_display("calloc: out of memory");
  return p;
}

FILE *error_fopen(char *path, char *mode)
{
  FILE *f = fopen(path, mode);
  if (f == NULL) error_display("fopen %s: %s", path, strerror(errno));
  return f;
}

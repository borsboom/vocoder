/******************************************************************************
 * $Id: error.c,v 1.2 1998/09/13 00:21:18 emanuel Exp $
 * Copyright (C) 1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "error.h"

extern char *prog_name;

void error(char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: ", prog_name);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
  exit(1);
}

void *error_malloc(size_t size)
{
  void *p = malloc(size);
  if (p == NULL) error("malloc: out of memory");
  return p;
}

void *error_calloc(size_t number, size_t size)
{
  void *p = calloc(number, size);
  if (p == NULL) error("calloc: out of memory");
  return p;
}

FILE *error_fopen(char *path, char *mode)
{
  FILE *f = fopen(path, mode);
  if (f == NULL) error("fopen %s: %s", path, strerror(errno));
  return f;
}

/******************************************************************************
 * $Id: error.h,v 1.2 1998/09/13 00:21:18 emanuel Exp $
 * Copyright (C) 1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>

void error(char *fmt, ...);
void *error_malloc(size_t size);
void *error_calloc(size_t number, size_t size);
FILE *error_fopen(char *path, char *mode);

#endif /* ERROR_H_INCLUDED */

/******************************************************************************
 * $Id: error.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998,2002 Emanuel Borsboom <em@nuel.ca>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#ifndef ERROR_H_INCLUDED
#define ERROR_H_INCLUDED

typedef void (*ERROR_DISPLAY_CB)(char *msg);
extern ERROR_DISPLAY_CB error_display_cb;

void error_display(char *fmt, ...);
void *error_malloc(size_t size);
void *error_calloc(size_t number, size_t size);
FILE *error_fopen(char *path, char *mode);

#endif /* ERROR_H_INCLUDED */

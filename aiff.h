/******************************************************************************
 * $Id: aiff.h,v 1.2 1998/09/13 00:21:18 emanuel Exp $
 * Copyright (C) 1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#ifndef AIFF_H_INCLUDED
#define AIFF_H_INCLUDED

#include "wave.h"

WAVE_FILE *aiff_open(FILE *fp, WAVE_INFO *info);
WAVE_FILE *aiff_create(FILE *fp, WAVE_INFO *info);
void aiff_close(WAVE_FILE *file);

#endif /* AIFF_H_INCLUDED */

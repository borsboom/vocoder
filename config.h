/******************************************************************************
 * $Id: config.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $
 * Copyright (C) 1998 Emanuel Borsboom <emanuel@zerius.com>
 * Permission is granted to make any use of this code subject to the condition
 * that all copies contain this notice and an indication of what has been
 * changed.
 *****************************************************************************/

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

typedef int VBOOL;
#define FALSE 0
#define TRUE 1

typedef unsigned long U32;
typedef signed long S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;

typedef S32 VINT;
typedef S16 VSHORT;
typedef S8  VBYTE;
typedef U32 VUINT;
typedef U16 VUSHORT;
typedef U8  VUBYTE;

typedef double VREAL;

#endif /* CONFIG_H_INCLUDED */


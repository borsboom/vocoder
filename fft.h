/* $Id: fft.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

/* (C) Copyright 1993 by Steven Trainoff.  Permission is granted to make
* any use of this code subject to the condition that all copies contain
* this notice and an indication of what has been changed.
*/
#ifndef _FFT_
#define _FFT_

#include "spt.h"

#ifndef REAL
#define REAL double		/* Precision of data */
#endif

/* Really ugly hack needed since #if only works on ints */
#define double 1
#define float 2

#if REAL==float
#define fft_create_arrays f_fft_create_arrays
#define getx f_getx
#define fft f_fft
#define invfft f_invfft
#define normalize_fft f_normalize_fft
#define fft1n f_fft1n
#define fftn f_fftn
#define invfftn f_invfftn
#define realfftmag f_realfftmag
#define normalize_fftn f_normalize_fftn
#endif
	
#undef double
#undef float

void fft_create_arrays(REAL **c, REAL **s, int **rev, int n);
REAL *getx(REAL (*)[2], int, int, int, int *);
void fft(REAL (*x)[2], int n, REAL*c, REAL*s, int *rev);
void invfft(REAL (*x)[2], int n, REAL*c, REAL*s, int *rev);
void normalize_fft(REAL (*x)[2], int n);
void fft1n(REAL (*x)[2], int  nu, int offset, int separation, REAL *c, REAL *s, int *rev);
void fftn(REAL (*x)[2], int n, int *dim);
void invfftn(REAL (*x)[2], int n, int *dim);
void realfftmag(REAL *data, int n);
void normalize_fftn(REAL (*x)[2], int ndim, int *dim);

unsigned bitrev(unsigned, int);	/* Bit reversal routine */
int getindex(int, int, int, int *);
int ipow(int, int);
int ilog2(int);



#endif /* _FFT_ */

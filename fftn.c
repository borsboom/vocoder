/* $Id: fftn.c,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

/* (C) Copyright 1993 by Steven Trainoff.  Permission is granted to make
* any use of this code subject to the condition that all copies contain
* this notice and an indication of what has been changed.
*/
#include <stdio.h>
#include <math.h>
#include "error.h"
#include "spt.h"

#ifndef REAL
#define REAL double
#endif

#include "fft.h"
        
#ifndef PI
#define PI      3.14159265358979324
#endif

#define emalloc error_malloc

/* This routine performs a complex fft.  It takes two arrays holding
 * the real and imaginary parts of the the complex numbers.  It performs
 * the fft and returns the result in the original arrays.  It destroys
 * the orginal data in the process.  Note the array returned is NOT
 * normalized.  Each element must be divided by n to get dimensionally
 * correct results.  This routine takes optional arrays for the sines, cosine
 * and bitreversal.  If any of these pointers are null, all of the arrays are 
 * regenerated.
 */

void fft(x, n, c, s, rev)
REAL x[][2];            /* Input data points */
int n;                  /* Number of points, n = 2**nu */
REAL *c, *s;            /* Arrays of cosine, sine */
int *rev;                       /* Array of bit reversals*/
{
        char temparray=FALSE;       /* Are we using internal c and s arrays? */
        int nu = ilog2(n);      /* Number of data points */
        int dual_space = n;     /* Spacing between dual nodes */
        int nu1 = nu;           /* = nu-1 right shift needed when finding p */
        int k;                          /* Iteration of factor array */
        register int i;                 /* Number of dual node pairs considered */
        register int j;                 /* Index into factor array */
        
        if (c == NULL || s == NULL || rev == NULL) {
                temparray = TRUE;
                fft_create_arrays(&c, &s, &rev, n);
        }
        
        /* For each iteration of factor matrix */
        for (k = 0; k < nu; k++) {
                /* Initialize */
                dual_space /= 2;        /* Fewer elements in each set of duals */
                nu1--;                  /* nu1 = nu - 1 */

                /* For each set of duals */
                for(j = 0; j < n; j += dual_space) {
                        /* For each dual node pair */
                        for (i = 0; i < dual_space; i++, j++) {
                                REAL treal, timag;              /* Temp of w**p */
                                register int p = rev[j >> nu1];
                                
                                treal = x[j+dual_space][0]*c[p] + x[j+dual_space][1]*s[p];
                                timag = x[j+dual_space][1]*c[p] - x[j+dual_space][0]*s[p];

                                x[j+dual_space][0] = x[j][0] - treal;
                                x[j+dual_space][1] = x[j][1] - timag;

                                x[j][0] += treal;
                                x[j][1] += timag;
                        }
                }
        }

        /* We are done with the transform, now unscamble results */
        for (j = 0; j < n; j++) {
                if ((i = rev[j]) > j) {
                        REAL treal, timag;

                        /* Swap */
                        treal = x[j][0];
                        timag = x[j][1];

                        x[j][0] = x[i][0];
                        x[j][1] = x[i][1];

                        x[i][0] = treal;
                        x[i][1] = timag;
                }
        }
                    
         /* Give back the temp storage */
        if (temparray) {
            free(c);
            free(s);
            free(rev);
        }
}

/* invfft performs an inverse fft */
void invfft(REAL (*x)[2], int n, REAL*c, REAL*s, int *rev)
{
        char temparray = FALSE;
        int i;
        
        if (c == NULL || s == NULL || rev == NULL) {
                temparray = TRUE;
                fft_create_arrays(&c, &s, &rev, n);
        }
        
        /* Negate the sin array to do the inverse transform */
        for (i = 0; i < n; i++)
                s[i] = -s[i];
        
        fft(x, n, c, s, rev);
        
        if (temparray) {
                free(c);
                free(s);
                free(rev);
        } else {                        /* Put the sin array back */
                for (i = 0; i < n; i++)
                        s[i] = - s[i];
        }
}



/* fft_create_arrays generates the sine and cosine arrays needed in the
 * forward complex fft.  It allocates storaged and return pointers
 * to the initialized arrays 
 */

void fft_create_arrays(c, s, rev, n)
REAL **c, **s;          /* Sin and Cos arrays (to be returned) */
int n;                          /* Number of points */
int **rev;                      /* Array of reversed bits */
{
    register int i;
    int nu = ilog2(n);

    /* Compute temp array of sins and cosines */
    *c = (REAL *)emalloc(n * sizeof(REAL));
    *s = (REAL *)emalloc(n * sizeof(REAL));
    *rev = (int *)emalloc(n * sizeof(int));
    
    for (i = 0; i < n; i++) {
        REAL arg = 2 * PI * i/n;
        (*c)[i] = cos(arg);
        (*s)[i] = sin(arg);
        (*rev)[i] = bitrev(i, nu);
    }
}

#if (1==2)                      /* Not needed now */
/* getx - gets one real, imag pair from the square multidimensional array */
REAL *getx(x, ndim, dim, n, elem)
REAL x[][2];                    /* Input data points */
int ndim;                       /* Number of dimensions */
int dim;                        /* Dim to extract point from */
int n;                          /* Number of points in each dim */
int elem[];                     /* Which element to extract (array by ndim) */
{
    register int i;
    int pos=0;                  /* Position in array when considered 1d */

    for (i = 0; i < ndim && (pos *= n); i++)  /* Loop over dimensions */
        pos += elem[i];

    return(x+pos);
}
#endif

/* This routine performs a multiple complex fft on a square
 * multidimensional array.  Each element is an array by two holding the
 * real and imaginary parts of the the complex numbers.  It performs the
 * fft and returns the result in the original arrays.  It destroys the
 * orginal data in the process.  Note the array returned is NOT
 * normalized.  Each element must be divided by n to get dimensionally
 * correct results.  The cosine and sine arrays are optional.  If null is
 * passed, a temp array will be allocated and filled, otherwise the
 * passed arrays are assumed to have the correct data in them.
 *
 * Note: if the sin array is negated, the routine performs the inverse
 * transform.
 */

void fftn(x, ndim, dim)
REAL x[][2];            /* Input data points */
int ndim;                       /* Number of dimensions */
int dim[];                      /* Number of points in each dim = 2**nu */
{
    int nel;                    /* Number of elements */
    int separation = 1;         /* Distance between elements (which "column") */
    int offset = 0;             /* 1st element in fft (which "row") */
    register int i;             /* Which dim we are performing the fft */
    register int j;             /* Which indices we are NOT performing the fft */
    register int k;             /* Loop over index j */
    REAL *c, *s;                /* Sine and Cosine arrays */
    int *rev;                   /* Array of bitreversed numbers */

    /* Compute number of elements in array */
    for (i = 0, nel = 1; i < ndim; i++)
        nel *= dim[i];
    
    for (i = ndim-1; i >= 0; i--) { /* Loop over dim on which we are doing the fft */
        int logi = ilog2(dim[i]);
        int nextseparation = separation * dim[i];

        /* Create Sin and Cos arrays */
        fft_create_arrays(&c, &s, &rev, dim[i]);
    
        /* Loop over other indices.  First do indicies bigger than i */
        for (j =0; j < nel; j += nextseparation) {
            for (k = 0; k < separation; k++) {
                offset = j+k;
                fft1n(x, logi, offset, separation, c, s, rev);
            }
        }
        /* Give back old sin and cos arrays */
        free(c); free(s);free(rev);
        separation = nextseparation;
    }
}

/* This routine is identical to fftn but negates the sin array to do an inverse transform */

void invfftn(x, ndim, dim)
REAL x[][2];            /* Input data points */
int ndim;                       /* Number of dimensions */
int dim[];                      /* Number of points in each dim = 2**nu */
{
    int nel;                    /* Number of elements */
    int separation = 1;         /* Distance between elements (which "column") */
    int offset = 0;             /* 1st element in fft (which "row") */
    register int i;             /* Which dim we are performing the fft */
    register int j;             /* Which indices we are NOT performing the fft */
    register int k;             /* Loop over index j */
    REAL *c, *s;                /* Sine and Cosine arrays */
    int *rev;                   /* Array of bitreversed numbers */

    /* Compute number of elements in array */
    for (i = 0, nel = 1; i < ndim; i++)
        nel *= dim[i];
    
    for (i = ndim-1; i >= 0; i--) { /* Loop over dim on which we are doing the fft */
        int logi = ilog2(dim[i]);
        int nextseparation = separation * dim[i];

        /* Create Sin and Cos arrays */
        fft_create_arrays(&c, &s, &rev, dim[i]);
        
        /* Negate the sin array */
        for (j = 0; j < dim[i]; j++)
                s[j] = -s[j];
    
        /* Loop over other indices.  First do indicies bigger than i */
        for (j =0; j < nel; j += nextseparation) {
            for (k = 0; k < separation; k++) {
                offset = j+k;
                fft1n(x, logi, offset, separation, c, s, rev);
            }
        }
        /* Give back old sin and cos arrays */
        free(c); free(s);free(rev);
        separation = nextseparation;
    }
}

/* This normalizes the elements of a multidimensional fft so that the forward transform
 * followed by the inverse transform is an identity
 */
void normalize_fftn(x, ndim, dim)
REAL x[][2];            /* Input data points */
int ndim;                       /* Number of dimensions */
int dim[];                      /* Number of points in each dim = 2**nu */
{
    int nel;            /* Number of elements */
    int i;
    
    /* Compute number of elements in array */
    for (i = 0, nel = 1; i < ndim; i++)
        nel *= dim[i];

    for (i = 0; i < nel; i++) {
        x[i][0] /= nel;
        x[i][1] /= nel;
    }
}

 /* This routine normalized the elements of a 1d fft by dividing by the number of elements
  * so that fft, inversefft, normalizefft is an identity
  */
void normalize_fft(REAL (*x)[2], int n)
{
    register int i;
    
    for (i = 0; i < n; i++) {
        x[i][0] /= n;
        x[i][1] /= n;
    }
}
 

/* This routine performs a complex fft on a single index of a
 * multidimensional array.  This routine is intended to be used
 * internally in a full multidimensional fft (on all indicies).  It will
 * be called repeatedly for each of the 1D fft's needed.  This routine is
 * designed primarily to optimize the memory fetches needed for the 1D
 * ffts.  Each element is an array by two holding the real and imaginary
 * parts of the the complex numbers.  Which index on which the fft is to
 * be performed is specified in a somewhat roundabout fashion.  What is
 * passed it an offset and the number of values between elements in the
 * array as if it were considered to be a big 1D array with dimension
 * equal to the product of the linear dimensions.  If the
 * multidimensional array is considered It performs the fft and returns
 * the result in the original arrays.  It destroys the orginal data in
 * the process.  Note the array returned is NOT normalized.  Each element
 * must be divided by n to get dimensionally correct results.  The cosine
 * and sine arrays are optional.  If null is passed, a temp array will be
 * allocated and filled, otherwise the passed arrays are assumed to have
 * the correct data in them.
 * 
 * Note: if the sin array is negated, the routine performs the inverse
 * transform.
 */

void fft1n(x, nu, offset, separation, c, s, rev)
REAL x[][2];            /* Input data points */
int nu;                         /* Number of elements in fft n = 2**nu */
int offset;                     /* Offset of 1st element */
int separation;                 /* Separation between elements */
REAL c[], s[];          /* Cosine and Sine arrays (array by n) */
int rev[];                      /* Array of bitreversed numbers */
{
    char temparray=FALSE;       /* Are we using internal c and s arrays? */
    int n = (1 << nu);          /* Number of data points */
    int dual_space = n; /* Spacing between dual nodes */
    int nu1 = nu;               /* = nu-1 right shift needed when finding p */
    int k;                              /* Iteration of factor array */
    register int i;                     /* Number of dual node pairs considered */
    register int j;             /* Index into factor array */
    
    if (c == NULL || s == NULL || rev == NULL) {
        temparray = TRUE;
        fft_create_arrays(&c, &s, &rev, n);
    }
    
    x += offset;                /* Move to the correct offset */

    /* For each iteration of factor matrix */
    for (k = 0; k < nu; k++) {
        /* Initialize */
        dual_space /= 2;        /* Fewer elements in each set of duals */
        nu1--;                  /* nu1 = nu - 1 */
        
        /* For each set of duals */
        for(j = 0; j < n; j += dual_space) {
            
            /* For each dual node pair */
            for (i = 0; i < dual_space; i++, j++) {
                REAL *pt1, *pt2;
                REAL treal, timag;      /* Temp of w**p */
                register int p = rev[j >> nu1];
                
                pt1 = x[separation * j];
                pt2 = x[separation * (j+dual_space)];

                treal = pt2[0]*c[p] + pt2[1]*s[p];
                timag = pt2[1]*c[p] - pt2[0]*s[p];
                
                pt2[0] = pt1[0] - treal;
                pt2[1] = pt1[1] - timag;
                
                pt1[0] += treal;
                pt1[1] += timag;
            }
        }
    }
    
    
    /* We are done with the transform, now unscamble results */
    for (j = 0; j < n; j++) {
        if ((i = rev[j]) > j) {
            REAL *pt1, *pt2;
            REAL treal, timag;

            pt1 = x[j*separation];
            pt2 = x[i*separation];
            /* Swap */
            treal = pt1[0];
            timag = pt1[1];
            
            pt1[0] = pt2[0];
            pt1[1] = pt2[1];
            
            pt2[0] = treal;
            pt2[1] = timag;
        }
    }
    
     /* Give back the temp storage */
    if (temparray) {
        free(c);
        free(s);
        free(rev);
    }
}

/* This routine takes an array of real numbers and performs a fft.  It
 * returns the magnitude of the fft in the original array.  This routine
 * uses an order n/2 complex ft and disentangles the results.  This is
 * much more efficient than using an order n complex fft with the
 * imaginary component set to zero.  We return the mean in data[0]
 * and the Nyquist frequency in data[n/w].  The rest of data is
 * left untouched.  The results are normalized.
 */

void realfftmag(data, n)
REAL *data;
int n;
{
        REAL (*x)[2];                   /* Temp array used perform fft */
        REAL *dataptr, *xptr;   /* Temp pointer into data array */
        int i;

        x = (REAL (*)[2])emalloc(n * sizeof(REAL));
        
        /* Load data into temp array
         * even terms end up in x[n][0] odd terms in x[n][1]
         */
        for (i = 0, dataptr = data, xptr = (REAL *)x; i < n; i++) 
                *xptr++ = *dataptr++;
        
        fft(x, n/2, NULL, NULL, NULL);

        /* Load results into output array */

        /* i = 0 needs to be treated separately */
        data[0] = (x[0][0] + x[0][1])/n;

        for (i = 1; i < n/2; i++) {
                double xr, xi;
                double  arg, ti, tr;
                double c, s;            /* Cosine and sin */

                arg = 2 * PI * i / n;
                c = cos(arg);   /* These are different c,s than used in fft */
                s = sin(arg);

                ti = (x[i][1] + x[n/2-i][1]) / 2;
                tr = (x[i][0] - x[n/2-i][0]) / 2;

                xr = (x[i][0] + x[n/2-i][0])/2 + c * ti - s * tr;
                xi = (x[i][1] - x[n/2-i][1])/2 - s * ti - c * tr;

                xr /= n/2;
                xi /= n/2;
                
                data[i] = sqrt(sqr(xr) + sqr(xi));
        }
        
        /* Nyquist frequency is returned in data[0] */
        data[n/2] = (x[0][0] - x[0][1])/n;
        
        free(x);
}
        




/* $Id: fftaux.c,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

#include "fft.h"

/* Here are the integer based support routines for the fftlib */

/* This routine reverses the bits in integer */
unsigned bitrev(unsigned int k, int nu)
/* register unsigned k; * Number to reverse the bits of */
/* int nu;              * Number of bits k is represented by */
{
    register int i;
    register unsigned out = 0;
    
    for (i = 0; i < nu; i++) {
        out <<= 1;
        out |= k & 1;
        k >>= 1;
    }
    return(out);
}


/* Computes a^b where a and b are integers */
int ipow(int a, int b)
{
    register int i;
    int sum = 1;
    
    for (i = 0; i < b; i++)     
        sum *= a;
    return (sum);
}

/* Computes log2(n).  Returns -1 if n == 0*/
int ilog2(int n)
{
    register int i;
    
    for (i = -1; n != 0; i++, n>>=1)
        ;
    return(i);
}

/* getindex - gets one real, imag pair from the square multidimensional array */
int getindex(int ndim, int dim, int n, int elem[])
/* int ndim;                      * Number of dimensions */
/* int dim;                        * Dim to extract point from */
/* int n;                           * Number of points in each dim */
/* int elem[];                    * Which element to extract (array by ndim) */
{
    register int i;
    int pos=0;                  /* Position in array when considered 1d */

    for (i = 0; i < ndim && (pos *= n); i++)  /* Loop over dimensions */
        pos += elem[i];

    return(pos);
}


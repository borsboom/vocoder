/* $Id: extended.h,v 1.2 1998/09/13 00:21:18 emanuel Exp $ */

#ifndef EXTENDED_H_INCLUDED
#define EXTENDED_H_INCLUDED

void ConvertToIeeeExtended(double num, unsigned char *bytes);
double ConvertFromIeeeExtended(unsigned char *bytes);

#endif /* EXTENDED_H_INCLUDED */

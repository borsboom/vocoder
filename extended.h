/* $Id: extended.h,v 1.4 2002/09/20 02:30:51 emanuel Exp $ */

#ifndef EXTENDED_H_INCLUDED
#define EXTENDED_H_INCLUDED

void ConvertToIeeeExtended(double num, unsigned char *bytes);
double ConvertFromIeeeExtended(unsigned char *bytes);

#endif /* EXTENDED_H_INCLUDED */

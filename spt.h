/* $Id: spt.h,v 1.2 1998/09/13 00:21:18 emanuel Exp $ */

#ifndef _SPT_
#define _SPT_

#include <stdlib.h>

#define MAXLEN		80		/* Usual input line length for prompts */
#define ESC			0x1b
#define ERROR		-1
#define EPSILON 	1e-15
#ifndef FALSE
#define FALSE	(1==2)
#endif /* FALSE */
#ifndef TRUE
#define TRUE		(1==1)
#endif /* TRUE */
#define SUCCESS	TRUE
#define FAILURE	FALSE
#ifndef NULL
#define NULL		((void *)0)
#endif /* NULL */
#ifndef MAXINT
#define MAXINT		((unsigned)(-1))
#endif /* MAXINT */
#ifndef MAXLONG
#define MAXLONG		((unsigned long)(-1L))
#endif /* MAXLONG */
#ifndef PI
#define PI			3.14159265358978323846
#endif /* PI */
#define ZERO_C		273.15

#define max(a, b)	(((a) > (b)) ? (a) : (b))
#define min(a, b)	(((a) > (b)) ? (b) : (a))
#define abs(a)		(((a) < 0) ? (-(a)) : (a))
#define sgn(x)		(((x) < 0) ? (-1) : (1))
#define round(x)	(sgn(x)*(int)abs((x) + .5))
#define sqr(x)		((x)*(x))
#define ln(x)		log(x)
#define power(x, y)	(exp((y)*log(x)))
#define delcrlf(str)	(str[strlen(str)-1] = '\0')

/* Used to signify a pointer which is intended to cast into several different
 * types.
 */
#define anyptr		void

#define forever 	for(;;)

#endif /* _SPT */

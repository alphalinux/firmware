#ifndef __XLATE_H_LOADED
#define __XLATE_H_LOADED
/*****************************************************************************

       Copyright � 1993, 1994 Digital Equipment Corporation,
                       Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, provided  
that the copyright notice and this permission notice appear in all copies  
of software and supporting documentation, and that the name of Digital not  
be used in advertising or publicity pertaining to distribution of the software 
without specific, written prior permission. Digital grants this permission 
provided that you prominently mark, as not part of the original, any 
modifications made to this software or documentation.

Digital Equipment Corporation disclaims all warranties and/or guarantees  
with regard to this software, including all implied warranties of fitness for 
a particular purpose and merchantability, and makes no representations 
regarding the use of, or the results of the use of, the software and 
documentation in terms of correctness, accuracy, reliability, currentness or
otherwise; and you rely on the software, documentation and results solely at 
your own risk. 

******************************************************************************/

/*
 *  $Id: xlate.h,v 1.1.1.1 1998/12/29 21:36:07 paradis Exp $;
 */

/*
 * $Log: xlate.h,v $
 * Revision 1.1.1.1  1998/12/29 21:36:07  paradis
 * Initial CVS checkin
 *
 * Revision 1.4  1994/08/05  20:13:47  fdh
 * Updated Copyright header and RCS $Id: identifier.
 *
 * Revision 1.3  1994/06/21  14:56:47  rusling
 * fixed up WNT compile warnings.
 *
 * Revision 1.2  1994/06/20  14:18:59  fdh
 * Fixup header file preprocessor #include conditionals.
 *
 * Revision 1.1  1993/06/08  19:56:17  fdh
 * Initial revision
 *
 */



#include "kbdscan.h"

#define MCTRL(x) ((x)&0x1f)

#define ASCII		0
#define SCROLL		1
#define CAPS		2
#define LSHIFT		3
#define LCTRL		4
#define RSHIFT		5
#define RCTRL		6
#define IGNORE		7
#define	ALT		8
#define	FUNCTION	9
#define	NUMLOCK		10
#define	NUMPAD		11
#define	UNKNOWN		12

typedef struct {
	unsigned char	unshifted;
	unsigned char	shifted;
	unsigned char	char_type;
} xlate_t;

extern xlate_t xlate[];

#endif /* __XLATE_H_LOADED */

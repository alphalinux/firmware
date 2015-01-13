
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
 *  $Id: int.h,v 1.1.1.1 1998/12/29 21:36:11 paradis Exp $;
 */

/*
 * $Log: int.h,v $
 * Revision 1.1.1.1  1998/12/29 21:36:11  paradis
 * Initial CVS checkin
 *
 * Revision 1.2  1994/08/05  20:16:23  fdh
 * Updated Copyright header and RCS $Id: identifier.
 *
 * Revision 1.1  1993/06/08  19:56:22  fdh
 * Initial revision
 *
 */



/*
 *  Write-only Interrupt controller registers
 */

#define INT2_OCW1   0xa1
#define INT2_OCW2   0xa0
#define INT2_OCW3   0xa0

#define INT1_OCW1   0x21
#define INT1_OCW2   0x20
#define INT1_OCW3   0x20

/*
 *  Read-only Interrupt controller registers
 */

#define INT2_STATUS 0xa0
#define INT1_STATUS 0x20
#define INT2_MASK   0xa1
#define INT1_MASK   0x21


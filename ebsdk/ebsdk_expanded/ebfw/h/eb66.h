#ifndef __EB66_H_LOADED
#define __EB66_H_LOADED
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
 *  $Id: eb66.h,v 1.1.1.1 1998/12/29 21:36:06 paradis Exp $;
 */

/*
 * $Log: eb66.h,v $
 * Revision 1.1.1.1  1998/12/29 21:36:06  paradis
 * Initial CVS checkin
 *
 * Revision 1.32  1996/08/19  04:09:09  fdh
 * Removed an obsoleted definition.
 *
 * Revision 1.31  1996/05/25  10:51:20  fdh
 * Disabled am79c960 driver again after it was enabled
 * for testing.
 *
 * Revision 1.30  1996/05/22  19:58:55  fdh
 * Added special PCI probe maximum range definition.
 * Removed obsolete definitions for am79c960 driver.
 *
 * Revision 1.29  1995/12/17  22:48:00  fdh
 * Added TARGET_NAME definition.
 *
 * Revision 1.28  1995/12/15  19:27:24  cruz
 * Updated banner to reflect the name change from DECChip to Alpha.
 *
 * Revision 1.27  1995/12/15  16:39:40  cruz
 * Defined FWUPDATE_FILE_ALT.
 *
 * Revision 1.26  1995/11/09  21:25:58  fdh
 * Disabled the am79c960 ethernet driver
 * to create a smaller ROM image.
 *
 * Revision 1.25  1995/11/01  16:24:18  cruz
 * Replaced ALLIED_TELESIS_AT_1500T by NEEDAM79C960_OPTIONAL.
 *
 * Revision 1.24  1995/10/19  20:12:33  cruz
 * Forgot to added comment delimiters.
 *
 * Revision 1.23  1995/10/19  20:08:45  cruz
 * Defined a new compile switch called NEED_ETHERNET_ADDRESS which enables
 * certain portions of the network code when LCA_PASS1 is defined.
 *
 * Revision 1.22  1995/10/03  20:14:36  fdh
 * Added compile conditionals to enable graphics console.
 *
 * Revision 1.21  1995/02/10  02:56:08  fdh
 * Disabled bootopt command because the current SROM doesn't support it.
 *
 * Revision 1.20  1994/12/08  16:42:29  fdh
 * Removed EB66P definitions. Moved to eb66p.h
 *
 * Revision 1.19  1994/11/17  14:15:30  fdh
 * Modified definitions for EB66P
 *
 * Revision 1.18  1994/11/08  21:38:33  fdh
 * Added ROM definitions. Moved from rom.c
 * Also added Flash ROM definitions to support
 * a variant of the EB66 design.
 *
 * Revision 1.17  1994/08/05  20:13:47  fdh
 * Updated Copyright header and RCS $Id: identifier.
 *
 * Revision 1.16  1994/07/21  18:09:32  fdh
 * Added MAXIMUM_SYSTEM_CACHE definition.
 *
 * Revision 1.15  1994/06/28  20:08:21  fdh
 * Modified filenames and build precedure to fit into a FAT filesystem.
 *
 * Revision 1.14  1994/06/20  14:18:59  fdh
 * Fixup header file preprocessor #include conditionals.
 *
 * Revision 1.13  1994/04/06  05:10:07  fdh
 * Removed and ethernet driver.
 *
 * Revision 1.12  1994/04/04  15:16:29  fdh
 * Added definition for LEDPORT.
 *
 * Revision 1.11  1994/04/01  13:58:53  fdh
 * Added MINIMUM_SYSTEM_MEMORY definition and
 * removed obsoleted PAL_IMPURE definition.
 *
 * Revision 1.10  1994/03/24  21:42:48  fdh
 * Removed unnecessary compile-time conditionals.
 *
 * Revision 1.9  1994/03/11  02:55:24  fdh
 * Modified the Amd79c960 definitions to accomodate Ladebug.
 *
 * Revision 1.8  1994/02/14  16:18:14  rusling
 * Allow remote debug support in NT build case.
 *
 * Revision 1.7  1994/01/19  10:22:28  rusling
 * Ported to ALpha Windows NT.
 *
 * Revision 1.6  1993/11/30  15:29:22  rusling
 * Mostly complete versions.  I've integrated changes to
 * the PCI to system address space mappings with all the
 * definitions in the system specific include file.
 *
 * Revision 1.5  1993/11/23  10:43:50  rusling
 * Added in IOC information (taken from pci.h).
 *
 * Revision 1.4  1993/11/22  14:23:03  rusling
 * Modified the am8=79c960 definitions.
 *
 * Revision 1.3  1993/11/22  13:17:55  rusling
 * *** empty log message ***
 *
 * Revision 1.2  1993/11/22  12:16:20  rusling
 * Added in further definitions
 *
 * Revision 1.2  1993/11/22  12:16:20  rusling
 * Added in further definitions
 *
 * Revision 1.1  1993/11/22  11:43:22  rusling
 * Initial revision
 *
 */

#ifdef EB66

#define TARGET_NAME "EB66"
#define BANNER "Alpha 21066 and 21068 Evaluation Board (EB66) Debug Monitor"
#define PROMPT "EB66> "

/****************************************************************************
 * Basic                                                                    *
 ****************************************************************************/

#define NEEDPCI
#define NEEDDEBUGGER
#define NEEDFLOPPY
#define NEED21040
#define DISABLEBOOTOPTION   /* Current SROM does not support this */

#ifndef DISABLE_GRAPHICS
#define NEEDVGA
#define NEEDVGA_BIOSEMULATION
#define NEEDTGA     /* Digital 21030 */
#endif

#ifdef LCA_PASS1
#define NEED_ETHERNET_ADDRESS  /* For Pass1 we don't have access to address */
#endif /* LCA_PASS1 */

/****************************************************************************
 * The IOC registers                                                        *
 ****************************************************************************/

/*
 * Status Register.
 */
#define IOC_REG_STATUS_0           0x40
#define IOC_STATUS_0_M_ERR         0x00000010
					/* 4 = error */
#define IOC_STATUS_0_M_CODE        0x00000700
					/* 10:8 = error code */
#define IOC_STATUS_0_ERROR_RETRY   0
#define IOC_STATUS_0_ERROR_NODEV   1
#define IOC_STATUS_0_ERROR_BADDATA 2
#define IOC_STATUS_0_ERROR_ABORT   3
#define IOC_STATUS_0_ERROR_BADADDR 4
#define IOC_STATUS_0_ERROR_PAGERD  5
#define IOC_STATUS_0_ERROR_BADPAGE 6
#define IOC_STATUS_0_ERROR_DATA    7

/*
 * PCI Target Window Registers (64bits wide).
 */
#define IOC_W_BASE_0               0x100
#define IOC_W_BASE_1               0x120
#define IOC_W_MASK_0               0x140
#define IOC_W_MASK_1               0x160
#define IOC_W_T_BASE_0             0x180
#define IOC_W_T_BASE_1             0x1A0

#define IOC_W_BASE_M_WEN           0x0000000200000000   /* bit 33 */

/* CPU address<28:7> passed to the PCI bus */
#define PCI_T0_LAST_IDSEL	28

/****************************************************************************
 * Plug in cards, what does it have?                                        *
 ****************************************************************************/

/*
 *  The DEC Etherworks ISA card is a LANCE like device.
 */
#define DEC_ETHERWORKS


/****************************************************************************
 * ISA Address Space                                                        *
 ****************************************************************************/

#define RTCBASE 0x70
#define LEDPORT                             0x80  /* JDR Microdevices P.O.S.T. Diagnostics Card. */

/*
 * AMD am79c960 option
 */
#if 0  /* Disabled to create a smaller ROM image */
#define NEEDAM79C960
#endif

#ifdef DEC_ETHERWORKS
#define DEC_ETHERWORKS_BASE                 0x300
#endif

/****************************************************************************
 * System Address Space                                                     *
 ****************************************************************************/

#define MINIMUM_SYSTEM_MEMORY    0x800000
#define MAXIMUM_SYSTEM_CACHE     0x100000
#define DMA_BUFF_BASE            0x100000


/****************************************************************************
 * PCI Address Space                                                        *
 ****************************************************************************/
/*
 * Definitions for the windows mapping PCI addresses into
 * system addresses.
 *
 * 1 megabyte window at 1 megabyte mapping into 1 megabyte.
 */
#define PCI_BASE_1_USED                     1
#define PCI_BASE_1                          0x00100000
#define PCI_MASK_1                          0x00000000
#define PCI_TBASE_1                         0x00100000

#define PCI_BASE_2_USED                     0
#define PCI_BASE_2                          0x00000000
#define PCI_MASK_2                          0x00000000
#define PCI_TBASE_2                         0x00000000

/*
 * Each mask translates to a number of these units.  For
 * 21066 this unit is 1Mbyte.
 */
#define PCI_MASK_UNIT                       0x100000

/*
 * ROM definitions.
 */
#define ROMBASE 0xFFF80000
#define ROMSIZE 0x80000
#define ROMINC 0x1

#endif /* EB66 */
#endif /* __EB66_H_LOADED */

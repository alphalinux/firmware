/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * 31-May-92  Alessandro Forin (af) at Carnegie-Mellon University
 *	Adapted for Alpha.
 *
 * $Log: coff.h,v $
 * Revision 1.1.1.1  1998/12/29 21:36:29  paradis
 * Initial CVS checkin
 *
 * 
 */
/*
 *	File: coff.h
 * 	Author: Alessandro Forin, Carnegie Mellon University
 *	Date:	10/89
 *
 *	Structure definitions for COFF headers
 */

struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	int		f_timdat;	/* time & date stamp */
	vm_offset_t	f_symptr;	/* file pointer to symtab */
	int		f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
};

#define  F_EXEC		0000002

#define ALPHAMAGIC	0603

struct scnhdr {
	char		s_name[8];	/* section name */
	vm_offset_t	s_paddr;	/* physical address */
	vm_offset_t	s_vaddr;	/* virtual address */
	vm_offset_t	s_size;		/* section size */
	vm_offset_t	s_scnptr;	/* file ptr to raw data for section */
	vm_offset_t	s_relptr;	/* file ptr to relocation */
	vm_offset_t	s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	int		s_flags;	/* flags */
};



struct aouthdr {
	short		magic;		/* see magic.h			*/
	short		vstamp;		/* version stamp		*/
	int		pad;		/* help for cross compilers	*/
	vm_offset_t	tsize;		/* text size in bytes, padded to FW
					   bdry				*/
	vm_offset_t	dsize;		/* initialized data "  "	*/
	vm_offset_t	bsize;		/* uninitialized data "   "	*/
	vm_offset_t	entry;		/* entry point, value of "start"*/
	vm_offset_t	text_start;	/* base of text used for this file*/
	vm_offset_t	data_start;	/* base of data used for this file*/
	vm_offset_t	bss_start;	/* base of bss used for this file */
	int		gprmask;	/* general purpose register mask*/
	int		fprmask;	/* FPA register mask		*/
	vm_offset_t	gp_value;	/* the gp value used for this object*/
};


#define OMAGIC	0407		/* old impure format */
#define NMAGIC	0410		/* read-only text */
#define ZMAGIC	0413		/* demand load format */

#define	N_BADMAG(a) \
  ((a).magic != OMAGIC && (a).magic != NMAGIC && (a).magic != ZMAGIC)

#define SCNROUND ((long)16)

struct exechdr {
	struct filehdr	f;
	struct aouthdr	a;
};

#define N_TXTOFF(f, a) \
 (((a).magic == ZMAGIC) ? 0 : \
   (sizeof(struct filehdr) + sizeof(struct aouthdr) + \
    (f).f_nscns * sizeof(struct scnhdr)))

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

#ifndef LINT
static char *rcsid = "$Id: ctype.c,v 1.1.1.1 1998/12/29 21:36:10 paradis Exp $";
#endif

/*
 * $Log: ctype.c,v $
 * Revision 1.1.1.1  1998/12/29 21:36:10  paradis
 * Initial CVS checkin
 *
 * Revision 1.2  1997/05/21  13:01:15  fdh
 * Added RCS Id directive.
 *
 * Revision 1.1  1995/09/02  03:47:17  fdh
 * Initial revision
 *
 */

#include "lib.h"
#include "ctype.h"

const uw ctypeArray[] =
{
/* 0 '\000'*/	_ISASCII_|_ISCNTRL_,
/* 1 '\001'*/	_ISASCII_|_ISCNTRL_,
/* 2 '\002'*/	_ISASCII_|_ISCNTRL_,
/* 3 '\003'*/	_ISASCII_|_ISCNTRL_,
/* 4 '\004'*/	_ISASCII_|_ISCNTRL_,
/* 5 '\005'*/	_ISASCII_|_ISCNTRL_,
/* 6 '\006'*/	_ISASCII_|_ISCNTRL_,
/* 7 '\007'*/	_ISASCII_|_ISCNTRL_,
/* 8 '\010'*/	_ISASCII_|_ISCNTRL_,
/* 9 '\011'*/	_ISASCII_|_ISCNTRL_|_ISSPACE_,
/* 10 '\012'*/	_ISASCII_|_ISCNTRL_|_ISSPACE_,
/* 11 '\013'*/	_ISASCII_|_ISCNTRL_|_ISSPACE_,
/* 12 '\014'*/	_ISASCII_|_ISCNTRL_|_ISSPACE_,
/* 13 '\015'*/	_ISASCII_|_ISCNTRL_|_ISSPACE_,
/* 14 '\016'*/	_ISASCII_|_ISCNTRL_,
/* 15 '\017'*/	_ISASCII_|_ISCNTRL_,
/* 16 '\020'*/	_ISASCII_|_ISCNTRL_,
/* 17 '\021'*/	_ISASCII_|_ISCNTRL_,
/* 18 '\022'*/	_ISASCII_|_ISCNTRL_,
/* 19 '\023'*/	_ISASCII_|_ISCNTRL_,
/* 20 '\024'*/	_ISASCII_|_ISCNTRL_,
/* 21 '\025'*/	_ISASCII_|_ISCNTRL_,
/* 22 '\026'*/	_ISASCII_|_ISCNTRL_,
/* 23 '\027'*/	_ISASCII_|_ISCNTRL_,
/* 24 '\030'*/	_ISASCII_|_ISCNTRL_,
/* 25 '\031'*/	_ISASCII_|_ISCNTRL_,
/* 26 '\032'*/	_ISASCII_|_ISCNTRL_,
/* 27 '\033'*/	_ISASCII_|_ISCNTRL_,
/* 28 '\034'*/	_ISASCII_|_ISCNTRL_,
/* 29 '\035'*/	_ISASCII_|_ISCNTRL_,
/* 30 '\036'*/	_ISASCII_|_ISCNTRL_,
/* 31 '\037'*/	_ISASCII_|_ISCNTRL_,
/* 32 '\040'*/	_ISASCII_|_ISPRINT_|_ISSPACE_,
/* 33 '!'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 34 '"'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 35 '#'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 36 '$'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 37 '%'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 38 '&'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 39 '''*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 40 '('*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 41 ')'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 42 '*'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 43 '+'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 44 ','*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 45 '-'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 46 '.'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 47 '/'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 48 '0'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 49 '1'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 50 '2'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 51 '3'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 52 '4'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 53 '5'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 54 '6'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 55 '7'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 56 '8'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 57 '9'*/	_ISALNUM_|_ISASCII_|_ISDIGIT_|_ISGRAPH_|_ISPRINT_|_ISXDIGIT_,
/* 58 ':'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 59 ';'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 60 '<'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 61 '='*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 62 '>'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 63 '?'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 64 '@'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 65 'A'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 66 'B'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 67 'C'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 68 'D'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 69 'E'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 70 'F'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_|_ISXDIGIT_,
/* 71 'G'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 72 'H'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 73 'I'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 74 'J'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 75 'K'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 76 'L'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 77 'M'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 78 'N'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 79 'O'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 80 'P'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 81 'Q'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 82 'R'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 83 'S'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 84 'T'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 85 'U'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 86 'V'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 87 'W'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 88 'X'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 89 'Y'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 90 'Z'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISUPPER_,
/* 91 '['*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 92 '\'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 93 ']'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 94 '^'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 95 '_'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 96 '`'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 97 'a'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 98 'b'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 99 'c'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 100 'd'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 101 'e'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 102 'f'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_|_ISXDIGIT_,
/* 103 'g'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 104 'h'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 105 'i'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 106 'j'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 107 'k'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 108 'l'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 109 'm'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 110 'n'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 111 'o'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 112 'p'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 113 'q'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 114 'r'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 115 's'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 116 't'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 117 'u'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 118 'v'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 119 'w'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 120 'x'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 121 'y'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 122 'z'*/	_ISALNUM_|_ISALPHA_|_ISASCII_|_ISGRAPH_|_ISLOWER_|_ISPRINT_,
/* 123 '{'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 124 '|'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 125 '}'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 126 '~'*/	_ISASCII_|_ISGRAPH_|_ISPRINT_|_ISPUNCT_,
/* 127 '\177'*/	_ISASCII_|_ISCNTRL_
};

/**************************************************************************
 * Ctype routines                                                         *
 **************************************************************************/
int isalnum(int c) {return(_isalnum(c));}
int isalpha(int c) {return(_isalpha(c));}
int isascii(int c) {return(_isascii(c));}
int iscntrl(int c) {return(_iscntrl(c));}
int isdigit(int c) {return(_isdigit(c));}
int isgraph(int c) {return(_isgraph(c));}
int islower(int c) {return(_islower(c));}
int isprint(int c) {return(_isprint(c));}
int ispunct(int c) {return(_ispunct(c));}
int isspace(int c) {return(_isspace(c));}
int isupper(int c) {return(_isupper(c));}
int isxdigit(int c) {return(_isxdigit(c));}

int tolower(int c)
{
  if ((c >= 'A') && (c <= 'Z')) c = c - 'A' + 'a';
  return(c);
}

int toupper(int c)
{
  if ((c >= 'a') && (c <= 'z')) c = c - 'a' + 'A';
  return(c);
}


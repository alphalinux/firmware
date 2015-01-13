/*---------------------------------------------------------------------------

  msdos.c

  MSDOS-specific routines for use with Info-ZIP's UnZip 5.3 and later.

  Contains:  Opendir()                      (from zip)
             Readdir()                      (from zip)
             do_wild()
             mapattr()
             mapname()
             map2fat()
             checkdir()
             isfloppy()
             volumelabel()                  (non-djgpp, non-emx)
             close_outfile()
             stamp_file()                   (TIMESTAMP only)
             dateformat()
             version()
             _dos_getcountryinfo()          (djgpp 1.x, emx)
            [_dos_getftime()                (djgpp 1.x, emx)   to be added]
             _dos_setftime()                (djgpp 1.x, emx)
             _dos_setfileattr()             (djgpp 1.x, emx)
             _dos_getdrive()                (djgpp 1.x, emx)
             _dos_creat()                   (djgpp 1.x, emx)
             _dos_close()                   (djgpp 1.x, emx)
             volumelabel()                  (djgpp, emx)
             _dos_getcountryinfo()          (djgpp 2.x)
             __crt0_glob_function()         (djgpp 2.x)
             __crt_load_environment_file()  (djgpp 2.x)
             screenlines()                  (emx)
             screencolumns()                (emx)
             int86x_realmode()              (Watcom 32-bit)
             stat_bandaid()                 (Watcom)

  ---------------------------------------------------------------------------*/



#define UNZIP_INTERNAL
#include "unzip.h"

#ifdef MAYBE_PLAIN_FAT
   static void map2fat OF((char *pathcomp, char *last_dot));
#endif
static int isfloppy OF((int nDrive));
static int volumelabel OF((char *newlabel));

static int created_dir;        /* used by mapname(), checkdir() */
static int renamed_fullpath;   /* ditto */
static unsigned nLabelDrive;   /* ditto, plus volumelabel() */



/*****************************/
/*  Strings used in msdos.c  */
/*****************************/

#ifndef SFX
  static char Far CantAllocateWildcard[] =
    "warning:  cannot allocate wildcard buffers\n";
#endif
static char Far Creating[] = "   creating: %s\n";
static char Far ConversionFailed[] = "mapname:  conversion of %s failed\n";
static char Far Labelling[] = "labelling %c: %-22s\n";
static char Far ErrSetVolLabel[] = "mapname:  error setting volume label\n";
static char Far PathTooLong[] = "checkdir error:  path too long: %s\n";
static char Far CantCreateDir[] = "checkdir error:  cannot create %s\n\
                 unable to process %s.\n";
static char Far DirIsntDirectory[] =
  "checkdir error:  %s exists but is not directory\n\
                 unable to process %s.\n";
static char Far PathTooLongTrunc[] =
  "checkdir warning:  path too long; truncating\n                   %s\n\
                -> %s\n";
#if (!defined(SFX) || defined(SFX_EXDIR))
   static char Far CantCreateExtractDir[] =
     "checkdir:  cannot create extraction directory: %s\n";
#endif
#ifdef __TURBOC__
   static char Far AttribsMayBeWrong[] =
     "\nwarning:  file attributes may not be correct\n";
#endif



/****************************/
/*  Macros used in msdos.c  */
/****************************/

#ifdef WATCOMC_386
#  define WREGS(v,r) (v##.w.##r)
#  define int86x int386x
   static int int86x_realmode(int inter_no, union REGS *in,
                              union REGS *out, struct SREGS *seg);
#  define F_intdosx(ir,or,sr) int86x_realmode(0x21, ir, or, sr)
#  define XXX__MK_FP_IS_BROKEN
#else
#  define WREGS(v,r) (v##.x.##r)
#  define F_intdosx(ir,or,sr) intdosx(ir, or, sr)
#endif

#if (defined(__GO32__) || defined(__EMX__))
#  include <dirent.h>        /* use readdir() */
#  define MKDIR(path,mode)   mkdir(path,mode)
#  define Opendir  opendir
#  define Readdir  readdir
#  define Closedir closedir
#  define zdirent  dirent
#  define zDIR     DIR
#  ifdef __EMX__
#    include <dos.h>
#    define GETDRIVE(d)      d = _getdrive()
#    define FA_LABEL         A_LABEL
#  else
#    define GETDRIVE(d)      _dos_getdrive(&d)
#  endif
#else /* !(__GO32__ || __EMX__) */
#  define MKDIR(path,mode)   mkdir(path)
#  ifdef __TURBOC__
#    define FATTR            FA_HIDDEN+FA_SYSTEM+FA_DIREC
#    define FVOLID           FA_VOLID
#    define FFIRST(n,d,a)    findfirst(n,(struct ffblk *)d,a)
#    define FNEXT(d)         findnext((struct ffblk *)d)
#    define GETDRIVE(d)      d=getdisk()+1
#    include <dir.h>
#  else /* !__TURBOC__ */
#    define FATTR            _A_HIDDEN+_A_SYSTEM+_A_SUBDIR
#    define FVOLID           _A_VOLID
#    define FFIRST(n,d,a)    _dos_findfirst(n,a,(struct find_t *)d)
#    define FNEXT(d)         _dos_findnext((struct find_t *)d)
#    define GETDRIVE(d)      _dos_getdrive(&d)
#    include <direct.h>
#  endif /* ?__TURBOC__ */
   typedef struct zdirent {
       char d_reserved[30];
       char d_name[13];
       int d_first;
   } zDIR;
   zDIR *Opendir OF((const char *));
   struct zdirent *Readdir OF((zDIR *));
#  define Closedir free




#ifndef SFX

/**********************/   /* Borland C++ 3.x has its own opendir/readdir */
/* Function Opendir() */   /*  library routines, but earlier versions don't, */
/**********************/   /*  so use ours regardless */

zDIR *Opendir(name)
    const char *name;        /* name of directory to open */
{
    zDIR *dirp;              /* malloc'd return value */
    char *nbuf;              /* malloc'd temporary string */
    int len = strlen(name);  /* path length to avoid strlens and strcats */


    if ((dirp = (zDIR *)malloc(sizeof(zDIR))) == (zDIR *)NULL)
        return (zDIR *)NULL;
    if ((nbuf = malloc(len + 6)) == (char *)NULL) {
        free(dirp);
        return (zDIR *)NULL;
    }
    strcpy(nbuf, name);
    if (nbuf[len-1] == ':') {
        nbuf[len++] = '.';
    } else if (nbuf[len-1] == '/' || nbuf[len-1] == '\\')
        --len;
    strcpy(nbuf+len, "/*.*");
    Trace((stderr, "Opendir:  nbuf = [%s]\n", nbuf));

    if (FFIRST(nbuf, dirp, FATTR)) {
        free((zvoid *)nbuf);
        return (zDIR *)NULL;
    }
    free((zvoid *)nbuf);
    dirp->d_first = 1;
    return dirp;
}





/**********************/
/* Function Readdir() */
/**********************/

struct zdirent *Readdir(d)
    zDIR *d;        /* directory stream from which to read */
{
    /* Return pointer to first or next directory entry, or NULL if end. */

    if (d->d_first)
        d->d_first = 0;
    else
        if (FNEXT(d))
            return (struct zdirent *)NULL;
    return (struct zdirent *)d;
}

#endif /* !SFX */
#endif /* ?(__GO32__ || __EMX__) */





#ifndef SFX

/************************/
/*  Function do_wild()  */   /* identical to OS/2 version */
/************************/

char *do_wild(__G__ wildspec)
    __GDEF
    char *wildspec;          /* only used first time on a given dir */
{
    static zDIR *dir = (zDIR *)NULL;
    static char *dirname, *wildname, matchname[FILNAMSIZ];
    static int firstcall=TRUE, have_dirname, dirnamelen;
    struct zdirent *file;


    /* Even when we're just returning wildspec, we *always* do so in
     * matchname[]--calling routine is allowed to append four characters
     * to the returned string, and wildspec may be a pointer to argv[].
     */
    if (firstcall) {        /* first call:  must initialize everything */
        firstcall = FALSE;

        if (!iswild(wildspec)) {
            strcpy(matchname, wildspec);
            have_dirname = FALSE;
            dir = NULL;
            return matchname;
        }

        /* break the wildspec into a directory part and a wildcard filename */
        if ((wildname = strrchr(wildspec, '/')) == (char *)NULL &&
            (wildname = strrchr(wildspec, ':')) == (char *)NULL) {
            dirname = ".";
            dirnamelen = 1;
            have_dirname = FALSE;
            wildname = wildspec;
        } else {
            ++wildname;     /* point at character after '/' or ':' */
            dirnamelen = (int)(wildname - wildspec);
            if ((dirname = (char *)malloc(dirnamelen+1)) == (char *)NULL) {
                Info(slide, 1, ((char *)slide,
                  LoadFarString(CantAllocateWildcard)));
                strcpy(matchname, wildspec);
                return matchname;   /* but maybe filespec was not a wildcard */
            }
/* GRR:  can't strip trailing char for opendir since might be "d:/" or "d:"
 *       (would have to check for "./" at end--let opendir handle it instead) */
            strncpy(dirname, wildspec, dirnamelen);
            dirname[dirnamelen] = '\0';       /* terminate for strcpy below */
            have_dirname = TRUE;
        }
        Trace((stderr, "do_wild:  dirname = [%s]\n", dirname));

        if ((dir = Opendir(dirname)) != (zDIR *)NULL) {
            while ((file = Readdir(dir)) != (struct zdirent *)NULL) {
                Trace((stderr, "do_wild:  readdir returns %s\n", file->d_name));
                if (match(file->d_name, wildname, 1)) {  /* 1 == ignore case */
                    Trace((stderr, "do_wild:  match() succeeds\n"));
                    if (have_dirname) {
                        strcpy(matchname, dirname);
                        strcpy(matchname+dirnamelen, file->d_name);
                    } else
                        strcpy(matchname, file->d_name);
                    return matchname;
                }
            }
            /* if we get to here directory is exhausted, so close it */
            Closedir(dir);
            dir = (zDIR *)NULL;
        }
        Trace((stderr, "do_wild:  Opendir(%s) returns NULL\n", dirname));

        /* return the raw wildspec in case that works (e.g., directory not
         * searchable, but filespec was not wild and file is readable) */
        strcpy(matchname, wildspec);
        return matchname;
    }

    /* last time through, might have failed opendir but returned raw wildspec */
    if (dir == (zDIR *)NULL) {
        firstcall = TRUE;  /* nothing left to try--reset for new wildspec */
        if (have_dirname)
            free(dirname);
        return (char *)NULL;
    }

    /* If we've gotten this far, we've read and matched at least one entry
     * successfully (in a previous call), so dirname has been copied into
     * matchname already.
     */
    while ((file = Readdir(dir)) != (struct zdirent *)NULL)
        if (match(file->d_name, wildname, 1)) {   /* 1 == ignore case */
            if (have_dirname) {
                /* strcpy(matchname, dirname); */
                strcpy(matchname+dirnamelen, file->d_name);
            } else
                strcpy(matchname, file->d_name);
            return matchname;
        }

    Closedir(dir);     /* have read at least one dir entry; nothing left */
    dir = (zDIR *)NULL;
    firstcall = TRUE;  /* reset for new wildspec */
    if (have_dirname)
        free(dirname);
    return (char *)NULL;

} /* end function do_wild() */

#endif /* !SFX */





/**********************/
/* Function mapattr() */
/**********************/

int mapattr(__G)
    __GDEF
{
    /* set archive bit (file is not backed up): */
    G.pInfo->file_attr = (unsigned)(G.crec.external_file_attributes | 32) &
      0xff;
    return 0;

} /* end function mapattr() */





/************************/
/*  Function mapname()  */
/************************/
                             /* return 0 if no error, 1 if caution (filename */
int mapname(__G__ renamed)   /*  truncated), 2 if warning (skip file because */
    __GDEF                   /*  dir doesn't exist), 3 if error (skip file), */
    int renamed;             /*  or 10 if out of memory (skip file) */
{                            /*  [also IZ_VOL_LABEL, IZ_CREATED_DIR] */
    char pathcomp[FILNAMSIZ];      /* path-component buffer */
    char *pp, *cp=(char *)NULL;    /* character pointers */
    char *lastsemi=(char *)NULL;   /* pointer to last semi-colon in pathcomp */
#ifdef MAYBE_PLAIN_FAT
    char *last_dot=(char *)NULL;   /* last dot not converted to underscore */
    int dotname = FALSE;           /* path component begins with dot? */
# ifdef USE_LFN
    int use_lfn = USE_LFN;         /* file system supports long filenames? */
# endif
#endif
    int error = 0;
    register unsigned workch;      /* hold the character being tested */


/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    /* can create path as long as not just freshening, or if user told us */
    G.create_dirs = (!G.fflag || renamed);

    created_dir = FALSE;        /* not yet */
    renamed_fullpath = FALSE;

    if (renamed) {
        cp = G.filename - 1;    /* point to beginning of renamed name... */
        while (*++cp)
            if (*cp == '\\')    /* convert backslashes to forward */
                *cp = '/';
        cp = G.filename;
        /* use temporary rootpath if user gave full pathname */
        if (G.filename[0] == '/') {
            renamed_fullpath = TRUE;
            pathcomp[0] = '/';  /* copy the '/' and terminate */
            pathcomp[1] = '\0';
            ++cp;
        } else if (isalpha(G.filename[0]) && G.filename[1] == ':') {
            renamed_fullpath = TRUE;
            pp = pathcomp;
            *pp++ = *cp++;      /* copy the "d:" (+ '/', possibly) */
            *pp++ = *cp++;
            if (*cp == '/')
                *pp++ = *cp++;  /* otherwise add "./"? */
            *pp = '\0';
        }
    }

    /* pathcomp is ignored unless renamed_fullpath is TRUE: */
    if ((error = checkdir(__G__ pathcomp, INIT)) != 0) /* initialize path buf */
        return error;           /* ...unless no mem or vol label on hard disk */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (!renamed) {             /* cp already set if renamed */
        if (G.jflag)            /* junking directories */
            cp = (char *)strrchr(G.filename, '/');
        if (cp == (char *)NULL) /* no '/' or not junking dirs */
            cp = G.filename;    /* point to internal zipfile-member pathname */
        else
            ++cp;               /* point to start of last component of path */
    }

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
#ifdef MAYBE_PLAIN_FAT
# ifdef USE_LFN
                if (!use_lfn)
# endif
                {
                    map2fat(pathcomp, last_dot);   /* 8.3 trunc. (in place) */
                    last_dot = (char *)NULL;
                }
#endif
                if ((error = checkdir(__G__ pathcomp, APPEND_DIR)) > 1)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = (char *)NULL; /* leave directory semi-colons alone */
                break;

            /* drive names are not stored in zipfile, so no colons allowed;
             *  no brackets or most other punctuation either (all of which
             *  can appear in Unix-created archives; backslash is particularly
             *  bad unless all necessary directories exist) */
#ifdef MAYBE_PLAIN_FAT
            case '[':          /* these punctuation characters forbidden */
            case ']':          /*  only on plain FAT file systems */
            case '+':
            case ',':
            case '=':
# ifdef USE_LFN
                if (use_lfn)
                    *pp++ = (char)workch;
                else
                    *pp++ = '_';
                break;
# endif
#endif
            case ':':           /* special shell characters of command.com */
            case '\\':          /*  (device and directory limiters, wildcard */
            case '"':           /*  characters, stdin/stdout redirection and */
            case '<':           /*  pipe indicators and the quote sign) are */
            case '>':           /*  never allowed in filenames on (V)FAT */
            case '|':
            case '*':
            case '?':
                *pp++ = '_';
                break;

#ifdef MAYBE_PLAIN_FAT
            case '.':
# ifdef USE_LFN
                if (use_lfn) {
                    *pp++ = (char)workch;
                    break;
                }
# endif
                if (pp == pathcomp) {     /* nothing appended yet... */
                    if (*cp == '/') {     /* don't bother appending a "./" */
                        ++cp;             /*  component to the path:  skip */
                        break;            /*  to next char after the '/' */
                    } else if (*cp == '.' && cp[1] == '/') {   /* "../" */
                        *pp++ = '.';      /* add first dot, unchanged... */
                        ++cp;             /* skip second dot, since it will */
                    } else {              /*  be "added" at end of if-block */
                        *pp++ = '_';      /* FAT doesn't allow null filename */
                        dotname = TRUE;   /*  bodies, so map .exrc -> _.exrc */
                    }                     /*  (extra '_' now, "dot" below) */
                } else if (dotname) {     /* found a second dot, but still */
                    dotname = FALSE;      /*  have extra leading underscore: */
                    *pp = '\0';           /*  remove it by shifting chars */
                    pp = pathcomp + 1;    /*  left one space (e.g., .p1.p2: */
                    while (pp[1]) {       /*  __p1 -> _p1_p2 -> _p1.p2 when */
                        *pp = pp[1];      /*  finished) [opt.:  since first */
                        ++pp;             /*  two chars are same, can start */
                    }                     /*  shifting at second position] */
                }
                last_dot = pp;    /* point at last dot so far... */
                *pp++ = '_';      /* convert dot to underscore for now */
                break;
#endif /* MAYBE_PLAIN_FAT */

            case ';':             /* start of VMS version? */
                lastsemi = pp;
#ifdef MAYBE_PLAIN_FAT
# ifdef USE_LFN
                if (use_lfn)
                    *pp++ = ';';  /* keep for now; remove VMS ";##" later */
# endif
#else
                *pp++ = ';';      /* keep for now; remove VMS ";##" later */
#endif
                break;

#ifdef MAYBE_PLAIN_FAT
            case ' ':                      /* change spaces to underscores */
# ifdef USE_LFN
                if (!use_lfn && G.sflag)   /*  only if requested and NO lfn! */
# else
                if (G.sflag)               /*  only if requested */
# endif
                    *pp++ = '_';
                else
                    *pp++ = (char)workch;
                break;
#endif /* MAYBE_PLAIN_FAT */

            default:
                /* allow ASCII 255 and European characters in filenames: */
                if (isprint(workch) || workch >= 127)
                    *pp++ = (char)workch;

        } /* end switch */
    } /* end while loop */

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended ";###") */
    if (!G.V_flag && lastsemi) {
#ifndef MAYBE_PLAIN_FAT
        pp = lastsemi + 1;
#else
# ifdef USE_LFN
        if (use_lfn)
            pp = lastsemi + 1;
        else
            pp = lastsemi;        /* semi-colon was omitted:  expect all #'s */
# else
        pp = lastsemi;            /* semi-colon was omitted:  expect all #'s */
# endif
#endif
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }

    if (G.pInfo->vollabel) {
        if (strlen(pathcomp) > 11)
            pathcomp[11] = '\0';
    }

#ifdef MAYBE_PLAIN_FAT
# ifdef USE_LFN
    if (!use_lfn)
        map2fat(pathcomp, last_dot);  /* 8.3 truncation (in place) */
# else
    map2fat(pathcomp, last_dot);  /* 8.3 truncation (in place) */
# endif
#endif

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (G.filename[strlen(G.filename) - 1] == '/') {
        checkdir(__G__ G.filename, GETPATH);
        if (created_dir) {
            if (QCOND2) {
                Info(slide, 0, ((char *)slide, LoadFarString(Creating),
                  FnFilter1(G.filename)));
            }
            return IZ_CREATED_DIR;   /* set dir time (note trailing '/') */
        }
        return 2;   /* dir existed already; don't look for data to extract */
    }

    if (*pathcomp == '\0') {
        Info(slide, 1, ((char *)slide, LoadFarString(ConversionFailed),
          FnFilter1(G.filename)));
        return 3;
    }

    checkdir(__G__ pathcomp, APPEND_NAME);  /* returns 1 if truncated: care? */
    checkdir(__G__ G.filename, GETPATH);

    if (G.pInfo->vollabel) {    /* set the volume label now */
        if (QCOND2)
            Info(slide, 0, ((char *)slide, LoadFarString(Labelling),
              (nLabelDrive + 'a' - 1),
              FnFilter1(G.filename)));
        if (volumelabel(G.filename)) {
            Info(slide, 1, ((char *)slide, LoadFarString(ErrSetVolLabel)));
            return 3;
        }
        return 2;   /* success:  skip the "extraction" quietly */
    }

    return error;

} /* end function mapname() */





#ifdef MAYBE_PLAIN_FAT

/**********************/
/* Function map2fat() */
/**********************/

static void map2fat(pathcomp, last_dot)
    char *pathcomp, *last_dot;
{
    char *pEnd = pathcomp + strlen(pathcomp);

/*---------------------------------------------------------------------------
    Case 1:  filename has no dot, so figure out if we should add one.  Note
    that the algorithm does not try to get too fancy:  if there are no dots
    already, the name either gets truncated at 8 characters or the last un-
    derscore is converted to a dot (only if more characters are saved that
    way).  In no case is a dot inserted between existing characters.

              GRR:  have problem if filename is volume label??

  ---------------------------------------------------------------------------*/

    if (last_dot == (char *)NULL) {   /* no dots:  check for underscores... */
        char *plu = strrchr(pathcomp, '_');   /* pointer to last underscore */

        if (plu == (char *)NULL) {  /* no dots, no underscores:  truncate at */
            if (pEnd > pathcomp+8)  /* 8 chars (could insert '.' and keep 11) */
                *(pEnd = pathcomp+8) = '\0';
        } else if (MIN(plu - pathcomp, 8) + MIN(pEnd - plu - 1, 3) > 8) {
            last_dot = plu;       /* be lazy:  drop through to next if-block */
        } else if ((pEnd - pathcomp) > 8)    /* more fits into just basename */
            pathcomp[8] = '\0';    /* than if convert last underscore to dot */
        /* else whole thing fits into 8 chars or less:  no change */
    }

/*---------------------------------------------------------------------------
    Case 2:  filename has dot in it, so truncate first half at 8 chars (shift
    extension if necessary) and second half at three.
  ---------------------------------------------------------------------------*/

    if (last_dot != (char *)NULL) {   /* one dot (or two, in the case of */
        *last_dot = '.';              /*  "..") is OK:  put it back in */

        if ((last_dot - pathcomp) > 8) {
            char *p, *q;
            int i;

            p = last_dot;
            q = last_dot = pathcomp + 8;
            for (i = 0;  (i < 4) && *p;  ++i)  /* too many chars in basename: */
                *q++ = *p++;                   /*  shift extension left and */
            *q = '\0';                         /*  truncate/terminate it */
        } else if ((pEnd - last_dot) > 4)
            last_dot[4] = '\0';                /* too many chars in extension */
        /* else filename is fine as is:  no change */

        if ((last_dot - pathcomp) > 0 && last_dot[-1] == ' ')
            last_dot[-1] = '_';                /* NO blank in front of '.'! */
    }
} /* end function map2fat() */

#endif /* MAYBE_PLAIN_FAT */





/***********************/
/* Function checkdir() */
/***********************/

int checkdir(__G__ pathcomp, flag)
    __GDEF
    char *pathcomp;
    int flag;
/*
 * returns:  1 - (on APPEND_NAME) truncated filename
 *           2 - path doesn't exist, not allowed to create
 *           3 - path doesn't exist, tried to create and failed; or
 *               path exists and is not a directory, but is supposed to be
 *           4 - path is too long
 *          10 - can't allocate memory for filename buffers
 */
{
    static int rootlen = 0;   /* length of rootpath */
    static char *rootpath;    /* user's "extract-to" directory */
    static char *buildpath;   /* full path (so far) to extracted file */
    static char *end;         /* pointer to end of buildpath ('\0') */
#ifdef MSC
    int attrs;                /* work around MSC stat() bug */
#endif

#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)



/*---------------------------------------------------------------------------
    APPEND_DIR:  append the path component to the path being built and check
    for its existence.  If doesn't exist and we are creating directories, do
    so for this one; else signal success or error as appropriate.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_DIR) {
        int too_long = FALSE;

        Trace((stderr, "appending dir segment [%s]\n", pathcomp));
        while ((*end = *pathcomp++) != '\0')
            ++end;

        /* GRR:  could do better check, see if overrunning buffer as we go:
         * check end-buildpath after each append, set warning variable if
         * within 20 of FILNAMSIZ; then if var set, do careful check when
         * appending.  Clear variable when begin new path. */

        if ((end-buildpath) > FILNAMSIZ-3)  /* need '/', one-char name, '\0' */
            too_long = TRUE;                /* check if extracting directory? */
#ifdef MSC /* MSC 6.00 bug:  stat(non-existent-dir) == 0 [exists!] */
        if (_dos_getfileattr(buildpath, &attrs) || stat(buildpath, &G.statbuf))
#else
        if (SSTAT(buildpath, &G.statbuf))    /* path doesn't exist */
#endif
        {
            if (!G.create_dirs) { /* told not to create (freshening) */
                free(buildpath);
                return 2;         /* path doesn't exist:  nothing to do */
            }
            if (too_long) {
                Info(slide, 1, ((char *)slide, LoadFarString(PathTooLong),
                  FnFilter1(buildpath)));
                free(buildpath);
                return 4;         /* no room for filenames:  fatal */
            }
            if (MKDIR(buildpath, 0777) == -1) {   /* create the directory */
                Info(slide, 1, ((char *)slide, LoadFarString(CantCreateDir),
                  FnFilter2(buildpath), FnFilter1(G.filename)));
                free(buildpath);
                return 3;      /* path didn't exist, tried to create, failed */
            }
            created_dir = TRUE;
        } else if (!S_ISDIR(G.statbuf.st_mode)) {
            Info(slide, 1, ((char *)slide, LoadFarString(DirIsntDirectory),
              FnFilter2(buildpath), FnFilter1(G.filename)));
            free(buildpath);
            return 3;          /* path existed but wasn't dir */
        }
        if (too_long) {
            Info(slide, 1, ((char *)slide, LoadFarString(PathTooLong),
              FnFilter1(buildpath)));
            free(buildpath);
            return 4;         /* no room for filenames:  fatal */
        }
        *end++ = '/';
        *end = '\0';
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(buildpath)));
        return 0;

    } /* end if (FUNCTION == APPEND_DIR) */

/*---------------------------------------------------------------------------
    GETPATH:  copy full path to the string pointed at by pathcomp, and free
    buildpath.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == GETPATH) {
        strcpy(pathcomp, buildpath);
        Trace((stderr, "getting and freeing path [%s]\n",
          FnFilter1(pathcomp)));
        free(buildpath);
        buildpath = end = (char *)NULL;
        return 0;
    }

/*---------------------------------------------------------------------------
    APPEND_NAME:  assume the path component is the filename; append it and
    return without checking for existence.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_NAME) {
#ifdef NOVELL_BUG_WORKAROUND
        if (end == buildpath && !G.pInfo->vollabel) {
            /* work-around for Novell's "overwriting executables" bug:
               prepend "./" to name when no path component is specified */
            *end++ = '.';
            *end++ = '/';
        }
#endif /* NOVELL_BUG_WORKAROUND */
        Trace((stderr, "appending filename [%s]\n", FnFilter1(pathcomp)));
        while ((*end = *pathcomp++) != '\0') {
            ++end;
            if ((end-buildpath) >= FILNAMSIZ) {
                *--end = '\0';
                Info(slide, 1, ((char *)slide, LoadFarString(PathTooLongTrunc),
                  FnFilter1(G.filename), FnFilter2(buildpath)));
                return 1;   /* filename truncated */
            }
        }
        Trace((stderr, "buildpath now = [%s]\n", FnFilter1(buildpath)));
        return 0;  /* could check for existence here, prompt for new name... */
    }

/*---------------------------------------------------------------------------
    INIT:  allocate and initialize buffer space for the file currently being
    extracted.  If file was renamed with an absolute path, don't prepend the
    extract-to path.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == INIT) {
        Trace((stderr, "initializing buildpath to "));
        /* allocate space for full filename, root path, and maybe "./" */
        if ((buildpath = (char *)malloc(strlen(G.filename)+rootlen+3)) ==
            (char *)NULL)
            return 10;
        if (G.pInfo->vollabel) {
/* GRR:  for network drives, do strchr() and return IZ_VOL_LABEL if not [1] */
            if (renamed_fullpath && pathcomp[1] == ':')
                *buildpath = (char)ToLower(*pathcomp);
            else if (!renamed_fullpath && rootlen > 1 && rootpath[1] == ':')
                *buildpath = (char)ToLower(*rootpath);
            else {
                GETDRIVE(nLabelDrive);   /* assumed that a == 1, b == 2, etc. */
                *buildpath = (char)(nLabelDrive - 1 + 'a');
            }
            nLabelDrive = *buildpath - 'a' + 1;        /* save for mapname() */
            if (G.volflag == 0 || *buildpath < 'a' ||  /* no label/bogus disk */
               (G.volflag == 1 && !isfloppy(nLabelDrive)))  /* -$:  no fixed */
            {
                free(buildpath);
                return IZ_VOL_LABEL;     /* skipping with message */
            }
            *buildpath = '\0';
            end = buildpath;
        } else if (renamed_fullpath) {   /* pathcomp = valid data */
            end = buildpath;
            while ((*end = *pathcomp++) != '\0')
                ++end;
        } else if (rootlen > 0) {
            strcpy(buildpath, rootpath);
            end = buildpath + rootlen;
        } else {
            *buildpath = '\0';
            end = buildpath;
        }
        Trace((stderr, "[%s]\n", FnFilter1(buildpath)));
        return 0;
    }

/*---------------------------------------------------------------------------
    ROOT:  if appropriate, store the path in rootpath and create it if neces-
    sary; else assume it's a zipfile member and return.  This path segment
    gets used in extracting all members from every zipfile specified on the
    command line.  Note that under OS/2 and MS-DOS, if a candidate extract-to
    directory specification includes a drive letter (leading "x:"), it is
    treated just as if it had a trailing '/'--that is, one directory level
    will be created if the path doesn't exist, unless this is otherwise pro-
    hibited (e.g., freshening).
  ---------------------------------------------------------------------------*/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (FUNCTION == ROOT) {
        Trace((stderr, "initializing root path to [%s]\n",
          FnFilter1(pathcomp)));
        if (pathcomp == (char *)NULL) {
            rootlen = 0;
            return 0;
        }
        if ((rootlen = strlen(pathcomp)) > 0) {
            int had_trailing_pathsep=FALSE, has_drive=FALSE, xtra=2;

            if (isalpha(pathcomp[0]) && pathcomp[1] == ':')
                has_drive = TRUE;   /* drive designator */
            if (pathcomp[rootlen-1] == '/' || pathcomp[rootlen-1] == '\\') {
                pathcomp[--rootlen] = '\0';
                had_trailing_pathsep = TRUE;
            }
            if (has_drive && (rootlen == 2)) {
                if (!had_trailing_pathsep)   /* i.e., original wasn't "x:/" */
                    xtra = 3;      /* room for '.' + '/' + 0 at end of "x:" */
            } else if (rootlen > 0) {     /* need not check "x:." and "x:/" */
#ifdef MSC
                /* MSC 6.00 bug:  stat(non-existent-dir) == 0 [exists!] */
                if (_dos_getfileattr(pathcomp, &attrs) ||
                    SSTAT(pathcomp,&G.statbuf) || !S_ISDIR(G.statbuf.st_mode))
#else
                if (SSTAT(pathcomp,&G.statbuf) || !S_ISDIR(G.statbuf.st_mode))
#endif
                {
                    /* path does not exist */
                    if (!G.create_dirs /* || iswild(pathcomp) */ ) {
                        rootlen = 0;
                        return 2;   /* treat as stored file */
                    }
/* GRR:  scan for wildcard characters?  OS-dependent...  if find any, return 2:
 * treat as stored file(s) */
                    /* create directory (could add loop here to scan pathcomp
                     * and create more than one level, but really necessary?) */
                    if (MKDIR(pathcomp, 0777) == -1) {
                        Info(slide, 1, ((char *)slide,
                          LoadFarString(CantCreateExtractDir),
                          FnFilter1(pathcomp)));
                        rootlen = 0;   /* path didn't exist, tried to create, */
                        return 3;  /* failed:  file exists, or need 2+ levels */
                    }
                }
            }
            if ((rootpath = (char *)malloc(rootlen+xtra)) == (char *)NULL) {
                rootlen = 0;
                return 10;
            }
            strcpy(rootpath, pathcomp);
            if (xtra == 3)                  /* had just "x:", make "x:." */
                rootpath[rootlen++] = '.';
            rootpath[rootlen++] = '/';
            rootpath[rootlen] = '\0';
            Trace((stderr, "rootpath now = [%s]\n", FnFilter1(rootpath)));
        }
        return 0;
    }
#endif /* !SFX || SFX_EXDIR */

/*---------------------------------------------------------------------------
    END:  free rootpath, immediately prior to program exit.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == END) {
        Trace((stderr, "freeing rootpath\n"));
        if (rootlen > 0) {
            free(rootpath);
            rootlen = 0;
        }
        return 0;
    }

    return 99;  /* should never reach */

} /* end function checkdir() */






/***********************/
/* Function isfloppy() */
/***********************/

static int isfloppy(nDrive)  /* more precisely, is it removable? */
    int nDrive;
{
    union REGS regs;

    regs.h.ah = 0x44;
    regs.h.al = 0x08;
    regs.h.bl = (uch)nDrive;
#ifdef __EMX__
    _int86(0x21, &regs, &regs);
    if (WREGS(regs,flags) & 1)
#else
    intdos(&regs, &regs);
    if (WREGS(regs,cflag))        /* error:  do default a/b check instead */
#endif
    {
        Trace((stderr,
          "error in DOS function 0x44 (AX = 0x%04x):  guessing instead...\n",
          WREGS(regs,ax)));
        return (nDrive == 1 || nDrive == 2)? TRUE : FALSE;
    } else
        return WREGS(regs,ax)? FALSE : TRUE;
}




#if (!defined(__GO32__) && !defined(__EMX__))

typedef struct dosfcb {
    uch  flag;        /* ff to indicate extended FCB */
    char res[5];      /* reserved */
    uch  vattr;       /* attribute */
    uch  drive;       /* drive (1=A, 2=B, ...) */
    uch  vn[11];      /* file or volume name */
    char dmmy[5];
    uch  nn[11];      /* holds new name if renaming (else reserved) */
    char dmmy2[9];
} dos_fcb;

/**************************/
/* Function volumelabel() */
/**************************/

static int volumelabel(newlabel)
    char *newlabel;
{
#ifdef DEBUG
    char *p;
#endif
    int len = strlen(newlabel);
    int fcbseg, dtaseg, fcboff, dtaoff, retv;
    dos_fcb  fcb, dta, far *pfcb=&fcb, far *pdta=&dta;
    struct SREGS sregs;
    union REGS regs;


/*---------------------------------------------------------------------------
    Label the diskette specified by nLabelDrive using FCB calls.  (Old ver-
    sions of MS-DOS and OS/2 DOS boxes can't use DOS function 3Ch to create
    labels.)  Must use far pointers for MSC FP_* macros to work; must pad
    FCB filenames with spaces; and cannot include dot in 8th position.  May
    or may not need to zero out FCBs before using; do so just in case.
  ---------------------------------------------------------------------------*/

#ifdef WATCOMC_386
    int truseg;

    memset(&sregs, 0, sizeof(sregs));
    memset(&regs, 0, sizeof(regs));
    /* PMODE/W does not support extended versions of any dos FCB functions, */
    /* so we have to use brute force, allocating real mode memory for them. */
    regs.w.ax = 0x0100;
    regs.w.bx = (2 * sizeof(dos_fcb) + 15) >> 4;   /* size in paragraphs */
    int386(0x31, &regs, &regs);            /* DPMI allocate DOS memory */
    if (regs.w.cflag)
        return DF_MDY;                     /* no memory, return default */
    truseg = regs.w.dx;                    /* protected mode selector */
    dtaseg = regs.w.ax;                    /* real mode paragraph */
    fcboff = 0;
    dtaoff = sizeof(dos_fcb);
#ifdef XXX__MK_FP_IS_BROKEN
    /* XXX  This code may not be trustworthy in general, though it is   */
    /* valid with DOS/4GW and PMODE/w, which is all we support for now. */
    regs.w.ax = 6;
    regs.w.bx = truseg;
    int386(0x31, &regs, &regs);            /* convert seg to linear address */
    pfcb = (dos_fcb far *) (((ulg) regs.w.cx << 16) | regs.w.dx);
    /* pfcb = (dos_fcb far *) ((ulg) dtaseg << 4); */
    pdta = pfcb + 1;
#else
    pfcb = MK_FP(truseg, fcboff);
    pdta = MK_FP(truseg, dtaoff);
#endif
    _fmemset((char far *)pfcb, 0, 2 * sizeof(dos_fcb));
    /* we pass the REAL MODE paragraph to the dos interrupts: */
    fcbseg = dtaseg;

#else /* !WATCOMC_386 */

    memset((char *)&dta, 0, sizeof(dos_fcb));
    memset((char *)&fcb, 0, sizeof(dos_fcb));
    fcbseg = FP_SEG(pfcb);
    fcboff = FP_OFF(pfcb);
    dtaseg = FP_SEG(pdta);
    dtaoff = FP_OFF(pdta);
#endif /* ?WATCOMC_386 */

#ifdef DEBUG
    for (p = (char *)&dta; (p - (char *)&dta) < sizeof(dos_fcb); ++p)
        if (*p)
            fprintf(stderr, "error:  dta[%d] = %x\n", (p - (char *)&dta), *p);
    for (p = (char *)&fcb; (p - (char *)&fcb) < sizeof(dos_fcb); ++p)
        if (*p)
            fprintf(stderr, "error:  fcb[%d] = %x\n", (p - (char *)&fcb), *p);
    printf("testing pointer macros:\n");
    segread(&sregs);
    printf("cs = %x, ds = %x, es = %x, ss = %x\n", sregs.cs, sregs.ds, sregs.es,
      sregs.ss);
#endif /* DEBUG */

#if 0
#ifdef __TURBOC__
    bdosptr(0x1a, dta, DO_NOT_CARE);
#else
    (intdosx method below)
#endif
#endif /* 0 */

    /* set the disk transfer address for subsequent FCB calls */
    sregs.ds = dtaseg;
    WREGS(regs,dx) = dtaoff;
    Trace((stderr, "segment:offset of pdta = %x:%x\n", dtaseg, dtaoff));
    Trace((stderr, "&dta = %lx, pdta = %lx\n", (ulg)&dta, (ulg)pdta));
    regs.h.ah = 0x1a;
    F_intdosx(&regs, &regs, &sregs);

    /* fill in the FCB */
    sregs.ds = fcbseg;
    WREGS(regs,dx) = fcboff;
    pfcb->flag = 0xff;          /* extended FCB */
    pfcb->vattr = 0x08;         /* attribute:  disk volume label */
    pfcb->drive = (uch)nLabelDrive;

#ifdef DEBUG
    Trace((stderr, "segment:offset of pfcb = %x:%x\n", sregs.ds, WREGS(regs,dx)));
    Trace((stderr, "&fcb = %lx, pfcb = %lx\n", (ulg)&fcb, (ulg)pfcb));
    Trace((stderr, "(2nd check:  labelling drive %c:)\n", pfcb->drive-1+'A'));
    if (pfcb->flag != fcb.flag)
        fprintf(stderr, "error:  pfcb->flag = %d, fcb.flag = %d\n",
          pfcb->flag, fcb.flag);
    if (pfcb->drive != fcb.drive)
        fprintf(stderr, "error:  pfcb->drive = %d, fcb.drive = %d\n",
          pfcb->drive, fcb.drive);
    if (pfcb->vattr != fcb.vattr)
        fprintf(stderr, "error:  pfcb->vattr = %d, fcb.vattr = %d\n",
          pfcb->vattr, fcb.vattr);
#endif /* DEBUG */

    /* check for existing label */
    Trace((stderr, "searching for existing label via FCBs\n"));
    regs.h.ah = 0x11;      /* FCB find first */
#ifdef WATCOMC_386
    _fstrncpy((char far *)&pfcb->vn, "???????????", 11);
#else
# if 0  /* THIS STRNCPY FAILS (MSC bug?): */
    strncpy(pfcb->vn, "???????????", 11);   /* i.e., "*.*" */
    Trace((stderr, "pfcb->vn = %lx\n", (ulg)pfcb->vn));
    Trace((stderr, "flag = %x, drive = %d, vattr = %x, vn = %s = %s.\n",
      fcb.flag, fcb.drive, fcb.vattr, fcb.vn, pfcb->vn));
# endif
    strncpy((char *)fcb.vn, "???????????", 11);   /* i.e., "*.*" */
#endif /* ?WATCOMC_386 */
    Trace((stderr, "fcb.vn = %lx\n", (ulg)fcb.vn));
    Trace((stderr, "regs.h.ah = %x, regs.x.dx = %04x, sregs.ds = %04x\n",
      regs.h.ah, WREGS(regs,dx), sregs.ds));
    Trace((stderr, "flag = %x, drive = %d, vattr = %x, vn = %s = %s.\n",
      fcb.flag, fcb.drive, fcb.vattr, fcb.vn, pfcb->vn));
    F_intdosx(&regs, &regs, &sregs);

/*---------------------------------------------------------------------------
    If not previously labelled, write a new label.  Otherwise just rename,
    since MS-DOS 2.x has a bug that damages the FAT when the old label is
    deleted.
  ---------------------------------------------------------------------------*/

    if (regs.h.al) {
        Trace((stderr, "no label found\n\n"));
        regs.h.ah = 0x16;                 /* FCB create file */
#ifdef WATCOMC_386
        _fstrncpy((char far *)pfcb->vn, newlabel, len);
        if (len < 11)
            _fstrncpy((char far *)(pfcb->vn+len), "           ", 11-len);
#else
        strncpy((char *)fcb.vn, newlabel, len);
        if (len < 11)   /* fill with spaces */
            strncpy((char *)(fcb.vn+len), "           ", 11-len);
#endif
        Trace((stderr, "fcb.vn = %lx  pfcb->vn = %lx\n", (ulg)fcb.vn,
          (ulg)pfcb->vn));
        Trace((stderr, "flag = %x, drive = %d, vattr = %x\n", fcb.flag,
          fcb.drive, fcb.vattr));
        Trace((stderr, "vn = %s = %s.\n", fcb.vn, pfcb->vn));
        F_intdosx(&regs, &regs, &sregs);
        regs.h.ah = 0x10;                 /* FCB close file */
        if (regs.h.al) {
            Trace((stderr, "unable to write volume name (AL = %x)\n",
              regs.h.al));
            F_intdosx(&regs, &regs, &sregs);
            retv = 1;
        } else {
            F_intdosx(&regs, &regs, &sregs);
            Trace((stderr, "new volume label [%s] written\n", newlabel));
            retv = 0;
        }
    } else {
        Trace((stderr, "found old label [%s]\n\n", dta.vn));  /* not term. */
        regs.h.ah = 0x17;                 /* FCB rename */
#ifdef WATCOMC_386
        _fstrncpy((char far *)pfcb->vn, (char far *)pdta->vn, 11);
        _fstrncpy((char far *)pfcb->nn, newlabel, len);
        if (len < 11)
            _fstrncpy((char far *)(pfcb->nn+len), "           ", 11-len);
#else
        strncpy((char *)fcb.vn, (char *)dta.vn, 11);
        strncpy((char *)fcb.nn, newlabel, len);
        if (len < 11)                     /* fill with spaces */
            strncpy((char *)(fcb.nn+len), "           ", 11-len);
#endif
        Trace((stderr, "fcb.vn = %lx  pfcb->vn = %lx\n", (ulg)fcb.vn,
          (ulg)pfcb->vn));
        Trace((stderr, "fcb.nn = %lx  pfcb->nn = %lx\n", (ulg)fcb.nn,
          (ulg)pfcb->nn));
        Trace((stderr, "flag = %x, drive = %d, vattr = %x\n", fcb.flag,
          fcb.drive, fcb.vattr));
        Trace((stderr, "vn = %s = %s.\n", fcb.vn, pfcb->vn));
        Trace((stderr, "nn = %s = %s.\n", fcb.nn, pfcb->nn));
        F_intdosx(&regs, &regs, &sregs);
        if (regs.h.al) {
            Trace((stderr, "Unable to change volume name (AL = %x)\n",
              regs.h.al));
            retv = 1;
        } else {
            Trace((stderr, "volume label changed to [%s]\n", newlabel));
            retv = 0;
        }
    }
#ifdef WATCOMC_386
    regs.w.ax = 0x0101;                    /* free dos memory */
    regs.w.dx = truseg;
    int386(0x31, &regs, &regs);
#endif
    return retv;

} /* end function volumelabel() */

#endif /* !__GO32__ && !__EMX__ */





/****************************/
/* Function close_outfile() */
/****************************/

void close_outfile(__G)
    __GDEF
 /*
  * MS-DOS VERSION
  *
  * Set the output file date/time stamp according to information from the
  * zipfile directory record for this member, then close the file and set
  * its permissions (archive, hidden, read-only, system).  Aside from closing
  * the file, this routine is optional (but most compilers support it).
  */
{
#ifdef USE_EF_UT_TIME
    iztimes z_utime;

    /* The following DOS date/time structure is machine-dependent as it
     * assumes "little-endian" byte order.  For MSDOS-specific code, which
     * is run on x86 CPUs (or emulators), this assumption is valid; but
     * care should be taken when using this code as template for other ports.
     */
    union {
        ulg z_dostime;
# ifdef __TURBOC__
        struct ftime ft;        /* system file time record */
# endif
        struct {                /* date and time words */
            union {             /* DOS file modification time word */
                ush ztime;
                struct {
                    unsigned zt_se : 5;
                    unsigned zt_mi : 6;
                    unsigned zt_hr : 5;
                } _tf;
            } _t;
            union {             /* DOS file modification date word */
                ush zdate;
                struct {
                    unsigned zd_dy : 5;
                    unsigned zd_mo : 4;
                    unsigned zd_yr : 7;
                } _df;
            } _d;
        } zt;
    } dos_dt;
#else /* !USE_EF_UT_TIME */
# ifdef __TURBOC__
    union {
        struct ftime ft;        /* system file time record */
        struct {
            ush ztime;          /* date and time words */
            ush zdate;          /* .. same format as in .ZIP file */
        } zt;
    } dos_dt;
# endif
#endif /* ?USE_EF_UT_TIME */


/*---------------------------------------------------------------------------
    Copy and/or convert time and date variables, if necessary; then set the
    file time/date.  WEIRD BORLAND "BUG":  if output is buffered, and if run
    under at least some versions of DOS (e.g., 6.0), and if files are smaller
    than DOS physical block size (i.e., 512 bytes) (?), then files MAY NOT
    get timestamped correctly--apparently setftime() occurs before any data
    are written to the file, and when file is closed and buffers are flushed,
    timestamp is overwritten with current time.  Even with a 32K buffer, this
    does not seem to occur with larger files.  UnZip output is now unbuffered,
    but if it were not, could still avoid problem by adding "fflush(outfile)"
    just before setftime() call.  Weird, huh?
  ---------------------------------------------------------------------------*/

#ifdef USE_EF_UT_TIME
    if (G.extra_field &&
        (ef_scan_for_izux(G.extra_field, G.lrec.extra_field_length, 0,
                          G.lrec.last_mod_file_date, &z_utime, NULL)
         & EB_UT_FL_MTIME))
    {
        struct tm *t;

        TTrace((stderr, "close_outfile:  Unix e.f. modif. time = %ld\n",
          z_utime.mtime));
        /* round up (down if "up" overflows) to even seconds */
        if (z_utime.mtime & 1)
            z_utime.mtime = (z_utime.mtime + 1 > z_utime.mtime) ?
                             z_utime.mtime + 1 : z_utime.mtime - 1;
        TIMET_TO_NATIVE(z_utime.mtime)   /* NOP unless MSC 7.0 or Macintosh */
        t = localtime(&(z_utime.mtime));
        if (t->tm_year < 80) {
            dos_dt.zt._t._tf.zt_se = 0;
            dos_dt.zt._t._tf.zt_mi = 0;
            dos_dt.zt._t._tf.zt_hr = 0;
            dos_dt.zt._d._df.zd_dy = 1;
            dos_dt.zt._d._df.zd_mo = 1;
            dos_dt.zt._d._df.zd_yr = 0;
        } else {
            dos_dt.zt._t._tf.zt_se = t->tm_sec >> 1;
            dos_dt.zt._t._tf.zt_mi = t->tm_min;
            dos_dt.zt._t._tf.zt_hr = t->tm_hour;
            dos_dt.zt._d._df.zd_dy = t->tm_mday;
            dos_dt.zt._d._df.zd_mo = t->tm_mon + 1;
            dos_dt.zt._d._df.zd_yr = t->tm_year - 80;
        }
    } else {
        dos_dt.zt._t.ztime = G.lrec.last_mod_file_time;
        dos_dt.zt._d.zdate = G.lrec.last_mod_file_date;
    }
# ifdef __TURBOC__
    setftime(fileno(G.outfile), &dos_dt.ft);
# else
    _dos_setftime(fileno(G.outfile), dos_dt.zt._d.zdate, dos_dt.zt._t.ztime);
# endif
#else /* !USE_EF_UT_TIME */
# ifdef __TURBOC__
    dos_dt.zt.ztime = G.lrec.last_mod_file_time;
    dos_dt.zt.zdate = G.lrec.last_mod_file_date;
    setftime(fileno(G.outfile), &dos_dt.ft);
# else
    _dos_setftime(fileno(G.outfile), G.lrec.last_mod_file_date,
                                     G.lrec.last_mod_file_time);
# endif
#endif /* ?USE_EF_UT_TIME */

/*---------------------------------------------------------------------------
    And finally we can close the file...at least everybody agrees on how to
    do *this*.  I think...  Also change the mode according to the stored file
    attributes, since we didn't do that when we opened the dude.
  ---------------------------------------------------------------------------*/

    fclose(G.outfile);

#ifdef __TURBOC__
#   if (defined(__BORLANDC__) && (__BORLANDC__ >= 0x0452))
#     define Chmod  _rtl_chmod
#   else
#     define Chmod  _chmod
#   endif
    if (Chmod(G.filename, 1, G.pInfo->file_attr) != G.pInfo->file_attr)
        Info(slide, 1, ((char *)slide, LoadFarString(AttribsMayBeWrong)));
#else /* !__TURBOC__ */
    _dos_setfileattr(G.filename, G.pInfo->file_attr);
#endif /* ?__TURBOC__ */

} /* end function close_outfile() */





#ifdef TIMESTAMP

/*************************/
/* Function stamp_file() */
/*************************/

int stamp_file(fname, modtime)
    ZCONST char *fname;
    time_t modtime;
{
    union {
        ulg z_dostime;
# ifdef __TURBOC__
        struct ftime ft;        /* system file time record */
# endif
        struct {                /* date and time words */
            union {             /* DOS file modification time word */
                ush ztime;
                struct {
                    unsigned zt_se : 5;
                    unsigned zt_mi : 6;
                    unsigned zt_hr : 5;
                } _tf;
            } _t;
            union {             /* DOS file modification date word */
                ush zdate;
                struct {
                    unsigned zd_dy : 5;
                    unsigned zd_mo : 4;
                    unsigned zd_yr : 7;
                } _df;
            } _d;
        } zt;
    } dos_dt;
    time_t t_even;
    struct tm *t;
    int fd;                             /* file handle */

    /* round up (down if "up" overflows) to even seconds */
    t_even = ((modtime + 1 > modtime) ? modtime + 1 : modtime) & (~1);
    TIMET_TO_NATIVE(t_even)             /* NOP unless MSC 7.0 or Macintosh */
    t = localtime(&t_even);
    if (t->tm_year < 80) {
        dos_dt.zt._t._tf.zt_se = 0;
        dos_dt.zt._t._tf.zt_mi = 0;
        dos_dt.zt._t._tf.zt_hr = 0;
        dos_dt.zt._d._df.zd_dy = 1;
        dos_dt.zt._d._df.zd_mo = 1;
        dos_dt.zt._d._df.zd_yr = 0;
    } else {
        dos_dt.zt._t._tf.zt_se = t->tm_sec >> 1;
        dos_dt.zt._t._tf.zt_mi = t->tm_min;
        dos_dt.zt._t._tf.zt_hr = t->tm_hour;
        dos_dt.zt._d._df.zd_dy = t->tm_mday;
        dos_dt.zt._d._df.zd_mo = t->tm_mon + 1;
        dos_dt.zt._d._df.zd_yr = t->tm_year - 80;
    }
    if (((fd = open(fname, 0)) == -1) ||
# ifdef __TURBOC__
        (setftime(fd, &dos_dt.ft)))
# else
        (_dos_setftime(fd, dos_dt.zt._d.zdate, dos_dt.zt._t.ztime)))
# endif
        return -1;
    close(fd);
    return 0;

} /* end function stamp_file() */

#endif /* TIMESTAMP */




#ifndef SFX

/*************************/
/* Function dateformat() */
/*************************/

int dateformat()
{

/*---------------------------------------------------------------------------
    For those operating systems that support it, this function returns a
    value that tells how national convention says that numeric dates are
    displayed.  Return values are DF_YMD, DF_DMY and DF_MDY (the meanings
    should be fairly obvious).
  ---------------------------------------------------------------------------*/

#ifndef WINDLL
    ush CountryInfo[18];
#if (!defined(__GO32__) && !defined(__EMX__))
    ush far *_CountryInfo = CountryInfo;
    struct SREGS sregs;
    union REGS regs;
#ifdef WATCOMC_386
    ush seg, para;

    memset(&sregs, 0, sizeof(sregs));
    memset(&regs, 0, sizeof(regs));
    /* PMODE/W does not support an extended version of dos function 38,   */
    /* so we have to use brute force, allocating real mode memory for it. */
    regs.w.ax = 0x0100;
    regs.w.bx = 3;                         /* 36 bytes rounds up to 48 */
    int386(0x31, &regs, &regs);            /* DPMI allocate DOS memory */
    if (regs.w.cflag)
        return DF_MDY;                     /* no memory, return default */
    seg = regs.w.dx;
    para = regs.w.ax;

#ifdef XXX__MK_FP_IS_BROKEN
    /* XXX  This code may not be trustworthy in general, though it is
     * valid with DOS/4GW and PMODE/w, which is all we support for now. */
 /* _CountryInfo = (ush far *) (para << 4); */ /* works for some extenders */
    regs.w.ax = 6;
    regs.w.bx = seg;
    int386(0x31, &regs, &regs);            /* convert seg to linear address */
    _CountryInfo = (ush far *) (((ulg) regs.w.cx << 16) | regs.w.dx);
#else
    _CountryInfo = (ush far *) MK_FP(seg, 0);
#endif

    sregs.ds = para;                       /* real mode paragraph */
    regs.w.dx = 0;                         /* no offset from segment */
    regs.w.ax = 0x3800;
    int86x_realmode(0x21, &regs, &regs, &sregs);
    CountryInfo[0] = regs.w.cflag ? 0 : _CountryInfo[0];
    regs.w.ax = 0x0101;
    regs.w.dx = seg;
    int386(0x31, &regs, &regs);              /* DPMI free DOS memory */

#else /* !WATCOMC_386 */

    sregs.ds  = FP_SEG(_CountryInfo);
    regs.x.dx = FP_OFF(_CountryInfo);
    regs.x.ax = 0x3800;
    intdosx(&regs, &regs, &sregs);
#endif /* ?WATCOMC_386 */

#else /* __GO32__ || __EMX__ */
    _dos_getcountryinfo(CountryInfo);
#endif /* ?(__GO32__ || __EMX__) */

    switch(CountryInfo[0]) {
        case 0:
            return DF_MDY;
        case 1:
            return DF_DMY;
        case 2:
            return DF_YMD;
    }
#endif /* !WINDLL && !WATCOMC_386 */

    return DF_MDY;   /* default for systems without locale info */

} /* end function dateformat() */




#ifndef WINDLL

/************************/
/*  Function version()  */
/************************/

void version(__G)
    __GDEF
{
    int len;
#if defined(__DJGPP__) || defined(__WATCOMC__) || \
    (defined(_MSC_VER) && (_MSC_VER != 800))
    char buf[80];
#endif

    len = sprintf((char *)slide, LoadFarString(CompiledWith),

#if defined(__GNUC__)
#  if defined(__DJGPP__)
      (sprintf(buf, "djgpp v%d.%02d / gcc ", __DJGPP__, __DJGPP_MINOR__), buf),
#  elif defined(__GO32__)         /* __GO32__ is defined as "1" only (sigh) */
      "djgpp v1.x / gcc ",
#  elif defined(__EMX__)          /* ...so is __EMX__ (double sigh) */
      "emx+gcc ",
#  else
      "gcc ",
#  endif
      __VERSION__,
#elif defined(__WATCOMC__)
#  if (__WATCOMC__ % 10 != 0)
      "Watcom C/C++", (sprintf(buf, " %d.%02d", __WATCOMC__ / 100,
                               __WATCOMC__ % 100), buf),
#  else
      "Watcom C/C++", (sprintf(buf, " %d.%d", __WATCOMC__ / 100,
                               (__WATCOMC__ % 100) / 10), buf),
#  endif
#elif defined(__TURBOC__)
#  ifdef __BORLANDC__
      "Borland C++",
#    if (__BORLANDC__ < 0x0200)
        " 1.0",
#    elif (__BORLANDC__ == 0x0200)   /* James:  __TURBOC__ = 0x0297 */
        " 2.0",
#    elif (__BORLANDC__ == 0x0400)
        " 3.0",
#    elif (__BORLANDC__ == 0x0410)   /* __BCPLUSPLUS__ = 0x0310 */
        " 3.1",
#    elif (__BORLANDC__ == 0x0452)   /* __BCPLUSPLUS__ = 0x0320 */
        " 4.0 or 4.02",
#    elif (__BORLANDC__ == 0x0460)   /* __BCPLUSPLUS__ = 0x0340 */
        " 4.5",
#    elif (__BORLANDC__ == 0x0500)
        " 5.0",
#    else
        " later than 5.0",
#    endif
#  else
      "Turbo C",
#    if (__TURBOC__ > 0x0401)        /* Kevin:  3.0 -> 0x0401 */
        "++ later than 3.0",
#    elif (__TURBOC__ >= 0x0400)
        "++ 3.0",
#    elif (__TURBOC__ == 0x0295)     /* [661] vfy'd by Kevin */
        "++ 1.0",
#    elif ((__TURBOC__ >= 0x018d) && (__TURBOC__ <= 0x0200)) /* James: 0x0200 */
        " 2.0",
#    elif (__TURBOC__ > 0x0100)
        " 1.5",                      /* James:  0x0105? */
#    else
        " 1.0",                      /* James:  0x0100 */
#    endif
#  endif
#elif defined(MSC)
      "Microsoft C ",
#  ifdef _MSC_VER
#    if (_MSC_VER == 800)
        "8.0/8.0c (Visual C++ 1.0/1.5)",
#    else
        (sprintf(buf, "%d.%02d", _MSC_VER/100, _MSC_VER%100), buf),
#    endif
#  else
      "5.1 or earlier",
#  endif
#else
      "unknown compiler", "",
#endif /* ?compilers */

      "MS-DOS",

#if (defined(__GNUC__) || defined(WATCOMC_386))
      " (32-bit)",
#else
#  if defined(M_I86HM) || defined(__HUGE__)
      " (16-bit, huge)",
#  elif defined(M_I86LM) || defined(__LARGE__)
      " (16-bit, large)",
#  elif defined(M_I86MM) || defined(__MEDIUM__)
      " (16-bit, medium)",
#  elif defined(M_I86CM) || defined(__COMPACT__)
      " (16-bit, compact)",
#  elif defined(M_I86SM) || defined(__SMALL__)
      " (16-bit, small)",
#  elif defined(M_I86TM) || defined(__TINY__)
      " (16-bit, tiny)",
#  else
      " (16-bit)",
#  endif
#endif

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    (*G.message)((zvoid *)&G, slide, (ulg)len, 0);
                                /* MSC can't handle huge macro expansion */

    /* temporary debugging code for Borland compilers only */
#if (defined(__TURBOC__) && defined(DEBUG))
    Info(slide, 0, ((char *)slide, "\tdebug(__TURBOC__ = 0x%04x = %d)\n",
      __TURBOC__, __TURBOC__));
#ifdef __BORLANDC__
    Info(slide, 0, ((char *)slide, "\tdebug(__BORLANDC__ = 0x%04x)\n",
      __BORLANDC__));
#else
    Info(slide, 0, ((char *)slide, "\tdebug(__BORLANDC__ not defined)\n"));
#endif
#ifdef __TCPLUSPLUS__
    Info(slide, 0, ((char *)slide, "\tdebug(__TCPLUSPLUS__ = 0x%04x)\n",
      __TCPLUSPLUS__));
#else
    Info(slide, 0, ((char *)slide, "\tdebug(__TCPLUSPLUS__ not defined)\n"));
#endif
#ifdef __BCPLUSPLUS__
    Info(slide, 0, ((char *)slide, "\tdebug(__BCPLUSPLUS__ = 0x%04x)\n\n",
      __BCPLUSPLUS__));
#else
    Info(slide, 0, ((char *)slide, "\tdebug(__BCPLUSPLUS__ not defined)\n\n"));
#endif
#endif /* __TURBOC__ && DEBUG */

} /* end function version() */

#endif /* !WINDLL */
#endif /* !SFX */





#if (defined(__GO32__) || defined(__EMX__))

unsigned volatile _doserrno;

#if (!defined(__DJGPP__) || (__DJGPP__ < 2))

unsigned _dos_getcountryinfo(void *countrybuffer)
{
    asm("movl %0, %%edx": : "g" (countrybuffer));
    asm("movl $0x3800, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    _doserrno = 0;
    asm("jnc 1f");
    asm("movl %%eax, %0": "=m" (_doserrno));
    asm("1:");
    return _doserrno;
}

unsigned _dos_setftime(int fd, ush dosdate, ush dostime)
{
    asm("movl %0, %%ebx": : "g" (fd));
    asm("movl %0, %%ecx": : "g" (dostime));
    asm("movl %0, %%edx": : "g" (dosdate));
    asm("movl $0x5701, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    _doserrno = 0;
    asm("jnc 1f");
    asm("movl %%eax, %0": "=m" (_doserrno));
    errno = EBADF;
    asm("1:");
    return _doserrno;
}

void _dos_setfileattr(char *name, int attr)
{
    asm("movl %0, %%edx": : "g" (name));
    asm("movl %0, %%ecx": : "g" (attr));
    asm("movl $0x4301, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
}

void _dos_getdrive(unsigned *d)
{
    asm("movl $0x1900, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    asm("xorb %ah, %ah");
    asm("incb %al");
    asm("movl %%eax, %0": "=a" (*d));
}

unsigned _dos_creat(char *path, unsigned attr, int *fd)
{
    asm("movl $0x3c00, %eax");
    asm("movl %0, %%edx": :"g" (path));
    asm("movl %0, %%ecx": :"g" (attr));
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    asm("movl %%eax, %0": "=a" (*fd));
    _doserrno = 0;
    asm("jnc 1f");
    _doserrno = *fd;
    switch (_doserrno) {
    case 3:
           errno = ENOENT;
           break;
    case 4:
           errno = EMFILE;
           break;
    case 5:
           errno = EACCES;
           break;
    }
    asm("1:");
    return _doserrno;
}

unsigned _dos_close(int fd)
{
    asm("movl %0, %%ebx": : "g" (fd));
    asm("movl $0x3e00, %eax");
    asm("int $0x21": : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi");
    _doserrno = 0;
    asm("jnc 1f");
    asm ("movl %%eax, %0": "=m" (_doserrno));
    if (_doserrno == 6) {
          errno = EBADF;
    }
    asm("1:");
    return _doserrno;
}

#endif /* !__DJGPP__ || (__DJGPP__ < 2) */


static int volumelabel(char *name)
{
    int fd;

    return _dos_creat(name, FA_LABEL, &fd) ? fd : _dos_close(fd);
}


#if (defined(__DJGPP__) && (__DJGPP__ > 1))

#include <dpmi.h>               /* These includes for the country info */
#include <go32.h>
#include <sys/farptr.h>

/* The above _dos_getcountryinfo function doesn't work with djgpp v2, presumably
 * because ds is not set correctly (does it really work at all?). Note that
 * this version only sets the date (ie. CountryInfo[0]).
 */
unsigned _dos_getcountryinfo(void *countrybuffer)
{
   __dpmi_regs regs;

   regs.x.ax = 0x3800;
   regs.x.dx = __tb & 0x0f;
   regs.x.ds = (__tb >> 4) & 0xffff;
   _doserrno = __dpmi_int(0x21, &regs);

   *(ush*)countrybuffer = _farpeekw(_dos_ds, __tb & 0xfffff);

   return _doserrno;
}


/* Prevent globbing of filenames.  This gives the same functionality as
 * "stubedit <program> globbing=no" did with DJGPP v1.
 */
int __crt0_glob_function(void)
{
    return 0;
}

/* Reduce the size of the executable and remove the functionality to read
 * the program's environment from whatever $DJGPP points to.
 */
void __crt_load_environment_file(void)
{
}

#endif /* __DJGPP__ > 1 */
#endif /* __GO32__ || __EMX__ */



#ifdef __EMX__
#ifdef MORE

/**************************/
/* Function screenlines() */
/**************************/

int screenlines()
{
    int scr_dimen[2];           /* scr_dimen[0]: columns, src_dimen[1]: rows */

    _scrsize(scr_dimen);
    return (scr_dimen[1]);
}


/****************************/
/* Function screencolumns() */
/****************************/

int screencolumns()
{
    int scr_dimen[2];           /* scr_dimen[0]: columns, src_dimen[1]: rows */

    _scrsize(scr_dimen);
    return (scr_dimen[0]);
}

#endif /* MORE */
#endif /* __EMX__ */



#ifdef WATCOMC_386

static struct RMINFO {
    ulg edi, esi, ebp;
    ulg reserved;
    ulg ebx, edx, ecx, eax;
    ush flags;
    ush es,ds,fs,gs;
    ush ip_ignored,cs_ignored;
    ush sp,ss;
};

/* This function is used to call dos interrupts that may not be supported
 * by some particular 32-bit DOS extender.  It uses DPMI function 300h to
 * simulate a real mode call of the interrupt.  The caller is responsible
 * for providing real mode addresses of any buffer areas used.  The docs
 * for PMODE/W imply that this should not be necessary for calling the DOS
 * interrupts that it doesn't extend, but it crashes when this isn't used. */

static int int86x_realmode(int inter_no, union REGS *in,
                            union REGS *out, struct SREGS *seg)
{
    union REGS local;
    struct SREGS localseg;
    struct RMINFO rmi;
    int r;

    rmi.eax = in->x.eax;
    rmi.ebx = in->x.ebx;
    rmi.ecx = in->x.ecx;
    rmi.edx = in->x.edx;
    rmi.edi = in->x.edi;
    rmi.esi = in->x.esi;
    rmi.ebp = rmi.reserved = 0L;
    rmi.es = seg->es;
    rmi.ds = seg->ds;
    rmi.fs = seg->fs;
    rmi.gs = seg->gs;
    rmi.sp = rmi.ss = rmi.ip_ignored = rmi.cs_ignored = rmi.flags = 0;
    memset(&local, 0, sizeof(local));
    memset(&localseg, 0, sizeof(localseg));
    local.w.ax = 0x0300;
    local.h.bl = inter_no;
    local.h.bh = 0;
    local.w.cx = 0;
    localseg.es = FP_SEG(&rmi);
    local.x.edi = FP_OFF(&rmi);
    r = int386x(0x31, &local, &local, &localseg);
    out->x.eax = rmi.eax;
    out->x.ebx = rmi.ebx;
    out->x.ecx = rmi.ecx;
    out->x.edx = rmi.edx;
    out->x.edi = rmi.edi;
    out->x.esi = rmi.esi;
    out->x.cflag = rmi.flags & INTR_CF;
    return r;
}

#endif /* WATCOMC_386 */




#ifdef __WATCOMC__

/* This papers over a bug in Watcom 10.6's standard library...sigh.
 * Apparently it applies to both the DOS and Win32 stat()s. */

int stat_bandaid(const char *path, struct stat *buf)
{
    char newname[4];

    if (!stat(path, buf))
        return 0;
    else if (!strcmp(path, ".") || (path[0] && !strcmp(path + 1, ":."))) {
        strcpy(newname, path);
        newname[strlen(path) - 1] = '\\';   /* stat(".") fails for root! */
        return stat(newname, buf);
    } else
        return -1;
}

#endif /* __WATCOMC__ */

/*---------------------------------------------------------------------------

  beos.c

  BeOS-specific routines for use with Info-ZIP's UnZip 5.30 and later.
  (based on unix/unix.c)

  Contains:  do_wild()           <-- generic enough to put in fileio.c?
             mapattr()
             mapname()
             checkdir()
             close_outfile()
             stamp_file()
             version()
             scanBeOSexfield()
             isBeOSexfield()
             set_file_attrs()
             setBeOSexfield()
             printBeOSexfield()

  ---------------------------------------------------------------------------*/

#define UNZIP_INTERNAL
#include "unzip.h"

#include "beos.h"
#include <errno.h>             /* Just make sure we've got a few things... */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <dirent.h>

/* For the new post-DR8 file attributes */
#include <kernel/fs_attr.h>
int set_file_attrs( const char *, const unsigned char *, const off_t );

static int created_dir;        /* used in mapname(), checkdir() */
static int renamed_fullpath;   /* ditto */

#ifndef SFX

/**********************/
/* Function do_wild() */   /* for porting:  dir separator; match(ignore_case) */
/**********************/

char *do_wild(__G__ wildspec)
    __GDEF
    char *wildspec;         /* only used first time on a given dir */
{
    static DIR *dir = (DIR *)NULL;
    static char *dirname, *wildname, matchname[FILNAMSIZ];
    static int firstcall=TRUE, have_dirname, dirnamelen;
    struct dirent *file;


    /* Even when we're just returning wildspec, we *always* do so in
     * matchname[]--calling routine is allowed to append four characters
     * to the returned string, and wildspec may be a pointer to argv[].
     */
    if (firstcall) {        /* first call:  must initialize everything */
        firstcall = FALSE;

        /* break the wildspec into a directory part and a wildcard filename */
        if ((wildname = strrchr(wildspec, '/')) == (char *)NULL) {
            dirname = ".";
            dirnamelen = 1;
            have_dirname = FALSE;
            wildname = wildspec;
        } else {
            ++wildname;     /* point at character after '/' */
            dirnamelen = wildname - wildspec;
            if ((dirname = (char *)malloc(dirnamelen+1)) == (char *)NULL) {
                Info(slide, 0x201, ((char *)slide,
                  "warning:  cannot allocate wildcard buffers\n"));
                strcpy(matchname, wildspec);
                return matchname;   /* but maybe filespec was not a wildcard */
            }
            strncpy(dirname, wildspec, dirnamelen);
            dirname[dirnamelen] = '\0';   /* terminate for strcpy below */
            have_dirname = TRUE;
        }

        if ((dir = opendir(dirname)) != (DIR *)NULL) {
            while ((file = readdir(dir)) != (struct dirent *)NULL) {
                if (file->d_name[0] == '.' && wildname[0] != '.')
                    continue;  /* Unix:  '*' and '?' do not match leading dot */
                if (match(file->d_name, wildname, 0)) {  /* 0 == case sens. */
                    if (have_dirname) {
                        strcpy(matchname, dirname);
                        strcpy(matchname+dirnamelen, file->d_name);
                    } else
                        strcpy(matchname, file->d_name);
                    return matchname;
                }
            }
            /* if we get to here directory is exhausted, so close it */
            closedir(dir);
            dir = (DIR *)NULL;
        }

        /* return the raw wildspec in case that works (e.g., directory not
         * searchable, but filespec was not wild and file is readable) */
        strcpy(matchname, wildspec);
        return matchname;
    }

    /* last time through, might have failed opendir but returned raw wildspec */
    if (dir == (DIR *)NULL) {
        firstcall = TRUE;  /* nothing left to try--reset for new wildspec */
        if (have_dirname)
            free(dirname);
        return (char *)NULL;
    }

    /* If we've gotten this far, we've read and matched at least one entry
     * successfully (in a previous call), so dirname has been copied into
     * matchname already.
     */
    while ((file = readdir(dir)) != (struct dirent *)NULL)
        if (match(file->d_name, wildname, 0)) {   /* 0 == don't ignore case */
            if (have_dirname) {
                /* strcpy(matchname, dirname); */
                strcpy(matchname+dirnamelen, file->d_name);
            } else
                strcpy(matchname, file->d_name);
            return matchname;
        }

    closedir(dir);     /* have read at least one dir entry; nothing left */
    dir = (DIR *)NULL;
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
    ulg tmp = G.crec.external_file_attributes;

    switch (G.pInfo->hostnum) {
        case UNIX_:
        case VMS_:
        case ACORN_:
        case ATARI_:
        case BEOS_:
        case QDOS_:
            G.pInfo->file_attr = (unsigned)(tmp >> 16);
            return 0;
        case AMIGA_:
            tmp = (unsigned)(tmp>>17 & 7);   /* Amiga RWE bits */
            G.pInfo->file_attr = (unsigned)(tmp<<6 | tmp<<3 | tmp);
            break;
        /* all remaining cases:  expand MSDOS read-only bit into write perms */
        case FS_FAT_:
        case FS_HPFS_:
        case FS_NTFS_:
        case MAC_:
        case TOPS20_:
        default:
            tmp = !(tmp & 1) << 1;   /* read-only bit --> write perms bits */
            G.pInfo->file_attr = (unsigned)(0444 | tmp<<6 | tmp<<3 | tmp);
            break;
    } /* end switch (host-OS-created-by) */

    /* for originating systems with no concept of "group," "other," "system": */
    umask( (int)(tmp=umask(0)) );    /* apply mask to expanded r/w(/x) perms */
    G.pInfo->file_attr &= ~tmp;

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
    char pathcomp[FILNAMSIZ];    /* path-component buffer */
    char *pp, *cp=(char *)NULL;  /* character pointers */
    char *lastsemi=(char *)NULL; /* pointer to last semi-colon in pathcomp */
    int quote = FALSE;           /* flags */
    int error = 0;
    register unsigned workch;    /* hold the character being tested */


/*---------------------------------------------------------------------------
    Initialize various pointers and counters and stuff.
  ---------------------------------------------------------------------------*/

    if (G.pInfo->vollabel)
        return IZ_VOL_LABEL;    /* can't set disk volume labels in Unix */

    /* can create path as long as not just freshening, or if user told us */
    G.create_dirs = (!G.fflag || renamed);

    created_dir = FALSE;        /* not yet */

    /* user gave full pathname:  don't prepend rootpath */
    renamed_fullpath = (renamed && (*G.filename == '/'));

    if (checkdir(__G__ (char *)NULL, INIT) == 10)
        return 10;              /* initialize path buffer, unless no memory */

    *pathcomp = '\0';           /* initialize translation buffer */
    pp = pathcomp;              /* point to translation buffer */
    if (G.jflag)                /* junking directories */
        cp = (char *)strrchr(G.filename, '/');
    if (cp == (char *)NULL)     /* no '/' or not junking dirs */
        cp = G.filename;        /* point to internal zipfile-member pathname */
    else
        ++cp;                   /* point to start of last component of path */

/*---------------------------------------------------------------------------
    Begin main loop through characters in filename.
  ---------------------------------------------------------------------------*/

    while ((workch = (uch)*cp++) != 0) {

        if (quote) {                 /* if character quoted, */
            *pp++ = (char)workch;    /*  include it literally */
            quote = FALSE;
        } else
            switch (workch) {
            case '/':             /* can assume -j flag not given */
                *pp = '\0';
                if ((error = checkdir(__G__ pathcomp, APPEND_DIR)) > 1)
                    return error;
                pp = pathcomp;    /* reset conversion buffer for next piece */
                lastsemi = (char *)NULL; /* leave directory semi-colons alone */
                break;

            case ';':             /* VMS version (or DEC-20 attrib?) */
                lastsemi = pp;
                *pp++ = ';';      /* keep for now; remove VMS ";##" */
                break;            /*  later, if requested */

            case '\026':          /* control-V quote for special chars */
                quote = TRUE;     /* set flag for next character */
                break;

            default:
                /* allow European characters in filenames: */
                if (isprint(workch) || (128 <= workch && workch <= 254))
                    *pp++ = (char)workch;
            } /* end switch */

    } /* end while loop */

    *pp = '\0';                   /* done with pathcomp:  terminate it */

    /* if not saving them, remove VMS version numbers (appended ";###") */
    if (!G.V_flag && lastsemi) {
        pp = lastsemi + 1;
        while (isdigit((uch)(*pp)))
            ++pp;
        if (*pp == '\0')          /* only digits between ';' and end:  nuke */
            *lastsemi = '\0';
    }

/*---------------------------------------------------------------------------
    Report if directory was created (and no file to create:  filename ended
    in '/'), check name to be sure it exists, and combine path and name be-
    fore exiting.
  ---------------------------------------------------------------------------*/

    if (G.filename[strlen(G.filename) - 1] == '/') {
        checkdir(__G__ G.filename, GETPATH);
        if (created_dir) {
            if (QCOND2) {
                Info(slide, 0, ((char *)slide, "   creating: %s\n",
                  G.filename));
            }
            {   /* Handle the BeOS extra field if present. */
                void *ptr = scanBeOSexfield( G.extra_field,
                                             G.lrec.extra_field_length );
                if( ptr ) {
                    setBeOSexfield( G.filename, ptr );
                }
            }
            return IZ_CREATED_DIR;   /* set dir time (note trailing '/') */
        }
        /* TODO: should we re-write the BeOS extra field data in case it's */
        /* changed?                                                        */
        return 2;   /* dir existed already; don't look for data to extract */
    }

    if (*pathcomp == '\0') {
        Info(slide, 1, ((char *)slide, "mapname:  conversion of %s failed\n",
          G.filename));
        return 3;
    }

    checkdir(__G__ pathcomp, APPEND_NAME);  /* returns 1 if truncated: care? */
    checkdir(__G__ G.filename, GETPATH);

    return error;

} /* end function mapname() */





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

#   define FN_MASK   7
#   define FUNCTION  (flag & FN_MASK)



/*---------------------------------------------------------------------------
    APPEND_DIR:  append the path component to the path being built and check
    for its existence.  If doesn't exist and we are creating directories, do
    so for this one; else signal success or error as appropriate.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_DIR) {
        int too_long = FALSE;
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending dir segment [%s]\n", pathcomp));
        while ((*end = *pathcomp++) != '\0')
            ++end;
#ifdef SHORT_NAMES   /* path components restricted to 14 chars, typically */
        if ((end-old_end) > FILENAME_MAX)  /* GRR:  proper constant? */
            *(end = old_end + FILENAME_MAX) = '\0';
#endif

        /* GRR:  could do better check, see if overrunning buffer as we go:
         * check end-buildpath after each append, set warning variable if
         * within 20 of FILNAMSIZ; then if var set, do careful check when
         * appending.  Clear variable when begin new path. */

        if ((end-buildpath) > FILNAMSIZ-3)  /* need '/', one-char name, '\0' */
            too_long = TRUE;                /* check if extracting directory? */
        if (stat(buildpath, &G.statbuf)) {  /* path doesn't exist */
            if (!G.create_dirs) { /* told not to create (freshening) */
                free(buildpath);
                return 2;         /* path doesn't exist:  nothing to do */
            }
            if (too_long) {
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  path too long: %s\n", buildpath));
                free(buildpath);
                return 4;         /* no room for filenames:  fatal */
            }
            if (mkdir(buildpath, 0777) == -1) {   /* create the directory */
                Info(slide, 1, ((char *)slide,
                  "checkdir error:  cannot create %s\n\
                 unable to process %s.\n", buildpath, G.filename));
                free(buildpath);
                return 3;      /* path didn't exist, tried to create, failed */
            }
            created_dir = TRUE;
        } else if (!S_ISDIR(G.statbuf.st_mode)) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  %s exists but is not directory\n\
                 unable to process %s.\n", buildpath, G.filename));
            free(buildpath);
            return 3;          /* path existed but wasn't dir */
        }
        if (too_long) {
            Info(slide, 1, ((char *)slide,
              "checkdir error:  path too long: %s\n", buildpath));
            free(buildpath);
            return 4;         /* no room for filenames:  fatal */
        }
        *end++ = '/';
        *end = '\0';
        Trace((stderr, "buildpath now = [%s]\n", buildpath));
        return 0;

    } /* end if (FUNCTION == APPEND_DIR) */

/*---------------------------------------------------------------------------
    GETPATH:  copy full path to the string pointed at by pathcomp, and free
    buildpath.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == GETPATH) {
        strcpy(pathcomp, buildpath);
        Trace((stderr, "getting and freeing path [%s]\n", pathcomp));
        free(buildpath);
        buildpath = end = (char *)NULL;
        return 0;
    }

/*---------------------------------------------------------------------------
    APPEND_NAME:  assume the path component is the filename; append it and
    return without checking for existence.
  ---------------------------------------------------------------------------*/

    if (FUNCTION == APPEND_NAME) {
#ifdef SHORT_NAMES
        char *old_end = end;
#endif

        Trace((stderr, "appending filename [%s]\n", pathcomp));
        while ((*end = *pathcomp++) != '\0') {
            ++end;
#ifdef SHORT_NAMES  /* truncate name at 14 characters, typically */
            if ((end-old_end) > FILENAME_MAX)      /* GRR:  proper constant? */
                *(end = old_end + FILENAME_MAX) = '\0';
#endif
            if ((end-buildpath) >= FILNAMSIZ) {
                *--end = '\0';
                Info(slide, 0x201, ((char *)slide,
                  "checkdir warning:  path too long; truncating\n\
                   %s\n                -> %s\n", G.filename, buildpath));
                return 1;   /* filename truncated */
            }
        }
        Trace((stderr, "buildpath now = [%s]\n", buildpath));
        return 0;  /* could check for existence here, prompt for new name... */
    }

/*---------------------------------------------------------------------------
    INIT:  allocate and initialize buffer space for the file currently being
    extracted.  If file was renamed with an absolute path, don't prepend the
    extract-to path.
  ---------------------------------------------------------------------------*/

/* GRR:  for VMS and TOPS-20, add up to 13 to strlen */

    if (FUNCTION == INIT) {
        Trace((stderr, "initializing buildpath to "));
        if ((buildpath = (char *)malloc(strlen(G.filename)+rootlen+1)) ==
            (char *)NULL)
            return 10;
        if ((rootlen > 0) && !renamed_fullpath) {
            strcpy(buildpath, rootpath);
            end = buildpath + rootlen;
        } else {
            *buildpath = '\0';
            end = buildpath;
        }
        Trace((stderr, "[%s]\n", buildpath));
        return 0;
    }

/*---------------------------------------------------------------------------
    ROOT:  if appropriate, store the path in rootpath and create it if neces-
    sary; else assume it's a zipfile member and return.  This path segment
    gets used in extracting all members from every zipfile specified on the
    command line.
  ---------------------------------------------------------------------------*/

#if (!defined(SFX) || defined(SFX_EXDIR))
    if (FUNCTION == ROOT) {
        Trace((stderr, "initializing root path to [%s]\n", pathcomp));
        if (pathcomp == (char *)NULL) {
            rootlen = 0;
            return 0;
        }
        if ((rootlen = strlen(pathcomp)) > 0) {
            if (pathcomp[rootlen-1] == '/') {
                pathcomp[--rootlen] = '\0';
            }
            if (rootlen > 0 && (stat(pathcomp, &G.statbuf) ||
                !S_ISDIR(G.statbuf.st_mode)))        /* path does not exist */
            {
                if (!G.create_dirs /* || iswild(pathcomp) */ ) {
                    rootlen = 0;
                    return 2;   /* skip (or treat as stored file) */
                }
                /* create the directory (could add loop here to scan pathcomp
                 * and create more than one level, but why really necessary?) */
                if (mkdir(pathcomp, 0777) == -1) {
                    Info(slide, 1, ((char *)slide,
                      "checkdir:  cannot create extraction directory: %s\n",
                      pathcomp));
                    rootlen = 0;   /* path didn't exist, tried to create, and */
                    return 3;  /* failed:  file exists, or 2+ levels required */
                }
            }
            if ((rootpath = (char *)malloc(rootlen+2)) == (char *)NULL) {
                rootlen = 0;
                return 10;
            }
            strcpy(rootpath, pathcomp);
            rootpath[rootlen++] = '/';
            rootpath[rootlen] = '\0';
            Trace((stderr, "rootpath now = [%s]\n", rootpath));
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






/****************************/
/* Function close_outfile() */
/****************************/

void close_outfile(__G)    /* GRR: change to return PK-style warning level */
    __GDEF
{
    iztimes zt;
    ush z_uidgid[2];
    unsigned eb_izux_flg;

/*---------------------------------------------------------------------------
    If symbolic links are supported, allocate a storage area, put the uncom-
    pressed "data" in it, and create the link.  Since we know it's a symbolic
    link to start with, we shouldn't have to worry about overflowing unsigned
    ints with unsigned longs.
  ---------------------------------------------------------------------------*/

#ifdef SYMLINKS
    if (G.symlnk) {
        unsigned ucsize = (unsigned)G.lrec.ucsize;
        char *linktarget = (char *)malloc((unsigned)G.lrec.ucsize+1);

        fclose(G.outfile);                      /* close "data" file... */
        G.outfile = fopen(G.filename, FOPR);    /* ...and reopen for reading */
        if (!linktarget || fread(linktarget, 1, ucsize, G.outfile) !=
                           (int)ucsize)
        {
            Info(slide, 0x201, ((char *)slide,
              "warning:  symbolic link (%s) failed\n", G.filename));
            if (linktarget)
                free(linktarget);
            fclose(G.outfile);
            return;
        }
        fclose(G.outfile);                  /* close "data" file for good... */
        unlink(G.filename);                 /* ...and delete it */
        linktarget[ucsize] = '\0';
        if (QCOND2)
            Info(slide, 0, ((char *)slide, "-> %s ", linktarget));
        if (symlink(linktarget, G.filename))  /* create the real link */
            perror("symlink error");

/* See beos.h; there's currently no way to save/restore a symbolic link's */
/* attributes from C, and I'm loathe to introduce any C++. [cjh]          */
#  ifndef BE_NO_SYMLINK_ATTRS
        {
            /* Symlinkcs can have attributes, too. */
            void *ptr = scanBeOSexfield( G.extra_field,
                                         G.lrec.extra_field_length );
            if( ptr ) {
                setBeOSexfield( G.filename, ptr );
            }
        }
#  endif

        free(linktarget);
        return;                             /* can't set time on symlinks */
    }
#endif /* SYMLINKS */

    fclose(G.outfile);

/*---------------------------------------------------------------------------
    Change the file permissions from default ones to those stored in the
    zipfile.
  ---------------------------------------------------------------------------*/

#ifndef NO_CHMOD
    if (chmod(G.filename, 0xffff & G.pInfo->file_attr))
        perror("chmod (file attributes) error");
#endif

/*---------------------------------------------------------------------------
    Convert from MSDOS-format local time and date to Unix-format 32-bit GMT
    time:  adjust base year from 1980 to 1970, do usual conversions from
    yy/mm/dd hh:mm:ss to elapsed seconds, and account for timezone and day-
    light savings time differences.  If we have a Unix extra field, however,
    we're laughing:  both mtime and atime are ours.  On the other hand, we
    then have to check for restoration of UID/GID.
  ---------------------------------------------------------------------------*/

    eb_izux_flg = (G.extra_field ? ef_scan_for_izux(G.extra_field,
                   G.lrec.extra_field_length, 0, G.lrec.last_mod_file_date,
                   &zt, z_uidgid) : 0);
    if (eb_izux_flg & EB_UT_FL_MTIME) {
        TTrace((stderr, "\nclose_outfile:  Unix e.f. modif. time = %ld\n",
          zt.mtime));
    } else {
        zt.mtime = dos_to_unix_time(G.lrec.last_mod_file_date,
                                    G.lrec.last_mod_file_time);
    }
    if (eb_izux_flg & EB_UT_FL_ATIME) {
        TTrace((stderr, "close_outfile:  Unix e.f. access time = %ld\n",
          zt.atime));
    } else {
        zt.atime = zt.mtime;
        TTrace((stderr, "\nclose_outfile:  modification/access times = %ld\n",
          zt.mtime));
    }

    /* if -X option was specified and we have UID/GID info, restore it */
    if (G.X_flag && eb_izux_flg & EB_UX2_VALID) {
        TTrace((stderr, "close_outfile:  restoring Unix UID/GID info\n"));
        if (chown(G.filename, (uid_t)z_uidgid[0], (gid_t)z_uidgid[1]))
        {
            if (G.qflag)
                Info(slide, 0x201, ((char *)slide,
                  "warning:  cannot set UID %d and/or GID %d for %s\n",
                  z_uidgid[0], z_uidgid[1], G.filename));
            else
                Info(slide, 0x201, ((char *)slide,
                  " (warning) cannot set UID %d and/or GID %d",
                  z_uidgid[0], z_uidgid[1]));
/* GRR: change return type to int and set up to return warning after utime() */
        }
    }

    /* set the file's access and modification times */
    if (utime(G.filename, (struct utimbuf *)&zt)) {
        if (G.qflag)
            Info(slide, 0x201, ((char *)slide,
              "warning:  cannot set time for %s\n", G.filename));
        else
            Info(slide, 0x201, ((char *)slide,
              " (warning) cannot set time"));
    }

    /* handle the BeOS extra field if present */
    {
        void *ptr = scanBeOSexfield( G.extra_field,
                                     G.lrec.extra_field_length );

        if( ptr ) {
            setBeOSexfield( G.filename, ptr );
        }
    }

} /* end function close_outfile() */




#ifdef TIMESTAMP

/***************************/
/*  Function stamp_file()  */
/***************************/

int stamp_file(fname, modtime)
    ZCONST char *fname;
    time_t modtime;
{
    struct utimbuf tp;

    tp.modtime = tp.actime = modtime;
    return (utime(fname, &tp));

} /* end function stamp_file() */

#endif /* TIMESTAMP */




#ifndef SFX

/************************/
/*  Function version()  */
/************************/

void version(__G)
    __GDEF
{
    sprintf((char *)slide, LoadFarString(CompiledWith),
#if defined(__MWERKS__)
      "Metrowerks CodeWarrior", "",
#elif defined(__GNUC__)
      "GNU C ", __VERSION__,
#endif
      "BeOS ",

#ifdef __POWERPC__
      "(PowerPC)",
#else
      "(unknown)",   /* someday we may have other architectures... */
#endif

#ifdef __DATE__
      " on ", __DATE__
#else
      "", ""
#endif
    );

    (*G.message)((zvoid *)&G, slide, (ulg)strlen((char *)slide), 0);

} /* end function version() */

#endif /* !SFX */

/******************************/
/* Extra field functions      */
/******************************/

/*
** Scan the extra fields in extra_field, and look for a BeOS EF; return a
** pointer to that EF, or NULL if it's not there.
*/
uch *scanBeOSexfield( uch *extra_field, unsigned ef_len )
{
    uch *ptr = extra_field;

    while( ptr < extra_field + ef_len ) {
        if( isBeOSexfield( ptr ) ) {
            return ptr;
        } else {
            ush  size;

            ptr += 2;                   /* skip over the ID         */
            size = makeword( ptr );     /* find the size of this EF */
            ptr += 2;

            ptr += size;                /* skip this EF */
        }
    }

    return NULL;
}

int isBeOSexfield( uch *extra_field )
{
    if( extra_field != NULL ) {
        uch *ptr  = extra_field;
        ush  id   = 0;
        ush  size = 0;

        id   = makeword( ptr );
        ptr += 2;
        size = makeword( ptr );
        ptr += 2;

        if( id == EF_BE_ID && size >= EF_BE_SIZE ) {
            return TRUE;
        }
    }

    return FALSE;
}

/* Used by setBeOSexfield():

Set a file/directory's attributes to the attributes passed in.

If set_file_attrs() fails, an error will be returned:

     EOK - no errors occurred

(other values will be whatever the failed function returned; no docs
yet, or I'd list a few)
*/
int set_file_attrs( const char *name,
                    const unsigned char *attr_buff,
                    const off_t attr_size )
{
    int                  retval = EOK;
    unsigned char       *ptr;
    const unsigned char *guard;
    int                  fd;

    ptr   = (unsigned char *)attr_buff;
    guard = ptr + attr_size;

    fd = open( name, O_RDWR );
    if( fd < 0 ) {
        return errno; /* should it be -fd ? */
    }

    while( ptr < guard ) {
        ssize_t              wrote_bytes;
        struct attr_info     fa_info;
        const char          *attr_name;
        const unsigned char *attr_data;

        attr_name  = (char *)&(ptr[0]);
        ptr       += strlen( attr_name ) + 1;

        memcpy( &fa_info, ptr, sizeof( struct attr_info ) );
        ptr     += sizeof( struct attr_info );

        attr_data  = ptr;
        ptr       += fa_info.size;

        if( ptr > guard ) {
            /* We've got a truncated attribute. */
            Info(slide, 0x201, ((char *)slide,
                 "warning: truncated attribute\n"));
            break;
        }

        wrote_bytes = fs_write_attr( fd, attr_name, fa_info.type, 0,
                                     attr_data, fa_info.size );
        if( wrote_bytes != fa_info.size ) {
            Info(slide, 0x201, ((char *)slide,
                 "warning: wrote %ld attribute bytes of %ld\n",(unsigned long)wrote_bytes,(unsigned long)fa_info.size));
        }
    }

    close( fd );

    return retval;
}

void setBeOSexfield( char *path, uch *extra_field )
{
    uch *ptr       = extra_field;
    ush  id        = 0;
    ush  size      = 0;
    ulg  full_size = 0;
    uch  flags     = 0;
    uch *attrbuff  = NULL;
    int retval;

    if( extra_field == NULL ) {
        return;
    }

    /* Collect the data from the extra field buffer. */
    id        = makeword( ptr );    ptr += 2;   /* we don't use this... */
    size      = makeword( ptr );    ptr += 2;
    full_size = makelong( ptr );    ptr += 4;
    flags     = *ptr;               ptr++;

    /* Do a little sanity checking. */
    if( flags & EF_BE_FL_BADBITS ) {
        /* corrupted or unsupported */
        Info(slide, 0x201, ((char *)slide,
             "Unsupported flags set for this BeOS extra field, skipping.\n"));
        return;
    }
    if( size <= EF_BE_SIZE ) {
        /* corrupted, unsupported, or truncated */
        Info(slide, 0x201, ((char *)slide,
             "BeOS extra field is %d bytes, should be at least %d.\n", size,
             EF_BE_SIZE));
        return;
    }
    if( full_size < ( size - EF_BE_SIZE ) ) {
        /* possible old archive? will this screw up on valid archives? */
        Info(slide, 0x201, ((char *)slide,
             "Skipping attributes: BeOS extra field is %d bytes, "
             "data size is %ld.\n", size - EF_BE_SIZE, full_size));
        return;
    }

    /* Find the BeOS file attribute data. */
    if( flags & EF_BE_FL_NATURAL ) {
        /* Uncompressed data */
        attrbuff = ptr;
    } else {
        /* Compressed data */
        attrbuff = (uch *)malloc( full_size );
        if( attrbuff == NULL ) {
            /* No memory to uncompress attributes */
            Info(slide, 0x201, ((char *)slide,
                 "Can't allocate memory to uncompress file attributes.\n"));
            return;
        }

        retval = memextract( __G__ attrbuff, full_size,
                             ptr, size - EF_BE_SIZE );
        if( retval != PK_OK ) {
            /* error uncompressing attributes */
            Info(slide, 0x201, ((char *)slide,
                 "Error uncompressing file attributes.\n"));

            /* Some errors here might not be so bad; we should expect */
            /* some truncated data, for example.  If the data was     */
            /* corrupt, we should _not_ attempt to restore the attrs  */
            /* for this file... there's no way to detect what attrs   */
            /* are good and which are bad.                            */
            free( attrbuff );
            return;
        }
    }

    /* Now attempt to set the file attributes on the extracted file. */
    retval = set_file_attrs( path, attrbuff, (off_t)full_size );
    if( retval != EOK ) {
        Info(slide, 0x201, ((char *)slide,
             "Error writing file attributes.\n"));
    }

    /* Clean up, if necessary */
    if( attrbuff != ptr ) {
        free( attrbuff );
    }

    return;
}

void printBeOSexfield( int isdir, uch *extra_field )
{
    uch *ptr       = extra_field;
    ush  id        = 0;
    ush  size      = 0;
    ulg  full_size = 0;
    uch  flags     = 0;

    /* Tell picky compilers to be quiet. */
    isdir = isdir;

    if( extra_field == NULL ) {
        return;
    }

    /* Collect the data from the buffer. */
    id        = makeword( ptr );    ptr += 2;
    size      = makeword( ptr );    ptr += 2;
    full_size = makelong( ptr );    ptr += 4;
    flags     = *ptr;               ptr++;

    if( id != EF_BE_ID ) {
        /* not a 'Be' field */
        printf( "\t*** Unknown field type (0x%04x, '%c%c')\n", id,
                (char)(id >> 8), (char)id );
    }

    if( flags & EF_BE_FL_BADBITS ) {
        /* corrupted or unsupported */
        printf( "\t*** Corrupted BeOS extra field:\n" );
        printf( "\t*** unknown bits set in the flags\n" );
        printf( "\t*** (Possibly created by an old version of zip for BeOS.\n" );
    }

    if( size <= EF_BE_SIZE ) {
        /* corrupted, unsupported, or truncated */
        printf( "\t*** Corrupted BeOS extra field:\n" );
        printf( "\t*** size is %d, should be larger than %d\n", size,
                EF_BE_SIZE );
    }

    if( flags & EF_BE_FL_NATURAL ) {
        /* Uncompressed data */
        printf( "\tBeOS extra field data (uncompressed):\n" );
        printf( "\t\t%ld data bytes\n", full_size );
    } else {
        /* Compressed data */
        printf( "\tBeOS extra field data (compressed):\n" );
        printf( "\t\t%d compressed bytes\n", size - EF_BE_SIZE );
        printf( "\t\t%ld uncompressed bytes\n", full_size );
    }
}

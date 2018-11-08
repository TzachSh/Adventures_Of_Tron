/* conf.c */
/* (GENERATED FILE: DO NOT EDIT) */

#include <conf.h>
#include <bios.h>

/* device independent I/O switch */

struct	devsw	devtab[NDEVS] = {

/*------------------------------------------------------------------------
 * Format of each entry is:
 *
 * device number, device name,
 * init, open, close,
 * read, write, seek,
 * getc, putc, cntl,
 * port addr, device input vector, device output vector,
 * input interrupt routine, output interrupt routine,
 * device i/o block, minor device number
 *------------------------------------------------------------------------
 */

/* CONSOLE is tty on BIOS */
0,"tty",
ttyinit,ttyopen,ioerr,
ttyread,ttywrite,ioerr,
ttygetc,ttyputc,ttycntl,
0,KBDVEC|BIOSFLG,0,
ttyiin,ioerr,
NULLPTR,0,

/* GENERIC is tty on WINDOW */
1,"",
lwinit,ionull,lwclose,
lwread,lwwrite,ioerr,
lwgetc,lwputc,lwcntl,
0,0,0,
ioerr,ioerr,
NULLPTR,1,

/* GENERIC is tty on WINDOW */
2,"",
lwinit,ionull,lwclose,
lwread,lwwrite,ioerr,
lwgetc,lwputc,lwcntl,
0,0,0,
ioerr,ioerr,
NULLPTR,2,

/* GENERIC is tty on WINDOW */
3,"",
lwinit,ionull,lwclose,
lwread,lwwrite,ioerr,
lwgetc,lwputc,lwcntl,
0,0,0,
ioerr,ioerr,
NULLPTR,3,

/* GENERIC is tty on WINDOW */
4,"",
lwinit,ionull,lwclose,
lwread,lwwrite,ioerr,
lwgetc,lwputc,lwcntl,
0,0,0,
ioerr,ioerr,
NULLPTR,4,

/* DS0 is dsk on BIOS */
5,"ds0",
dsinit,dsopen,ioerr,
dsread,dswrite,dsseek,
ioerr,ioerr,dscntl,
0,0,0,
ioerr,ioerr,
NULLPTR,0,

/* GENERIC is df on DSK */
6,"",
lfinit,ioerr,lfclose,
lfread,lfwrite,lfseek,
lfgetc,lfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,0,

/* GENERIC is df on DSK */
7,"",
lfinit,ioerr,lfclose,
lfread,lfwrite,lfseek,
lfgetc,lfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,1,

/* GENERIC is df on DSK */
8,"",
lfinit,ioerr,lfclose,
lfread,lfwrite,lfseek,
lfgetc,lfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,2,

/* GENERIC is df on DSK */
9,"",
lfinit,ioerr,lfclose,
lfread,lfwrite,lfseek,
lfgetc,lfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,3,

/* GENERIC is df on DSK */
10,"",
lfinit,ioerr,lfclose,
lfread,lfwrite,lfseek,
lfgetc,lfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,4,

/* DOS is dos on MSDOS */
11,"dos",
ionull,msopen,ioerr,
ioerr,ioerr,ioerr,
ioerr,ioerr,mscntl,
0,0,0,
ioerr,ioerr,
NULLPTR,0,

/* GENERIC is mf on DOS */
12,"",
mfinit,ioerr,mfclose,
mfread,mfwrite,mfseek,
mfgetc,mfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,0,

/* GENERIC is mf on DOS */
13,"",
mfinit,ioerr,mfclose,
mfread,mfwrite,mfseek,
mfgetc,mfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,1,

/* GENERIC is mf on DOS */
14,"",
mfinit,ioerr,mfclose,
mfread,mfwrite,mfseek,
mfgetc,mfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,2,

/* GENERIC is mf on DOS */
15,"",
mfinit,ioerr,mfclose,
mfread,mfwrite,mfseek,
mfgetc,mfputc,ioerr,
0,0,0,
ioerr,ioerr,
NULLPTR,3
};

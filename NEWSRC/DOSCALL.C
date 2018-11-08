/* doscall.c - doscall, dos_creat, dos_open, dos_close, dos_read, dos_write, dos_lseek */

#include <dos.h>
#include <conf.h>
#include <kernel.h>

#define AX(r)		((r).x.ax)
#define BX(r)		((r).x.bx)
#define	CX(r)		((r).x.cx)
#define DX(r)		((r).x.dx)
#define CFLAG(r)	((r).x.cflag)
#define AH(r)		((r).h.ah)
#define AL(r)		((r).h.al)
#define BH(r)		((r).h.bh)
#define BL(r)		((r).h.bl)
#define CH(r)		((r).h.ch)
#define CL(r)		((r).h.cl)
#define DH(r)		((r).h.dh)
#define DL(r)		((r).h.dl)

#define DOS_CREAT	0x3c
#define DOS_OPEN	0x3d
#define DOS_CLOSE	0x3e
#define DOS_READ	0x3f
#define DOS_WRITE	0x40
#define DOS_LSEEK	0x42

/* alias for converting between longs & 2 ints	*/
/* this is highly machine/compiler specific	*/
/* 8088 longs are stored lowword/highword	*/

struct L2I {
	int	lowword;
	int	highword;
};

/*------------------------------------------------------------------------
 *  doscall  --  call a dos function
 *------------------------------------------------------------------------
 */
doscall(regs)
union REGS *regs;
{
	intdos(regs,regs);	/* make the DOS call			*/
	return(CFLAG(*regs));	/* nonzero return means error		*/
}

/*------------------------------------------------------------------------
 *  dos_creat  --  create a dos file
 *------------------------------------------------------------------------
 */
dos_creat(pathname,attr)
char *pathname;	/* filename */
int attr;	/* file attributes */
{
	union REGS	regs;

	(char *) DX(regs) = pathname;
	CX(regs) = attr;
	AH(regs) = DOS_CREAT;
	if ( doscall(&regs) )
		return(SYSERR);
	return(AX(regs));		/* return the DOS file handle	*/
}

/*------------------------------------------------------------------------
 *  dos_open  --  open a dos file
 *------------------------------------------------------------------------
 */
dos_open (pathname,mode)
char *pathname;		/* DOS filename	*/
int mode;		/* DOS file mode bits */
{
	union REGS	regs;
	
	(char *) DX(regs) = pathname;
	AL(regs) = mode;
	AH(regs) = DOS_OPEN;
	if ( doscall(&regs) )
		return(SYSERR);
	return(AX(regs));		/* return the DOS file handle	*/
}

/*------------------------------------------------------------------------
 *  dos_close  --  close a dos file
 *------------------------------------------------------------------------
 */
dos_close (fd)
int fd;		/* file handle to close */
{
	union REGS	regs;

	BX(regs) = fd;
	AH(regs) = DOS_CLOSE;
	if ( doscall(&regs) )
		return(SYSERR);
	return(OK);
}

/*------------------------------------------------------------------------
 *  dos_read  --  read from a dos file
 *------------------------------------------------------------------------
 */
dos_read (fd,buffer,count)
int fd;		/* file handle to read from */
char *buffer;	/* destination buffer */
int count;	/* no. of bytes to transfer */
{
	union REGS	regs;
	
	BX(regs) = fd;
	(char *) DX(regs) = buffer;
	CX(regs) = count;
	AH(regs) = DOS_READ;
	if ( doscall(&regs) )
		return(SYSERR);
	return(AX(regs));		/* no. of bytes actually read */
}

/*------------------------------------------------------------------------
 *  dos_write  --  write to a dos file
 *------------------------------------------------------------------------
 */
dos_write (fd,buffer,count)
int fd;		/* file handle to write to */
char *buffer;	/* source buffer */
int count;	/* no. of bytest to transfer */
{
	union REGS	regs;
	
	BX(regs) = fd;
	(char *) DX(regs) = buffer;
	CX(regs) = count;
	AH(regs) = DOS_WRITE;
	if ( doscall(&regs) )
		return(SYSERR);
	return(AX(regs));		/* no. of bytes actually written */
}

/*------------------------------------------------------------------------
 *  dos_lseek  --  perform an LSEEK on a dos file
 *------------------------------------------------------------------------
 */
long
dos_lseek(fd,offset,origin)
int fd;		/* file handle */
long offset;	/* offset into file */
int origin;	/* origin of seek - 0=beginning, 1=current, 2=end */
{
	union	REGS	regs;
	register struct	L2I	*lp;
	
	BX(regs) = fd;
	lp = (struct L2I *) &offset;
	CX(regs) = lp->highword;
	DX(regs) = lp->lowword;
	AL(regs) = origin;
	AH(regs) = DOS_LSEEK;
	if ( doscall(&regs) )
		return( (long)SYSERR );
	lp->highword = DX(regs);
	lp->lowword  = AX(regs);
	return(offset);
}

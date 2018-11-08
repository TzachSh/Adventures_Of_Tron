/* dswrite.c - dswrite */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <disk.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  dswrite  --  write a block (system buffer) onto a disk device
 *------------------------------------------------------------------------
 */
dswrite(devptr, buff, block)
struct devsw *devptr;
char *buff;
DBADDR block;
{
	struct	dsblk	*dsptr;
	struct	dreq	*drptr;
	int	ps;

	dsptr = (struct dsblk *)devptr->dvioblk;
	if ( block<0 || block>=NDSECT
		|| dsptr->dsprocnum < 0
		|| (drptr=(struct dreq *) getbuf(dskrbp)) == DRNULL )
		return(SYSERR);
	disable(ps);
	drptr->drbuff = buff;
	drptr->drdba = block;
	drptr->drpid = currpid;
	drptr->drop = DWRITE;
	if ( dskenq(drptr, devptr->dvioblk) == SYSERR ) {
		freebuf(drptr);
		restore(ps);
		return(SYSERR);
	}
	restore(ps);
	return(OK);
}

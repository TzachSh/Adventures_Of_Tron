/* dsseek.c - dsseek */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <disk.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  dsseek  --  schedule a request to move the disk arm
 *------------------------------------------------------------------------
 */
dsseek(devptr, block)
struct devsw *devptr;
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
	drptr->drdba = block;
	drptr->drpid = currpid;
	drptr->drbuff = NULL;
	drptr->drop = DSEEK;

	/* enqueued with normal policy like other read/write requests */

	dskenq(drptr, devptr->dvioblk);
	restore(ps);
	return(OK);
}

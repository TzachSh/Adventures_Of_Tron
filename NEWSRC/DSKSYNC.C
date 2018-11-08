/* dsksync.c - dsksync */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <disk.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  dsksync  --  wait for all outstanding disk requests before returning
 *------------------------------------------------------------------------
 */
dsksync(devptr)
struct	devsw	*devptr;
{
	struct	dsblk	*dsptr;
	struct	dreq	*drptr, *p, *q;
	int	stat;

	dsptr = (struct dsblk *)devptr->dvioblk;
	if ( dsptr->dsprocnum < 0 )
		return(SYSERR);
	if ( (q=dsptr->dreqlst) == DRNULL )
		return(OK);
	if ( (drptr=(struct dreq *) getbuf(dskrbp)) == DRNULL )
		return(SYSERR);
	drptr->drdba = 0;
	drptr->drpid = currpid;
	drptr->drbuff = NULL;
	drptr->drop = DSYNC;
	drptr->drnext = DRNULL;

	/* place at end of request list */

	for (p=q->drnext ; p!=DRNULL ; q=p,p=p->drnext)
		;
	q->drnext = drptr;
	drptr->drstat = SYSERR;
	suspend(currpid);
	stat = drptr->drstat;
	freebuf(drptr);
	return(stat);
}

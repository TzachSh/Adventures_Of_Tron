/* dsread.c - dsread */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <disk.h>
#include <mark.h>
#include <bufpool.h>

/*------------------------------------------------------------------------
 *  dsread  --  read a block from a disk device
 *------------------------------------------------------------------------
 */
dsread(devptr, buff, block)
struct devsw *devptr;
char *buff;
DBADDR block;
{
	struct	dsblk	*dsptr;
	struct	dreq	*drptr;
	int	stat;
	int	ps;

	dsptr = (struct dsblk *)devptr->dvioblk;
	if ( block<0 || block>=NDSECT
		|| dsptr->dsprocnum < 0
		|| (drptr=(struct dreq *) getbuf(dskrbp)) == DRNULL )
		return(SYSERR);
	disable(ps);
	drptr->drdba = block;
	drptr->drpid = currpid;
	drptr->drbuff = buff;
	drptr->drop = DREAD;
	if ( (stat=dskenq(drptr, devptr->dvioblk)) == DONQ) {
		suspend(currpid);
		stat = drptr->drstat;
	}
	freebuf(drptr);
	restore(ps);
	return(stat);
}

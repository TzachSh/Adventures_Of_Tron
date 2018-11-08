/* lwclose.c - lwclose */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <tty.h>

/*------------------------------------------------------------------------
 *  lwclose  --  routine to close a window device
 *------------------------------------------------------------------------
 */

lwclose(devptr)
struct	devsw	*devptr;
{
	struct	tty	*iptr;
	int 	ps;
	int	sct;

	disable(ps);
	iptr = &tty[devptr->dvminor];
	if ( iptr->wstate != LWALLOC ) {/* can't close unopened window	*/
		restore(ps);
		return(SYSERR);
	}
	iptr->wstate = LWLIMBO;
	iptr->seq++;
	kill(iptr->oprocnum);		/* kill the output process	*/
	if ( winofcur == devptr->dvminor )
		winofcur = 0;	/* current cursor can't be here anymore	*/
	sdelete(iptr->isem);
	sdelete(iptr->osem);
	if (iptr->hasborder) {
		--iptr->topleft.col;
		--iptr->topleft.row;
		++iptr->botright.col;
		++iptr->botright.row;
	} 
	scrollup(iptr->topleft,iptr->botright,0,BW);	/* erase window	*/
	iptr->wstate = LWFREE;
	restore(ps);
	return(OK);
}

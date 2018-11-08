/* lwgetc.c - lwgetc */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <io.h>
#include <proc.h>

/*------------------------------------------------------------------------
 *  lwgetc  --  read one character from a window device
 *------------------------------------------------------------------------
 */
lwgetc(devptr)
struct	devsw	*devptr;
{
	struct	tty	*iptr;
	int	ps;
	char	ch;
	int	seq;

	iptr = &tty[devptr->dvminor];
	disable(ps);
	if ( iptr->wstate != LWALLOC ) {/* is window open?		*/
		restore(ps);
		return(SYSERR);
	}
	seq = iptr->seq;
	wait(iptr->isem);		/* wait	for a character	in buff	*/
	if ( iptr->wstate != LWALLOC	/* is window still open?	*/ 
	     || iptr->seq != seq ) {
		restore(ps);
		return(SYSERR);
	}
	ch = iptr->ibuff[iptr->itail++];
	--iptr->icnt;
	if (iptr->itail	>= IBUFLEN)
		iptr->itail = 0;
	restore(ps);
	return(ch);
}

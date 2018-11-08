/* lwputc.c - lwputc */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <proc.h>

/*------------------------------------------------------------------------
 *  lwputc  --  put a character into a logical window
 *------------------------------------------------------------------------
 */

lwputc(devptr, ch)
struct	devsw	*devptr;
char	ch;
{
	struct	tty	*iptr;
	int	ps;
	int	seq;

	iptr = &tty[devptr->dvminor];
	disable(ps);
	if ( iptr->wstate != LWALLOC ) {/* is window open?		*/
		restore(ps);
		return(SYSERR);
	}
	seq = iptr->seq;
	wait(iptr->osem);		/* wait	for space in queue	*/
	if ( iptr->wstate != LWALLOC	/* is window still open?	*/
	     || iptr->seq != seq ) {
	     	restore(ps);
		return(SYSERR);
	}
	iptr->obuff[iptr->ohead++] = ch;
	++iptr->ocnt;
	if (iptr->ohead	>= OBUFLEN)
		iptr->ohead = 0;
	sendn(iptr->oprocnum,TMSGOK);	/* wake up the tty process	*/
	restore(ps);
	return(OK);
}

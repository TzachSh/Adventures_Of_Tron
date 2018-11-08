/* lwwrite.c - lwwrite */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <io.h>

/*------------------------------------------------------------------------
 *  lwwrite  --  write one or more characters to a console window
 *------------------------------------------------------------------------
 */
lwwrite(devptr, buff, count)
struct devsw *devptr;
char *buff;
int count;
{
	register struct tty *ttyp;
	int	avail;
	int	ps;

	if ( count<0 )
		return(SYSERR);
	if (count == 0)
		return(OK);
	disable(ps);
	ttyp = &tty[devptr->dvminor];
	if ( ttyp->wstate != LWALLOC ) {/* is window open?		*/
		restore(ps);
		return(SYSERR);
	}
	avail = scount( ttyp->osem );
	if ( avail >= count) {
		writcopy(buff, ttyp, avail, count);
		sendn(ttyp->oprocnum,TMSGOK);
	} else {
		if (avail > 0) {
			writcopy(buff, ttyp, avail, avail);
			sendn(ttyp->oprocnum,TMSGOK);
			buff += avail;
			count -= avail;
		}
		for (; count>0 ; count--)
			if ( lwputc(devptr, *buff++) == SYSERR ) {
				restore(ps);
				return(SYSERR);
			}
	}
	restore(ps);
	return(OK);
}

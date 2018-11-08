/* lwread.c - lwread */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <io.h>

/*------------------------------------------------------------------------
 *  lwread  --  read one or more characters from a logical window
 *------------------------------------------------------------------------
 */
lwread(devptr, buff, count)
struct devsw *devptr;
char *buff;
int count;
{
	register struct	tty *ttyp;
	int	avail, nread;
	int	ps;

	if ( count<0 )
		return(SYSERR);
	disable(ps);
	ttyp = &tty[devptr->dvminor];
	if ( ttyp->wstate != LWALLOC ) {/* is window open?		*/
		restore(ps);
		return(SYSERR);
	}
	avail = scount( ttyp->isem );
	if ( (count = (count==0 ? avail : count)) == 0) {
		restore(ps);
		return(0);
	}
	nread = count;
	if ( count <= avail )
		readcopy(buff, ttyp, avail, count);
	else {
		if (avail > 0) {
			readcopy(buff, ttyp, avail, avail);
			buff += avail;
			count -= avail;
		}
		for ( ; count>0 ; count--)
			if ( (*buff++ = lwgetc(devptr)) == SYSERR ) {
				restore(ps);
				return(nread-count);
			}
	}
	restore(ps);
	return(nread);
}

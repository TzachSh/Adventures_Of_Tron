/* lwcntl.c - lwcntl */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <tty.h>

/*------------------------------------------------------------------------
 *  lwcntl  --  do misc. tty control functions
 *------------------------------------------------------------------------
 */

SYSCALL lwcntl(devptr, func)
struct devsw *devptr;
int func;
{
	register struct	tty	*ttyp;
	int	ps;
	int	c;

	disable(ps);
	ttyp = &tty[devptr->dvminor];
	if ( ttyp->wstate != LWALLOC ) {/* can't access unopened window	*/
		restore(ps);
		return(SYSERR);
	}
	c = OK;				/* assume the best		*/
	switch ( func )	{
	case TCNEXTC:
		wait(ttyp->isem);
		c = ttyp->ibuff[ttyp->itail];
		signal(ttyp->isem);
		break;
	case TCMODER:
		ttyp->imode = IMRAW;
		break;
	case TCMODEC:
		ttyp->imode = IMCOOKED;
		break;
	case TCMODEK:
		ttyp->imode = IMCBREAK;
		break;
	case TCECHO:
		ttyp->iecho = TRUE;
		break;
	case TCNOECHO:
		ttyp->iecho = FALSE;
		break;
	case TCICHARS:
		c = scount(ttyp->isem);
		break;
	default:
		c = SYSERR;
	}
	restore(ps);
	return(c);
}

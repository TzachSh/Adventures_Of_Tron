/* lwinit.c - lwinit */

#include <conf.h>
#include <kernel.h>
#include <tty.h>
#include <io.h>
#include <bios.h>

/*------------------------------------------------------------------------
 *  lwinit  --  initialize console window
 *------------------------------------------------------------------------
 */
lwinit(devptr)
	struct	devsw	*devptr;
{
	register struct	tty *iptr;

	iptr = &tty[devptr->dvminor];
	iptr->dnum = devptr->dvnum;
	iptr->oprocnum = -1;			/* no output process	*/
	iptr->wstate = LWFREE;			/* window is free	*/
	iptr->seq = 0;				/* init sequence no.	*/
	devptr->dvioblk = (char *) iptr;	/* fill tty control blk	*/
}

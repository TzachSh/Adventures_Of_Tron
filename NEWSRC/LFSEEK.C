/* lfseek.c - lfseek */

#include <conf.h>
#include <kernel.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 *  lfseek  --  seek to a specified position of a file
 *------------------------------------------------------------------------
 */
lfseek(devptr, offset)
struct	devsw	*devptr;
long	offset;
{
	struct	flblk	*flptr;
	int	retcode;
	int	ps;

	disable(ps);
	flptr = (struct flblk *)devptr->dvioblk;
	if (flptr->fl_mode & FLWRITE) {
		if (flptr->fl_dch)
			lfsflush(flptr);
	} else if (offset > (flptr->fl_dent)->fdlen) {
			restore(ps);
			return(SYSERR);
	}
	flptr->fl_pos = offset;
	lfsetup(flptr->fl_dev, flptr);
	restore(ps);
	return(OK);
}

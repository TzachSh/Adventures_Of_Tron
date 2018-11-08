/* dsopen.c - dsopen */

#include <conf.h>
#include <kernel.h>
#include <disk.h>
#include <file.h>

/*------------------------------------------------------------------------
 *  dsopen  --  open/create a file on the specified disk device
 *------------------------------------------------------------------------
 */
#ifdef	Ndf
dsopen(devptr, filenam, mode)
struct	devsw	*devptr;
char	*filenam;
char	*mode;
{
	struct	flblk	*flptr;
	struct	fdes	*fdptr;
	DBADDR	dba;
	int	mbits, findex;
	int	id;
	int	ps;

	if ( (mbits=dfckmd(mode)) == SYSERR )
		return(SYSERR);
	disable(ps);
	if( (int)(fdptr=dfdsrch(devptr->dvioblk,filenam,mbits)) == SYSERR
		|| (findex=dfalloc()) == SYSERR ) {
		restore(ps);
		return(SYSERR);
	}
	flptr = &fltab[findex];
	flptr->fl_dev = devptr->dvnum;
	flptr->fl_dent = fdptr;
	flptr->fl_mode = mbits & FLRW;
	flptr->fl_iba = fdptr->fdiba;
	ibget(flptr->fl_dev, flptr->fl_iba, &(flptr->fl_iblk));
	flptr->fl_pos = 0L;
	flptr->fl_dch = FALSE;
	dba = flptr->fl_iblk.ib_dba[flptr->fl_ipnum = 0];
	if (dba != DBNULL) {
		read(flptr->fl_dev, flptr->fl_buff, dba);
		flptr->fl_bptr = flptr->fl_buff;
	} else
		flptr->fl_bptr = &flptr->fl_buff[DBUFSIZ];
	id = flptr->fl_id;
	restore(ps);
	return(id);
}
#endif
